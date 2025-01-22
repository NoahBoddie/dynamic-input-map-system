#pragma once

#include "Input.h"

#include "Impl/IComponent.h"

namespace DIMS
{
	struct Argument;
	struct Parameter;

	struct InputInterface;
	struct ActiveData;

	enum struct DelayState
	{
		None,		//If none, it cannot be delayed at all, and is thus, undelayable.
		Continue,
		Failure,	//The conditions have promoted failure, and this it will not continue.
	};

	//The interface for trigger information. Holds data for how to treat the trigger
	struct ITrigger
	{
		virtual ~ITrigger() = default;

		//This doesn't seem terribly needed.
		virtual TriggerType GetTriggerType() const = 0;

		virtual EventStage GetEventFilter() const = 0;

		virtual std::span<Parameter> GetParameters() const;

		virtual Input GetInput(const Argument* list) const = 0;

		virtual DelayState GetDelayState(std::span<Argument* const> args, InputInterface* input, ActiveData* data) const
		{
			return args.size() <= 1 ? 
				DelayState::None : GetDelayComboState(args, input, data) ?
				DelayState::Continue : DelayState::Failure;
		}

		virtual bool GetDelayComboState(std::span<Argument* const>& args, InputInterface* input, ActiveData* data) const = 0;

		virtual uint32_t GetPrecedence(uint32_t a_input_size) const { return a_input_size; }

		//This likely will ONLY be used in the event that things like double tapping come into play. not before then.
		virtual bool CanHandleEvent(RE::InputEvent* event, Argument* list) const { return true; }

		//Something about parameters, but unsure how to cover that bit.
		//virtual std::pair<uint32_t, uint32_t
	};



	//An input trigger base that delays inputs by default. Instead, it doesn't consider combo when
	struct DelayTriggerBase : public ITrigger
	{

		virtual uint32_t GetPrecedence(uint32_t a_input_size) const override = 0;

		virtual bool CanHandleEvent(RE::InputEvent* event, Argument* list) const override = 0;

		virtual DelayState GetDelayState(std::span<Argument* const> args, InputInterface* input, ActiveData* data) const override = 0;

		virtual bool GetDelayComboState(std::span<Argument* const>& args, InputInterface* input, ActiveData* data) const override { return true; }
	};
}