//The point of IComponent will be for types like Trigger or Action that will need to possibly destroy data such as
// parameters or arguments. Basically, it will be something that allows it to unhandle some element close to it.
// A list of parameters or a list of arguments are the 2 that will be addressed as of right now.

#pragma once

namespace DIMS
{
	struct Parameter;
	struct Argument;

	struct IComponent
	{
		virtual ~IComponent() = default;

		virtual void UnhandleArguments(Argument* list) const = 0;
		virtual void UnhandleParameters(Parameter* list) const = 0;
	};
}