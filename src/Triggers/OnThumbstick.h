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


		constexpr static Parameter PARAMETERS[]
		{
			Parameter{"useRight", ParameterType::Bool},
		};

		size_t GetInputMax() const override { return 2; }


		//Assign this manually.
		TriggerType GetTriggerType() const override { return TriggerType::OnThumbstick; }

		EventStage GetEventFilter() const override { return EventStage::All; }

		//This is currently unused, so it should throw for now.

		Input GetInput(const Argument* args) const override;


		std::span<const Parameter> GetParameters() const override { return PARAMETERS; }


		virtual bool CanHandleEvent(RE::InputEvent* event, Argument* list) const
		{
			return event->eventType == RE::INPUT_EVENT_TYPE::kThumbstick ||
				event->eventType == VirtualEvent::EVENT_TYPE;
		}



	};

}