#pragma once

namespace DIMS
{
	//Virtual input event is an event that essentially reads as any possible event. However, do know, this should NOT
	// really be reading as any kind of device type
	struct VirtualEvent : public RE::IDEvent
	{
		static constexpr auto EVENT_TYPE = (RE::INPUT_EVENT_TYPE)-1;

		float heldDownSecs{};
		float value{};
		union
		{
			RE::NiPoint2 axis{};
			struct
			{
				float x;
				float y;
			};
		};
		
		RE::InputEventType proxy = EVENT_TYPE;
		EventStage stage = EventStage::None;


		~VirtualEvent() override = default;


		bool HasIDCode() const override { return true; }//false for now.
		const RE::BSFixedString& QUserEvent() const override { return userEvent; }
		

		//make some create events, and let this handle what previous is and all that.

		VirtualEvent& AsThumbstick(const RE::BSFixedString& event_name, bool is_right, float a_x, float a_y, bool prev = false)& 
		{
			userEvent = event_name;
			proxy = RE::InputEventType::kThumbstick;
			device = RE::InputDevice::kGamepad;
			idCode = is_right ? RE::ThumbstickEvent::InputType::kRightThumbstick : RE::ThumbstickEvent::InputType::kLeftThumbstick;

			x = a_x;
			y = a_y;


			if (x || y) {
				stage = prev ? EventStage::Repeating : EventStage::Start;
			}
			else if (prev) {
				stage = EventStage::Finish;
			}
			else {
				stage = EventStage::None;
			}

			return *this;
		}


		VirtualEvent&& AsThumbstick(const RE::BSFixedString& event_name, bool is_right, float a_x, float a_y, bool prev = false)&&
		{
			return std::move(AsThumbstick(event_name, is_right, a_x, a_y, prev));
		}


		VirtualEvent& AsMouseMove(const RE::BSFixedString& event_name, int32_t a_x, int32_t a_y, bool prev = false)&
		{
			userEvent = event_name;
			proxy = RE::InputEventType::kMouseMove;
			device = RE::InputDevice::kMouse;
			idCode = 0;

			x = a_x;
			y = a_y;

			if (x || y) {
				stage = prev ? EventStage::Repeating : EventStage::Start;
			}
			else if (prev) {
				stage = EventStage::Finish;
			}
			else {
				stage = EventStage::None;
			}

			return *this;
		}


		VirtualEvent&& AsMouseMove(const RE::BSFixedString& event_name, int32_t a_x, int32_t a_y, bool prev = false)&&
		{
			return std::move(AsMouseMove(event_name, a_x, a_y, prev));
		}


		VirtualEvent& AsButton(const RE::BSFixedString& event_name, float a_value, float a_held, RE::InputDevice a_device = RE::InputDevice::kNone, uint32_t code = -1)&
		{
			userEvent = event_name;
			proxy = RE::InputEventType::kButton;
			device = a_device;
			idCode = code;

			value = a_value;
			heldDownSecs = a_held;


			if (value) {
				stage = heldDownSecs ? EventStage::Repeating : EventStage::Start;
			}
			else if (heldDownSecs) {
				stage = EventStage::Finish;
			}
			else {
				stage = EventStage::None;
			}
			
			return *this;
		}

		VirtualEvent&& AsButton(const RE::BSFixedString& event_name, float a_value, float a_held, RE::InputDevice a_device = RE::InputDevice::kNone, uint32_t code = -1)&&
		{
			return std::move(AsButton(event_name, a_value, a_held, a_device, code));
		}





		VirtualEvent()
		{
			idCode = -1;
			device = RE::InputDevice::kNone;
			eventType = EVENT_TYPE;
			next = nullptr;
		}
	};


}