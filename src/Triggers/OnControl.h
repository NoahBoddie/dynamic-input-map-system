#pragma once

#include "IHoldTrigger.h"

//*src
#include "ActiveData.h"
#include "VirtualEvent.h"
namespace DIMS
{
	struct OnControl : public IHoldTrigger
	{
		//All trigger classes will have these big old parameter constants. While the names of these parameters are found
		static constexpr auto CONTROL_ID = 0;


		constexpr static Parameter PARAMETERS[]
		{
			Parameter{"Control", ParameterType::String},
		};

		//While this could technically support more, realistically this comes out of button events and likely won't have more than 10
		size_t GetInputMax() const override { return 10; }

		TriggerType GetTriggerType() const override { return TriggerType::OnControl; }

		EventStage GetEventFilter() const override { return EventStage::All; }

		//This is currently unused, so it should throw for now.

		bool CanHandleEvent(RE::InputEvent* event, Argument* list) const override
		{
			return event->eventType == RE::INPUT_EVENT_TYPE::kButton || event->eventType == VirtualEvent::EVENT_TYPE;
		}

		std::span<const Parameter> GetParameters() const override { return PARAMETERS; }


		Input GetInput(const Argument* args) const override;


	};
}