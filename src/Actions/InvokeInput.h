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

		constexpr static Parameter PARAMETERS[]
		{
			Parameter{"Input", ParameterType::Input},
		};

		//If it's a button it's value is translated.
		//if its a mouse move or 


		ActionType GetActionType() const override { return ActionType::InvokeInput; }

		std::span<const Parameter> GetParameters() const override { return PARAMETERS; }


		bool Execute(EventData data, EventFlag& flags, const Argument* list) const override;

	};
}