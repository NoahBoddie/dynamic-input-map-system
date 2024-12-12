#pragma once

#include "IAction.h"

//*src
#include "Argument.h"

namespace DIMS
{
	
	using ActionFunction = void(ActionData&& data, EventFlag& flags, bool& result, const Argument& param1, const Argument& param2);

	struct InvokeFunction : public IAction
	{
		static constexpr auto FUNCTION_PTR = 0;
		static constexpr auto CUST_PARAM_1 = 1;
		static constexpr auto CUST_PARAM_2 = 2;

		ActionType GetActionType() const override { return ActionType::InvokeFunction; }


		bool Execute(EventData data, EventFlag& flags, const Argument* list) const override
		{
			auto function = list[FUNCTION_PTR].As<ActionFunction*>();

			bool result = true;

			function(std::forward<EventData>(data), flags, result, list[CUST_PARAM_1], list[CUST_PARAM_2]);

			return result;
		}

	};
}