#pragma once

namespace DIMS
{

	struct Var
	{
		union
		{
			uint64_t raw{};

		};


		ParameterType type = ParameterType::None;

		/*
		ArgumentVar(const ArgumentVar& other)
		{
			switch (type)
			{

			}
		}

		ArgumentVar(ArgumentVar&& other)
		{
			switch (type)
			{

			}
		}

		ArgumentVar(const ArgumentVar& other)
		{
			switch (type)
			{

			}
		}

		ArgumentVar(ArgumentVar&& other)
		{
			switch (type)
			{

			}
		}

		~ArgumentVar()
		{
			switch (type)
			{

			}
		}
		//*/
	};

}