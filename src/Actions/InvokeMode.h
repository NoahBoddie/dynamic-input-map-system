#pragma once

#include "IAction.h"

//*src
#include "Argument.h"
#include "InputQueue.h"

namespace DIMS
{
	struct InvokeMode : public IAction
	{
		static constexpr auto MODE_PTR = 0;
		
		constexpr static Parameter PARAMETERS[]
		{
			Parameter{"Mode", ParameterType::Mode},
		};

		//If it's a button it's value is translated.
		//if its a mouse move or 

		ActionType GetActionType() const override { return ActionType::InvokeMode; }


		std::span<const Parameter> GetParameters() const override { return PARAMETERS; }

		EventStage GetRequiredStages() const override { return EventStage::StartFinish; }

		bool Execute(EventData data, EventFlag& flags, const Argument* list) const override;

	};
}