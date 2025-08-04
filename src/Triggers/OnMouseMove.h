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
		//Assign this manually.
		TriggerType GetTriggerType() const override { return TriggerType::OnMouseMove; }

		EventStage GetEventFilter() const override { return EventStage::All; }

		//This is currently unused, so it should throw for now.

		size_t GetInputMin() const override { return 0; }
		size_t GetInputMax() const override { return 0; }

		Input GetInput(const Argument*) const override
		{
			return Input{ RE::INPUT_DEVICE::kMouse, 0xA };
		}

		//Only one mouse can move, no parameters
		std::span<const Parameter> GetParameters() const override { return {}; }

		virtual bool CanHandleEvent(RE::InputEvent* event, Argument* list) const
		{
			return event->eventType == RE::INPUT_EVENT_TYPE::kMouseMove || event->eventType == VirtualEvent::EVENT_TYPE;
		}


	};

}