#pragma once

#include "ITrigger.h"

//*src
#include "ActiveData.h"
#include "VirtualEvent.h"

namespace DIMS
{
	struct Argument;
	struct Parameter;

	struct OnAxis : public ITrigger
	{
		//All trigger classes will have these big old parameter constants. While the names of these parameters are found
		static constexpr auto AXIS_ID = 0;



		//Assign this manually.
		TriggerType GetTriggerType() const override { return TriggerType::OnAxis; }

		EventStage GetEventFilter() const override { return EventStage::All; }

		//This is currently unused, so it should throw for now.

		Input GetInput(const Argument* args) const override;

		virtual bool CanHandleEvent(RE::InputEvent* event, Argument* list) const
		{
			return event->eventType == RE::INPUT_EVENT_TYPE::kMouseMove || 
				event->eventType == RE::INPUT_EVENT_TYPE::kThumbstick || 
				event->eventType == VirtualEvent::EVENT_TYPE;
		}



	};

}