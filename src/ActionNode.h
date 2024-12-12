#pragma once


#include "Argument.h"
#include "Parameter.h"

//*src
#include "ActionInfo.h"


namespace DIMS
{
	//Holds the data for a particular specific action.
	struct ActionNode
	{
		
		ActionType type;
		EventStage stageFilter = EventStage::All;
		ActionRecovery recovery = ActionRecovery::Break;
		uint16_t size = 0;//size may be able to change later.


		
		std::unique_ptr<Argument[]> args;		//Only needs one set of parameters thankfully.



		EventStage GetActionFilter() const
		{
			return stageFilter;
		}

		bool Execute(const EventData& data, EventFlag& flags) const
		{
			bool result = actionInfo[type]->Execute(data, flags, args.get());

			if (!result)//If continue, this action doesn't care about success or not
				result = recovery == ActionRecovery::Continue;
			
			return result;
		}

		bool IsDependent() const
		{
			return recovery == ActionRecovery::Persist;
		}
	};
}