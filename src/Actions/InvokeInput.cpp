#include "InvokeInput.h"

#include "ActiveData.h"

namespace DIMS
{

	bool InvokeInput::Execute(EventData data, EventFlag& flags, const Argument* list) const
	{
		//Needs to force 0 to start because on keydown will not work unless seconds held is 0.
		InputQueue::AddEvent(list[VIRTUAL_INPUT].As<Input>(), "", data.event.GetValue(), data.stage == EventStage::Start ? 0 : data.inputData->SecondsHeld());

		return true;
	}
}