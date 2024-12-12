#pragma once

#include "Input.h"

namespace DIMS
{
	struct Argument;
	struct Parameter;


	//The interface for trigger information. Holds data for how to treat the trigger
	struct ITrigger
	{
		virtual ~ITrigger() = default;

		//This doesn't seem terribly needed.
		virtual TriggerType GetTriggerType() const = 0;

		virtual EventStage GetEventFilter() const = 0;

		virtual std::span<Parameter> GetParameters() const;

		virtual Input GetInput(Argument* list) const = 0;

		virtual ControlID GetControl(Argument* list) const = 0;


		virtual bool CanHandleEvent(RE::InputEvent* event, Argument* list) const = 0;

		//Something about parameters, but unsure how to cover that bit.
		//virtual std::pair<uint32_t, uint32_t
	};

	struct InputTriggerBase : public ITrigger
	{
		ControlID GetControl(Argument*) const
		{
			return 0;
		}
	};

	struct ControlTriggerBase : public ITrigger
	{
		Input GetInput(Argument*) const override
		{
			return Input::CONTROL;
		}
	};

}