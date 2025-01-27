#pragma once

#include "ITrigger.h"

//*src
#include "ActiveData.h"
#include "VirtualEvent.h"
namespace DIMS
{
	struct IHoldTrigger : public ITrigger
	{
		static constexpr auto HOLD_TIME = 0;

		DelayState GetDelayState(const Argument* args, const ActiveData* data, EventStage stage) const override;
		
		uint16_t GetRank(const Argument* args) const override;

	};
}