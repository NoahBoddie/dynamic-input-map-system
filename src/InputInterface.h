#pragma once

#include "InputNumber.h"
#include "VirtualEvent.h"

namespace DIMS
{
	struct InputInterface
	{
		//TODO: This needs to learn how to deal with virtual events
		InputInterface(RE::InputEvent* evt) : event{ evt } {

		}



		constexpr operator bool() const noexcept
		{
			return event;
		}

		EventStage GetEventStage() const
		{
			//controls like look can have different life times that of a single buttons. Example is Look. For something like that I'll have
			// to replicate it. The question is, how to find a way to actually sign that I'm replicating it. I guess a special control name
			// that gets consumed?

			constexpr RE::NiPoint2 k_zero_point = RE::NiPoint2{};

			switch (event->GetEventType())
			{
			case RE::INPUT_EVENT_TYPE::kButton:


				if (auto button = event->AsButtonEvent(); !button->heldDownSecs)
					return EventStage::Start;
				else if (!button->value)
					return EventStage::Finish;
				else
					return EventStage::Repeating;

				//For these 2, use get axis instead.
			case RE::INPUT_EVENT_TYPE::kMouseMove:
			case RE::INPUT_EVENT_TYPE::kThumbstick:
				//While this is not normally possible, I will fake an input event at the very end of the cycle of no inputs are detected.
				if (GetAxis() == k_zero_point)
					return EventStage::Finish;
				else if (GetAxisPrevious() == k_zero_point)
					return EventStage::Start;
				else
					return EventStage::Repeating;
			
			case virtualEventType:
				return virInput->stage;

			default:
				
				return EventStage::None;
			}

		}


		float GetValue() const
		{
			//Later, the point of this will be to assess what level of strength someone is putting into
			// a mouse move, or thumbstick movement. Some agreed setting will be used with the mouse movement, which will cap
			// at a certain value.

			switch (event->GetEventType())
			{
			case RE::INPUT_EVENT_TYPE::kButton:
					return event->AsButtonEvent()->value;

			case RE::INPUT_EVENT_TYPE::kMouseMove:
				return 0.f;

			case RE::INPUT_EVENT_TYPE::kThumbstick: {
				auto thumb = event->AsThumbstickEvent();
				return sqrt(thumb->xValue * thumb->xValue + thumb->yValue * thumb->yValue);
			}

			case VirtualEvent::EVENT_TYPE:
				return virInput->value;
			

				

			default:

				return 0.f;
			}
		}

		RE::NiPoint2 GetAxis() const
		{
			//Id like to use a double version of this for the promise of accuracy
			switch (event->GetEventType())
			{
			case RE::INPUT_EVENT_TYPE::kButton:
				return {};


			case RE::INPUT_EVENT_TYPE::kMouseMove: {
				auto mouse = event->AsMouseMoveEvent();

				return RE::NiPoint2{ (float)mouse->mouseInputX, (float)mouse->mouseInputY };
			}



			case RE::INPUT_EVENT_TYPE::kThumbstick: {
				auto thumb = event->AsThumbstickEvent();
				return RE::NiPoint2{ thumb->xValue, thumb->yValue };
			}

			case VirtualEvent::EVENT_TYPE:
				return RE::NiPoint2{ virInput->x, virInput->y };

			default:
				return {};


			}
		}



		RE::NiPoint2 GetAxisPrevious() const
		{
			static auto orbis_vtable = RE::BSPCOrbisGamepadDevice::VTABLE[0].address();
			static auto win32_vtable = RE::BSWin32GamepadDevice::VTABLE[0].address();

			//Id like to use a double version of this for the promise of accuracy
			switch (event->GetEventType())
			{
			case RE::INPUT_EVENT_TYPE::kMouseMove: {
				auto device_manager = RE::BSInputDeviceManager::GetSingleton();

				if (!device_manager)
					break;

				auto mouse = device_manager->GetMouse();

				if (!mouse)
					break;

				return RE::NiPoint2{ (float)mouse->dInputPrevState.x, (float)mouse->dInputPrevState.y };
			}



			case RE::INPUT_EVENT_TYPE::kThumbstick: {
				auto device_manager = RE::BSInputDeviceManager::GetSingleton();

				if (!device_manager)
					break;

				auto delegate = device_manager->GetGamepad();

				if (!delegate)
					break;

				auto vtable = *reinterpret_cast<uint64_t*>(delegate);

				bool right = event->AsThumbstickEvent()->IsRight();

				if (vtable == win32_vtable)
				{
					auto gamepad = static_cast<RE::BSWin32GamepadDevice*>(delegate);
					return RE::NiPoint2{ right ? gamepad->previousRX : gamepad->previousLX, right ? gamepad->previousRY : gamepad->previousLY };
				}
				else if (vtable == win32_vtable)
				{
					auto gamepad = static_cast<RE::BSPCOrbisGamepadDevice*>(delegate);
					return RE::NiPoint2{ right ? gamepad->previousRX : gamepad->previousLX, right ? gamepad->previousRY : gamepad->previousLY };
				}
				else
				{
					assert(false);
					break;
				}
				
			}

			//case VirtualEvent::EVENT_TYPE:
			default:
				return {};


			}

			return {};
		}


		std::pair<InputNumber, InputNumber> GetEventValues() const
		{

			switch (event->GetEventType())
			{
			case RE::INPUT_EVENT_TYPE::kButton: {
				auto button = event->AsButtonEvent();
				return { button->value, button->heldDownSecs };
			}
			case RE::INPUT_EVENT_TYPE::kMouseMove: {
				auto mouse = event->AsMouseMoveEvent();
				return { mouse->mouseInputX, mouse->mouseInputY };
			}

			case RE::INPUT_EVENT_TYPE::kThumbstick: {
				auto thumb = event->AsThumbstickEvent();
				return { thumb->xValue, thumb->yValue };
			}
			case VirtualEvent::EVENT_TYPE:
				switch (virInput->proxy)
				{
				case RE::INPUT_EVENT_TYPE::kButton: {
					auto button = event->AsButtonEvent();
					return { virInput->value, virInput->heldDownSecs };
				}
				case RE::INPUT_EVENT_TYPE::kMouseMove:
				case RE::INPUT_EVENT_TYPE::kThumbstick: {
					return { virInput->x, virInput->y };
				}
				default://If it's null, we just don't care.
					break;
				}

			}
			
			return {};
			
		}

		//I think this should probably accept either int or float to be safe.
		void SetEventValues(InputNumber value1, InputNumber value2) const
		{

			switch (event->GetEventType())
			{
			case RE::INPUT_EVENT_TYPE::kButton: {
				auto button = event->AsButtonEvent();
				button->value = value1;
				button->heldDownSecs = value2;
				break;
			}
			case RE::INPUT_EVENT_TYPE::kMouseMove: {
				auto mouse = event->AsMouseMoveEvent();
				mouse->mouseInputX = value1;
				mouse->mouseInputY = value2;
				break;
			}

			case RE::INPUT_EVENT_TYPE::kThumbstick: {
				auto thumb = event->AsThumbstickEvent();
				thumb->xValue = value1;
				thumb->yValue = value2;
			}
			case VirtualEvent::EVENT_TYPE:
				switch (virInput->proxy) 
				{
				case RE::INPUT_EVENT_TYPE::kButton: {
					auto button = event->AsButtonEvent();
					button->value = value1;
					button->heldDownSecs = value2;
					break;
				}
				case RE::INPUT_EVENT_TYPE::kMouseMove: {
					virInput->x = (int)value1;
					virInput->y = (int)value2;
					break;
				}

				case RE::INPUT_EVENT_TYPE::kThumbstick: {
					virInput->x = value1;
					virInput->y = value2;
				}

				default:
					break;
				}

			default:
				//warn
				break;
			}


		}



		//Checks if an input event is an axis, either by user or by nature
		bool IsAxis() const
		{
			return IsAxisInputImpl() != 0;
		}

		//Checks if an input event is an axis by nature
		bool IsAxisInput() const
		{
			return IsAxisInputImpl() == 1;
		}


		//Checks if a button event is also an axis event.
		bool IsAxisButton() const
		{
			return IsAxisInputImpl() == 2;
		}

		operator RE::InputEvent* ()
		{
			return event;
		}

		operator const RE::InputEvent* () const
		{
			return event;
		}

		RE::InputEvent* operator-> ()
		{
			return event;
		}

		const RE::InputEvent* operator-> () const
		{
			return event;
		}



	private:

		int IsAxisInputImpl() const
		{
			switch (event->GetEventType())
			{


			case RE::INPUT_EVENT_TYPE::kMouseMove:
			case RE::INPUT_EVENT_TYPE::kThumbstick:
				return 1;

			case RE::INPUT_EVENT_TYPE::kButton:
				switch (Hash(event->QUserEvent().c_str()))
				{
				case "Forward"_h:
				case "Back"_h:
				case "Strafe Right"_h:
				case "Strafe Left"_h:
					return 2;
				}

				[[fallthrough]];
			default:
				//Manually check device type and possible ID codes from here.
				return 0;

			}
		}
	public:

		union
		{
			RE::InputEvent* const event{};
			VirtualEvent* const virInput;
			//RE::PlayerControls* const controls{};
		};
	};

}