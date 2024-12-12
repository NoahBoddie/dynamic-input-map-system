#pragma once

//*src
#include "TriggerNode.h"
#include "InputCommand.h"

namespace DIMS
{


	struct CommandEntry
	{
		//I save some space if I make this the number of inputs it needs to complete (and also assert it needs to be 0 or above)
		int16_t inputs = 0;			//This is how many things are currently interacting with this
		int16_t activeCount = 0;	//This is how many inputs need to happen before this is considered for active.

		//Might have activeCount negative mean that it is unable to be used for the time being.
		TriggerNode* trigger = nullptr;//because a command can have multiple trigger sets, something like this would be needed to differ them.
		InputCommand* command = nullptr;

		uint32_t priority()
		{
			//Cache this result so it doesn't change midway through
			return trigger->priority;
		}


		bool AllowInput(Input input) const
		{
			return trigger->AllowInput(input);
		}

		bool AllowControl(ControlID control) const
		{
			return trigger->AllowControl(control);
		}


		//I need to move a lot of these back into the trigger nodes
		ConflictLevel GetConflictLevel() const
		{
			get_switch(trigger->GetConflictLevel())
			{
			case ConflictLevel::None:
				//What this will actually do later is query the action instead.
				if (auto res = command->GetConflictLevel(); res != ConflictLevel::None)
					return res;

				return ConflictLevel::Sharing;

			default:
				return switch_value;
			}

		}

		EventStage GetTriggerFilter() const
		{
			return trigger->GetStageFilter();
		}

		EventStage GetActionFilter() const
		{
			return command->GetStageFilter();
		}


		EventStage GetFirstStage() const
		{
			auto test = GetTriggerFilter();

			auto inch = std::countr_zero(std::to_underlying(test));

			return (EventStage)(1 << inch);
		}


		EventStage GetBlockingFilter() const
		{
			switch (trigger->GetConflictLevel())
			{
			case ConflictLevel::Guarding:
			case ConflictLevel::Blocking:
				return GetTriggerFilter();

			case ConflictLevel::Defending:
			case ConflictLevel::Capturing:
				return EventStage::All;

			default:
				return EventStage::None;
			}
		}

		//Gets if this CommandEntry should block other non-basic triggers from firing.
		bool ShouldBlockTriggers() const
		{
			switch (trigger->GetConflictLevel())
			{
			case ConflictLevel::Capturing:
			case ConflictLevel::Blocking:
				return true;

			default:
				return false;
			}
		}

		bool ShouldBlockNative() const
		{
			switch (trigger->GetConflictLevel())
			{
			case ConflictLevel::Guarding:
			case ConflictLevel::Defending:
				//Fallthrough
			case ConflictLevel::Capturing:
			case ConflictLevel::Blocking:
				return true;

			default:
				return false;
			}
		}

		bool ShouldBeBlocked() const
		{
			switch (trigger->GetConflictLevel())
			{
			case ConflictLevel::None:
			case ConflictLevel::Sharing:
				return false;

			default:
				return true;
			}
		}



		bool CanHandleEvent(RE::InputEvent* event) const
		{
			return trigger->CanHandleEvent(event);
		}


		bool Execute(const EventData& data, EventFlag& flags) const
		{
			return command->Execute(data, flags);
		}


		bool HasMultipleEventStages() const
		{
			return trigger->HasMultipleEventStages();
		}

		bool HasMultipleBlockStages() const
		{
			if (auto stages = GetBlockingFilter())
				return stages & EventStage::Repeating || stages == EventStage::StartFinish;

			return false;
		}

		bool HasMultipleStages() const
		{
			return HasMultipleEventStages() || HasMultipleBlockStages();
		}


		//TODO: This needs to confirm.
		CommandEntry(InputCommand* cmd, TriggerNode* node) : trigger{ node }, command{ cmd }
		{


		}
	};

	using CommandEntryPtr = std::shared_ptr<CommandEntry>;
}