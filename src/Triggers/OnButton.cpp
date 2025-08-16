#include "OnButton.h"

#include "Argument.h"
#include "StringTable.h"

namespace DIMS
{
	Input OnButton::GetInput(const Argument* list) const
	{
		Input result = list[BUTTON_ID].As<Input>();

		if (result.IsUserEvent() == true) {
			result.device = RE::InputDevice::kTotal;
		}

		return result;
	}

	DynamicInput OnButton::GetDynamicInput(const Argument* list) const
	{
		if (Input event = list[BUTTON_ID].As<Input>(); event.IsUserEvent() == true)
		{
			DynamicInput result;

			result.name = StringTable::FindString(event.code);
			assert(!result.name.empty());

			result.device = list[DEVICE_ID].As<RE::InputDevice>();
			result.context = list[CONTEXT_ID].As<RE::InputContextID>();
			result.key = Input::CreateDynamicInput(event.code, result.device, result.context);

			return result;
		}

		return {};
	}
}