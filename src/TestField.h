#pragma once

#include "Enums.h"
#include "Parameter.h"
#include "Argument.h"

#include "TriggerNode.h"
#include "ActiveCommand.h"
#include "InputCommand.h"

#include "InputInterface.h"

#include "CommandEntry.h"

#include "ActiveData.h"

#include "Utility.h"

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

	void tmp_YELL_a_nameNEW(EventData&& data, EventFlag& flags, bool& result, const Argument& param1, const Argument& param2)
	{
		auto report = std::format("Action: Name No. {} has been called on. Stage: {}", param1.As<int32_t>(), magic_enum::enum_name(data.stage));
		logger::info("{}", report);
		RE::DebugMessageBox(report.c_str());

	}
	//
	
	struct DelayedCommand
	{
		std::vector<CommandEntry> queue;
		EventStage stage;
	};



	struct ActiveInput
	{
		using Data = ActiveData;

		//Move this, not really needed at all times is it?
		static constexpr std::array<ActiveCommand::ID, EventStage::Total> emptyBlock{};

		Data data;

		
		//If preserve isn't 0, then it means something is still using this.
		uint8_t preserve = 0;
		bool isStrong = true;

		bool _init = false;//This can be turned back off do note.

		std::array<ActiveCommand::ID, EventStage::Total> blockCommands{};
		

		std::vector<ActiveCommand> sharedCommands;

		//At a later point, make this a pointer. Not all commands would ever need this.
		// I have no idea if I may use a ref later.
		//std::unordered_map<ActiveCommand::ID, uint16_t> delayCommands;
		//If I could make some sort of struct that default creates a targeted object when accessed, that would be cool.
		std::set<ActiveCommand::ID> delayCommands;
		
		void Initialize(InputNumber v1, InputNumber v2)
		{
			_init = true;

			data.value1 = v1;
			data.value2 = v2;
			
			data.timestamp = RE::GetDurationOfApplicationRunTime();
		}

		bool IsInitialized()
		{
			return _init;
		}



		auto& ObtainDelayCommands()
		{
			return delayCommands;
		}


		bool HasDelayedCommands() const
		{
			//combine once delaycommands is a pointer.
			//if (!delayCommands)
			//	return false;

			if (delayCommands.size() == 0)
				return false;

			return true;
		}

		//Recieves a command and returns the reference to the address of the emplaced entry
		ActiveCommand& AddCommand(ActiveCommand& command)
		{

			if (HasDelayedCommands() == true) {
				for (auto id : ObtainDelayCommands())
				{
					auto delayed = GetCommandFromID(id);

					if (!delayed) {
						continue;//This shouldn't happen. But checks and all.
					}




					if (delayed->entry->tmpname_ShouldWaitOnMe(*command.entry) == true) {
						command.tempname_IncWaiters();
					}
				}
			}


			command.Activate();

			auto& result = sharedCommands.emplace_back(std::move(command));


			if (result->IsDelayable() == true) {
				//TODO: This may have issues suppressing other non-delayed commands. I think should be added regardless if it's actually running or not, ...
				// otherwise it may not suppress events it my normally depending on the order of inputs
				EmplaceDelayCommand(result);
			}

			return result;
		}

		void EmplaceDelayCommand(const ActiveCommand& act)
		{
			auto& commands = ObtainDelayCommands();

			if (commands.emplace(act.id()).second) {
				
				//This should only be trigger if the command says the input should wait too.
				// For now, this means nothing.
				preserve++;

				for (auto& other : sharedCommands)
				{
					if (&other == &act)
						continue;

					if (act.entry->tmpname_ShouldWaitOnMe(*other.entry) == true) {
						other.tempname_IncWaiters();

						//i'm just gonna do this shit manually for now ok? Im sleepy and hungry
						//TODO: Make this a dedicated function
						if (auto& blocker = blockCommands[0]; other.id() == blocker)
							blocker = 0;
						if (auto& blocker = blockCommands[1]; other.id() == blocker)
							blocker = 0;
						if (auto& blocker = blockCommands[2]; other.id() == blocker)
							blocker = 0;
					}



				}
			}
			
			//commands.insert_range(commands.end(), entries);
		}


		void FailDelayCommand(ActiveCommand& act, EventStage stage)
		{
			//To do this here I need all the information that one has firing stuff normally.

			auto& commands = ObtainDelayCommands();

			if (commands.erase(act.id()) != 0)
			{
				bool found_self = false;

				preserve--;

				for (auto& other : sharedCommands)
				{
					if (&other == &act)
						continue;

					if (act.entry->tmpname_ShouldWaitOnMe(*other.entry) == true) {

						if (other.tempname_DecWaiters() == 0) {
							//Here we refire all action related data.
							//Here's another thought though, build playing catch up into the execution.

							//Don't control what gets to go off by whether it has
							ActiveCommand::ID dump;

							EmplaceCommand(other.entry, stage, dump, &other);
						}


					}
				}

				//A good check is if I actually find this within the thing.
				//assert(found_self);

				act.SetDelayUndone();
			}

		}


		//TODO: Need a function to fail a command, which clears out the waiters.

		//private
		bool EmplaceBlockCommandID(const ActiveCommand& cmd)
		{
			if (cmd.entry->ShouldBlockTriggers() == false)
				return true;

			std::vector<ActiveCommand::ID*> adjust_list;

			auto stages = cmd.entry->GetBlockingFilter();

			for (int x = 1, y = 0; x < EventStage::Total; (x <<= 1), y++)
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



		//Returns the active id so one can check if they are the blocking one.
		bool EmplaceCommand(CommandEntryPtr cmd, EventStage stage, ActiveCommand::ID& id, ActiveCommand* act_ptr = nullptr)
		{//TODO: Make a different version of emplace function. I don't want this option exposed.
			if (!act_ptr)
				act_ptr = GetCommandFromEntry(cmd);

			auto block_id = GetIDFromStage(stage);

			if (!act_ptr) {
				if (cmd->GetFirstStage() < stage || block_id) {
					//This refuses to use commands that isn't on it's proper stage.
					return false;
				}
			}
			else 
			{

				if (stage == EventStage::Finish) {
					auto t_stage = act_ptr->entry->GetTriggerFilter();

					//If it doesn't have a finish stage and currently isn't running.
					if (t_stage & ~(t_stage ^ EventStage::Finish) && act_ptr->IsRunning() == false)
						act_ptr->Deactivate();
				}
				if (auto failure = act_ptr->IsFailing(); !failure || !act_ptr->IsDelayUndone())
				{
					//Here is where I want to do failure checks. To which, it should have a force update on all stuff that was waiting.
					auto delay_state = !failure ?
						act_ptr->entry->GetDelayState(nullptr, data) : act_ptr->entry->IsDelayable() ?
						DelayState::Failure : DelayState::None;

					if (delay_state == DelayState::Failure) {
						//signify some failure.
						FailDelayCommand(*act_ptr, stage);
					}
				}


				
			}


		
			ActiveCommand act_cmd{ cmd, stage };

			ActiveCommand* command = act_ptr ? act_ptr : &act_cmd;
			
			if (cmd->ShouldBlockTriggers() && EmplaceBlockCommandID(*command) == false)
				return 0;

			if (!act_ptr) {
				command = &AddCommand(act_cmd);
			}
			else if (command->entry->GetTriggerFilter() & stage) {
				command->stages |= stage;
			}

			id = act_cmd.id();

			if (command->IsRunning() == true) {
				return true;
			}

			//This needs to be done in the handle function
			//ObtainDelayedCommands().emplace(command->id());
			
			return false;
		}


		//Feel like something like UpdateCommand would be best here.

		ActiveCommand* GetCommandFromID(ActiveCommand::ID id)
		{
			if (id) {
				auto it = sharedCommands.begin();
				auto end = sharedCommands.end();

				it = std::find_if(it, end, [id](ActiveCommand& entry) {return entry.id() == id; });

				if (it != end)
					return std::addressof(*it);
			}
			
			return nullptr;
		}
		
		inline ActiveCommand::ID GetIDFromStage(EventStage stage) const
		{
			auto inch = std::countr_zero(std::to_underlying(stage));
			return blockCommands[inch];
		}

		ActiveCommand* GetBlockCommandFromStage(EventStage stage)
		{
			auto id = GetIDFromStage(stage);
			return GetCommandFromID(id);
		}


		ActiveCommand* GetCommandFromEntry(const CommandEntryPtr& command)
		{
			auto it = sharedCommands.begin();
			auto end = sharedCommands.end();

			it = std::find_if(it, end, [command](ActiveCommand& entry) {return entry.entry == command; });

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
			if (trig) {
				//For regular trig blocking you need to visit all active commands
				if (auto cmd = GetBlockCommandFromStage(stage)) {
					return cmd->entry->ShouldBlockTriggers();
				}
			}
			else {
				for (auto& act : sharedCommands) {
					//This doesn't check for stages because if it blocks once it blocks every one after
					if (act.entry->ShouldBlockNative() == true)
						return true;
				}

			}
			return false;
		}
		

		bool IsCommandBlockingStage(ActiveCommand::ID id, EventStage stage)
		{
			if (!stage || !id)
				return false;

			auto inch = std::countr_zero(std::to_underlying(stage));
			return id == blockCommands[inch];
		}

		//To prevent the need to ask if the stage is blocked over and over, I'll delegate it to a hash, that will just represent the entries.
		// If change occurs, we know what's what.
		bool IsStageBlockedHashed(bool& previous_value, size_t& hash, ActiveCommand::ID id, EventStage stage)
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
			bool result = previous_value;

			if (result) {
				//While this will not change whether it's generally blocked, it will change if this one query returns true or false
				result = !IsCommandBlockingStage(id, stage);
			}

			return result;
		}

		//Tries to release the delayed command list. Return true if delayed commands no longer exists.
		bool TryReleaseDelayed()
		{
			//Unsure if I want to do this, mainly because
			if (HasDelayedCommands() == false)
				return true;

			if (delayCommands.size() != 0)
				return false;

			return false;
		}


		//returns false when it no longer has any commands to be updated.
		bool UpdateDelayed()
		{
			if (HasDelayedCommands() == false)
				return false;

			auto& commands = ObtainDelayCommands();

			for (auto it = commands.begin(); it != commands.end(); ) {
				if (auto command = GetCommandFromID(*it); !command || command->IsRunning() == true) {
					it = commands.erase(it);
				}
				else {
					++it;
				}
			}
			//I would desperately like to have delete the entry, but I figure that's basically just having it flicker isn't it?
			// if another delay gets added I have to make it, and if one doesn't it gets deleted anyways.
			return commands.size();
		}
		[[deprecated("ActiveCommand is running handles this currently")]]
		bool CanRunCommand(const ActiveCommand& command) const
		{
			//Later when I'm feeling like having a brain, this needs to check the queue list.
			return command.IsRunning();
		}


		std::vector<ActiveCommand*> GetActiveCommands()
		{
			std::vector<ActiveCommand*> result{};
			result.reserve(sharedCommands.size());

			for (auto& command : sharedCommands) {
				if (command.IsRunning() == true) {
					result.push_back(&command);
				}
			}

			return result;
		}

		void VisitActiveCommands(std::function<void(ActiveCommand&)> func)
		{
			for (auto& command : sharedCommands) {
				if (command.IsRunning() == true) {
					func(command);
				}
			}
		}


	};

	using ActiveInputPtr = std::unique_ptr<ActiveInput>;

	struct ActiveInputHandle
	{
		using ActiveInputMap = std::unordered_map<Input::Hash, ActiveInputPtr>;
		using IValuePair = std::pair<InputNumber, InputNumber>;

		ActiveInputMap& map;

		Input::Hash hash;

		ActiveInput* input = nullptr;

		IValuePair iValues;
		
		ActiveInput* ObtainActiveInput()
		{
			if (!input)
			{
				auto& ptr = map[hash];

				if (!ptr) {
					ptr = std::make_unique<ActiveInput>();
					ptr->Initialize(iValues.first, iValues.second);
				}

				input = ptr.get();
			}

			return input;
		}

		ActiveInput* operator->()
		{
			return ObtainActiveInput();
		}

		operator bool() const
		{
			return input;
		}

		ActiveInputHandle(Input h, ActiveInputMap& m, IValuePair i) : hash{ h }, map{ m }, iValues{ i }
		{
			assert(!h.IsControl());

			if (h.IsControl() == true)
				throw std::exception("Cannot obtain active input for control");

		}

	};


	class MatrixController
	{
	public:

		//0 for no mode currently loaded.
		
		




		//This will be the object that controls all the inputs arrangement. Data such as who's capturing an input and such will be loaded here.
		
		//This is the matrix that one selects to use. Basically 
		InputMatrix* defaultMatrix = nullptr;//This is allowed to be null.

		//If this is null it will pass go.
		MatrixMap selectedMatrix;


		//Having an extra map here that basically serves as the pending release map might be useful.
		// That, or having a given active input dump all of it's command inputs. I think this actually might be a good idea,
		// just preventing things already in the set form firing off.

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

		std::set<Input::Hash> delayList;


		

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

#define MAKE_INPUT(mc_action, mc_func, mc_aParam, mc_priority, mc_trigger, mc_conflict, mc_tFilter, mc_tParam)\
			InputCommand* CONCAT(command,__LINE__) = new InputCommand;\
			auto& CONCAT(action,__LINE__) = CONCAT(command,__LINE__)->actions.emplace_back();\
			CONCAT(action,__LINE__).type = mc_action;\
			auto& CONCAT(args,__LINE__) = CONCAT(action,__LINE__).args = std::make_unique<Argument[]>(3);\
			CONCAT(args,__LINE__)[InvokeFunction::FUNCTION_PTR] = mc_func;\
			CONCAT(args,__LINE__)[InvokeFunction::CUST_PARAM_1] = mc_aParam;\
			auto& CONCAT(trigger,__LINE__) = CONCAT(command,__LINE__)->triggers.emplace_back();\
			CONCAT(trigger,__LINE__).priority = mc_priority;\
			CONCAT(trigger,__LINE__).type = mc_trigger;\
			CONCAT(trigger,__LINE__).conflict = mc_conflict;\
			CONCAT(trigger,__LINE__).stageFilter = mc_tFilter;\
			CONCAT(trigger,__LINE__).args.emplace_back(std::make_unique<Argument[]>(1))[OnControl::CONTROL_ID] = mc_tParam;\
			something.push_back(CONCAT(command,__LINE__));


			


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
			
			command3->name = "Silent 69";

			InputCommand* command4 = new InputCommand;


			auto& action4 = command4->actions.emplace_back();
			action4.type = ActionType::InvokeFunction;
			auto& args4 = action4.args = std::make_unique<Argument[]>(3);
			args4[InvokeFunction::FUNCTION_PTR] = tmp_YELL_a_nameNEW;
			args4[InvokeFunction::CUST_PARAM_1] = 10;

			auto& trigger4 = command4->triggers.emplace_back();
			trigger4.priority = 10;
			trigger4.type = TriggerType::OnControl;
			trigger4.conflict = ConflictLevel::Guarding;
			trigger4.stageFilter = EventStage::StartFinish;
			trigger4.args.emplace_back(std::make_unique<Argument[]>(1))[OnControl::CONTROL_ID] = "Left Attack/Block";
			trigger4.args.emplace_back(std::make_unique<Argument[]>(1))[OnControl::CONTROL_ID] = "Right Attack/Block";



			std::vector<InputCommand*> something{ command1, command2, command3, command4, command5, command6 };


			//MAKE_INPUT(ActionType::InvokeFunction, tmp_say_a_nameNEW, 20, 20, TriggerType::OnControl, ConflictLevel::Blocking, EventStage::Start, "Left Attack/Block");


		
			for (auto command : something)
			{
				for (auto& trigger : command->triggers)
				{
					auto entry = std::make_shared<CommandEntry>(command, &trigger, trigger.IsControlTrigger());

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

		//Takes input because it is genuinely easier to do it like this.
		bool ClearActiveInput(Input input)
		{
			if (input.IsControl() == true)
				throw std::exception("Cannot obtain active input for control");
			
			//Later, this will very likely stick around and clear itself instead. For now, this works.


			return activeMap.erase(input);
			
		}

		bool ClearActiveInput(decltype(activeMap)::iterator& it)
		{
			auto test = activeMap.erase(it);
			//auto result = test == it;
			it = test;
			return true;

		}

		//I'm actually not sure if I'll even need this. SO I'm holding off on it for now.
		bool UpdateDelayedInputs()
		{
			for (auto it = delayList.begin(); it != delayList.end(); ) {
				auto found = activeMap.find(*it);
				
				if (activeMap.end() == found || found->second->UpdateDelayed() == true) {
					it = delayList.erase(it);
				}
				else {
					++it;
				}
			}
			//I would desperately lik
			return delayList.size();
		}

		void CheckRelease(InputInterface& event, Input input)
		{
			std::unique_ptr<RE::InputEvent> dump;//This concept doesn't currently work, so I'm not really giving it any credit.

			auto it = activeMap.begin();
			auto end = activeMap.end();
			it = activeMap.find(input);
			
			if (it == end)
				return;


			auto pair = event.GetEventValues();

			if (it->second->IsInitialized() == false){
				//This is for when tap comes into play. It's characterized by having an ActiveInput but no initialization.
				logger::warn("This isn't supposed to happen yet");
				it->second->Initialize(pair.first, pair.second);
				return;
			}

			if (event->eventType == RE::INPUT_EVENT_TYPE::kButton)
				event.SetEventValues(0, it->second->data.SecondsHeld());
			else
				event.SetEventValues(0, 0);

			HandleEvent(event, dump);

			event.SetEventValues(pair.first, pair.second);
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

			if (stage == EventStage::Start) {
				CheckRelease(event, input);
			}


			ActiveInputHandle active_input{ input, activeMap, event.GetEventValues() };

			//So what's the order to do?

			//StatePremode
			//Mode
			//StatePostmode
			//Dynamic


			auto ctrl_entry = dynamicMap.find(Input::CONTROL);
			auto inp_entry = dynamicMap.find(input.hash());

			auto d_end = dynamicMap.end();

			detail::VisitorList list;

			if (ctrl_entry != d_end){
				list.push_back(std::ref(ctrl_entry->second));
			}

			if (inp_entry != d_end) {
				list.push_back(std::ref(inp_entry->second));
			}

			EventFlag flags = EventFlag::None;

			bool blocking = false;
			

			bool block_ = false;

			size_t hash = 0;//starts as zero so it will always need to load the first time.

			//I don't remember why, but I think it was best to do this.
			// I remember why, it's because only the stuff in active input is actually fired. When a multiple stage command, or a 
			// single stage blocking command finds itself in the active input it simply puts itself into the call list.
			// non trigger blocking single stage stuff will add itself so long as there's no blocking.
			//Actually, fuck the list. 
			//std::vector<CommandEntry*> call_list;

			std::vector<CommandEntryPtr> after_list;

			//I think I should perhaps redo this bit. I think I would ONLY ever need this to retrieve viable options.

			VisitLists([&](CommandEntryPtr entry, bool& should_continue)
			{
				EntryIndexCleaner cleaner{ entry };//This cleaner increments when it dies.
				
				if (entry->CanHandleEvent(event))
				{
					

					bool result = true;

					//TODO: asking for the id in emplacement might not be necessary, or continuing past emplacement.
					ActiveCommand::ID id = 0;


					if (1){//entry->GetTriggerFilter() & stage) {
						//If it has multiple stages, blocks multiple actions.
						if (entry->HasMultipleBlockStages() || entry->ShouldBlockTriggers() || entry->HasFinishStage())
						{
							if (active_input->EmplaceCommand(entry, stage, id) == false) {
								return;
							}
						}
						else if (entry->GetTriggerFilter() & stage) {
							after_list.push_back(entry);
						}
						return;

						//I think I'm actually going to put this in emplace.
						if (!active_input->IsStageBlockedHashed(block_, hash, id, stage) || entry->ShouldBeBlocked() == false) {

							EventData data{ this, nullptr, event, stage };

							if (!entry->Execute(data, flags) ) {
								//This should do something, but currently I'm unsure what exactly.
								//allow_execute
							}
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
			

			auto it = after_list.begin();
			auto end = after_list.end();


			if (active_input)
			{
				//TODO: I just realized these don't really account for if they are supposed to be blocked.

				auto mid_function = [&](ActiveCommand* act)
					{
						while (it != end) {

							if (act && it->get()->priority() <= act->entry->priority())
								break;


							//I'm just going to boiler plate this cause I'm fucking lazy
							//Also, testing this one out, no more check functions on stage blocking, updates will not be happening so best to do it here.

							auto& entry = *(it++);



							if (!active_input->IsStageBlockedHashed(block_, hash, 0, stage) || entry->ShouldBeBlocked() == false) {
								//if (!block_ || entry->ShouldBeBlocked() == false) {

								EventData data{ this, nullptr, event, stage };

								if (!entry->Execute(data, flags)) {
									//This should do something, but currently I'm unsure what exactly.
									//allow_execute
								}
							}
						}
					};

				//I just want to say, this set up is literally fucking unhinged and I shold be ashamed of it.
				active_input->VisitActiveCommands([&](ActiveCommand& act)
					{
						bool waited = act.HasWaited();

						if (act.stages & stage || waited) {// if it's waited, maybe perform some of the previous.

							mid_function(&act);


							//As before but even more so, I'm REALLY digging the idea of putting this in active inputs.
							// Doing so would prevent these from ever forming as an activeCommand, and thus cutdown on the amount of
							// computing needed to process things that will never come into success.

							auto trigger_stages = act->GetTriggerFilter();

							for (EventStage i = waited ? EventStage::Start : stage; i <= stage; i <<= 1)
							{
								if (trigger_stages & i)
								{
									act.stages |= i;


									if (!active_input->IsStageBlockedHashed(block_, hash, act.id(), stage) || act.entry->ShouldBeBlocked() == false) {
										//if (!block_ || act.entry->ShouldBeBlocked() == false) {

										EventData data{ this, nullptr, event, i };

										if (!act.entry->Execute(data, flags)) {
											//This should do something, but currently I'm unsure what exactly.
											//allow_execute
										}
									}
								}
							}
							act.ClearWaiting();
						}



					});

				mid_function(nullptr);
			}
			//Do the actual thing here.



			bool result = (!active_input || !active_input->IsStageBlocked(stage, false)) || (flags & EventFlag::Continue);

			if (stage == EventStage::Finish) {
				ClearActiveInput(input);
			}

			return result;


			return allow_execute || (flags & EventFlag::Continue);
		}

		void HandleRelease(RE::PlayerControls* a_controls)
		{
			//Emplaces can be handled here. Please handle them.
			

			bool block_;

			size_t hash = 0;

			std::unique_ptr<RE::ButtonEvent> button{ RE::ButtonEvent::Create(RE::INPUT_DEVICES::kNone, "", 0, 0, 0) };

			InputInterface event{ button.get(), a_controls };


			//PLEASE note, delay event refiring cannot happen here as proper, because emplace command is not happening. So, 
			// I need to divide that function in such a way that I can use it's components.

			//*
			for (auto it = activeMap.begin(); it != activeMap.end(); ) {
				auto dump = it->first;
				auto& active = it->second;
				
				EventFlag flags = EventFlag::None;

				Input input = dump;

				button->device = input.device;
				button->heldDownSecs = active->data.SecondsHeld();

				active->VisitActiveCommands([&](ActiveCommand& act)
				{
					if (act->GetTriggerFilter() & EventStage::Finish)
					{
						if (act.stages & EventStage::Finish) {
							//This already processed a finish, no need.
							return;
						}
						//As before but even more so, I'm REALLY digging the idea of putting this in active inputs.
						// Doing so would prevent these from ever forming as an activeCommand, and thus cutdown on the amount of
						// computing needed to process things that will never come into success.

						if (!active->IsStageBlockedHashed(block_, hash, act.id(), EventStage::Finish) || act.entry->ShouldBeBlocked() == false) {
							//if (!block_ || act.entry->ShouldBeBlocked() == false) {

							EventData data{ this, nullptr, event, EventStage::Finish };

							if (!act.entry->Execute(data, flags)) {
								//This should do something, but currently I'm unsure what exactly.
								//allow_execute
							}
						}
					}
				});

				
				if (ClearActiveInput(it) == false) {
					++it;
				}
			}
			//*/
			

			return;
			for (auto& [dump, active] : activeMap)
			{

				


				//ClearActiveInput



				//How do we do this exactly? call InputInterface?

				//We could have an easy store of every device as well as every input or something like that. Soooo, active input.

				//From there, we have a function that we fire off that basically updates every control that we currently have,
				// we input stuff for controls the whole shebang and fire it off.
				

				//So basically it will be up to each ActiveInput to help resolve. But unlike skyrim we cant just leave because we don't have all
				// active stuff loaded.

				//HOWEVER, a good resolve for that might just be allowing for things to be added even if it isn't it's time. This might be prefered.
				// the rule about the first stage must then change to "If the stage given is later than the first stage".
			}

			//An emergency release.
		}
	};

	inline MatrixController* testController = new MatrixController;
	inline std::array<MatrixController*, (int)ControlType::Total> Controllers;
}