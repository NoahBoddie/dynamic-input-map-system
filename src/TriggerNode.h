#pragma once

#include "Argument.h"
#include "Parameter.h"

//*src
#include "TriggerInfo.h"


namespace DIMS
{

	struct TriggerNode
	{
		TriggerType type;
		EventStage stageFilter = EventStage::All;
		//Priority is handled as a 16 bit number, but getting it's value is a 32 bit number. This allows for mutations
		// of impossible to replicate values to take place, making it so some states are always on modes, and the other way around.
		uint16_t priority;
		uint16_t size = 0;//size of trigger arguments
		ConflictLevel conflict = ConflictLevel::None;//I may move this to action, due to the fact I'm not 100% sure triggers will want unique conflicts.
		TriggerFlag flags = TriggerFlag::None;

		//rename this pls
		std::vector<std::unique_ptr<Argument[]>> args;//Note, maximum amount of inputs for any button is 10. thumbsticks 2, mouse 1.


		void* conditions;//These are conditions SOLELY for the trigger in question.

		size_t trigger_size()
		{
			return args.size();
		}

		bool IsControlTrigger() const
		{
			auto trig_info = triggerInfo[type];
			return trig_info->IsControlTrigger();
		}


		const Argument* GetInputArgument(size_t input_index, size_t arg_index)
		{
			auto& input = args[input_index];

			//if (arg_index >= size)
			//	return nullptr;

			&input[arg_index];
		}

		std::vector<Input> GetInputs() const
		{
			//No numbers because it initializes with numbers
			std::vector<Input> result{ };
			result.resize(args.size());

			std::transform(args.begin(), args.end(), result.begin(), [this](const std::unique_ptr<Argument[]>& it)
				{
					return triggerInfo[type]->GetInput(it.get());
				});

			return result;
		}


		std::vector<ControlID> GetControls()
		{
			//No numbers because it initializes with numbers
			std::vector<ControlID> result{ };
			result.resize(args.size());

			std::transform(args.begin(), args.end(), result.begin(), [this](std::unique_ptr<Argument[]>& it)
				{
					return triggerInfo[type]->GetControl(it.get());
				});

			return result;
		}

		//Returns both controls and inputs, with controls being designated with a null device
		std::vector<Input> GetControlInputs()
		{
			std::vector<Input> result{ };
	
			result.resize(args.size());

			std::transform(args.begin(), args.end(), result.begin(), [this](std::unique_ptr<Argument[]>& it)
			{
				if (IsControlTrigger() == true)
					return Input{ RE::INPUT_DEVICE::kNone, triggerInfo[type]->GetControl(it.get()) };
				else
					return triggerInfo[type]->GetInput(it.get());
			});

			return result;
		}




		bool CanHandleEvent(RE::InputEvent* event, int8_t index) const
		{
			auto trig_info = triggerInfo[type];

			//This is for contros
			auto size = index == -1 ? args.size() : index + 1;
			
			//This shit is a poorly attempt to get it to loop only if it's not a control
			for (auto i = index == -1 ? 0 : index; i < size; i++)
			{
				if (trig_info->CanHandleEvent(event, args[i].get()) == true)
					return true;
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
			

			return GetDelayState(nullptr, nullptr) != DelayState::None;
		}


		DelayState GetDelayState(InputInterface* input, ActiveData* data) const
		{
			auto trig_info = triggerInfo[type];

			auto& re_args = reinterpret_cast<const std::vector<Argument*>&>(args);

			return trig_info->GetDelayState(re_args, input, data);
		}

		uint32_t Precedence() const
		{
			return triggerInfo[type]->GetPrecedence(args.size());
		}
		
	};


}