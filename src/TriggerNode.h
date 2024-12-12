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
		uint16_t size = 0;//Event filter will be uint8, this is literal free space. Might as well cache.
		ConflictLevel conflict = ConflictLevel::None;//I may move this to action, due to the fact I'm not 100% sure triggers will want unique conflicts.
		TriggerFlag flags = TriggerFlag::None;

		//rename this pls
		std::vector<std::unique_ptr<Argument[]>> args;//Note, maximum amount of inputs for any button is 10. thumbsticks 2, mouse 1.


		void* conditions;//These are conditions SOLELY for the trigger in question.


		const Argument* GetInputArgument(size_t input_index, size_t arg_index)
		{
			auto& input = args[input_index];

			//if (arg_index >= size)
			//	return nullptr;

			&input[arg_index];
		}

		std::vector<Input> GetInputs()
		{
			//No numbers because it initializes with numbers
			std::vector<Input> result{ };
			result.resize(args.size());

			std::transform(args.begin(), args.end(), result.begin(), [this](std::unique_ptr<Argument[]>& it)
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


		bool AllowInput(Input input) const
		{
			if (input.IsControl())
				return false;

			auto trig_info = triggerInfo[type];

			for (auto& arg : args)
			{
				if (trig_info->GetInput(arg.get()) == input)
					return true;
			}

			return false;
		}

		bool AllowControl(ControlID control) const
		{
			if (!control)
				return false;

			auto trig_info = triggerInfo[type];

			for (auto& arg : args)
			{
				if (trig_info->GetControl(arg.get()) == control)
					return true;
			}

			return false;
		}



		bool CanHandleEvent(RE::InputEvent* event) const
		{
			auto trig_info = triggerInfo[type];

			for (auto& arg : args)
			{
				if (trig_info->CanHandleEvent(event, arg.get()) == true)
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
		
		
	};


}