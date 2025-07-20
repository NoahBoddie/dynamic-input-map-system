#pragma once

namespace DIMS
{
	struct ConditionLibary
	{
		static void AddCondition(std::string_view name, ConditionFunction* func)
		{
			auto& entry = _map[name];

			if (entry) {
				//warning
			}

			entry = func;
		}

		static ConditionFunction* FindCondition(std::string_view name)
		{
			auto it = _map.find(name);

			if (_map.end() != it)
				return it->second;

			return nullptr;
		}




	private:
		inline static std::unordered_map<std::string_view, ConditionFunction*> _map;



	};
}