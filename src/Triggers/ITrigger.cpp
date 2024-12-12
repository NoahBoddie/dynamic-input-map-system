#include "ITrigger.h"

#include "Parameter.h"


namespace DIMS
{
	std::span<Parameter> ITrigger::GetParameters() const { throw std::exception("This is unused"); }
}