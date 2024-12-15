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
			if (_state >= ActiveState::Managing) {
				entry->DecInputRef();
				
				if (_state == ActiveState::Running)
					entry->DecSuccess();

				logger::info("Ending active entry {}, entry name {}", _id, entry->command->name);
			}
		}

		ActiveCommand(CommandEntryPtr ptr, EventStage stg) : entry{ ptr }, stages{ stg }
		{

		}

		ActiveCommand(const ActiveCommand& other) = delete;
	
		ActiveCommand(ActiveCommand&& other) : entry{ other.entry }, stages{ other.stages }, _id{ other._id }, _inputs{ other._inputs },
			_state{ std::exchange(other._state, ActiveState::Inactive) }, waiters{ other.waiters }
		{
			
		}



		ActiveCommand& operator=(const ActiveCommand& other)
		{
			entry = other.entry;
			stages = other.stages;
			_id = other._id;
			waiters = other.waiters;
			_inputs = other._inputs;
			_state = ActiveState::Inactive;

		}

		ActiveCommand& operator=(ActiveCommand&& other)
		{
			entry = other.entry;
			stages = other.stages;
			_id = other._id; 
			waiters = other.waiters;
			_inputs = other._inputs;
			_state = std::exchange(other._state, ActiveState::Inactive);
		}

		CommandEntry* operator->()
		{
			return entry.get();
		}

		const CommandEntry* operator->() const
		{
			return entry.get();
		}

		//Activates the input
		void Activate()
		{
			int16_t val;
			if (_state == ActiveState::Inactive) {
				_state = ActiveState::Managing;
				val = entry->IncInputRef();
			}
			else {
				val = entry->GetInputRef();
			}
			
			Update(val);
		}

		void Deactivate() const
		{

			switch (_state)
			{
			case ActiveState::Running:
				entry->DecSuccess();
				[[fallthrough]];
			case ActiveState::Managing:
				entry->IncFailure();
				_state = ActiveState::Failing;
				break;
			}

			int16_t val;
			if (_state == ActiveState::Managing ||  _state == ActiveState::Running) {
				_state = ActiveState::Managing;
				val = entry->IncInputRef();
			}
			else {
				val = entry->GetInputRef();
			}

			Update(val);
		}

	private:
		void Update(int16_t in) const
		{
			if (entry->GetFailure() > 0)
				Deactivate();

			switch (_state)
			{
			default:
			case ActiveState::Inactive:
				//Won't matter, it's inactive.
				break;


			case ActiveState::Managing:
				if (!in) {
					_state = ActiveState::Running;
					entry->IncSuccess();
				}
				[[fallthrough]];

			case ActiveState::Running:
			case ActiveState::Failing:
				_inputs = in;
				break;
			}

			return;

			_inputs = in;

			if (!in && _state == ActiveState::Managing) {
				_state = ActiveState::Running;
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
			
			return _state == ActiveState::Running && !waiters;
		}


		//16+1+1+1+4
		mutable CommandEntryPtr entry;
		
		EventStage stages = EventStage::None;


		int16_t waiters = 0;

		void tempname_IncWaiters()
		{
			waiters++;
		}


		void tempname_DecWaiters()
		{
			waiters--;
		}

	private:
		
		mutable ActiveState _state = ActiveState::Inactive;

		mutable uint16_t _inputs;

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