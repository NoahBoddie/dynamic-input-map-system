#include "OnControl.h"

#include "Argument.h"

namespace DIMS
{

	DelayState OnControl::GetDelayState(const Argument* args, const ActiveData* data, EventStage stage) const
	{
		if (!args)
			return DelayState::None;

		if (!data)
			return DelayState::Advancing;


		auto time = args[HOLD_TIME].As<float>();

		return time >= data->RuntimeDurationHeld() ? DelayState::Success : DelayState::Advancing;
	}

	Input  OnControl::GetInput(const Argument* args) const
	{
		auto id = args[CONTROL_ID].As<ControlID>();
		return Input{ RE::INPUT_DEVICE::kNone, id };
	}
}