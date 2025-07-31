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



	inline void ExecuteMenuInput(BSTArray<MenuEventHandler*>& a_this, InputEvent*& a_event)
	{
		//SE: 8A93E0, AE:---, VR:----
		using func_t = decltype(&ExecuteMenuInput);
		REL::Relocation<func_t> func{ REL::RelocationID(51377, 0) };
		return func(a_this, a_event);
	}

	//TODO: Instead of using this manually, please move over to alandtse's clib at some point
	inline void GetTopMostMenu(RE::UI* ui, RE::IMenu*& a_result, std::uint32_t a_depthLimit)
	{
		using func_t = decltype(GetTopMostMenu);
		static REL::Relocation<func_t> func{ RELOCATION_ID(79944, 82081) };
		return func(ui, a_result, a_depthLimit);
	}

}
