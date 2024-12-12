#pragma once

#include "Triggers/ITrigger.h"
#include "Triggers/OnButton.h"
#include "Triggers/OnControl.h"

namespace DIMS
{
	inline std::array<ITrigger*, TriggerType::Total> triggerInfo
	{
		new OnButton,
		new OnControl,
	};
}