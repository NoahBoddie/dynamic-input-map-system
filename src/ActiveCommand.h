#pragma once

//#include "InputCommand.h"
#include "CommandEntry.h"
namespace DIMS
{
	class InputCommand;


	struct ActiveCommand
	{
		using ID = uint32_t;
		
		inline static std::atomic<uint32_t> nextID = 1;

		

		CommandEntryPtr command;

		EventStage stages = EventStage::None;

		bool failure = false;
		
	private:
		ID _id = nextID == -1 ? ++nextID : nextID++;//A simple way to
	public:


		ID id() const
		{
			assert(_id);
			return _id;
		}

		ActiveCommand(CommandEntryPtr ptr, EventStage stg) : command{ ptr }, stages{ stg }
		{

		}
	};

	using ActiveCommandPtr = std::shared_ptr<ActiveCommand>;

}