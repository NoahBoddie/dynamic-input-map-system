#pragma once

#include "TriggerNode.h"
#include "ActionNode.h"

namespace DIMS
{
	class InputMatrix;

	class InputCommand
	{
	public:
		//Placeholder name. This is the result of both trigger and action. Be sure to give it enough information that OnLayer can work properly.
		// That is, give it all the input information it needs to have to confirm it's the one.
		std::vector<TriggerNode> triggers;

		InputMatrix* parent = nullptr;

		void* conditions = nullptr;

		std::vector<ActionNode> actions;

		ConflictLevel conflict = ConflictLevel::Sharing;
		EventStage stageFilter = EventStage::All;			//Controls if the command actually does any actions
		EventStage reqStage = EventStage::None;			//Stages required by actions on the command.
		ActionRecovery recovery = ActionRecovery::Break;

		std::string name;


		MatrixType GetParentType() const;


		std::strong_ordering CompareOrder(const InputCommand* other) const;




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