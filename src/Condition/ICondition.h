#pragma once

namespace DIMS
{
	struct ICondition
	{
		virtual ~ICondition() = default;

		virtual bool DoTheThing(ConditionData& args) const = 0;

		bool RunCondition(ConditionData& args) const
		{
			if (!this)
				return true;
			return true;

		}
	};

	using ConditionPtr = std::unique_ptr<ICondition>;
}