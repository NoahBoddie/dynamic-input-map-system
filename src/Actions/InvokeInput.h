#pragma once

#include "IAction.h"

//*src
#include "Argument.h"
#include "InputQueue.h"

namespace DIMS
{
	struct InvokeInput : public IAction
	{
		static constexpr auto VIRTUAL_INPUT = 0;

		//If it's a button it's value is translated.
		//if its a mouse move or 

		ActionType GetActionType() const override { return ActionType::InvokeInput; }


		bool Execute(EventData data, EventFlag& flags, const Argument* list) const override;

	};
}