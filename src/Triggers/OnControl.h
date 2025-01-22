#pragma once

#include "ITrigger.h"

//*src
#include "ActiveData.h"
#include "VirtualEvent.h"
namespace DIMS
{
	struct OnControl : public ITrigger
	{
		//All trigger classes will have these big old parameter constants. While the names of these parameters are found
		static constexpr auto CONTROL_ID = 0;




		TriggerType GetTriggerType() const override { return TriggerType::OnControl; }

		EventStage GetEventFilter() const override { return EventStage::All; }

		//This is currently unused, so it should throw for now.

		bool CanHandleEvent(RE::InputEvent* event, Argument* list) const override
		{
			return event->eventType == RE::INPUT_EVENT_TYPE::kButton || event->eventType == VirtualEvent::EVENT_TYPE;
		}


		Input GetInput(const Argument* args) const override;

		bool GetDelayComboState(std::span<Argument* const>& args, InputInterface* input, ActiveData* data) const override
		{
			return data ? data->SecondsHeld() < Settings::comboPressTime : true;
		}

	};
}