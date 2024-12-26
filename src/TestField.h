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

#include "RE/Functions.h"

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





	using CommandMap = std::unordered_map<Input::Hash, std::vector<std::shared_ptr<CommandEntry>>>;








	inline void ExecuteInput(InputInterface& event)
	{
		RE::ExecuteInput(event.controls, event.event);
		RE::UnkFunc01(event.controls);
	}




	






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

		//When invoked, only commands with the same type of matrix type will be allowed used.
		virtual bool CanInputPass(RE::InputEvent* event) const
		{
			return true;
		}

		CommandMap CreateCommandMap()
		{
			CommandMap map;

			for (auto& command : commands)
			{
				for (auto& trigger : command.triggers)
				{
					CommandEntryPtr entry = std::make_shared<CommandEntry>(&command, &trigger, trigger.IsControlTrigger());

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
	
	struct LayerMatrix : public InputMatrix
	{
		//Blocked user events are stored as device less id codes.
		std::set<Input> blockedInputs;


		bool CanInputPass(RE::InputEvent* event) const override
		{
			auto& event_name = event->QUserEvent();
			Input input = event;

			Input userEvent{ RE::INPUT_DEVICE::kNone, Hash(event_name.c_str(), event_name.length()) };

			return !blockedInputs.contains(input) && !blockedInputs.contains(userEvent);
		}
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



	namespace detail
	{
		using VisitorList = std::vector<std::reference_wrapper<std::vector<std::shared_ptr<CommandEntry>>>>;
		using VisitorFunc = std::variant<std::function<void(CommandEntry*, bool&)>, std::function<void(CommandEntryPtr, bool&)>>;
	}

	//This what hell looks like.
	inline void VisitLists(detail::VisitorFunc func, detail::VisitorList& a_lists)
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

	struct MatrixMap
	{
		InputMatrix* matrix = nullptr;
		CommandMap commands;
		
		//ActiveCommands, and wait lists should hold onto this shared pointer when executing data. This way, it prevents the entry
		// from dying before being unhandled.
		


	};

	
	struct ModeMap : public MatrixMap
	{
		CommandEntryPtr progenitor;

		bool isStrong = true;	//If the mode isn't strong, all actions it takes will also be tenative. This will basically set up the active
								// input to have a waiter built into it. 

		//This isn't quite right. If the mode is weak, then it should prefer other actions first. I guess this is the same thing as a waiter huh?

		//SO maybe something like this. We forcibly put a waiter on the command, then we check if there were any other combo actions
		

		//Actually global waiter seems about right. Due to the whole multiple press thing. We could be using more than 3 buttons.


		//Now that I think about it, if it was ever to truly be interupted


		/*
		I think the initial wait is the easy part. The hard part is what comes next. Say I need to crumple all my stuff down. What do I do?
		Unsure.

		For failure I can have some play with the sign bit, to denote that external failure has happened. I COULD do something similar for inputs,
		 but inputs are bitfielded. So they don't have much room to play with. it needs it's sign bit

		Actually, never mind, input is free to have signed values mean something, inputs can never go past 10 so this is a complete win.

		So if inputs has the sign bit it means it has an external waiter (for this I'd say only the owner of the commandEntry can actually declare it a waiter).

		Successes seperately will be completion flag, denoting something is done and doesn't need to be run anymore.

		//*/


		//TODO: This needs to refine itself, also needs to actually create it's command map.
		ModeMap(LayerMatrix* mode, CommandEntryPtr& prog, bool strong) : isStrong{ strong }
		{
			matrix = mode; 
			progenitor = prog;

			commands = matrix->CreateCommandMap();
		}
	};


	struct FakeThumbstick
	{
		static constexpr auto VTABLE = RE::VTABLE_ThumbstickEvent;
	};


	struct FakeMouseMove
	{
		static constexpr auto VTABLE = RE::VTABLE_MouseMoveEvent;
	};

	inline static RE::ThumbstickEvent* CreateThumb(const RE::BSFixedString& a_userEvent, RE::ThumbstickEvent::InputType a_idCode, float a_x, float a_y)
	{


		auto result = RE::malloc<RE::ThumbstickEvent>(sizeof(RE::ThumbstickEvent));
		std::memset(reinterpret_cast<void*>(result), 0, sizeof(RE::ThumbstickEvent));
		if (result) {
			
			SKSE::stl::emplace_vtable<FakeThumbstick>(reinterpret_cast<FakeThumbstick*>(result));
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


	inline static RE::MouseMoveEvent* CreateMouse(const RE::BSFixedString& a_userEvent, float a_x, float a_y)
	{
		auto result = RE::malloc<RE::MouseMoveEvent>(sizeof(RE::MouseMoveEvent));
		std::memset(reinterpret_cast<void*>(result), 0, sizeof(RE::MouseMoveEvent));
		if (result) {

			SKSE::stl::emplace_vtable<FakeMouseMove>(reinterpret_cast<FakeMouseMove*>(result));
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

	
	inline void tmp_say_a_nameNEW(EventData&& data, EventFlag& flags, bool& result, const Argument& param1, const Argument& param2)
	{
		auto report = std::format("Action: No. {} called. Stage: {}, time {:X}", 
			param1.As<int32_t>(), magic_enum::enum_name(data.stage), RE::GetDurationOfApplicationRunTime());

		logger::info("{}", report);
		RE::DebugNotification(report.c_str(), "UIObjectiveNew");

	}

	inline void tmp_YELL_a_nameNEW(EventData&& data, EventFlag& flags, bool& result, const Argument& param1, const Argument& param2)
	{
		auto report = std::format("Action: No. {} called. Stage: {}", param1.As<int32_t>(), magic_enum::enum_name(data.stage));
		logger::info("{}", report);
		RE::DebugMessageBox(report.c_str());

	}

	inline void KillingMeSlowly(EventData&& data, EventFlag& flags, bool& result, const Argument& param1, const Argument& param2)
	{
		RE::PlayerCharacter::GetSingleton()->AsActorValueOwner()->RestoreActorValue(RE::ACTOR_VALUE_MODIFIER::kDamage,
			RE::ActorValue::kHealth, -param1.As<float>() * RE::GetSecondsSinceLastFrame());

	}
	
	inline void tmp_YELL_a_Mode(EventData&& data, EventFlag& flags, bool& result, const Argument& param1, const Argument& param2)
	{
		auto report = std::format("Mode Action: No. {} called. Stage: {}", param1.As<int32_t>(), magic_enum::enum_name(data.stage));
		logger::info("{}", report);
		RE::DebugMessageBox(report.c_str());
		RE::PlaySound("UIObjectiveNew");
	}
	
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
		EventStage basicCommands = EventStage::None;
		int16_t waiters = 0;
		uint32_t highestPrecedence = 0;//The block delay is the highest level of delay priority active. If something is below this
									// it cannot block other actions

#pragma region basic waiting/block funcs
		bool IsBasicRunning() const
		{
			return !waiters;
		}

		void SetBasicFailure()
		{
			//This is called once something has successfully run once.
			waiters = -1;
		}

		void DecBasicDelay()
		{
			if (waiters >= 0)
			{
				waiters--;
				assert(waiters >= 0);
			}
		}

		void IncBasicDelay()
		{
			if (waiters >= 0)
			{
				waiters++;
			}
		}

		void SetRedoStage(EventStage stage)
		{
			basicCommands |= stage;
		}

		bool GetRedoStages() const
		{
			return basicCommands;
		}
		void ClearRedoStages()
		{
			basicCommands = EventStage::None;
		}

		std::pair<InputNumber, InputNumber> GetInputValues()
		{
			return  { data.value1, data.value2 };
		}

#pragma endregion

#pragma region new block funcs

		void ClearBlockStages()
		{
			blockCommands = emptyBlock;
			highestPrecedence = 0;
		}

		void FillBlockStages()
		{
			ClearBlockStages();

			//This prevents other items of precedence from taking place.
			//EventStage reqs = EventStage::None;

			for (auto& command : sharedCommands)
			{
				//if (reqs == EventStage::All)
				//	continue;

				if (command.IsRunning() == true)
					EmplaceBlockStages(command);
				
				//auto pred = highestPrecedence;

				//auto result = EmplaceBlockStages(command);
				

				//Precedence has changed. previous value no longer value.
				//if (pred == highestPrecedence) {
				//	reqs = EventStage::None;
				//}

				//if (result) {
				//	reqs |= command->GetTriggerFilter();
				//}
			}
		}

		//Note, a lot of remove block stages can be replaced with something that decrements waiters and removes stuff at once. Maybe combine the precedence check too.
		void RemoveBlockStages(ActiveCommand& cmd)
		{
			auto remove_all = cmd->GetTriggerFilter() == EventStage::All;

			if (!remove_all) {
				for (int x = 1, y = 0; x < EventStage::Total; (x <<= 1), y++)
				{
					if (cmd.id() == blockCommands[y])
					{
						blockCommands[y] = 0;
					}
				}
			}
			if (!remove_all || blockCommands == emptyBlock)
			{
				FillBlockStages();
			}
		}


		bool EmplaceBlockStages(ActiveCommand& cmd)
		{

			auto pred = cmd->Precedence();

			if (pred > highestPrecedence){
				ClearBlockStages();
				highestPrecedence = pred;
			}
			else if (pred < highestPrecedence) {
				return false;
			}

			auto emplace_array = blockCommands;

			auto stages = cmd->GetTriggerFilter();

			if (stages)
			{
				for (int x = 1, y = 0; x < EventStage::Total; (x <<= 1), y++)
				{
					if (stages & x)
					{
						auto& block_id = emplace_array[y];

						if (cmd.id() == block_id)
							continue;

						switch (x)
						{
						case EventStage::Start:
						case EventStage::Repeating:
						case EventStage::Finish:
							if (!block_id)
								block_id = cmd.id();
							else
								return false;


						default:
							break;
						}
					}
				}

				blockCommands = emplace_array;
			}
			return true;
		}


#pragma endregion


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

						RemoveBlockStages(command);
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
				IncBasicDelay();

				for (auto& other : sharedCommands)
				{
					if (&other == &act)
						continue;

					if (act.entry->tmpname_ShouldWaitOnMe(*other.entry) == true) {
						other.tempname_IncWaiters();

						RemoveBlockStages(other);
						continue;
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
				DecBasicDelay();

				for (auto& other : sharedCommands)
				{
					if (&other == &act)
						continue;

					if (act.entry->tmpname_ShouldWaitOnMe(*other.entry) == true) {
						//TODO: Here there seems to be some kind of loop that will ultimately cause the waiter on a particular item to time out. Unknown why
						if (other.tempname_DecWaiters() == 0) {
							//Here we refire all action related data.
							//Here's another thought though, build playing catch up into the execution.

							//Don't control what gets to go off by whether it has
							ActiveCommand::ID dump;

							//EmplaceCommand(other.entry, stage, dump, &other);
							UpdateCommand(other, stage);
						}


					}
				}

				//A good check is if I actually find this within the thing.
				//assert(found_self);

				act.SetDelayUndone();
			}

		}


		//TODO: Need a function to fail a command, which clears out the waiters.

	

		void UpdateCommand(ActiveCommand& act, EventStage stage)

		{//TODO: Make a different version of emplace function. I don't want this option exposed.

			if (act.entry->IsDelayable() == true)
			{
				if (!act.IsRunning() && stage == EventStage::Finish) {
					act.Deactivate();
				}


				if (!act.IsRunning() && act.IsDelayUndone() == false)
				{
					DelayState delay_state = act.IsFailing() ? DelayState::Failure : act.entry->GetDelayState(nullptr, data);


					if (delay_state == DelayState::Failure) {
						//signify some failure.
						FailDelayCommand(act, stage);
					}
				}
			}


			if (act.IsRunning() && act->ShouldBlockTriggers() == true)
				EmplaceBlockStages(act);
		}

		void MakeCommand(CommandEntryPtr& cmd, EventStage stage)
		{//TODO: Make a different version of emplace function. I don't want this option exposed.
			if (GetCommandFromEntry(cmd) != nullptr)
				return;


			//This bit gets saved, but for what I do not know.
			//cmd->GetFirstStage() < stage;


			//This doesn't quite fit.
			if (cmd->ShouldBeBlocked() && IsStagesBlocked(cmd->GetTriggerFilter()) == true) {
				return;
			}
			

			ActiveCommand act{ cmd };

			if (cmd->ShouldBlockTriggers() && EmplaceBlockStages(act) == false)
				return;

			AddCommand(act);
		}
		//This is actually useless btw.
		void EmplaceCommand(CommandEntryPtr cmd, EventStage stage)
		{
			if (stage == EventStage::Start) {
				if (cmd->GetSuccess() == 0)//Should failure be 0 too?
					MakeCommand(cmd, stage);
			}
			else
			{
				auto act = GetCommandFromEntry(cmd);

				if (act)
					UpdateCommand(*act, stage);
			}
		}


		void Update(EventStage stage)
		{
			for (auto& act : sharedCommands)
			{
				UpdateCommand(act, stage);
			}
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

		inline bool IsStagesBlocked(EventStage stage) const
		{
			if (stage)
			{
				for (auto x = 1, y = 0; x < EventStage::Total; x <<= 1, y++)
				{
					if (stage & x && blockCommands[y]) {
						return true;
					}

				}
			}
			return false;
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


		/// <summary>
		/// Checks if a given event stage is blocked.
		/// </summary>
		/// <param name="stage">The given event stage to check. Should be a single flag.</param>
		/// <param name="trig">If only triggers should be checked for blocking.</param>
		/// <returns></returns>
		bool IsStageBlocked(EventStage stage, bool trig)
		{
			if (trig) {
				//TEMP: Testing if this would suffice better. Way I see it, if it's in here it blocks triggers.
				auto inch = std::countr_zero(std::to_underlying(stage));
				return blockCommands[inch];
				
				//DELETE~>
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


		void VisitActiveCommands(bool ignore_running, std::function<void(ActiveCommand&)> func)
		{
			for (auto& command : sharedCommands) {
				if (ignore_running || command.IsRunning() == true) {
					func(command);
				}
			}
		}

		auto VisitActiveCommands(std::function<void(ActiveCommand&)> func)
		{
			return VisitActiveCommands(false, func);
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

			auto it = map.find(h);
			auto end = map.end();

			if (it != end) {
				input = it->second.get();
			}

		}

	};

	inline LayerMatrix* testMode = new LayerMatrix;


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


		std::vector<std::unique_ptr<ModeMap>> modeMaps;//The last mode is most important.


		void EmplaceMode(LayerMatrix* mode, CommandEntryPtr& command, bool strengthen) 
		{
			auto it = modeMaps.begin();
			auto end = modeMaps.end();

			it = std::find_if(it, end, [&](std::unique_ptr<ModeMap>& i) {return i->progenitor == command && i->matrix == mode; });

			if (it == end) {
				auto& config = modeMaps.emplace_back(std::make_unique<ModeMap>(mode, command, strengthen));



				return;
			}

			if (strengthen)//This needs to do other things than just declare strength BTW, but this works for now.
				it->get()->isStrong = true;

		}

		void RemoveMode(LayerMatrix* mode, CommandEntryPtr& command)
		{
			auto it = modeMaps.begin();
			auto end = modeMaps.end();

			it = std::find_if(it, end, [&](std::unique_ptr<ModeMap>& i) {return i->progenitor == command && i->matrix == mode; });

			if (it != end) {
				modeMaps.erase(it);
			}
		}

		ModeMap* GetCurrentMode()
		{
			return modeMaps.size() ? modeMaps.back().get() : nullptr;
		}





		bool PrepVisitorList(detail::VisitorList& list, RE::InputEvent* event, Input input, MatrixType type)
		{
			//Returns if it should be allowed to continue on down the line.

			//std::vector<CommandEntryPtr>& 
			CommandMap* map;
			InputMatrix* matrix = nullptr;

			switch (type)
			{
			case MatrixType::Selected:
			case MatrixType::State:

			default:
				return true;


			case MatrixType::Mode: {
				ModeMap* mode = GetCurrentMode();
				if (!mode) {
					return true;
				}

				map = &mode->commands;
				matrix = mode->matrix;

				break;
			}
			case MatrixType::Dynamic:
				map = &dynamicMap;
				break;
			}

			if (!map)
				return true;
			

			auto ctrl_it = map->find(Input::CONTROL);
			auto input_it = map->find(input.hash());

			auto end = map->end();

			//change these to AttemptToPull functions.
			if (ctrl_it != end) {
				list.push_back(std::ref(ctrl_it->second));
			}

			if (input_it != end) {
				list.push_back(std::ref(input_it->second));
			}


			return !matrix || matrix->CanInputPass(event);
		}




		MatrixController()
		{

#define MAKE_INPUT(mc_action, mc_func, mc_aParam, mc_priority, mc_trigger, mc_conflict, mc_tFilter, mc_tParam)\
			InputCommand* CONCAT(command,__LINE__) = new InputCommand; \
			{\
				auto& CONCAT(action, __LINE__) = CONCAT(command, __LINE__)->actions.emplace_back();\
				CONCAT(action, __LINE__).type = mc_action; \
				auto& CONCAT(args, __LINE__) = CONCAT(action, __LINE__).args = std::make_unique<Argument[]>(3); \
				CONCAT(args, __LINE__)[InvokeFunction::FUNCTION_PTR] = mc_func; \
				CONCAT(args, __LINE__)[InvokeFunction::CUST_PARAM_1] = mc_aParam; \
				auto& CONCAT(trigger, __LINE__) = CONCAT(command, __LINE__)->triggers.emplace_back(); \
				CONCAT(trigger, __LINE__).priority = mc_priority; \
				CONCAT(trigger, __LINE__).type = mc_trigger; \
				CONCAT(trigger, __LINE__).conflict = mc_conflict; \
				CONCAT(trigger, __LINE__).stageFilter = mc_tFilter; \
				CONCAT(trigger, __LINE__).args.emplace_back(std::make_unique<Argument[]>(1))[OnControl::CONTROL_ID] = mc_tParam; \
				something.push_back(CONCAT(command, __LINE__)); \
			}

			{
				testMode->blockedInputs = {
					Input { RE::INPUT_DEVICE::kNone, "Jump"_h },
					Input { RE::INPUT_DEVICE::kNone, "Sprint"_h },
					Input { RE::INPUT_DEVICE::kNone, "Sneak"_h },
					Input { RE::INPUT_DEVICE::kNone, "Shout"_h },
					Input { RE::INPUT_DEVICE::kNone, "Left Attack/Block"_h },
					Input { RE::INPUT_DEVICE::kNone, "Right Attack/Block"_h },
					Input { RE::INPUT_DEVICE::kNone, "Ready Weapon"_h },
					Input { RE::INPUT_DEVICE::kNone, "Toggle POV"_h },
					Input { RE::INPUT_DEVICE::kNone, "Activate"_h },
				};

				testMode->commands.reserve(10);
				InputCommand* commandA = &testMode->commands.emplace_back();
				InputCommand* commandB = &testMode->commands.emplace_back();

				auto& actionA = commandA->actions.emplace_back();
				actionA.type = ActionType::InvokeFunction;
				auto& argsA= actionA.args = std::make_unique<Argument[]>(3);
				argsA[InvokeFunction::FUNCTION_PTR] = KillingMeSlowly;
				argsA[InvokeFunction::CUST_PARAM_1] = 15.0f;

				auto& triggerA = commandA->triggers.emplace_back();
				triggerA.priority = 69;
				triggerA.type = TriggerType::OnControl;
				triggerA.conflict = ConflictLevel::Defending;
				triggerA.stageFilter = EventStage::All;
				triggerA.args.emplace_back(std::make_unique<Argument[]>(1))[OnControl::CONTROL_ID] = "Left Attack/Block";


				auto& actionB = commandB->actions.emplace_back();
				actionB.type = ActionType::InvokeFunction;
				auto& argsB = actionB.args = std::make_unique<Argument[]>(3);
				argsB[InvokeFunction::FUNCTION_PTR] = tmp_YELL_a_Mode;
				argsB[InvokeFunction::CUST_PARAM_1] = 11000;

				auto& triggerB = commandB->triggers.emplace_back();
				triggerB.priority = 10;
				triggerB.type = TriggerType::OnControl;
				triggerB.conflict = ConflictLevel::Defending;
				triggerB.stageFilter = EventStage::StartFinish;
				triggerB.args.emplace_back(std::make_unique<Argument[]>(1))[OnControl::CONTROL_ID] = "Jump";

			}


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

			//auto& action3 = command3->actions.emplace_back();
			//action3.type = ActionType::InvokeInput;
			//action3.stageFilter = EventStage::Start;
			//auto& args3 = action3.args = std::make_unique<Argument[]>(1);
			//args3[InvokeInput::VIRTUAL_INPUT] = Input{ RE::INPUT_DEVICE::kKeyboard, 2 };
			
			auto& action3b = command3->actions.emplace_back();
			action3b.type = ActionType::InvokeFunction;
			auto& args3b = action3b.args = std::make_unique<Argument[]>(3);
			args3b[InvokeFunction::FUNCTION_PTR] = KillingMeSlowly;
			args3b[InvokeFunction::CUST_PARAM_1] = 15.0f;
			
			auto& trigger3 = command3->triggers.emplace_back();
			trigger3.priority = 69;
			trigger3.type = TriggerType::OnControl;
			trigger3.conflict = ConflictLevel::Defending;
			trigger3.stageFilter = EventStage::All;
			trigger3.args.emplace_back(std::make_unique<Argument[]>(1))[OnControl::CONTROL_ID] = "Right Attack/Block";
			trigger3.args.emplace_back(std::make_unique<Argument[]>(1))[OnControl::CONTROL_ID] = "Left Attack/Block";
			trigger3.args.emplace_back(std::make_unique<Argument[]>(1))[OnControl::CONTROL_ID] = "Jump";
			
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
			trigger4.conflict = ConflictLevel::Defending;
			trigger4.stageFilter = EventStage::StartFinish;
			trigger4.args.emplace_back(std::make_unique<Argument[]>(1))[OnControl::CONTROL_ID] = "Left Attack/Block";
			trigger4.args.emplace_back(std::make_unique<Argument[]>(1))[OnControl::CONTROL_ID] = "Right Attack/Block";



			std::vector<InputCommand*> something{ command1, command2, command3, command4, command5, command6 };




			InputCommand* command1554 = new InputCommand; {
				auto& action1554 = command1554->actions.emplace_back(); 
				action1554.type = ActionType::InvokeMode;
				auto& args1554 = action1554.args = std::make_unique<Argument[]>(1); 
				args1554[InvokeMode::MODE_PTR] = testMode;
				auto& trigger1554 = command1554->triggers.emplace_back(); 
				trigger1554.priority = 20; 
				trigger1554.type = TriggerType::OnControl; 
				trigger1554.conflict = ConflictLevel::Blocking; 
				trigger1554.stageFilter = EventStage::StartFinish;
				trigger1554.args.emplace_back(std::make_unique<Argument[]>(1))[OnControl::CONTROL_ID] = "Right Attack/Block";
				trigger1554.args.emplace_back(std::make_unique<Argument[]>(1))[OnControl::CONTROL_ID] = "Jump";
				something.push_back(command1554);
			};




			MAKE_INPUT(ActionType::InvokeFunction, tmp_say_a_nameNEW, 20, 20, TriggerType::OnControl, ConflictLevel::Blocking, EventStage::Start, "Left Attack/Block");


		
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

			//Incrementing and decrementing static time will allow this to not interfer with later entries.
			CommandEntry::IncStaticTimestamp();

			HandleEvent(event, dump);

			CommandEntry::DecStaticTimestamp();

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


			EventFlag flags = EventFlag::None;

			bool blocking = false;
			

			bool block_ = false;

			size_t hash = 0;//starts as zero so it will always need to load the first time.

			bool execute_basic = true;


			if (stage == EventStage::Start)
			{
				detail::VisitorList list;

				for (MatrixType i = (MatrixType)0; i < MatrixType::Total; i++) 
				{
					if (PrepVisitorList(list, event.event, input, i) == false) {
						//If one of these blocks further inputs, it needs an active command as a reminder.
						active_input->SetBasicFailure();
						break;
					}
				}

				

				VisitLists([&](CommandEntryPtr entry, bool& should_continue)
				{
					//TODO:Big note here, this should not allow new inputs for a thing once when it's been declared success.
					// The proposed situation where that happens is when some input is the hold out. So forgo it.


					EntryIndexCleaner cleaner{ entry };//This cleaner increments when it dies.

					if (entry->CanHandleEvent(event))
					{
						active_input->MakeCommand(entry, stage);
					}

				}, list);
			}
			else if (active_input)
			{
				active_input->Update(stage);
			}


			if (active_input)
			{
				active_input->VisitActiveCommands([&](ActiveCommand& act)
				{

					act.SetEarlyExit(false);


					//For merely being present, regardless if it's blocked or not, it will prevent the original from going off.
					// Guarding and defending are the same thing here, btw. They shouldn't be seperate.
					if (act->ShouldBlockNative() == true)
						active_input->SetBasicFailure();
					
						
					{

						//As before but even more so, I'm REALLY digging the idea of putting this in active inputs.
						// Doing so would prevent these from ever forming as an activeCommand, and thus cutdown on the amount of
						// computing needed to process things that will never come into success.

						auto trigger_stages = act->GetTriggerFilter();

						bool executed = false;

						for (EventStage i = act.HasWaited() ? EventStage::Start : stage; i <= stage; i <<= 1)
						{
							if (trigger_stages & i)
							{
								if (i == EventStage::Finish && act->GetSuccess() > 1) {
									//We'll want to let input go but not fire the action.
									logger::info("Retaining at success level {}", act->GetSuccess());
									break;
								}



								if (!active_input->IsStageBlockedHashed(block_, hash, act.id(), i) || act.entry->ShouldBeBlocked() == false) {
									//if (!block_ || act.entry->ShouldBeBlocked() == false) {

									EventData data{ this, act.entry, &active_input->data, event, i };
										
									act.entry->RepeatExecute(data, flags, executed);
								}
							}
						}

						act.ClearWaiting();
					}



				});

				if (active_input->IsBasicRunning() == false)
				{
					execute_basic = false;
					active_input->SetRedoStage(stage);
				}
				else if (auto redo = active_input->GetRedoStages(); redo) {
					auto pair = active_input->GetInputValues();
					auto backup = pair;
					
					for (EventStage i = EventStage::Start; i < EventStage::Last; i <<= 1)
					{
						if (redo & i && i != stage)
						{
							if (i == EventStage::Start) {
								backup = event.GetEventValues();
								event.SetEventValues(pair.first, pair.second);
							}


							ExecuteInput(event);
							
							if (i == EventStage::Start) {
								event.SetEventValues(backup.first, backup.second);
							}
						}
					}

					active_input->ClearRedoStages();
				}

			}


			if (stage == EventStage::Finish) {
				ClearActiveInput(input);
			}

			return execute_basic;
		}

		void HandleRelease(RE::PlayerControls* a_controls)
		{
			//Emplaces can be handled here. Please handle them.
			
			//TODO: HandleRelease has a small issue in that when it happens it happens regardless if it's actually been updated or not.

			bool block_;

			size_t hash = 0;

			std::unique_ptr<RE::ButtonEvent> button{ RE::ButtonEvent::Create(RE::INPUT_DEVICES::kNone, "", 0, 0, 0) };

			InputInterface event{ button.get(), a_controls };


			//PLEASE note, delay event refiring cannot happen here as proper, because emplace command is not happening. So, 
			// I need to divide that function in such a way that I can use it's components.

			for (auto it = activeMap.begin(); it != activeMap.end(); ) {
				bool purge = true;
				
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
						//if (act.stages & EventStage::Finish) {
						if (act->HasVisitedStage(EventStage::Finish) == true) {
							//This already processed a finish, no need.
							return;
						}

						
						if (act.HasEarlyExit() == false) {
							purge = false;
							return;
						}



						//TODO: A genuine input check would be better here, because successes may not have registered yet.
						//This seems to work, but have an issue where it only works if one is removed, then the other.

						if (act->GetSuccess() > 1) {
							//We'll want to let input go but not fire the action.
							logger::info("Retaining at success level {}", act->GetSuccess());
							return;
						}
						logger::info("Releasing at success level {}", act->GetSuccess());


						//As before but even more so, I'm REALLY digging the idea of putting this in active inputs.
						// Doing so would prevent these from ever forming as an activeCommand, and thus cutdown on the amount of
						// computing needed to process things that will never come into success.

						if (!active->IsStageBlockedHashed(block_, hash, act.id(), EventStage::Finish) || act.entry->ShouldBeBlocked() == false) {
							//if (!block_ || act.entry->ShouldBeBlocked() == false) {

							EventData data{ this, act.entry, &active->data, event, EventStage::Finish };

							if (!act.entry->Execute(data, flags)) {
								//This should do something, but currently I'm unsure what exactly.
								//allow_execute
							}
						}
					}
				});

				
				if (!purge || ClearActiveInput(it) == false) {
					++it;
				}
			}
		}

		void QueueRelease()
		{
			//*
			for (auto it = activeMap.begin(); it != activeMap.end(); it++) {
			
				Input input = it->first;
				auto& active = it->second;

				active->VisitActiveCommands([&](ActiveCommand& act)
				{
					//This actually needs to work for the individual active entry.

					//For each entry active, try to mark it for release.
					
					//act->ResetExecute();
					act.SetEarlyExit(true);
				});
			}
			//*/
		}
	};

	inline MatrixController* testController = new MatrixController;
	inline std::array<MatrixController*, (int)ControlType::Total> Controllers;
}