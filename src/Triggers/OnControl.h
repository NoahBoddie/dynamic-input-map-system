#pragma once

#include "ITrigger.h"

//*src
#include "ActiveData.h"

namespace DIMS
{
	struct OnControl : public ITrigger
	{
		//All trigger classes will have these big old parameter constants. While the names of these parameters are found
		static constexpr auto CONTROL_ID = 0;




		TriggerType GetTriggerType() const override { return TriggerType::OnControl; }

		EventStage GetEventFilter() const override { return EventStage::All; }

		//This is currently unused, so it should throw for now.


		Input GetInput(const Argument* args) const override;

		bool GetDelayComboState(std::span<Argument* const>& args, InputInterface* input, ActiveData* data) const override
		{
			return data ? data->SecondsHeld() < Settings::comboPressTime : true;
		}

	};
}