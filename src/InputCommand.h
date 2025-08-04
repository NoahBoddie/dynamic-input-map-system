#pragma once

#include "TriggerNode.h"
#include "ActionNode.h"
#include "Condition/ICondition.h"

namespace DIMS
{
	class InputMatrix;

	class InputCommand
	{
	public:
		//Placeholder name. This is the result of both trigger and action. Be sure to give it enough information that OnLayer can work properly.
		// That is, give it all the input information it needs to have to confirm it's the one.
		std::vector<TriggerNode> triggers;

		//General condition for if the repeating action actually fires or not.
		//InputMatrix* parent = nullptr;

		
		//It'd be neat to have activation conditions and run conditions be seperate.
		Condition conditions;

		std::vector<ActionNode> actions;

		ConflictLevel conflict = ConflictLevel::Sharing;
		EventStage stageFilter = EventStage::All;			//Controls if the command actually does any actions
		EventStage reqStage = EventStage::None;			//Stages required by actions on the command.
		ActionRecovery recovery = ActionRecovery::Break;

		std::string name;

		std::unique_ptr<std::vector<StringHash>> menuReqs = nullptr;


		bool CheckConditions()
		{
			constexpr EventStage test = magic_enum::enum_cast<EventStage>("Start").value_or(EventStage::None);

			if (!conditions)
				return true;

			return conditions().ToBoolean(true);
		}



		bool ShouldUpdate() const
		{
			if (menuReqs) {

			}
			
			return true;
		}


		ActionNode& CreateAction(ActionType type)
		{
			auto& result = actions.emplace_back();
			result.SetActionType(type);
			return result;
		}


		TriggerNode& CreateTrigger(TriggerType type)
		{
			auto& result = triggers.emplace_back();
			result.SetTriggerType(type);
			return result;
		}



		uint64_t GetPriority(uint16_t value)
		{
			return value;
		}

		ConflictLevel GetConflictLevel() const
		{
			return conflict;
		}

		EventStage GetStageFilter() const
		{
			return stageFilter;
		}

		bool Execute(const EventData& data, EventFlag& flags) const
		{
			bool failure = false;
			bool result = true;

			for (auto& action : actions)
			{
				result = action.Execute(data, flags);

				if (!result) {
					switch (recovery)
					{
					case ActionRecovery::Continue:
						result = true;
						break;

					case ActionRecovery::Persist:
						failure = false;
						break;
					
					case ActionRecovery::Break:
						if (action.IsDependent() == true)
							continue;

						return false;

					}
				}
			}

			return result && !failure;
		}
	};
}