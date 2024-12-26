#include "InvokeMode.h"

#include "TestField.h"

namespace DIMS
{
	bool InvokeMode::Execute(EventData data, EventFlag& flags, const Argument* list) const
	{
		auto mode = list[MODE_PTR].As<LayerMatrix*>();

		bool strong = true;

		switch (data.stage)
		{
		case EventStage::Preface:
			strong = false;
			[[fallthrough]];
		case EventStage::Start:
			RE::DebugNotification("Activating mode");
			data.controller->EmplaceMode(mode, data.command, strong);
			break;

		case EventStage::Repeating:
			break;

		case EventStage::Finish:
			RE::DebugNotification("Deactivating mode");
			data.controller->RemoveMode(mode, data.command);
			break;
		}

		return true;
	}
}