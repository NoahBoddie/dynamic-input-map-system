#pragma once

namespace DIMS
{
	static uint32_t GetRunTime()
	{
		static uint32_t* g_runTime = (uint32_t*)REL::RelocationID(523662, 410201).address();
		return *g_runTime;
	}

}