#pragma once

#include "ITrigger.h"

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

		Input GetInput(Argument* list) const override;

		bool CanHandleEvent(RE::InputEvent* event, Argument* list) const override;


	};

}