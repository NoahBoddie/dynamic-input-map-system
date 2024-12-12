#pragma once


#include "Input.h"

#include "EventData.h"


namespace DIMS
{

	struct Argument;
	struct Parameter;


	struct IAction
	{
		virtual ~IAction() = default;


		//This doesn't seem terribly needed.
		virtual ActionType GetActionType() const = 0;


		virtual std::span<Parameter> GetParameters() const;

		virtual bool Execute(EventData data, EventFlag& flags, const Argument* list) const = 0;

	};
}