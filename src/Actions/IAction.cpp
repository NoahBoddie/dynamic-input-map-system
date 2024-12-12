#include "IAction.h"

#include "Parameter.h"

namespace DIMS
{

	std::span<Parameter> IAction::GetParameters() const
	{
		throw std::exception("This is unused");
	}

}