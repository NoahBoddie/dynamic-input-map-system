#pragma once

#include "Triggers/ITrigger.h"
#include "Triggers/OnButton.h"
#include "Triggers/OnControl.h"
#include "Triggers/OnMouseMove.h"
#include "Triggers/OnThumbstick.h"
#include "Triggers/OnAxis.h"

namespace DIMS
{
	inline std::array<ITrigger*, TriggerType::Total> triggerInfo
	{
		new OnButton,
		new OnControl,
		new OnMouseMove,
		new OnThumbstick,
		new OnAxis,
	};
}