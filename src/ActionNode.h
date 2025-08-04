#pragma once


#include "Argument.h"
#include "Parameter.h"

//*src
#include "ActionInfo.h"


namespace DIMS
{
	struct InputMode;

	//Holds the data for a particular specific action.
	struct ActionNode
	{
		
		ActionType type;
		EventStage stageFilter = EventStage::All;
		ActionRecovery recovery = ActionRecovery::Break;
		uint16_t size = 0;//size may be able to change later.


		
		std::unique_ptr<Argument[]> args;		//Only needs one set of parameters thankfully.

		IAction* GetActionInfo() const
		{
			auto result = actionInfo[type];

			assert(result);

			return result;
		}

		void SetActionType(ActionType a_type)
		{
			auto info = GetActionInfo();

			type = a_type;
			
			if (auto ptr = args.get())
				info->UnhandleArguments(ptr);
			
			args = info->CreateArguments();
		}
		
		bool SetArgument(size_t i, Argument value, ParameterType type)
		{
			auto info = GetActionInfo();

			auto params = info->GetParameters();

			if (params.size() > i) {
				if (params[i].type == ParameterType::Any || params[i].type == type) {
					args[i] = value;
					return true;
				}
			}

			return false;
		}
		


		bool SetArgument(size_t i, bool value) { return SetArgument(i, value, ParameterType::Bool); }
		bool SetArgument(size_t i, int value) { return SetArgument(i, value, ParameterType::Int); }
		bool SetArgument(size_t i, float value) { return SetArgument(i, value, ParameterType::Float); }
		bool SetArgument(size_t i, std::string_view value) { return SetArgument(i, value, ParameterType::String); }
		bool SetArgument(size_t i, RE::TESForm* value) { return SetArgument(i, value, ParameterType::Form); }
		bool SetArgument(size_t i, Input value) { return SetArgument(i, value, ParameterType::Input); }
		bool SetArgument(size_t i, InputMode* value) { return SetArgument(i, value, ParameterType::Mode); }
		bool SetArgument(size_t i, ActionFunction* value) { return SetArgument(i, value, ParameterType::Function); }


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