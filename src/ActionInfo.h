#pragma once

#include "Actions/IAction.h"

#include "Actions/InvokeFunction.h"
#include "Actions/InvokeInput.h"
#include "Actions/InvokeMode.h"

namespace DIMS
{
	//Make an initialize here instead.
	inline std::array<IAction*, ActionType::Total> actionInfo
	{
		new InvokeFunction,
		new InvokeInput,
		new InvokeMode,
	};
}