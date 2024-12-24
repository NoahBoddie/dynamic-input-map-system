#pragma once

//*src
#include "TriggerNode.h"
#include "InputCommand.h"

namespace DIMS
{

	using InputCount = int8_t;

	struct CommandEntry
	{
		//Prohibit this from EVER being copy constructed

		inline static uint32_t globalTimestamp = -1;
		
		mutable uint32_t localTimestamp = -1;
		
		constexpr static uint8_t invalidIndex = 15;

		
		//This method does not work. Multiple viable (and intended) inputs will not result in a very sound combination. The only solution
		// is increasing the size of literally everything.
		mutable uint8_t _index : 4 = invalidIndex;
		mutable EventStage stagesVisit : 4 = EventStage::None;

		//I'm clumping these together because they'll be used most together.

		//Use negative counts to turn these off.


	
		//I save some space if I make this the number of inputs it needs to complete (and also assert it needs to be 0 or above)
		InputCount inputs = 1;			//This is how many things are currently interacting with this
		InputCount success = 0;		//This is how many inputs need to happen before this is considered for active.
		//TODO: Input is a bit large, you can only press so many buttons at once. Maybe curb the amount some.
		



		//If failure is not 0, you not to regard. Failure is SPECIFICALLY used when something like a delay function fails requirements like
		// the amount of time required to
		InputCount failure = 0;

		//This index is incremented just know this is cursed btw.


		//Might have activeCount negative mean that it is unable to be used for the time being.
		TriggerNode* trigger = nullptr;//because a command can have multiple trigger sets, something like this would be needed to differ them.
		InputCommand* command = nullptr;

		uint64_t priority()
		{
			//Priority while 
			return command->GetPriority(trigger->priority);
		}


		bool IsReady() const
		{
			return inputs == 0;
		}

		void PushIndex() const
		{
			//With this I don't think I actually need waiters anymore
			if (_index != -1)
			{
				if (++_index >= trigger->trigger_size())
					_index = 0;

			}
		}

#pragma region GlobalIdea

		//Instead of this, I think I want a function that checks if we are in static time, and if so increments (warning against exceeding 255).
		// Then, once it decrements, it keeps decrementing until it eventually would be trying to decrement 0, to which it will simply set it to
		// the value it's supposed to be

		//Alternatively, -1 can be used as a  force clear. Meaning the next time this value is found, please re get both the global timestamp
		// and the local time stamp

		static void IncStaticTimestamp()
		{
			if (globalTimestamp <= 255)
			{
				//Past 255 it no longer a static timestamp
				assert(globalTimestamp < 255);

				globalTimestamp++;
			}
			else
			{
				globalTimestamp = 0;
			}
		}

		static void DecStaticTimestamp()
		{
			if (globalTimestamp <= 255)
			{
				if (globalTimestamp == 0) {
					globalTimestamp = -1;
				}
				else {
					globalTimestamp--;
				}
			}
		}


		static uint32_t GetGlobalTimestamp()
		{
			if (globalTimestamp > 255)
			{
				globalTimestamp = RE::GetDurationOfApplicationRunTime();
			}

			return globalTimestamp;
		}

		uint32_t GetTimestamp()
		{
			auto time = GetGlobalTimestamp();

			if (localTimestamp != time) {
				localTimestamp = time;
			}

			return localTimestamp;
		}


		bool UpdateExecute(EventStage stage) const
		{
			//I just realized a thought rq. Do I actually need to support refiring a button while it's already being held down?
			// that just doesn't make any sense does it?

			if (auto time = GetGlobalTimestamp(); localTimestamp != time && (stage == EventStage::Repeating || (stage & stagesVisit) == EventStage::None)) {
				localTimestamp = time;
				stagesVisit = stagesVisit | stage;
				return true;
			}

			return false;
		}

		bool HasVisitedStage(EventStage stage)
		{
			return stagesVisit & stage;
		}

		bool HasRanThisFrame() const
		{
			return GetGlobalTimestamp() == localTimestamp;
		}

		void TryResetStages() const
		{
			if (trigger->trigger_size() == inputs) {
				stagesVisit = EventStage::None;
			}
		}

		void ResetExecute() const
		{
			localTimestamp = -1;
		}

		void TryMarkForRealease()
		{
			if (GetGlobalTimestamp() != localTimestamp) {
				ResetExecute();
			}
		}

		bool HasEarlyRelease() const
		{
			return localTimestamp == -1;
		}
		
#pragma endregion

		uint8_t index() const
		{
			//if (_index != -1) {
			//	if (auto time = RE::GetDurationOfApplicationRunTime(); oldTimestamp != time) {
			//		oldTimestamp = time;
			//		_index = 0;
			//	}
			//}

			auto value = _index;

			return value == invalidIndex ? -1 : value;

		}


		InputCount GetInputRef() const { return inputs; }


		InputCount GetSuccess() const { return success; }
		InputCount GetFailure() const { return failure; }


		//These are reversed due to 0 being valued as "all inputs active"
		InputCount IncInputRef() { inputs--; assert(inputs >= 0); return inputs; }

		InputCount DecInputRef() { inputs++;  assert(trigger->trigger_size() >= inputs); TryResetStages(); return inputs; }


		
		InputCount IncSuccess() { success++; assert(trigger->trigger_size() >= success); return success; }

		InputCount DecSuccess() { success--;  assert(success >= 0); return success; }

		InputCount IncFailure() { failure++; assert(trigger->trigger_size() >= failure); return failure; }

		InputCount DecFailure() { failure--;  assert(failure >= 0); return failure; }




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
			return trigger->GetStageFilter() | command->reqStage;
		}


		bool IsStageInTrigger(EventStage stage) const
		{
			return GetTriggerFilter() & stage;
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



		bool CanHandleEvent(RE::InputEvent* event, bool push = true) const
		{
			return trigger->CanHandleEvent(event, index());
		}


		//Execute returns if it actually executed anything or if the event was stifled
		bool Execute(const EventData& data, EventFlag& flags, bool& success) const
		{
			bool result = UpdateExecute(data.stage);

			if (result)
				success = command->Execute(data, flags);
			else
				success = false;

			return result;
		}

		//TODO:Action success doesn't matter much right now, when it does delete this so we can hunt down what we need.
		bool Execute(const EventData& data, EventFlag& flags) const
		{
			bool toss;
			return Execute(data, flags, toss);
		}


		//Used when a single event may want to repeatedly fire execute in the same frame. Uses first time success to tell if it should reset or not.
		bool RepeatExecute(const EventData& data, EventFlag& flags, bool& repeat) const
		{
			if (repeat)
				ResetExecute();

			auto result = Execute(data, flags);

			if (result)
				repeat = true;

			return result;
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

			return Precedence() > other.Precedence();
		}


		uint32_t Precedence() const
		{
			return trigger->Precedence();
		}

		bool IsDelayable() const
		{
			return trigger->IsDelayable();
		}


		inline DelayState GetDelayState(InputInterface* input, ActiveData* data) const
		{
			return trigger->GetDelayState(input, data);
		}


		inline DelayState GetDelayState(InputInterface* input, ActiveData& data) const
		{
			return GetDelayState(input, &data);
		}

		//May not need
		//bool IsDelayable() const{return trigger->trigger_size() > 1;}

		//TODO: This needs to confirm these both come from the same space.
		CommandEntry(InputCommand* cmd, TriggerNode* node, bool a_control) : trigger{ node }, command{ cmd }, _index{ a_control ? (uint8_t)0 : invalidIndex }
		{
			//This doesn't need an parameter for control checking, I can ask the trigger node this. 
			// Make this a function.
			//assert(cmd->triggers...);
			inputs = node->trigger_size();
		}
	};

	using CommandEntryPtr = std::shared_ptr<CommandEntry>;


	struct EntryIndexCleaner
	{
		CommandEntryPtr& entry;
		
		~EntryIndexCleaner()
		{
			entry->PushIndex();
		}

		EntryIndexCleaner(CommandEntryPtr& e) : entry{ e }
		{

		}
	};
}