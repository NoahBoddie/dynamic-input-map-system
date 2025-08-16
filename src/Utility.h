#pragma once

namespace DIMS
{
	static uint32_t GetRunTime()
	{
		static uint32_t* g_runTime = (uint32_t*)REL::RelocationID(523662, 410201).address();
		return *g_runTime;
	}

	static RE::InputContextID GetCurrentInputContext()
	{
		RE::ControlMap* controls = RE::ControlMap::GetSingleton();

		if (controls && controls->contextPriorityStack.size() != 0)
			return controls->contextPriorityStack.back();

		return RE::InputContextID::kGameplay;
	}


	static bool GetCurrentInputDevice()
	{
		//So this is unique, to get the device associated you'd need to manually use something like a control and a context.
		//However, this can be on multiple different input types. So instead, I'll just as the question of what type of device we'll be accepting.
		// So ignore keyboard or not.

		RE::ControlMap* controls = RE::ControlMap::GetSingleton();

		if (controls && controls->ignoreKeyboardMouse)
			return REL::Module::IsVR() ? true : true;

	}



	static size_t GetMaxDevices() noexcept
	{
		return RE::ControlMap::InputContext::GetNumDeviceMappings();
	}

	static std::vector<RE::BSFixedString> GetUserEvents(uint32_t id, RE::InputContextID context, RE::InputDevice device, bool useGroupFlags = false)
	{
		RE::ControlMap* cmap = RE::ControlMap::GetSingleton();


		//assert(a_device < RE::INPUT_DEVICE::kTotal);
		//assert(a_context < RE::InputContextID::kTotal);

		if (auto context_list = cmap->controlMap[context]) {
			const auto& mappings = context_list->deviceMappings[device];
			RE::UserEventMapping tmp{};
			tmp.inputKey = (uint16_t)id;
			auto range = std::equal_range(
				mappings.begin(),
				mappings.end(),
				tmp,
				[](auto&& a_lhs, auto&& a_rhs) {
					return a_lhs.inputKey < a_rhs.inputKey;
				});


			std::vector<RE::BSFixedString> result{};

			auto ctrls = cmap->enabledControls.get();

			for (auto it = range.first; it != range.second; it++) {	
				if (useGroupFlags) {
					if (auto flags = it->userEventGroupFlag & ctrls; flags != ctrls)
						continue;
				}
				
				result.push_back(it->eventID);
			}
			return result;

		}

		return {};
	}
}