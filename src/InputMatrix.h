#pragma once

#include "Input.h"
#include "CommandEntry.h"

namespace DIMS
{
	struct DIMSFile;

	//Move to type aliases maybe?
	using CommandMapOld = std::unordered_map<Input::Hash, std::vector<CommandEntryPtr>>;

	struct CommandMap : std::unordered_map<Input::Hash, std::vector<CommandEntryPtr>>
	{
		using base = std::unordered_map<Input::Hash, std::vector<CommandEntryPtr>>;

		using base::base;
		using base::operator=;

		//*
		CommandMap(std::vector<InputCommand*> commands)
		{
			auto& self = *this;
			for (auto command : commands)
			{
				for (auto& trigger : command->triggers)
				{
					CommandEntryPtr entry = std::make_shared<CommandEntry>(command, &trigger, nullptr);

					for (auto input : trigger.GetInputs())
					{
						auto& list = self[input];

						list.insert(std::upper_bound(list.begin(), list.end(), entry, [](const std::shared_ptr<CommandEntry>& lhs, const std::shared_ptr<CommandEntry>& rhs)
							{return lhs->priority() > rhs->priority(); }),
							entry);
					}
				}
			}
		}
		//*/
	};


	struct InputMatrix
	{
		//Thinking of respliting this back into matrix so I can give it to matrix controller
		virtual ~InputMatrix() = default;

		//InputMatrix(MatrixType t) : type{ t } {}

		std::set<InputCommand*> addedCommands;
		
		CommandMap storage;

		std::string name;


		ControlType controlType = ControlType::Gameplay;
		//MatrixType type = MatrixType::Total;




	public:

		template <std::derived_from<InputMatrix> Matrix>
		Matrix* Cast(MatrixType type)
		{
			if (FetchMatrixType() == type) {
				return static_cast<Matrix*>(this);
			}

			return nullptr;
		}

		template <std::derived_from<InputMatrix> Matrix>
		const Matrix* Cast(MatrixType type) const
		{
			if (FetchMatrixType() == type) {
				return static_cast<const Matrix*>(this);
			}

			return nullptr;
		}



		void AddCommand(InputCommand* command)
		{
			if (addedCommands.contains(command) == false)
			{
				for (auto& trigger : command->triggers)
				{
					CommandEntryPtr entry = std::make_shared<CommandEntry>(command, &trigger, this);

					for (auto input : trigger.GetInputs())
					{
						auto& list = storage[input];

						list.insert(std::upper_bound(list.begin(), list.end(), entry, [](const std::shared_ptr<CommandEntry>& lhs, const std::shared_ptr<CommandEntry>& rhs)
							{return lhs->priority() > rhs->priority(); }),
							entry);
					}
				}

				addedCommands.emplace(command);
			}
		}

		void AddCommand(InputCommand& command)
		{
			return AddCommand(&command);
		}




		virtual MatrixType GetMatrixType() const = 0;
		//{
		//	return type;
		//}
		virtual ControlType GetControlType() const //= 0;
		{
			return controlType;
		}

		bool IsMenu() const
		{
			return GetControlType();
		}

		MatrixType FetchMatrixType() const
		{
			return this ? GetMatrixType() : MatrixType::Dynamic;
		}


		virtual bool CanInputPass(RE::InputEvent* event) const
		{
			return true;
		}

		virtual std::strong_ordering CompareOrder(const InputMatrix* other) const
		{
			return std::strong_ordering::equal;
		}

	};
}