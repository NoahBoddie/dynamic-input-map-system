#pragma once

#include "InputNumber.h"

namespace DIMS
{

	struct ActiveData
	{
		uint32_t timestamp = 0;
		uint32_t pressCount = 0;

		//These are the inputs each event uses.
		//Mouse: xMove, yMove
		//Thumb: xValue, yValue
		//Button: value, held
		//+Note, these are the values stored at the start of the ActiveInput.
		InputNumber value1{ 0 };
		InputNumber value2{ 0 };

		//Retrieves the raw runtime duration this input has been active. Returns in milleseconds
		int32_t RuntimeDurationHeld() const
		{
			return RE::GetDurationOfApplicationRunTime() - timestamp;
		}
		float SecondsHeld() const
		{
			return RuntimeDurationHeld() / 1000.f;
		}
	};

}