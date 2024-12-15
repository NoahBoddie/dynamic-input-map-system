#pragma once

//*src
#include "TriggerNode.h"
#include "InputCommand.h"

namespace DIMS
{


	struct CommandEntry
	{
		//Prohibit this from EVER being copy constructed


		//All of these can be 8 bit btw. You can't have more than 10 inputs per trigger.

		//I save some space if I make this the number of inputs it needs to complete (and also assert it needs to be 0 or above)
		int16_t inputs = 1;			//This is how many things are currently interacting with this
		int16_t success = 0;		//This is how many inputs need to happen before this is considered for active.
		//TODO: Input is a bit large, you can only press so many buttons at once. Maybe curb the amount some.
		
		//If failure is not 0, you not to regard. Failure is SPECIFICALLY used when something like a delay function fails requirements like
		// the amount of time required to
		int16_t failure = 0;

		//Might have activeCount negative mean that it is unable to be used for the time being.
		TriggerNode* trigger = nullptr;//because a command can have multiple trigger sets, something like this would be needed to differ them.
		InputCommand* command = nullptr;

		uint32_t priority()
		{
			//Cache this result so it doesn't change midway through
			return trigger->priority;
		}


		bool IsReady() const
		{
			return inputs == 0;
		}

		int16_t GetInputRef() const { return inputs; }


		int16_t GetSuccess() const { return success; }
		int16_t GetFailure() const { return failure; }


		//These are reversed due to 0 being valued as "all inputs active"
		int16_t IncInputRef() { inputs--; assert(inputs >= 0); return inputs; }

		int16_t DecInputRef() { inputs++; assert(trigger->trigger_size() >= inputs); return inputs; }


		
		int16_t IncSuccess() { success++; assert(trigger->trigger_size() >= success); return success; }

		int16_t DecSuccess() { success--;  assert(success >= 0); return success; }

		int16_t IncFailure() { failure++; assert(trigger->trigger_size() >= failure); return failure; }

		int16_t DecFailure() { failure--;  assert(failure >= 0); return failure; }



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

		bool HasFinishStage() const
		{
			return GetTriggerFilter() & EventStage::Finish;
		}

		bool tmpname_ShouldWaitOnMe(const CommandEntry& other)
		{
			//This function will return space ship at some point maybe.

			//This function will change after a while, but this is basically the idea. The idea of whether something changes or not is basically
			// Does this thing have more trigger inputs than I do?

			//Another note for later,
			// For the basic types I have now, they can only declare something wait on them  if they are a similar trigger type. Both controls/buttons
			// But for other types like OnFlick or OnTap these have their own category, and cannot be sidelined by the previous trigger types.

			return trigger->trigger_size() > other.trigger->trigger_size();
		}


		bool IsDelayable() const
		{
			return trigger->IsDelayable();
		}

		//May not need
		//bool IsDelayable() const{return trigger->trigger_size() > 1;}

		//TODO: This needs to confirm these both come from the same space.
		CommandEntry(InputCommand* cmd, TriggerNode* node) : trigger{ node }, command{ cmd }
		{
			//assert(cmd->triggers...);
			inputs = node->trigger_size();
		}
	};

	using CommandEntryPtr = std::shared_ptr<CommandEntry>;
}