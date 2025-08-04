#pragma once

#include "Input.h"
#include "Parameter.h"
#include "Impl/IComponent.h"

namespace DIMS
{
	struct Argument;
	struct Parameter;

	struct InputInterface;
	struct ActiveData;



	//The interface for trigger information. Holds data for how to treat the trigger
	struct ITrigger
	{
		virtual ~ITrigger() = default;

		//This doesn't seem terribly needed.
		virtual TriggerType GetTriggerType() const = 0;

		virtual EventStage GetEventFilter() const = 0;

		virtual size_t GetInputMin() const { return 1; }
		virtual size_t GetInputMax() const = 0;

		virtual std::span<const Parameter> GetParameters() const = 0;
		virtual std::span<const Parameter> GetDelayParameters() const { return {}; }

		virtual Input GetInput(const Argument* list) const = 0;


		
		virtual DelayState GetDelayState(const Argument* args, const ActiveData* data, EventStage stage/*, double value1, double value2*/) const
		{
			//This is the new set up here, there's a single argument, as this doesn't really extra care what each arg says, just how many there are.
			// So instead, I'll just give it the size of inputs.
			//Correction, the previous will just handle this. If the value is none, it will check the amount of inputs and make it Listening
			return DelayState::None;
		}


		virtual uint32_t GetPrecedence(uint32_t base) const { return base; }

		virtual uint16_t GetRank(const Argument* args) const { return 0; }


		//This likely will ONLY be used in the event that things like double tapping come into play. not before then.
		virtual bool CanHandleEvent(RE::InputEvent* event, Argument* list) const { return true; }

		//Something about parameters, but unsure how to cover that bit.
		//virtual std::pair<uint32_t, uint32_t



		std::unique_ptr<Argument[]> CreateArguments() const
		{
			auto params = GetParameters();

			if (params.size() == 0)
				return {};

			auto result = std::make_unique<Argument[]>(params.size());

			for (int i = 0; i < params.size(); i++) {
				auto& param = params[i];

				if (param.def) {
					result[i] = param.def.value();
				}
			}

			return result;
		}

		std::unique_ptr<Argument[]> CreateDelayArguments() const
		{
			auto params = GetDelayParameters();

			if (params.size() == 0)
				return {};

			auto result = std::make_unique<Argument[]>(params.size());

			for (int i = 0; i < params.size(); i++) {
				auto& param = params[i];

				if (param.def) {
					result[i] = param.def.value();
				}
			}
			return result;
		}
	};



	//An input trigger base that delays inputs by default. Instead, it doesn't consider combo when
	struct DelayTriggerBase : public ITrigger
	{

		virtual uint32_t GetPrecedence(uint32_t a_input_size) const override = 0;

		virtual bool CanHandleEvent(RE::InputEvent* event, Argument* list) const override = 0;

		//virtual DelayState GetDelayState(std::span<Argument* const> args, InputInterface* input, ActiveData* data) const override = 0;

		//virtual bool GetDelayComboState(std::span<Argument* const>& args, InputInterface* input, ActiveData* data) const override { return true; }
	};
}