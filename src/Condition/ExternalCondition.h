#pragma once

#include "ICondition.h"

namespace DIMS
{

	struct ExternCondition : public ICondition
	{
		enum struct State
		{
			Failure,
			Neutral,
			Success,
		};




		double ExecuteCondition(ConditionData& data) const
		{
			if (!function)
				return NAN;//I think any comparison fails

			return function(RE::PlayerCharacter::GetSingleton(), data);
		}


		bool Do(ConditionData& data) const
		{
			//Keeping the value as a double is an easy way to represent both a float and an integer
			
			double value = ExecuteCondition(data);

			switch (compType & ~CompareType::kSecondary)
			{
			case CompareType::kNotEqual:
				return value != compValue;

			case CompareType::kLesser:
				return value < compValue;

			case CompareType::kGreaterOrEqual:
				return value >= compValue;

			case CompareType::kGreater:
				return value > compValue;

			case CompareType::kLesserOrEqual:
				return value <= compValue;

			case CompareType::kEqual:
				return value == compValue;
			}

			return false;
		}


		static bool RealInnerMaybe(ConditionData& data, const ExternCondition*& it, uint16_t& index, uint16_t limit, State state)
		{

			if (limit)
			{
				return InnerMaybe(data, it, index, limit + index, state);
			}
			else if (state == State::Neutral) {
				return it->Do(data);
			}

			return state != State::Failure;

		}

		static bool InnerMaybe(ConditionData& data, const ExternCondition*& it, uint16_t& index, uint16_t limit, State state)
		{
			bool first = true;

			do
			{
				auto result = RealInnerMaybe(data, it, index, first ? 0 : it->parenthesisGroup, state);

				if (!result)
					state = State::Failure;


				if (it->isOr)
				{
					switch (state)
					{
					case State::Neutral:
						state = State::Success;
						break;

					case State::Failure:
						state = State::Neutral;
						break;
					}
				}
				first = false;
				it = it->next.get();
				index++;
			} while (it && index <= limit);

			return state != State::Failure;
		}


		bool DoTheThing(ConditionData& data) const override
		{
			const ExternCondition* it = this;

			uint16_t index = 0;


			return InnerMaybe(data, it, index, -1, State::Neutral);
		}


		


		ConditionFunction* function = nullptr;

		std::unique_ptr<ExternCondition> next = nullptr;
		
		CompareType compType = CompareType::kEqual;

		bool isOr = false;

		uint16_t parenthesisGroup = 0;

		double compValue = 1.0;




	};



	struct HeaderExternCondition : public ExternCondition
	{
		using ExternCondition::ExternCondition;

		ExternCondition* tail = nullptr;//Used to tell what to append something to


		void AddCondition(ExternCondition* condition)
		{
			
			if (!tail) {
				
				next.reset(condition);
			}
			else {
				tail->next.reset(condition);
			}

			tail = condition;
		}


	};
}