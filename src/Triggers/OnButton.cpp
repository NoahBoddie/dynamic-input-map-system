#include "OnButton.h"

#include "Argument.h"

namespace DIMS
{

	Input OnButton::GetInput(const Argument* list) const
	{
		return list[BUTTON_ID].As<Input>();
	}

	bool OnButton::CanHandleEvent(RE::InputEvent* event, Argument* list) const
	{
		//Command maps resolve this, no need to query.

		//return Input{ event } == list->As<Input>();

		return true;
	}

}