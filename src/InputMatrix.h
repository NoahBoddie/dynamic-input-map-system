#pragma once

#include "Input.h"
#include "CommandEntry.h"
#include "Utility.h"
namespace DIMS
{
	struct DIMSFile;

	//Move to type aliases maybe?
	using CommandMapOld = std::unordered_map<Input::Hash, std::vector<CommandEntryPtr>>;

	

	struct CommandMap : std::unordered_map<Input::Hash, std::vector<CommandEntryPtr>>
	{
		//TODO: I want to stop using this as a proxy for an unordered map

		//The main purpose of this is to take an input event and then sort out what inputs it's supposed to expect.
		// The idea is to have a vector of these things that I can then use to pull a vector of commands from.
		//If I can find a way to store this within the map, that'd be cool.
		
		std::set<DynamicInput> dynamicEntries;
		

		using base = std::unordered_map<Input::Hash, std::vector<CommandEntryPtr>>;

		using base::base;
		using base::operator=;


		std::vector<CommandEntryPtr>* FindCommand(Input::Hash hash)
		{

			auto it = find(hash);
			

			auto _end = end();

			//change these to AttemptToPull functions.
			if (it != _end) {
				return &it->second;
			}

			return nullptr;
		}

		//Sort is completely more effective than this method. Use that instead. If I can, I'll try to emplace it. But I think just sorting it
		// is preferable.

		void GetDynamicCommands(InputInterface event, std::vector<CommandEntryPtr>& out, std::set<Input>& used)
		{
			auto& self = *this;

			RE::InputContextID curr_context = GetCurrentInputContext();
			
			auto cmap = RE::ControlMap::GetSingleton();
			
			uint16_t id = event.GetInputID();

			for (auto& entry : dynamicEntries)
			{
				//Checks only need to happen if the name is different, if it's the same we can just forcibly run it.
				auto list = FindCommand(entry.key.hash());
				
				if (list) {

					if (event->QUserEvent() != entry.name)
					{
						auto device = entry.device == RE::InputDevice::kNone ? *event->device : entry.device;

						auto context = entry.context == RE::InputContextID::kNone ? curr_context : entry.context;

						if (entry.device != device || entry.context != context)
							continue;

						auto key = cmap->GetMappedKey(entry.name, device, context);

						if (id != key) {
							continue;
						}
					}

					out.insert_range(out.begin(), *list);
					used.emplace(entry.key);
				}
			}
		}

		void OtherGetCommands(InputInterface event, std::vector<CommandEntryPtr>& out, std::set<Input>& used)
		{
			auto end = this->end();

			{
				Input input = Input::CreateInput(event);

				auto list = FindCommand(input.hash());

				if (list) {
					out.append_range(*list);
					used.emplace(input);

				}
			}


			for (auto& user_event : GetUserEvents(event.GetInputID(), GetCurrentInputContext(), *event->device))
			{
				Input ctrl = Input::CreateUserEvent(user_event);

				auto list = FindCommand(ctrl.hash());

				if (list) {
					out.append_range(*list);
					used.emplace(ctrl);

				}
			}

			if (Input ctrl = Input::CreateUserEvent(event->QUserEvent()); used.contains(ctrl) == false) {
				auto list = FindCommand(ctrl.hash());

				if (list) {
					out.append_range(*list);
					used.emplace(ctrl);

				}
			}

			//if (event->GetEventType() == RE::InputEventType::kButton)
			//	GetDynamicCommands(event, out, used);
		}


		auto GetCommands(RE::InputEvent* event, std::set<Input>& used)
		{
			std::vector<std::reference_wrapper<std::vector<CommandEntryPtr>>> result;

			Input input = Input::CreateInput(event);
			Input control = Input::CreateUserEvent(event);


			auto ctrl_it = find(control.hash());
			auto input_it = find(input.hash());

			auto _end = end();

			//change these to AttemptToPull functions.
			if (ctrl_it != _end) {
				result.push_back(std::ref(ctrl_it->second));
				used.emplace(input);
			}

			if (input_it != _end) {
				result.push_back(std::ref(input_it->second));
				used.emplace(control);
			}

			return result;
		}

		void AddCommand(InputCommand* command, InputMatrix* parent = nullptr)
		{
			auto& self = *this;

			for (auto& trigger : command->triggers)
			{
				CommandEntryPtr entry = std::make_shared<CommandEntry>(command, &trigger, parent);

				for (auto var : trigger.GetInputs())
				{
					Input input;

					switch (var.index())
					{
					case 0://Regular Input
					{
						input = std::get<Input>(var);
						break;
					}

					case 1://Dynamic Input
					{
						DynamicInput din = std::get<DynamicInput>(var);
						dynamicEntries.emplace(din);
						input = din.key;
						break;
					}
					default: 
						continue;
					}

					auto& list = self[input];

					//Do we actual need to sort here? We'll have to sort later maybe, so perhaps we just do it then.
					list.insert(std::upper_bound(list.begin(), list.end(), entry,
						[](const std::shared_ptr<CommandEntry>& lhs, const std::shared_ptr<CommandEntry>& rhs)
						{return lhs->priority() > rhs->priority(); }),
						entry);

				}
			}

		}


		//*
		CommandMap(std::vector<InputCommand*> commands, InputMatrix* parent = nullptr)
		{
			auto& self = *this;

			for (auto command : commands)
			{
				AddCommand(command, parent);
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
				storage.AddCommand(command, this);

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