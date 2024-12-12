#pragma once

#include "Enums.h"
#include "Argument.h"

namespace DIMS
{

	struct Parameter
	{
		std::string_view name;
		ParameterType type;
		std::optional<Argument> def;
	};


}