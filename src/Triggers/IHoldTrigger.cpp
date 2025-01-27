#include "IHoldTrigger.h"

#include "Argument.h"
#include "Settings.h"
namespace DIMS
{
	DelayState IHoldTrigger::GetDelayState(const Argument* args, const ActiveData* data, EventStage stage) const
	{
		if (!args)
			return DelayState::None;

		if (!data)
			return DelayState::Advancing;

		auto holding_time = args[HOLD_TIME].As<float>();

		return holding_time <= data->SecondsHeld() ? DelayState::Success : DelayState::Advancing;
	}

	uint16_t IHoldTrigger::GetRank(const Argument* args) const
	{
		if (!args)
			return __super::GetRank(args);

		auto holding_time = args[HOLD_TIME].As<float>();

		//This is basically an proximation of rank. Holding time will have a max value of around 255 seconds
		return std::roundf(holding_time / Settings::holdingEpsilon);
	}
}