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

		//If it's a button it's value is translated.
		//if its a mouse move or 

		ActionType GetActionType() const override { return ActionType::InvokeMode; }

		
		EventStage GetRequiredStages() const override { return EventStage::StartFinish; }

		bool Execute(EventData data, EventFlag& flags, const Argument* list) const override;

	};
}