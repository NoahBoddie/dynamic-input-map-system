#pragma once

namespace DIMS
{

	struct ConditionData
	{
		//Size helps assertain what members can be accessed.
		const size_t size = sizeof(ConditionData);
		RE::IMenu* openMenu{};

	};


}