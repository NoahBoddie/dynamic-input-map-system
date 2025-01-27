#include "OnControl.h"

#include "Argument.h"

namespace DIMS
{
	Input OnControl::GetInput(const Argument* args) const
	{
		auto id = args[CONTROL_ID].As<ControlID>();
		return Input{ RE::INPUT_DEVICE::kNone, id };
	}
}