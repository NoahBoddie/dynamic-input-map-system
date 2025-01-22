#include "OnThumbstick.h"

#include "Argument.h"

namespace DIMS
{
	Input OnThumbstick::GetInput(const Argument* args) const
	{
		return args[RIGHT_THUMBSTICK].As<bool>() ? Input{ RE::InputDevice::kGamepad, kRightThumbstick } : Input{ RE::InputDevice::kGamepad, kLeftThumbstick };
	}
}