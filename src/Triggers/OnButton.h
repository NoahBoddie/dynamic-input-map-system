#pragma once

#include "IHoldTrigger.h"

//*src
#include "ActiveData.h"
#include "VirtualEvent.h"

namespace DIMS
{
	struct Argument;
	struct Parameter;

	struct OnButton : public IHoldTrigger
	{
		//All trigger classes will have these big old parameter constants. While the names of these parameters are found
		static constexpr auto BUTTON_ID = 0;
		static constexpr auto DEVICE_ID = 1;
		static constexpr auto CONTEXT_ID = 2;


		constexpr static Parameter PARAMETERS[]
		{
			Parameter{"Button", ParameterType::Input},
			Parameter{"Device", ParameterType::Enum, RE::InputDevice::kNone },
			Parameter{"Context", ParameterType::Input, RE::InputContextID::kNone },

		};


		size_t GetInputMax() const override { return 10; }

		TriggerType GetTriggerType() const override { return TriggerType::OnButton; }

		EventStage GetEventFilter() const override { return EventStage::All; }

		//This is currently unused, so it should throw for now.

		Input GetInput(const Argument* list) const override;
		
		DynamicInput GetDynamicInput(const Argument* list) const override;

		std::span<const Parameter> GetParameters() const override { return PARAMETERS; }

		virtual bool CanHandleEvent(RE::InputEvent* event, Argument* list) const
		{
			return event->eventType == RE::INPUT_EVENT_TYPE::kButton || event->eventType == VirtualEvent::EVENT_TYPE;
		}


	};

}