#pragma once

#include "Actions/IAction.h"

#include "Actions/InvokeFunction.h"

namespace DIMS
{
	inline std::array<IAction*, ActionType::Total> actionInfo
	{
		new InvokeFunction
	};
}