#pragma once


#include "Input.h"

#include "Parameter.h"

#include "EventData.h"

#include "Impl/IComponent.h"

namespace DIMS
{

	struct Argument;
	struct Parameter;


	struct IAction : public IComponent
	{
		virtual ~IAction() = default;

		//This doesn't seem terribly needed.
		virtual ActionType GetActionType() const = 0;

		virtual EventStage GetRequiredStages() const { return EventStage::None; }

		virtual std::span<const Parameter> GetParameters() const = 0;


		virtual bool Execute(EventData data, EventFlag& flags, const Argument* list) const = 0;


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
	};
}