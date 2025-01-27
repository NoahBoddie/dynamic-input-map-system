#pragma once

namespace DIMS
{
	struct Settings
	{
		static constexpr float comboPressTime = 0.06f;
		static constexpr float updateTime = 0.75f;

		//The value the hold time is divided by to get the rank of precedence
		static constexpr float holdingEpsilon = 0.0039f;
	};
}