#pragma once

#include "InputInterface.h"

namespace DIMS
{

	class MatrixController;
	struct ActiveData;

	struct ActionData
	{
		//ActiveInput::Data* inputData;
		ActiveData* inputData;
		InputInterface event;
		EventStage stage;
		ActionFlag flags{};
		//Need an unmoving flag here.

		ActionData(ActiveData* data, InputInterface evt, EventStage stg) : inputData{ data }, event{ evt }, stage{ stg }
		{

		}
	};

	struct EventData : public ActionData
	{
		MatrixController* controller = nullptr;

		EventData(MatrixController* ctrl, ActiveData* data, InputInterface evt, EventStage stg) : controller{ ctrl }, ActionData { data, evt, stg }
		{

		}
	};


}