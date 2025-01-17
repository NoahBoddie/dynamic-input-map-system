#pragma once

//*src
#include "TriggerNode.h"
#include "InputCommand.h"

namespace DIMS
{

	using InputCount = int8_t;

	constexpr InputCount k_signedInput = 1 << (sizeof(InputCount) * 8 - 1);

	struct CommandEntry
	{
		//Prohibit this from EVER being copy constructed

		inline static uint32_t globalTimestamp = -1;
		
		mutable uint32_t localTimestamp = -1;
		

		//TODO: With the advent of control inputs existing, we no longer need index.
		
		//This method does not work. Multiple viable (and intended) inputs will not result in a very sound combination. The only solution
		// is increasing the size of literally everything.

		mutable EventStage stagesVisit  = EventStage::None;

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

		uint64_t priority() const
		{
			//Priority while 
			return command->GetPriority(trigger->priority);
		}


		MatrixType GetParentType() const
		{
			return command->GetParentType();
		}

		

		bool CompareOrder(const CommandEntry& other) const
		{
			//This function is a comparison of 2 command entries. Ultimately it boils down to something like this.
			// if the type is larger.
			// if they are the same, a function call happens on the matrix (which is only allowed to happen if they are the same.
			// if that reports that this derives from the other, it's greater. if the other way around it's lesser.
			// if they are unrelated, THEN priority swoops in.
			
			
				switch (strong_ordering_to_int(command->CompareOrder(other.command)))
				{
					//This is reversed, as 0 is considered higher than 1 and so on.
				case  1:
					return true;
				case -1:
					return false;

				default:
					return priority() > priority();
				}


		}

		bool IsReady() const
		{
			return inputs == 0;
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
			if (trigger->trigger_size() == GetInputRef()) {
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


		InputCount GetInputRef() const { return inputs & ~k_signedInput; }

		InputCount GetRealInputRef() const { return trigger->trigger_size() - GetInputRef(); }


		InputCount GetSuccess() const { return success & ~k_signedInput; }
		InputCount GetFailure() const { return failure & ~k_signedInput; }

		bool IsActive() const
		{
			return GetSuccess() || GetFailure();
		}

		//These are reversed due to 0 being valued as "all inputs active"
		InputCount IncInputRef() 
		{	
			auto val = inputs;
			auto out = val & k_signedInput;
			val &= ~k_signedInput;
			inputs = --val;
			assert(inputs >= 0);
			inputs |= out;
			return inputs; 
		}

		InputCount DecInputRef() 
		{
			auto val = inputs;
			auto out = val & k_signedInput;
			val &= ~k_signedInput;
			inputs = ++val;
			
			inputs |= out;
			TryResetStages();
			return inputs; 
		}


		
		InputCount IncSuccess() 
		{ 
			auto val = success;
			auto out = val & k_signedInput;
			val &= ~k_signedInput;
			success = ++val;
			assert(trigger->trigger_size() >= success);
			success |= out;
			return val;

			//success++; 
			//assert(trigger->trigger_size() >= success); 
			//return success; 
		}

		InputCount DecSuccess() 
		{ 
			auto val = success;
			auto out = val & k_signedInput;
			val &= ~k_signedInput;
			success = --val;
			assert(success >= 0);
			success |= out;
			if (!val) ClearComplete();
			return val;

			//success--;  
			//assert(success >= 0); 
			//return success; 
		}

		InputCount IncFailure() 
		{ 
			auto val = failure;
			auto out = val & k_signedInput;
			val &= ~k_signedInput;
			failure = ++val;
			assert(trigger->trigger_size() >= failure);
			failure |= out;
			return val;

			//failure++; 
			//assert(trigger->trigger_size() >= failure); 
			//return failure; 
		}

		InputCount DecFailure() 
		{
			auto val = failure;
			auto out = val & k_signedInput;
			val &= ~k_signedInput;
			failure = --val;
			assert(failure >= 0);
			failure |= out;
			if (!val) ClearCancel();
			return val;

			//failure--;  
			//assert(failure >= 0); 
			//return failure; 
		}


		bool IsEnabled() const { return !IsDisabled(); }
		bool IsDisabled() const { return inputs & k_signedInput; }
		bool IsComplete() const { return success & k_signedInput; }
		bool IsCancelled() const { return failure & k_signedInput; }

		void SetEnabled(bool value) 
		{ 
			if (value)
				inputs &= ~k_signedInput;
			else
				inputs |= k_signedInput;
		}

		//Other than disable, do not use these yet.
		
		void Enable() { SetEnabled(true); }
		void Disable() { SetEnabled(false); }
		void SetComplete() { success |= k_signedInput; }
		void SetCancel() { failure |= k_signedInput; }

	protected:
		void ClearComplete() { success &= ~k_signedInput; }
		void ClearCancel() { failure &= ~k_signedInput; }
		
	public:

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
			case ConflictLevel::Blocking:
				return GetTriggerFilter();
			
			case ConflictLevel::Guarding:
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
			case ConflictLevel::Guarding:
				return false;

			default:
				return true;
			}
		}



		bool CanHandleEvent(RE::InputEvent* event) const
		{
			return trigger->CanHandleEvent(event);
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
		CommandEntry(InputCommand* cmd, TriggerNode* node) : trigger{ node }, command{ cmd }
		{
			//This doesn't need an parameter for control checking, I can ask the trigger node this. 
			// Make this a function.
			//assert(cmd->triggers...);
			inputs = node->trigger_size();
		}
	};

	using CommandEntryPtr = std::shared_ptr<CommandEntry>;
}