#pragma once

namespace RE
{
	inline void ExecuteInput(PlayerControls* a_this, InputEvent* a_event)
	{
		using func_t = decltype(&ExecuteInput);
		REL::Relocation<func_t> func{ REL::RelocationID(41289, 0) };
		return func(a_this, a_event);
	}

	inline void UnkFunc01(PlayerControls* a_this)
	{
		using func_t = decltype(&UnkFunc01);
		REL::Relocation<func_t> func{ REL::RelocationID(41290, 0) };
		return func(a_this);
	}

}
