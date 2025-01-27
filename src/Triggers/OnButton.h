#pragma once

#include "IHoldTrigger.h"

//*src
#include "ActiveData.h"
#include "VirtualEvent.h"

namespace DIMS
{
	struct Argument;
	struct Parameter;

	struct OnButton : public IHoldTrigger
	{
		//All trigger classes will have these big old parameter constants. While the names of these parameters are found
		static constexpr auto BUTTON_ID = 0;




		TriggerType GetTriggerType() const override { return TriggerType::OnButton; }

		EventStage GetEventFilter() const override { return EventStage::All; }

		//This is currently unused, so it should throw for now.

		Input GetInput(const Argument* list) const override;

		virtual bool CanHandleEvent(RE::InputEvent* event, Argument* list) const
		{
			return event->eventType == RE::INPUT_EVENT_TYPE::kButton || event->eventType == VirtualEvent::EVENT_TYPE;
		}


	};

}