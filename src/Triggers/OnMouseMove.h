#pragma once

#include "ITrigger.h"

//*src
#include "ActiveData.h"
#include "VirtualEvent.h"

namespace DIMS
{
	struct Argument;
	struct Parameter;

	struct OnMouseMove : public ITrigger
	{
		//All trigger classes will have these big old parameter constants. While the names of these parameters are found
		static constexpr auto BUTTON_ID = 0;



		//Assign this manually.
		TriggerType GetTriggerType() const override { return TriggerType::OnMouseMove; }

		EventStage GetEventFilter() const override { return EventStage::All; }

		//This is currently unused, so it should throw for now.

		Input GetInput(const Argument*) const override
		{
			return Input{ RE::INPUT_DEVICE::kMouse, 0xA };
		}

		virtual bool CanHandleEvent(RE::InputEvent* event, Argument* list) const
		{
			return event->eventType == RE::INPUT_EVENT_TYPE::kMouseMove || event->eventType == VirtualEvent::EVENT_TYPE;
		}


	};

}