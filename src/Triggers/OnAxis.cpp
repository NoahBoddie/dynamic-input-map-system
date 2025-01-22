#include "OnAxis.h"

#include "Argument.h"

namespace DIMS
{
	Input OnAxis::GetInput(const Argument* args) const
	{
		auto id = args[AXIS_ID].As<ControlID>();
		return Input{ RE::INPUT_DEVICE::kNone, id };
	}
}