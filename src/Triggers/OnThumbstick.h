#pragma once

#include "ITrigger.h"

//*src
#include "ActiveData.h"
#include "VirtualEvent.h"

namespace DIMS
{
	struct Argument;
	struct Parameter;

	struct OnThumbstick : public ITrigger
	{
		//All trigger classes will have these big old parameter constants. While the names of these parameters are found
		static constexpr auto RIGHT_THUMBSTICK = 0;

		static constexpr auto kRightThumbstick = RE::ThumbstickEvent::InputType::kRightThumbstick;
		static constexpr auto kLeftThumbstick = RE::ThumbstickEvent::InputType::kLeftThumbstick;


		//Assign this manually.
		TriggerType GetTriggerType() const override { return TriggerType::OnThumbstick; }

		EventStage GetEventFilter() const override { return EventStage::All; }

		//This is currently unused, so it should throw for now.

		Input GetInput(const Argument* args) const override;

		virtual bool CanHandleEvent(RE::InputEvent* event, Argument* list) const
		{
			return event->eventType == RE::INPUT_EVENT_TYPE::kThumbstick ||
				event->eventType == VirtualEvent::EVENT_TYPE;
		}

		bool GetDelayComboState(std::span<Argument* const>& args, InputInterface* input, ActiveData* data) const override
		{
			return data ? data->SecondsHeld() < Settings::comboPressTime : true;
		}



	};

}