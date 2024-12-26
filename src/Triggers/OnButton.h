#pragma once

#include "ITrigger.h"

//*src
#include "ActiveData.h"

namespace DIMS
{
	struct Argument;
	struct Parameter;

	struct OnButton : public InputTriggerBase
	{
		//All trigger classes will have these big old parameter constants. While the names of these parameters are found
		static constexpr auto BUTTON_ID = 0;




		TriggerType GetTriggerType() const override { return TriggerType::OnButton; }

		EventStage GetEventFilter() const override { return EventStage::All; }

		//This is currently unused, so it should throw for now.

		Input GetInput(const Argument* list) const override;


		bool GetDelayComboState(std::span<Argument* const>& args, InputInterface* input, ActiveData* data) const override
		{
			return data ? data->SecondsHeld() < Settings::comboPressTime : true;
		}


		bool CanHandleEvent(RE::InputEvent* event, Argument* list) const override;


	};

}