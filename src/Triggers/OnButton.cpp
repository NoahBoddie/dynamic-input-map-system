#include "OnButton.h"

#include "Argument.h"

namespace DIMS
{
	Input OnButton::GetInput(const Argument* list) const
	{
		return list[BUTTON_ID].As<Input>();
	}
}