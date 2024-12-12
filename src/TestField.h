#pragma once

#include "Enums.h"
#include "Parameter.h"
#include "Argument.h"

#include "TriggerNode.h"
#include "ActiveCommand.h"
#include "InputCommand.h"

#include "InputInterface.h"

#include "CommandEntry.h"

namespace DIMS
{
	//Lookup types
	RE::PlayerControls;
	RE::ButtonEvent;
	RE::MouseMoveEvent;
	RE::ThumbstickEvent;
	RE::ControlMap;
	RE::InputEvent;
	RE::PlayerControls;
	RE::BSInputDeviceManager;
	RE::CharEvent;
	RE::FormID;
	//May make this an actual class with the ability to restore what's stored.

















	






	enum struct ControlType : uint16_t //This needs to be at MOST 16 bytes. It can be less.
	{
		//This controls which matrix controller is being considered, the game or the menu
		Game,
		Menu,

		Total
	};


	struct IMatrix
	{
		//Consult below.
	};

	enum struct MatrixType
	{
		//Outdated.

		Default,		//The default state. Default configurations attach to this, rather than the default made config
		Custom,			//A matrix that is selected from the controls menu. Should be serialized outside of control map.
		State,			//The a state matrix that layers over default/custom ones, but over
		Mode,
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

	};
	

	namespace
	{
		std::unordered_map<Input::Hash, std::vector<std::string>> tmp_Controls;

		std::vector<std::string>* GetControls(Input input)
		{
			//This shit kinda temp ngl
			return nullptr;
		}
	}

	void TestComp()
	{
		DIMS::Input key1{ 1 };
		DIMS::Input key2;

		//std::unordered_map<Key, std::string> mapers;

		//mapers[key1] = "";
		//std::array<int
		key1 != key2;
	}


	namespace detail
	{
		using VisitorList = std::vector<std::reference_wrapper<std::vector<std::shared_ptr<CommandEntry>>>>;
		using VisitorFunc = std::variant<std::function<void(CommandEntry*, bool&)>, std::function<void(CommandEntryPtr, bool&)>>;
	}

	//This what hell looks like.
	void VisitLists(detail::VisitorFunc func, detail::VisitorList& a_lists)
	{
		using ListIterator = std::vector<std::shared_ptr<CommandEntry>>::iterator;

		using PairLIT = std::pair<ListIterator, ListIterator>;
		//This should compound into another static list function, this one should be inline.

		//The core idea behind this is basically that it will record and mind the priorities of each, treating them as if they are in the same list
		// even if they aren't.


		//std::vector lists{ std::ref(a_lists)... };

		//std::array it_list{ std::make_pair(a_lists.begin(), a_lists.end())... };
		//std::array end_list{ std::make_pair(a_lists.end(), a_lists.end())... };
		auto size = a_lists.size();
		if (!size)
		{
			return;
		}


		std::vector<PairLIT> it_list{ a_lists.size() };

		std::vector<PairLIT> end_list{ a_lists.size() };


		for (int i = 0;  i < a_lists.size(); i++)
		{
			auto& list = a_lists[i];

			auto begin = list.get().begin();
			auto end = list.get().end();

			it_list[i] = std::make_pair(begin, end);
			end_list[i] = std::make_pair(end, end);
		}



		bool should_continue = true;

		while (should_continue && it_list != end_list)
		{

			auto& iterator = std::max_element(it_list.begin(), it_list.end(), [](auto&& lhs, auto&& rhs)
				{
					if (rhs.first == rhs.second)
						return false;

					if (lhs.first == lhs.second)
						return true;

					return (*lhs.first)->priority() < (*rhs.first)->priority();
				})->first;

			//I'll likely want to keep this alive
			std::shared_ptr<CommandEntry> entry = *(iterator++);


			if (auto index = func.index(); index == 0)
				std::get<0>(func)(entry.get(), should_continue);

			else if (index == 1)
				std::get<1>(func)(entry, should_continue);

			else
				throw std::exception("index not found?");

			if (!should_continue)
				break;
		
		}
	}


	//Make this a non-template function with a template version. (Func can be changed to std::function that takes lambda that uses the func)
	template <typename Func, std::same_as<std::vector<CommandEntryPtr>>... List>
	void VisitListsTMPL(Func func, const List&... a_lists)
	{
		using TList = std::vector<std::shared_ptr<CommandEntry>>;

		//This should compound into another static list function, this one should be inline.
		
		//The core idea behind this is basically that it will record and mind the priorities of each, treating them as if they are in the same list
		// even if they aren't.


		//std::vector lists{ std::ref(a_lists)... };

		std::array it_list{ std::make_pair(a_lists.begin(), a_lists.end())... };
		std::array end_list{ std::make_pair(a_lists.end(), a_lists.end())... };

		using TEMP1 = decltype(it_list);
		using TEMP = TEMP1::value_type;//??? WHY DOES IT NEED TO WORK LIKE THIS?

		while (it_list != end_list)
		{

			auto iterator = std::max_element(it_list.begin(), it_list.end(), [](auto&& lhs, auto&& rhs)
			{
				if (rhs.first == rhs.second)
					return false;

				if (lhs.first == lhs.second)
					return true;

				return (*lhs.first)->priority() < (*rhs.first)->priority();
			})->first;

			//I'll likely want to keep this alive
			std::shared_ptr<CommandEntry> entry = *(++iterator);



			using Result = std::invoke_result_t<Func, CommandEntry*>;

			if constexpr (std::is_same_v<Result, void>)
			{
				func(entry.get());
			}
			else
			{
				bool go_on = func(entry.get());

				if (!go_on)
					break;


			}
		}
	}


	//Specific polymorphic types like this likely will not exist.
	//struct StateMatrix : public InputMatrix {};
	//using tmp_Key = uint64_t;
	using CommandMap = std::unordered_map<Input::Hash, std::vector<std::shared_ptr<CommandEntry>>>;

	struct MatrixMap
	{
		InputMatrix* matrix = nullptr;
		CommandMap commands;
		
		//ActiveCommands, and wait lists should hold onto this shared pointer when executing data. This way, it prevents the entry
		// from dying before being unhandled.

	};

	struct DelayedCommand
	{
		//The purpose of a waiter is for an object who has fulfilled their inputs, but other objects are also waiting on further inputs which may
		// invalidate that waiter.

		//It's important to note this primarily will only occur from the result of a successful blocking obstructing command.

		std::shared_ptr<CommandEntry> cause;

		//This is the successful obstructing command. It will be resolved if this waiting function doesn't succeed, and if 
		std::shared_ptr<CommandEntry> queued;
		//The above can actually use ActiveCommands instead, since they'll be tentatives.
	};



	struct FakeThumbstick
	{
		static constexpr auto VTABLE = RE::VTABLE_ThumbstickEvent;
	};


	struct FakeMouseMove
	{
		static constexpr auto VTABLE = RE::VTABLE_MouseMoveEvent;
	};

	static RE::ThumbstickEvent* CreateThumb(const RE::BSFixedString& a_userEvent, RE::ThumbstickEvent::InputType a_idCode, float a_x, float a_y)
	{


		auto result = RE::malloc<RE::ThumbstickEvent>(sizeof(RE::ThumbstickEvent));
		std::memset(reinterpret_cast<void*>(result), 0, sizeof(RE::ThumbstickEvent));
		if (result) {
			
			stl::emplace_vtable<FakeThumbstick>(reinterpret_cast<FakeThumbstick*>(result));
			result->device = RE::INPUT_DEVICE::kGamepad;
			result->eventType = RE::INPUT_EVENT_TYPE::kThumbstick;
			result->next = nullptr;
			result->userEvent = a_userEvent;
			result->idCode = (uint32_t)a_idCode;
			result->xValue = a_x;
			result->yValue = a_y;
		}
		return result;
	}


	static RE::MouseMoveEvent* CreateMouse(const RE::BSFixedString& a_userEvent, float a_x, float a_y)
	{
		auto result = RE::malloc<RE::MouseMoveEvent>(sizeof(RE::MouseMoveEvent));
		std::memset(reinterpret_cast<void*>(result), 0, sizeof(RE::MouseMoveEvent));
		if (result) {

			stl::emplace_vtable<FakeMouseMove>(reinterpret_cast<FakeMouseMove*>(result));
			result->device = RE::INPUT_DEVICE::kMouse;
			result->eventType = RE::INPUT_EVENT_TYPE::kMouseMove;
			result->next = nullptr;
			result->userEvent = a_userEvent;
			result->idCode = 0;//This is static but no fucking care honestly
			result->mouseInputX = a_x;
			result->mouseInputY = a_y;
		}
		return result;
	}

	
	void tmp_say_a_nameNEW(EventData&& data, EventFlag& flags, bool& result, const Argument& param1, const Argument& param2)
	{
		auto report = std::format("Action: Name No. {} has been called on. Stage: {}", param1.As<int32_t>(), magic_enum::enum_name(data.stage));
		logger::info("{}", report);
		RE::DebugNotification(report.c_str(), "UIObjectiveNew");

	}
	//
	

	struct ActiveInput
	{
		static constexpr std::array<ActiveCommand::ID, EventStage::Total> emptyBlock{};
		struct Data
		{
			//I want to have this in ActiveCommand. Somehow. Maybe give it some how.
			float secondsHeld = 0;
			uint32_t timestamp = 0;
			uint32_t pressCount = 0;
		} data;

		
		//If preserve isn't 0, then it means something is still using this.
		uint8_t preserve = 0;
		bool isStrong = true;

		std::array<ActiveCommand::ID, EventStage::Total> blockCommands{};


		std::vector<ActiveCommand> sharedCommands;


		//private
		bool EmplaceBlockCommandID(const ActiveCommand& cmd)
		{
			std::vector<ActiveCommand::ID*> adjust_list;

			auto stages = cmd.command->GetBlockingFilter();

			for (int x, y = 0; x < EventStage::Total; x <<= 1, y++)
			{
				if (stages & x)
				{
					auto& block_id = blockCommands[y];

					switch (stages)
					{
					case EventStage::Start:
					case EventStage::Repeating:
					case EventStage::Finish:
						if (!block_id || cmd.id() == block_id)
							goto jump;
						
						return false;


					default:
						break;

					jump:
						adjust_list.push_back(&block_id);
						break;
					}
				}
			}

			for (auto block_id : adjust_list)
			{
				*block_id = cmd.id();
			}

			return true;
		}




		bool EmplaceCommand(CommandEntryPtr cmd, EventStage stage)
		{
			/*
			First try to get command from entry.
			If it fails, this only controls if we make it later or not.

			//*/

			ActiveCommand* act_ptr = GetCommandFromEntry(cmd);


			if (!act_ptr && cmd->GetFirstStage() != stage) {
				//This refuses to use commands that isn't on it's proper stage.
				return false;
			}


			//It looks cursed, I know.
			ActiveCommand act_cmd{ cmd, stage };

			ActiveCommand* command = act_ptr ? GetCommandFromEntry(cmd) : &act_cmd;
			
			if (cmd->ShouldBlockTriggers() && EmplaceBlockCommandID(*command) == false)
				return false;

			if (!act_ptr) {
				sharedCommands.push_back(act_cmd);
				//command = std::addressof(sharedCommands.emplace_back(act_cmd));//This one is in case I need something later
			}
			else {
				command->stages |= stage;
			}

			return true;
		}

		//Feel like something like UpdateCommand would be best here.

		ActiveCommand* GetCommandFromID(ActiveCommand::ID id)
		{
			auto it = sharedCommands.begin();
			auto end = sharedCommands.end();

			it = std::find_if(it, end, [id](ActiveCommand&& entry) {return entry.id() == id; });

			if (it != end)
				return std::addressof(*it);

			return nullptr;
		}
		
		ActiveCommand* GetBlockCommandFromStage(EventStage stage)
		{
			auto inch = std::countr_zero(std::to_underlying(stage));
			auto id = blockCommands[inch];
			return GetCommandFromID(id);
		}


		ActiveCommand* GetCommandFromEntry(const CommandEntryPtr& command)
		{
			auto it = sharedCommands.begin();
			auto end = sharedCommands.end();

			it = std::find_if(it, end, [command](ActiveCommand&& entry) {return entry.command == command; });

			if (it != end)
				return std::addressof(*it);

			return nullptr;
		}

		BlockingState GetBlockingState() const
		{
			return blockCommands == emptyBlock ?
				BlockingState::None : isStrong ?
				BlockingState::Strong : BlockingState::Weak;
		}

		bool IsBlocking() const { return GetBlockingState() != BlockingState::None; }

		bool IsBlockingWeak() const { return GetBlockingState() == BlockingState::Weak; }
		
		bool IsBlockingStrong() const { return GetBlockingState() == BlockingState::Strong; }

		/// <summary>
		/// Checks if a given event stage is blocked.
		/// </summary>
		/// <param name="stage">The given event stage to check. Should be a single flag.</param>
		/// <param name="trig">If only triggers should be checked for blocking.</param>
		/// <returns></returns>
		bool IsStageBlocked(EventStage stage, bool trig)
		{
			if (auto cmd = GetBlockCommandFromStage(stage)) {
				return trig ? cmd->command->ShouldBlockTriggers() : cmd->command->ShouldBlockNative();
			}
			return false;
		}
		
		//To prevent the need to ask if the stage is blocked over and over, I'll delegate it to a hash, that will just represent the entries.
		// If change occurs, we know what's what.
		bool IsStageBlockedHashed(bool& previous_value, size_t& hash, EventStage stage)
		{
			//I don't like the idea of this terribly, but it will have to do.
			// This is effectively always going to be faster than the alternative of finding EVERY time I want to check it.
			std::hash<ActiveCommand::ID> hasher{};
			
			size_t seed = 0;

			for (auto id : blockCommands) {
				seed ^= hasher(id) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
			}

			if (seed != hash) {
				previous_value = IsStageBlocked(stage, true);//This has no need to ask for not trigger. it will already do non-triggers.
				hash = seed;
			}

			return previous_value;
		}
	};

	using ActiveInputPtr = std::unique_ptr<ActiveInput>;

	struct MatrixController
	{
		//0 for no mode currently loaded.
		
		




		//This will be the object that controls all the inputs arrangement. Data such as who's capturing an input and such will be loaded here.
		
		//This is the matrix that one selects to use. Basically 
		InputMatrix* defaultMatrix = nullptr;//This is allowed to be null.

		//If this is null it will pass go.
		MatrixMap selectedMatrix;

		CommandMap dynamicMap;

		CommandMap stateMap;


		std::vector<InputMatrix*> stateMatrices;

		//Used the command entry pointer instead
		std::vector<std::pair<CommandEntryPtr, MatrixMap>> modeMaps;//The last mode is most important.

		MatrixMap* GetMode()
		{
			return modeMaps.size() ? std::addressof(modeMaps.back().second) : nullptr;
		}
		
		std::vector<InputCommand*> updateCommands;

		std::vector<DelayedCommand> delayList;


		

		//TODO: Below
		//The more proper way I wanted to set up command entries is like this, there is the dynamic command entry list, and the sentinel
		// list. the idea being that the sentinels will do updating when need be in order to remove stuff from the command entry list.
		//Not sure about how modes then handle command entries. I think it will probably just make those by hand.

		
		//TODO: I may stop using ActiveCommand::ID and start using a shared pointer, primarily to help tell when a mode is donezo


		//TODO: For active command I really want to stop using the ID idea. I mean, I think I may want to find some way to keep it, but shared
		// pointer also might not exactly be what I'm looking for either. Maybe just one unique pointer, and a bunch of regular pointers.


		//This is for active commands basically. Based on loaded settings these will prevent an event from passing.
		//NOTE, UNLIKE EVERYWHERE ELSE THIS IS NOT ALLOWED TO STORE A CONTROL.
		//In other words, this is the key being pressed, not the control
		std::unordered_map<Input::Hash, ActiveInputPtr> activeMap;

		//After the active map is decremented, any value that has 1 is removed from here, and also any other needed places.
		//May be defunct now
		//std::set<ActiveCommandPtr> activeCommands;


		MatrixController()
		{
			auto conflict = ConflictLevel::Capturing;
			


			InputCommand* command1 = new InputCommand;

			auto& action1 = command1->actions.emplace_back();
			action1.type = ActionType::InvokeFunction;
			auto& args1 = action1.args = std::make_unique<Argument[]>(3);
			args1[InvokeFunction::FUNCTION_PTR] = tmp_say_a_nameNEW;
			args1[InvokeFunction::CUST_PARAM_1] = 1;

			auto& trigger1 = command1->triggers.emplace_back();
			trigger1.priority = 1;
			trigger1.type = TriggerType::OnButton;
			trigger1.conflict = ConflictLevel::Defending;
			trigger1.stageFilter = EventStage::Start;
			trigger1.args.emplace_back(std::make_unique<Argument[]>(1))[OnButton::BUTTON_ID] = Input{ RE::INPUT_DEVICE::kMouse, 0 };
			
			InputCommand* command2 = new InputCommand;

			auto& action2 = command2->actions.emplace_back();
			action2.type = ActionType::InvokeFunction;
			auto& args2 = action2.args = std::make_unique<Argument[]>(3);
			args2[InvokeFunction::FUNCTION_PTR] = tmp_say_a_nameNEW;
			args2[InvokeFunction::CUST_PARAM_1] = 2;

			auto& trigger2 = command2->triggers.emplace_back();
			trigger2.priority = 2;
			trigger2.type = TriggerType::OnButton;
			trigger2.conflict = ConflictLevel::Defending;
			trigger2.stageFilter = EventStage::Start;
			trigger2.args.emplace_back(std::make_unique<Argument[]>(1))[OnButton::BUTTON_ID] = Input{ RE::INPUT_DEVICE::kMouse, 1 };



			InputCommand* command5 = new InputCommand;

			auto& action5 = command5->actions.emplace_back();
			action5.type = ActionType::InvokeFunction;
			auto& args5 = action5.args = std::make_unique<Argument[]>(3);
			args5[InvokeFunction::FUNCTION_PTR] = tmp_say_a_nameNEW;
			args5[InvokeFunction::CUST_PARAM_1] = 35;

			auto& trigger5 = command5->triggers.emplace_back();
			trigger5.priority = 35;
			trigger5.type = TriggerType::OnButton;
			trigger5.conflict = ConflictLevel::Defending;
			trigger5.stageFilter = EventStage::Start;
			trigger5.args.emplace_back(std::make_unique<Argument[]>(1))[OnButton::BUTTON_ID] = Input{ RE::INPUT_DEVICE::kMouse, 0 };

			InputCommand* command6 = new InputCommand;

			auto& action6 = command6->actions.emplace_back();
			action6.type = ActionType::InvokeFunction;
			auto& args6 = action6.args = std::make_unique<Argument[]>(3);
			args6[InvokeFunction::FUNCTION_PTR] = tmp_say_a_nameNEW;
			args6[InvokeFunction::CUST_PARAM_1] = 40;

			auto& trigger6 = command6->triggers.emplace_back();
			trigger6.priority = 40;
			trigger6.type = TriggerType::OnButton;
			trigger6.conflict = ConflictLevel::Defending;
			trigger6.stageFilter = EventStage::Start;
			trigger6.args.emplace_back(std::make_unique<Argument[]>(1))[OnButton::BUTTON_ID] = Input{ RE::INPUT_DEVICE::kMouse, 1 };



			InputCommand* command3 = new InputCommand;

			auto& action3 = command3->actions.emplace_back();
			action3.type = ActionType::InvokeFunction;
			auto& args3 = action3.args = std::make_unique<Argument[]>(3);
			args3[InvokeFunction::FUNCTION_PTR] = tmp_say_a_nameNEW;
			args3[InvokeFunction::CUST_PARAM_1] = 69;

			auto& trigger3 = command3->triggers.emplace_back();
			trigger3.priority = 69;
			trigger3.type = TriggerType::OnControl;
			trigger3.conflict = ConflictLevel::Defending;
			trigger3.stageFilter = EventStage::Start;
			trigger3.args.emplace_back(std::make_unique<Argument[]>(1))[OnControl::CONTROL_ID] = "Right Attack/Block";


			InputCommand* command4 = new InputCommand;


			auto& action4 = command4->actions.emplace_back();
			action4.type = ActionType::InvokeFunction;
			auto& args4 = action4.args = std::make_unique<Argument[]>(3);
			args4[InvokeFunction::FUNCTION_PTR] = tmp_say_a_nameNEW;
			args4[InvokeFunction::CUST_PARAM_1] = 10;

			auto& trigger4 = command4->triggers.emplace_back();
			trigger4.priority = 10;
			trigger4.type = TriggerType::OnControl;
			trigger4.conflict = ConflictLevel::Guarding;
			trigger4.stageFilter = EventStage::StartFinish;
			trigger4.args.emplace_back(std::make_unique<Argument[]>(1))[OnControl::CONTROL_ID] = "Left Attack/Block";
			
			std::vector<InputCommand*> something{ command1, command2, command3, command4, command5, command6 };

		
			for (auto command : something)
			{
				for (auto& trigger : command->triggers)
				{
					auto entry = std::make_shared<CommandEntry>(command, &trigger);

					for (auto input : trigger.GetInputs())
					{

						auto& list = dynamicMap[input];

						list.insert(std::upper_bound(list.begin(), list.end(), entry, [](const std::shared_ptr<CommandEntry>& lhs, const std::shared_ptr<CommandEntry>& rhs)
							{return lhs->priority() > rhs->priority(); }),
							entry);
					}


				}

			}


			//dynamicMap[Input{ RE::INPUT_DEVICE::kMouse, 0 }].emplace;
			//dynamicMap[Input{ RE::INPUT_DEVICE::kMouse, 1 }] = {};
		}


		ActiveInput& ObtainActiveInput(Input input)
		{
			if (input.IsControl() == true)
				throw std::exception("Cannot obtain active input for control");

			auto& ptr = activeMap[input];

			if (!ptr)
				ptr = std::make_unique<ActiveInput>();

			return *ptr;
		}

		bool HandleEvent(InputInterface event, std::unique_ptr<RE::InputEvent>& out)
		{
			//HandleEvent is basically the core driving function. It takes the player controls, and the given input event, as well as an
			// InputEvent that is mutated to be refired.
			// it will return true if it intends to fire the original function, and will do so with the out function if present.
			// if it's true and the out function isn't there, it will fire as normal.

			//I think I'll put the interface object here, along with the event data.

			

			EventStage stage = event.GetEventStage();

			if (stage == EventStage::None)
				return true;



			bool allow_execute = true;

			Input input{ event };


			auto& active_input = ObtainActiveInput(input);

			//So what's the order to do?

			//StatePremode
			//Mode
			//StatePostmode
			//Dynamic


			auto ctrl_entry = dynamicMap.find(Input::CONTROL);
			auto inp_entry = dynamicMap.find(input.hash());

			auto end = dynamicMap.end();

			detail::VisitorList list;

			if (ctrl_entry != end){
				list.push_back(std::ref(ctrl_entry->second));
			}

			if (inp_entry != end) {
				list.push_back(std::ref(inp_entry->second));
			}

			EventFlag flags = EventFlag::None;

			bool blocking = false;
			

			bool block_ = false;

			size_t hash = 0;//starts as zero so it will always need to load the first time.

			//I don't remember why, but I think it was best to do this.
			//std::vector<CommandEntry*> call_list;

			VisitLists([&](CommandEntryPtr entry, bool& should_continue)
			{
				if (entry->CanHandleEvent(event))
				{
					
					bool result = true;


					if (entry->GetTriggerFilter() & stage) {
						//If it has multiple stages, blocks multiple actions 
						if (entry->HasMultipleStages() || entry->ShouldBlockTriggers() == true)
						{
							if (active_input.EmplaceCommand(entry, stage) == false) {
								return;
							}
						}

						if (!active_input.IsStageBlockedHashed(block_, hash, stage) || entry->ShouldBeBlocked() == false) {

							EventData data{ this, nullptr, event, stage };

							result = entry->Execute(data, flags);
						}
					}


					return;

					if (!blocking || entry->ShouldBeBlocked() == false) {
						if (entry->GetTriggerFilter() & stage) {

							EventData data{ this, nullptr, event, stage };

							result = entry->Execute(data, flags);
						}
					}

					//This isn't quite right, but the space just isn't ready for reprisal.
					// The core problem is that the event effectively gets resent after active command gets it's update in.
					if (entry->GetBlockingFilter() & stage)
					{
						if (result)
							allow_execute = false;

						if (entry->ShouldBlockTriggers() == true)
							blocking = true;
					}
				}



			}, list);

			return active_input.IsStageBlocked(stage, false) || (flags & EventFlag::Continue);


			return allow_execute || (flags & EventFlag::Continue);
		}
	};

	inline MatrixController* testController = new MatrixController;
	inline std::array<MatrixController*, (int)ControlType::Total> Controllers;
}