#pragma once

#include "InputNumber.h"

namespace DIMS
{
	struct InputInterface
	{
		InputInterface(RE::InputEvent* evt, RE::PlayerControls* ctrl) : event{ evt }, controls{ ctrl } {

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
				return EventStage::None;

				//While this is not normally possible, I will fake an input event at the very end of the cycle of no inputs are detected.
				if (GetAxis() == k_zero_point)
					return EventStage::Finish;
				else if (GetAxisPrevious() == k_zero_point)
					return EventStage::Start;
				else
					return EventStage::Repeating;

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
			case RE::INPUT_EVENT_TYPE::kThumbstick:
				return 0;

			default:

				return 0.f;
			}
		}

		RE::NiPoint2 GetAxis() const
		{
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

			default:
				return {};


			}
		}

		RE::NiPoint2 GetAxisPrevious() const
		{
			switch (event->GetEventType())
			{
			case RE::INPUT_EVENT_TYPE::kButton:
				//This has a way to handle it, I just don't have it right now.
				return {};

			case RE::INPUT_EVENT_TYPE::kMouseMove:
				return controls->data.prevLookVec;

			case RE::INPUT_EVENT_TYPE::kThumbstick:
				return event->AsThumbstickEvent()->IsRight() ? controls->data.prevLookVec : controls->data.prevMoveVec;

			default:
				return {};
			}
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
			default:
				return {};
			}

			
		}


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
				return 0;

			}
		}


		RE::InputEvent* const event{};
		RE::PlayerControls* const controls{};

	};

}