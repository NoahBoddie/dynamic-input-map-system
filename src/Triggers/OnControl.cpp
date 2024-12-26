#include "OnControl.h"

#include "Argument.h"

namespace DIMS
{
	ControlID OnControl::GetControl(const Argument* list) const
	{
		return list[CONTROL_ID].As<ControlID>();
	}

	bool OnControl::CanHandleEvent(RE::InputEvent* event, Argument* list) const
	{
		auto hash = std::hash<std::string_view>{}(event->QUserEvent().c_str());

		return GetControl(list) == hash;
	}
}