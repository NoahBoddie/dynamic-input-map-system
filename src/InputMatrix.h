#pragma once

#include "Input.h"
#include "CommandEntry.h"

namespace DIMS
{


	//Move to type aliases maybe?
	using CommandMap = std::unordered_map<Input::Hash, std::vector<std::shared_ptr<CommandEntry>>>;


	struct IMatrix
	{
		virtual ~IMatrix() = default;
		//Consult below.

		//When invoked, only commands with the same type of matrix type will be allowed used.
		virtual bool CanInputPass(RE::InputEvent* event) const
		{
			return true;
		}
	};


	struct InputMatrix : public IMatrix
	{
		//The input matrix will likely be the thing that basically houses all the nodes associated with a thing. So for example a mode's nodes,
		// a default set up's nodes, a null set ups nodes (IE the very most basic node that simply passes on.

		//This is also the thing that possible handles state nodes as well, to which I'm not entirely sure those will combine.
		//I'm thinking perhaps with states, there's an input matrix built into the Matrix controller that develops it as time goes on.


		//Note, the default config (A completely empty configuration) will always have to exist.


		//This is a dummy value, but in the setting configs, ownership is basically saying "does it belong to a mode/state/matrix named this"
		// followed by which one of these someone is using. Having no ownership will assign to whatever the loaded default is.
		//std::string ownership;

		std::vector<InputCommand> commands;//Once this is finalized, this cannot have it's values changed. so it should be private.

		virtual MatrixType GetMatrixType() const { return MatrixType::Total; }

		virtual std::strong_ordering CompareOrder(const InputMatrix* other) const
		{
			return std::strong_ordering::equal;
		}

		MatrixType FetchMatrixType() const
		{
			return this ? GetMatrixType() : MatrixType::Dynamic;
		}

		CommandMap CreateCommandMap()
		{
			CommandMap map;

			for (auto& command : commands)
			{
				for (auto& trigger : command.triggers)
				{
					CommandEntryPtr entry = std::make_shared<CommandEntry>(&command, &trigger);

					for (auto input : trigger.GetInputs())
					{
						auto& list = map[input];

						list.insert(std::upper_bound(list.begin(), list.end(), entry, [](const std::shared_ptr<CommandEntry>& lhs, const std::shared_ptr<CommandEntry>& rhs)
							{return lhs->priority() > rhs->priority(); }),
							entry);
					}
				}
			}

			return map;
		}

	};

}