#pragma once

#include "Argument.h"
#include "Parameter.h"

#include "Condition/ICondition.h"

//*src
#include "TriggerInfo.h"


namespace DIMS
{


	struct TriggerNode
	{
		TriggerType type = TriggerType::None;
		EventStage stageFilter = EventStage::All;
		//Priority is handled as a 16 bit number, but getting it's value is a 32 bit number. This allows for mutations
		// of impossible to replicate values to take place, making it so some states are always on modes, and the other way around.
		uint16_t priority;
		uint16_t size = 0;//size of trigger arguments
		ConflictLevel conflict = ConflictLevel::None;//I may move this to action, due to the fact I'm not 100% sure triggers will want unique conflicts.
		TriggerFlag flags = TriggerFlag::None;
		

		//rename this pls
		//TODO: It seems TriggerNode::args is kinda just used for inputs. Please 
		std::vector<std::unique_ptr<Argument[]>> args;//Note, maximum amount of inputs for any button is 10. thumbsticks 2, mouse 1.

		//TODO: Rename "delayArgs" to something fitting like requirements.
		std::unique_ptr<Argument[]> delayArgs;

		//std::vector<std::pair<std::string, StringHash>> requiresMenu

		ConditionPtr conditions;//These are conditions SOLELY for the trigger in question.


		ITrigger* GetTriggerInfo()
		{
			auto info = triggerInfo[type];
			assert(info);
			return info;
		}

		ITrigger* GetTriggerInfo() const
		{
			return unconst(this)->GetTriggerInfo();
		}


		Argument* CreateRequiredInput()
		{
			auto info = GetTriggerInfo();

			auto& result = args.emplace_back(info->CreateArguments());

			return result.get();
		}


		void SetTriggerType(TriggerType a_type)
		{
			auto info = GetTriggerInfo();

			type = a_type;

			for (auto& arg : args)
			{
				arg = info->CreateArguments();
			}

			delayArgs = info->CreateDelayArguments();
		}



		bool SetArgument(Argument* input, size_t i, Argument value, ParameterType type)
		{
			auto info = GetTriggerInfo();

			auto params = info->GetParameters();

			if (params.size() > i) {
				if (params[i].type == ParameterType::Any || params[i].type == type) {
					input[i] = value;
					return true;
				}
			}

			return false;
		}


		bool SetArgument(size_t input, size_t arg, Argument value, ParameterType type)
		{
			if (args.size() > input)
				return false;

			return SetArgument(args[input].get(), arg, value, type);
		}



		bool SetArgument(size_t x, size_t y, bool value) { return SetArgument(x, y, value, ParameterType::Bool); }
		bool SetArgument(size_t x, size_t y, int value) { return SetArgument(x, y, value, ParameterType::Int); }
		bool SetArgument(size_t x, size_t y, float value) { return SetArgument(x, y, value, ParameterType::Float); }
		bool SetArgument(size_t x, size_t y, std::string_view value) { return SetArgument(x, y, value, ParameterType::String); }
		bool SetArgument(size_t x, size_t y, RE::TESForm* value) { return SetArgument(x, y, value, ParameterType::Form); }
		bool SetArgument(size_t x, size_t y, Input value) { return SetArgument(x, y, value, ParameterType::Input); }





		bool SetArgument(Argument* x, size_t y, bool value) { return SetArgument(x, y, value, ParameterType::Bool); }
		bool SetArgument(Argument* x, size_t y, int value) { return SetArgument(x, y, value, ParameterType::Int); }
		bool SetArgument(Argument* x, size_t y, float value) { return SetArgument(x, y, value, ParameterType::Float); }
		bool SetArgument(Argument* x, size_t y, std::string_view value) { return SetArgument(x, y, value, ParameterType::String); }
		bool SetArgument(Argument* x, size_t y, RE::TESForm* value) { return SetArgument(x, y, value, ParameterType::Form); }
		bool SetArgument(Argument* x, size_t y, Input value) { return SetArgument(x, y, value, ParameterType::Input); }




		bool SetDelayArgument(size_t i, Argument value, ParameterType type)
		{
			auto info = GetTriggerInfo();

			auto params = info->GetDelayParameters();

			if (params.size() > i) {
				if (params[i].type == ParameterType::Any || params[i].type == type) {
					delayArgs[i] = value;
					return true;
				}
			}

			return false;
		}



		bool SetDelayArgument(size_t i, bool value) { return SetDelayArgument(i, value, ParameterType::Bool); }
		bool SetDelayArgument(size_t i, int value) { return SetDelayArgument(i, value, ParameterType::Int); }
		bool SetDelayArgument(size_t i, float value) { return SetDelayArgument(i, value, ParameterType::Float); }
		bool SetDelayArgument(size_t i, std::string_view value) { return SetDelayArgument(i, value, ParameterType::String); }
		bool SetDelayArgument(size_t i, RE::TESForm* value) { return SetDelayArgument(i, value, ParameterType::Form); }
		bool SetDelayArgument(size_t i, Input value) { return SetDelayArgument(i, value, ParameterType::Input); }






		size_t GetInputCount() const
		{
			return std::max((size_t)1, args.size());
		}


		const Argument* GetInputArgument(size_t input_index, size_t arg_index)
		{
			auto& input = args[input_index];

			//if (arg_index >= size)
			//	return nullptr;

			&input[arg_index];
		}

		std::vector<std::variant<Input, DynamicInput>> GetInputs() const
		{
			//No numbers because it initializes with numbers
			std::vector<std::variant<Input, DynamicInput>> result{ };
			result.resize(args.size());

			ITrigger* info = GetTriggerInfo();

			if (args.size() == 0)//TODO: needs to require that the parameters have a size of 0 to do this
			{
				result.push_back(triggerInfo[type]->GetInput(nullptr));
			}
			else
			{
				std::transform(args.begin(), args.end(), result.begin(), [this, info](const std::unique_ptr<Argument[]>& it) -> std::variant<Input, DynamicInput>
				{
					auto ptr = it.get();

					if (auto input = info->GetInput(ptr); input.IsDynamicInput() == true)
						return input;

					return info->GetDynamicInput(ptr);
				});
			}
			return result;
		}




		//[[deprecated("No longer needed, command maps handle this, put on retirement")]]
		bool CanHandleEvent(RE::InputEvent* event) const
		{
			auto trig_info = triggerInfo[type];

			if (args.size() == 0)//TODO: needs to require that the parameters have a size of 0 to do this
			{
				return trig_info->CanHandleEvent(event, nullptr);
			}
			else
			{
				for (auto i = 0; i < args.size(); i++)
				{
					if (trig_info->CanHandleEvent(event, args[i].get()) == true)
						return true;
				}
			}


			return false;
		}

		ConflictLevel GetConflictLevel() const
		{
			return conflict;
		}

		EventStage GetStageFilter() const
		{
			return stageFilter;
		}

		bool HasMultipleEventStages() const
		{
			if (auto stages = GetStageFilter())
				return stages & EventStage::Repeating || stages == EventStage::StartFinish;
			
			return false;
		}
		

		bool IsDelayable() const
		{
			return GetDelayState(nullptr, EventStage::None) != DelayState::None;
		}


		DelayState GetDelayState(const ActiveData* data, EventStage stage) const
		{
			auto trig_info = triggerInfo[type];

			DelayState result = trig_info->GetDelayState(delayArgs.get(), data, stage);

			if (args.size() > 1) {


				if (data && data->SecondsHeld() < Settings::comboPressTime) {
					if (result == DelayState::None)
						result = DelayState::Listening;
				}
				else {
					result = DelayState::Failure;
				}
			}

			return result;
		}

		uint32_t Precedence() const
		{
			//0 shouldn't be a used value.
			// if it has 0 args, it's 1. If it has 1, it's one.
			// If it has 2, it's 3.
			// if it has some inate stuff, that's plus 1.
			//Here's what I get from this. min(args.size(), 0) + IsDelayable() if it's delayable

			auto size = args.size();

			//If it's delayable (or has multiple entries +1. Base size min 1.
			//1: Commands with 1 or a fixed trigger
			//2: Commands with 1 or a fixed trigger that causes delay.
			//3+: Commands with multiple triggers.
			//uint32_t base = std::max(size, (size_t)1) + (size > 1 ? 1 : IsDelayable());
			uint32_t base = std::max(size, (size_t)1);

			//GetDelayState(nullptr, EventStage::None)

			return triggerInfo[type]->GetPrecedence(base);
		}


		uint32_t Rank() const
		{
			return triggerInfo[type]->GetRank(delayArgs.get());
		}
		
	};


}