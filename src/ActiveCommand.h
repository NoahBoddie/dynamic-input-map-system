#pragma once

//#include "InputCommand.h"
#include "CommandEntry.h"
namespace DIMS
{
	class InputCommand;


	struct ActiveCommand
	{
		//kinda think this shouldn't be unsigned. Also, size can probably be reduced. The likeliness of encountering the same id 
		// in the same list is next to impossible, even with a ton of commands. Because remember, they'd have to be in the same 
		// ActiveInput.
		using ID = uint32_t;

	private:
		inline static std::atomic<ID> nextID = 1;

	public:

		~ActiveCommand()
		{
			get_switch (state())
			{
			case ActiveState::Running:
				entry->DecSuccess();
				goto manage;

			case ActiveState::Failing:
				entry->DecFailure();
				goto manage;

			case ActiveState::Managing:
			manage:
				entry->DecInputRef();
				break;

			default:
				//logger::info("* nothing, {} OR {}, {}", magic_enum::enum_name(switch_value), (int)switch_value, id());
				break;
					
			}

		}

		ActiveCommand(CommandEntryPtr ptr) : entry{ ptr }
		{

		}

		ActiveCommand(const ActiveCommand& other) = delete;
	
		ActiveCommand(ActiveCommand&& other) noexcept : entry{ other.entry }, _id{ other._id }, _inputs{ other._inputs },
			_state{ std::exchange(other._state, ActiveState::Inactive) }, waiters{ other.waiters }
		{
			
		}



		ActiveCommand& operator=(const ActiveCommand& other)
		{
			entry = other.entry;
			_id = other._id;
			waiters = other.waiters;
			_inputs = other._inputs;
			_state = ActiveState::Inactive;
			

		}

		ActiveCommand& operator=(ActiveCommand&& other)
		{
			entry = other.entry;
			_id = other._id; 
			waiters = other.waiters;
			_inputs = other._inputs;
			_state = std::exchange(other._state, ActiveState::Inactive);
			return *this;
		}

		CommandEntry* operator->()
		{
			return entry.get();
		}

		const CommandEntry* operator->() const
		{
			return entry.get();
		}

		CommandEntry& operator*()
		{
			return *entry;
		}

		const CommandEntry& operator*() const
		{
			return *entry;
		}



		//Activates the input
		void Activate()
		{
			InputCount val;
			if (state() == ActiveState::Inactive) {
				SetState(ActiveState::Managing);
				val = entry->IncInputRef();
			}
			else {
				val = entry->GetInputRef();
			}
			
			Update(val);
		}

		void Deactivate() const
		{

			switch (state())
			{
			case ActiveState::Running:
				entry->DecSuccess();
				[[fallthrough]];
			case ActiveState::Managing:
				entry->IncFailure();
				SetState(ActiveState::Failing);
				logger::info("failing {}", id());
				break;
			}
		}

	private:
		void Update(InputCount in) const
		{
			if (entry->GetFailure() > 0)
				Deactivate();

			switch (state())
			{
			default:
			case ActiveState::Inactive:
				//Won't matter, it's inactive.
				break;


			case ActiveState::Managing:
				if (entry->GetSuccess() || !in) {
					SetState(ActiveState::Running);
					entry->IncSuccess();
				}
				[[fallthrough]];

			case ActiveState::Running:
			case ActiveState::Failing:
				_inputs = in;
				break;
			}
		}
	public:

		void Update() const
		{
			return Update(entry->GetInputRef());
		}

		bool IsDirty() const
		{
			return entry->GetInputRef() != _inputs;
		}

		bool UpdateIfDirty() const
		{
			auto result = IsDirty();

			if (result) {
				Update();
			}
			return result;
		}

		bool IsRunning() const
		{
			UpdateIfDirty();
			
			return state() == ActiveState::Running && !waiters;
		}

		bool IsFailing() const
		{
			UpdateIfDirty();

			return state() == ActiveState::Failing;
		}

		bool IsManaging() const
		{
			return !IsFailing() && !IsRunning() && state() != ActiveState::Inactive;
		}

		void tempname_IncWaiters()
		{
			_state |= ActiveState::Waited;
			waiters++;
		}


		int16_t tempname_DecWaiters()
		{
			waiters--; 
			assert(waiters >= 0); 
			return waiters;			
		}

		bool IsWaiting() const
		{
			return waiters;
		}

		bool HasWaited() const
		{
			return _state & ActiveState::Waited;
		}

		bool IsDelayUndone() const
		{
			return _state & ActiveState::DelayUndo;
		}

		void ClearWaiting() const
		{
			_state &= ~ActiveState::Waited;
		}

		void SetDelayUndone() const
		{
			_state |= ActiveState::DelayUndo;
		}

		void SetEarlyExit(bool value) const
		{
			if (value)
				_state |= ActiveState::EarlyExit;
			else 
				_state &= ~ActiveState::EarlyExit;
		}

		bool HasEarlyExit() const
		{
			return _state & ActiveState::EarlyExit;
		}

	protected:
		ActiveState state() const
		{
			return _state & ~ActiveState::Flags;
		}

		void SetState(ActiveState value) const
		{
			auto flags = _state & ActiveState::Flags;
			
			_state = value | flags;
		}


public:
		//16+2+1+1+4
		mutable CommandEntryPtr entry;


		int16_t waiters = 0;


	private:
		
		mutable ActiveState _state = ActiveState::Inactive;

		mutable InputCount _inputs = 0;

		ID _id = nextID == -1 ? ++nextID : nextID++;//A simple way to

	public:


		ID id() const
		{
			assert(_id);
			return _id;
		}

	};

	using ActiveCommandPtr = std::shared_ptr<ActiveCommand>;

}