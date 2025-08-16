#pragma once

namespace DIMS
{
	struct DynamicInput
	{
		
		std::string_view name;
		Input key;
		RE::InputDevice device= RE::InputDevice::kTotal;
		RE::InputContextID context = RE::InputContextID::kTotal;

		constexpr auto operator<=>(const DynamicInput&) const = default;

		constexpr operator bool() const noexcept
		{
			return device != RE::InputDevice::kTotal && context != RE::InputContextID::kTotal;
		}
	};
}