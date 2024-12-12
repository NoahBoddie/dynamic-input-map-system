#pragma once

#include "InputInterface.h"

namespace DIMS
{

	struct ActionData
	{
		//ActiveInput::Data* inputData;
		void* inputData;
		InputInterface event;
		EventStage stage;
		ActionFlag flags{};
		//Need an unmoving flag here.

		ActionData(void* data, InputInterface evt, EventStage stg) : inputData{ data }, event{ evt }, stage{ stg }
		{

		}
	};

	struct EventData : public ActionData
	{
		MatrixController* controller = nullptr;

		EventData(MatrixController* ctrl, void* data, InputInterface evt, EventStage stg) : controller{ ctrl }, ActionData { data, evt, stg }
		{

		}
	};


}