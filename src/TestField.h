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
#include "InputMatrix.h"

#include "VirtualEvent.h"



namespace DIMS
{
	//Lookup types
	RE::PlayerControls;
	RE::MenuControls;
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


	struct ControlInterface
	{
		//This is a control for menu controls and player controls (maybe other controls if there are some we find.

		RE::BSTEventSink<RE::InputEvent*>* control{};
	};



	inline void CONTROLESQUE(RE::ControlMap* a_controls)
	{

		for (auto x = 0; x < RE::InputContextID::kTotal; x++)
		{

			//RE::ControlMap::GetSingleton()->controlMap[RE::InputContextID::kGameplay]->deviceMappings[RE::INPUT_DEVICE::kKeyboard].back();
			for (auto i = 0; i < RE::INPUT_DEVICE::kVirtualKeyboard; i++)
			{

				for (auto& value : a_controls->controlMap[x]->deviceMappings[i])
				{
					//Gameplay controls

					logger::info("Context:{}, Device: {}({}), code: {}, userEvent: {}, modifier: {}, linked(?): {}, index: {}, handle: {}",
						magic_enum::enum_name((RE::InputContextID)x),
						magic_enum::enum_name((RE::INPUT_DEVICE)i), i, value.inputKey, value.eventID.c_str(), value.modifier, value.linked, value.indexInContext
						, value.pad14);
				}
			}
		}


		for (auto& link : a_controls->linkedMappings)
		{

			logger::info("Device: {}({}), Mapped Name: {}, From Name: {}, Mapped Context: {}, From Context: {}",
				magic_enum::enum_name(link.device), (int)link.device, link.linkedMappingName.c_str(), link.linkFromName.c_str(),
				magic_enum::enum_name(link.linkedMappingContext),
				magic_enum::enum_name(link.linkFromContext));
		}

		auto& in_you_go = a_controls->controlMap[RE::InputContextID::kGameplay]->deviceMappings[RE::INPUT_DEVICE::kKeyboard];
		auto use_events = RE::UserEvents::GetSingleton();
		logger::info("end {} and {}, or {} {} {}", in_you_go.size(), a_controls->pad123,
			use_events->pad001, use_events->pad002, use_events->pad004);

		constexpr auto test = offsetof(RE::UserEvents, pad001);
	}


	inline void ExecuteInput(RE::PlayerControls* controls, InputInterface& event)
	{
		RE::ExecuteInput(controls, event);
		RE::UnkFunc01(controls);
	}




	namespace detail
	{
		//visitor list should be a set.
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


		for (int i = 0; i < a_lists.size(); i++)
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
					//TODO: I'd like this to be handled entirely based on priority
					//return (*lhs.first)->priority() < (*rhs.first)->priority();
					//return (*lhs.first)->CompareOrder(**rhs.first);
					return (*rhs.first)->CompareOrder(**lhs.first);

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









	struct LayerMatrix : public InputMatrix
	{
		//Blocked user events are stored as device less id codes.
		std::set<Input> blockedInputs;

		//I would like some extra settings for this. If you have a device that has -1 it means that device is entirely disabled.
		// if you have one that has a none device and is fully negative that means ALL controls are prevented from passing through.
		// Selected will do this sometimes, fully preventing passing through to completely reinvent the bindings.

		bool CanInputPass(RE::InputEvent* event) const override
		{
			auto& event_name = event->QUserEvent();
			Input input = event;

			Input userEvent{ RE::INPUT_DEVICE::kNone, Hash(event_name.c_str(), event_name.length()) };

			return !blockedInputs.contains(input) && !blockedInputs.contains(userEvent);
		}
	};




	struct InputMode : public LayerMatrix
	{

		struct Instance : public LayerMatrix::Instance
		{
		public:
			CommandEntryPtr source = nullptr;

			bool isStrong = true;

			InputMode* GetBaseObject()
			{
				return static_cast<InputMode*>(__super::GetBaseObject());
			}
			const InputMode* GetBaseObject() const
			{
				return static_cast<const InputMode*>(__super::GetBaseObject());
			}

		};

		InputMode()
		{
			type = MatrixType::Mode;
		}

		Instance* GetContextInstance(RE::InputContextID context, bool create_if_required = false) override
		{
			return LoadInstance<Instance>(context, create_if_required);
		}

	};



	enum struct CompareType : uint8_t
	{
		//The comparison type for most default stuff is 

		kEqual,
		kLesser,
		kGreater,
		kNotEqual,
		kLesserOrEqual,
		kGreaterOrEqual,

		kSecondary = 1 << 7,
	};

	struct RefreshEvent
	{


		RefreshCode code = RefreshCode::Update;

		CompareType _compare = CompareType::kEqual;

		double value1 = 0;
		double value2 = 0;
		//I'm giving this an optional second value. if the compare type has a flag of "using the second value, the first value is used
		// as a first pass, and must be equal. This increases the size some, but this isn't a particularly expensive increase so who cares.
		// also static size increases aren't to be feared.


		CompareType compare() const
		{
			return _compare & ~CompareType::kSecondary;
		}

		CompareType IsSecondary() const
		{
			return (_compare & CompareType::kSecondary);
		}

		constexpr bool CheckEvent(RefreshCode a_code, double data1, double data2) const
		{
			//Keeping the value as a double is an easy way to represent both a float and an integer

			double value;
			double data;
			if (code == a_code)
			{
				if (IsSecondary() == true) {
					if (value1 != data1)
						return false;

					value = value2;
					data = data2;
				}
				else
				{
					value = value1;
					data = data1;
				}

				switch (compare())
				{
				case CompareType::kNotEqual:
					return value != data;

				case CompareType::kLesser:
					return value < data;

				case CompareType::kGreaterOrEqual:
					return value >= data;

				case CompareType::kGreater:
					return value > data;

				case CompareType::kLesserOrEqual:
					return value <= data;

				case CompareType::kEqual:
					return value == data;
				}
			}

			return false;
		}

		constexpr RefreshEvent() = default;

		constexpr auto operator <=>(const RefreshEvent&) const = default;

		constexpr RefreshEvent(RefreshCode a_code, double a_value, CompareType t = CompareType::kEqual) : value1{ a_value }, value2{ 0 }, code{ a_code }, _compare{ t } {}
		constexpr RefreshEvent(RefreshCode a_code, double a_value1, double a_value2, CompareType t = CompareType::kEqual) :
			value1{ a_value1 },
			value2{ a_value2 },
			code{ a_code },
			_compare{ t | CompareType::kSecondary } {}
	};

	struct RefreshData
	{
		double value;

		RefreshData(double v) : value{ v } {}
		RefreshData(const RE::BSFixedString& str) : value{ (double)Hash<HashFlags::Insensitive>(str.c_str(), str.length()) } {}
	};


	struct CustomMapping
	{
		struct cust_less
		{

			bool operator()(const RE::BSFixedString& lhs, const RE::BSFixedString& rhs) const
			{
				return lhs.data() < rhs.data();
			}

		};


		static void Create(const RE::BSFixedString& event_name, std::string_view a_filename, std::string_view a_category, std::optional<uint8_t> a_index)
		{
			//Please make this handle nothing if it doesn't warrant a handle

			//Check for it already existing here.

			CustomMapping& result = mappings[event_name];
			

			result.fileHash = std::hash<std::string_view>{}(a_filename);

			result._category = Hash<HashFlags::Insensitive>(a_category);

			auto& next = nextIndexMap[result.fileHash];

			result._id = next = a_index.value_or(next++);

			return;
		}

		inline static std::map<RE::BSFixedString, CustomMapping, cust_less> mappings;



		inline static std::unordered_map<uint64_t, uint8_t> nextIndexMap;

		static CustomMapping* Get(const RE::BSFixedString& str)
		{
			auto it = mappings.find(str);

			if (mappings.end() != it)
				return std::addressof(it->second);

			return nullptr;
		}

		uint64_t file() const
		{

			if (!this)
				return 0;

			return fileHash;
		}

		uint32_t id() const
		{

			if (!this)
				return 0;

			return _id;
		}

		uint32_t category() const
		{
			if (!this)
				return 0;

			return _category;
		}



		//I have no idea if I want to do this, but it'll be what I use if I ever make an in game editor. Probably not though.
		//inline static std::unordered_map<uint32_t, CustomMapping> tempMap;

		uint64_t fileHash;	//The hash of the file name this spawned from. Can be manually set to restore an existing config

		uint32_t _id;			//ID given or assigned within the file. Can be manually set to restore an existing config

		//This can be an editor data only object.
		//uint16_t priority;	//Determines the order of that the inputs are loaded in, also determining the order of secondary controls

		uint32_t _category;	//To be a string hash that determines what category it should belong to

		//If the tag of customevent is -1, this means that it has no custom mapping data.
		// Custom Mapping data is required if the control is either A



	};
	
	struct Break {
		Break()
		{
			//assert(false);
			//throw nullptr;
		}
	} inline breaker{};

	struct CustomEvent : public RE::UserEventMapping
	{



	public:
		uint32_t& tag()
		{
			return pad14;
		}

		CustomMapping* mapping() const
		{
			if (!IsCustomEvent() || !remappable)
				return nullptr;

			return CustomMapping::Get(eventID);
		}

		bool Compare(CustomEvent& other) const
		{
			//I'd like to expand on this at some point, making this the prefered method of sorting by supplanting the original
			return inputKey < other.inputKey;
		}

		bool IsSignature(uint64_t hash, uint8_t index) const
		{
			return mapping()->file() == hash && mapping()->id() == index;
		}

		bool CanRemapTo(uint64_t hash, uint8_t index) const
		{
			return IsCustomEvent() && remappable && IsSignature(hash, index);
		}



		bool IsCustomEvent() const
		{
			return indexInContext == -1;
		}


		static CustomEvent* CtorImpl2(CustomEvent* a_this)
		{
			logger::info("Hit");
			//Have to brought force this bit because it's uninitialized.
			reinterpret_cast<void*&>(a_this->eventID) = nullptr;
			a_this->inputKey = -1;
			a_this->modifier = -1;
			a_this->indexInContext = 0;
			a_this->remappable = false;
			a_this->linked = false;
			a_this->userEventGroupFlag = RE::UserEventFlag::kAll;
			return a_this;
		}


		static CustomEvent* CtorImpl(CustomEvent* a_this)
		{

			//Have to brought force this bit because it's uninitialized.
			reinterpret_cast<void*&>(a_this->eventID) = nullptr;
			a_this->inputKey = -1;
			a_this->modifier = -1;
			a_this->indexInContext = 0;
			a_this->remappable = false;
			a_this->linked = false;
			a_this->userEventGroupFlag = RE::UserEventFlag::kAll;
			return a_this;
		}

		//Not to be used unless it's in a constructor, either this or a hooked one.
		CustomEvent* Ctor()
		{
			return CtorImpl(this);
		}

		CustomEvent()
		{
			Ctor();
			indexInContext = -1;
		}

		CustomEvent(const RE::BSFixedString& event_name, std::string_view a_filename, std::string_view a_category, std::optional<uint8_t> a_index, bool remap)
		{
			//This needs some other settable data, such as remappable
			Ctor();
			eventID = event_name;
			indexInContext = -1;
			modifier = 0;

			if (remap) {
				remappable = true;
				CustomMapping::Create(eventID, a_filename, a_category, a_index);
			}
		}

	};
	static_assert(sizeof(CustomEvent) == sizeof(RE::UserEventMapping));
	//TODO: Static assert the offsets too. That's important.

	inline auto& operator~(RE::UserEventMapping& a_this) { return reinterpret_cast<CustomEvent&>(a_this); }
	inline auto& operator~(const RE::UserEventMapping& a_this) { return reinterpret_cast<const CustomEvent&>(a_this); }

	inline auto& operator~(RE::BSTArray<RE::UserEventMapping>& a_this) { return reinterpret_cast<RE::BSTArray<CustomEvent>&>(a_this); }
	inline auto& operator~(const RE::BSTArray<RE::UserEventMapping>& a_this) { return reinterpret_cast<const RE::BSTArray<CustomEvent>&>(a_this); }



	constexpr uint8_t controlMapVersion = 1;
	//I'll be storing this here because it's config independent. If I store it in data VFS's are going to snatch it up.
	constexpr const char* controlMapPath = "ControlMap_Dynamic.txt";


	//enum struct Refres


	//inline std::unordered_map<RefreshCode 


	struct InputState : public LayerMatrix
	{
		struct Instance : public LayerMatrix::Instance
		{
		public:


			InputState* GetBaseObject()
			{
				return static_cast<InputState*>(__super::GetBaseObject());
			}
			const InputState* GetBaseObject() const
			{
				return static_cast<const InputState*>(__super::GetBaseObject());
			}


			auto priority() const
			{
				return  GetBaseObject()->_priority;
			}


			auto level() const
			{
				return GetBaseObject()->level;
			}


			bool IsInConflict(Instance* other) const
			{
				return GetBaseObject()->IsInConflict(other->GetBaseObject());
			}


			void SetInputEnabled(Input input, bool value)
			{

				auto it = storage.find(input.hash());

				auto end = storage.end();

				if (it == end) {
					return;
				}

				for (auto& entry : it->second)
				{
					entry->SetEnabled(value);
				}

				for (auto parent : GetBaseObject()->parents)
				{
					parent->ObtainContextInstance(context)->SetInputEnabled(input, value);
				}
			}

			//This is called on all activeStates at the end of a successful update.
			void SetAllInputsEnabled(bool value = true)
			{
				for (auto& [key, lists] : storage)
				{
					for (auto& entry : lists)
					{
						entry->SetEnabled(value);
					}

				}

				for (auto parent : GetBaseObject()->parents)
				{
					parent->ObtainContextInstance(context)->SetAllInputsEnabled(value);
				}
			}

			bool CanInputPass(RE::InputEvent* event) const
			{
				return GetBaseObject()->CanInputPass(event);
			}

			bool DerivesFrom(Instance* other)
			{
				if (!other)
					return false;

				return GetBaseObject()->DerivesFrom(other->GetBaseObject());

			}


			std::vector<Instance*> GetViableStates(RefreshCode code, double data1, double data2, std::span<Instance*>& limit, InputState*& winner, bool& change, bool inner_change = false)
			{
				//TODO: GetViableStates needs to be changed pretty badly. It should only add "this" if it experiences complete success with it's parents.
				// or if it lacks parents. Basically, it must maintain the expectations of it's previous as well. That, or it must exist in the limit list.


				//It should be noted that having activated this frame is not grounds for 

				RefreshCode k_fakeUpdateCode = RefreshCode::Update;
				RefreshCode k_fakeExpectedCode = RefreshCode::Update;

				Instance* self = nullptr;

				auto end = limit.end();
				bool in_previous = std::find(limit.begin(), end, this) != end;

				if (!inner_change && GetBaseObject()->CheckEvent(code, data1, data2) == false) {
					//Given update either isn't within here or doesn't match parameters.

					//If it's not within the expected update but it's currently active, it will just put it in there.
					// I may actually just make a setting for this specifically to make it faster. For now, this will work.


					if (in_previous) {
						self = this;
					}
					else
						return {};
				}
				//The rule is that the states in question must remain above
				else {

					//What this actually should want to do is check the parents before checking itself so if it fails its children it doesn't take place.
					// But that won't be needed for a while even if this isn't a very smart way of doing this.
					if (GetBaseObject()->tmpCheckParentCondition() == true && GetBaseObject()->tmpCheckCondition() == true) {
						self = this;
					}


					if (self && !in_previous || !self && in_previous) {
						inner_change = change = true;

					}
					else {
						inner_change = false;
					}
				}



				if (self) {

					//Here a question about whether this should be viable based on the winner is cast forth.

					if (!winner) {
						winner = GetBaseObject();
						return { self };
					}
					else {
						bool collapse = GetBaseObject()->ShouldCollapse() || winner->ShouldSmash() && (!winner->IsInputRelevant() || winner->IsInConflict(GetBaseObject()));

						if (collapse)
							return {};
					}
				}



				//if (in_previous) {
				//	return {};
				//}



				std::vector<Instance*> result;


				for (auto parent : GetBaseObject()->parents)
				{
					auto add = parent->ObtainContextInstance(context)->GetViableStates(code, data1, data2, limit, winner, change, inner_change);

					if (add.size() != 0)
						result.append_range(add);
				}

				return result;
			}

			void LoadVisitorList(detail::VisitorList& list, Input input, Input control, InputState*& winner)
			{
				auto ctrl_it = storage.find(control.hash());
				auto input_it = storage.find(input.hash());

				auto end = storage.end();


				bool add = true;

				bool c_find = ctrl_it != end;
				bool i_find = input_it != end;

				auto settings = GetBaseObject();

				if  constexpr (0)
				{
					//I'm doing this real crude like for the visuals
					if (c_find || i_find)
					{
						//Do this bit first if you can, it'd be nice to have the searching not done if it's not viable.
						if (!winner) {
							//Winner only matters here if it does either of these things
							if (settings->ShouldSmother() || settings->ShouldSmash())
								winner = settings;
						}
						else
						{
							if (settings->ShouldCollapse() || winner->ShouldSmash())
							{
								return;
							}
						}


						if (c_find) {
							list.push_back(std::ref(ctrl_it->second));
							SetInputEnabled(control, true);
						}

						if (i_find) {
							list.push_back(std::ref(input_it->second));
							SetInputEnabled(input, true);
						}
					}
				}
				else
				{
					if (ctrl_it != end) {
						list.push_back(std::ref(ctrl_it->second));
						SetInputEnabled(control, true);
					}

					if (input_it != end) {
						list.push_back(std::ref(input_it->second));
						SetInputEnabled(input, true);
					}
				}


				//Basically if it's already taken, take it back
				InputState* _winner = winner;

				for (auto& parent : settings->parents)
				{
					parent->ObtainContextInstance(context)->LoadVisitorList(list, input, control, _winner);
				}


			}
		};


		InputState()
		{
			type = MatrixType::State;
		}

		Instance* GetContextInstance(RE::InputContextID context, bool create_if_required = false) override
		{
			return LoadInstance<Instance>(context, create_if_required);
		}




		static constexpr RefreshEvent genericUpdateEvent{ RefreshCode::Update, 0.0, CompareType::kGreaterOrEqual };



		//This is a matrix setting that creates the setting
		std::vector<InputState*> parents;

		StateLevel level = StateLevel::Smother;
		int16_t _priority = 1;
		tmp_Condition* condition = nullptr;

		std::string_view debugName;

		std::set<Input> conflictList;

		//If refresh events exist, then the default update is used.
		std::set<RefreshEvent> refreshEvents;


		bool CheckEvent(RefreshCode a_code, double data1, double data2)
		{
			//returns 1 if true, 0 if false, -1 if it fails the basic state update and doesn't have any entries.

			if (a_code == RefreshCode::Absolute) {
				return true;
			}


			if (refreshEvents.empty() && parents.empty() == true) {
				return genericUpdateEvent.CheckEvent(a_code, data1, data2);
			}


			for (auto& event : refreshEvents)
			{
				if (event.CheckEvent(a_code, data1, data2) == true)
				{
					return true;
				}
			}

			for (auto parent : parents)
			{
				if (parent->CheckEvent(a_code, data1, data2) == true)
				{
					return true;
				}
			}

			return false;
		}



		MatrixType GetMatrixType() const override { return MatrixType::State; }

		std::strong_ordering CompareOrder(const Matrix* o) const override
		{
			auto other = dynamic_cast<const InputState*>(o);

			if (DerivesFrom(other) == true) {
				return std::strong_ordering::greater;
			}

			if (other->DerivesFrom(this) == true) {
				return std::strong_ordering::less;
			}

			return std::strong_ordering::equal;
		}

		//If input is relevant to the below
		bool IsInputRelevant() const
		{
			switch (level)
			{
			case StateLevel::Smother:
			case StateLevel::Clobber:
				return true;

			default:
				return false;
			}
		}

		bool ShouldSmash() const
		{
			switch (level)
			{
			case StateLevel::Smash:
			case StateLevel::Clobber:
				return true;

			default:
				return false;
			}
		}

		//Collapse is the act of crumpling despite not being asked to.
		bool ShouldCollapse() const
		{
			switch (level)
			{
			case StateLevel::Smash:
			case StateLevel::Clobber:
			case StateLevel::Collapse:
				return true;

			default:
				return false;
			}
		}

		bool ShouldSmother() const
		{

			switch (level)
			{
			case StateLevel::Smash:
			case StateLevel::Clobber:
			case StateLevel::Smother:
				return true;

			default:
				return false;
			}
		}


		bool ShouldBeSmothered() const
		{
			return level != StateLevel::Merge;
		}




		void tmpBuildConflictList()
		{
			conflictList.clear();

			for (auto& command : commands)
			{
				for (auto& trigger : command.triggers)
				{
					conflictList.insert_range(trigger.GetInputs());
				}
			}

			for (auto parent : parents)
			{
				//This assumes that the previous has built it's own list.
				conflictList.insert_range(parent->conflictList);
			}
		}

		bool IsInConflict(const InputState* other) const
		{
			auto control_map = RE::ControlMap::GetSingleton();


			for (auto input : conflictList) {
				if (other->HasInput(input) == true)
					return true;
			}

			if (other->HasRawInputs() == true)
			{
				for (auto input : other->conflictList) {
					if (HasInputAsUserEvent(input) == true)
						return true;


					continue;

					//saving this just in case
					if (input.IsUserEvent() == true)
						continue;


					if (auto name = control_map->GetUserEventName(input.code, input.device); name.empty() == false) {
						//TODO: When custom controls exist, this will have to be done many times over to check for personal user events.
						Input ctrl = Input::CreateUserEvent(name);

						if (conflictList.contains(ctrl) == true)
							return true;
					}


				}
			}

			return false;
		}


		bool IsInConflict(const InputState* other, Input input) const
		{
			return HasInput(input) && other->HasInput(input);
		}


		//A bool will get flipped on if we even need to look for user events in the conflict list.
		bool HasRawInputs() const
		{
			//When a raw input is added to a state, a boolean flag will be ticked that will start the search from the other side.
			return true;
		}



		bool HasInputAsUserEvent(Input input) const
		{
			if (input.IsUserEvent() == false) {
				auto control_map = RE::ControlMap::GetSingleton();

				if (auto name = control_map->GetUserEventName(input.code, input.device); name.empty() == false) {
					//TODO: When custom controls exist, this will have to be done many times over to check for personal user events.
					Input ctrl = Input::CreateUserEvent(name);

					if (conflictList.contains(ctrl) == true)
						return true;
				}
			}

			return false;
		}


		bool HasInput(Input input) const
		{
			if (conflictList.contains(input) == true)
				return true;

			return HasInputAsUserEvent(input);
		}

		bool IsInConflict(const InputState& other)
		{
			return IsInConflict(&other);
		}


		bool DerivesFrom(const InputState* other) const
		{
			if (!other)
				return false;

			if (this == other)
				return true;

			for (auto parent : parents)
			{
				if (parent->DerivesFrom(other) == true)
					return true;
			}

			return false;

		}

		bool CanInputPass(RE::InputEvent* event) const override
		{
			auto prev = __super::CanInputPass(event);

			if (prev)
			{
				for (auto parent : parents)
				{
					if (parent->CanInputPass(event) == false) {
						return false;
					}
				}
			}

			return prev;
		}

		bool tmpCheckCondition()
		{
			if (condition)
				return condition(RE::PlayerCharacter::GetSingleton());

			return true;
		}

		bool tmpCheckParentCondition()
		{
			for (auto parent : parents)
			{
				if (!parent->tmpCheckCondition() || !parent->tmpCheckParentCondition()) {
					return false;
				}
			}

			return true;
		}

		int16_t priority() const
		{
			return _priority;
		}
	};

	struct StateMap;

	struct StateManager
	{
		//TODO: StateManager doesn't quite respect the idea of different contexts. this needs to be explored at some point.
		// Perhaps I can make it so access to a state manager is tied to a context. Some states might be able to exist in
		// multiple places, but they'd have to be copies.



		//Manager doesn't quite represent what this is. A "default" state basically controls this.
		// So basically one for menu, one for gameplay, maybe other states. The gist is this is a maker.







		std::vector<std::unique_ptr<InputState>> storage;//The purpose of this is basically just storage

		//std::vector<StateMap*> record;//Every state map it's ever lended to. May not be required.

		std::vector<InputState*> headers;

		InputState* CreateState(std::vector<InputState*> parents = {})
		{
			auto& result = storage.emplace_back(new InputState);

			result->parents = parents;

			for (const auto& par : parents) {
				headers.erase(std::remove(headers.begin(), headers.end(), par), headers.end());
			}

			
			//headers.push_back(result.get());
			
			//I have no idea if what's placed will actually adhere to order, so it's best to do it here or whatever
			headers.insert(std::upper_bound(headers.begin(), headers.end(), result.get(), [](const auto& lhs, const auto& rhs)
					{return lhs->priority() > rhs->priority(); }),
					result.get());

			return result.get();
		}


		StateMap CreateMap();
	};

	struct StateMap//This carries the unique pointers of active states
	{
		
		//These are the childless active states, who collectively maintain the existence
		//std::vector<std::unique_ptr<EntryState>> headers;
		
		//I'm unsure if this should be shared or not. The unique pointers existing makes sure these will not die, so I guess
		// this is good.

		StateManager* manager = nullptr;
		

		//The active state notes which entry it comes from
		std::vector<InputState::Instance*> activeStates;


		uint32_t updateTimestamp = 0;
		//Active states

		//TODO: State map should come with a list of inputs (similar to the whole block thing) that it wants destroyed.
		//It should be updated every time a state changes, and cleared once the changes are accepted. 
		// this system should exist for the purposes

	


		void Update(RefreshCode code, RefreshData data1, RefreshData data2 = 0)
		{
			constexpr auto context = RE::InputContextID::kGameplay;

			std::span<InputState::Instance*> limits{ activeStates };

			//This should only be made once we're sure that there's actually even gonna be an update.
			// To this, each time the real update is called, it will be associated with an update event. If one of those update event's match the code given,
			// it will attempt to update.

			//Correction, if it's within the sp
			std::vector<InputState::Instance*> updateList;


			InputState* winner = nullptr;

			bool change = false;

			//I think while updating, if the headers don't say they want to update, we just take a span of their data, and add it into the span.
			for (int i = 0; i < manager->headers.size(); i++)
			{
				auto header = manager->headers[i];
				//How do we handle updates?
 				//First, we get the 
				//if ()
				updateList.append_range(header->ObtainContextInstance(context)->GetViableStates(code, data1.value, data2.value, limits, winner, change));
			}

			//if the event has done nothing, forgo all this.
			if (!change)
				return;

			std::unordered_set<InputState::Instance*> seen;

			auto it = std::remove_if(
				updateList.begin(), updateList.end(),
				[&seen](InputState::Instance* value) {
					return !seen.emplace(value).second;
				});


			//This does not fucking work btw, we need to keep sorted to priority.
			//std::sort(updateList.begin(), updateList.end());
			//auto it = std::unique(updateList.begin(), updateList.end());
			updateList.erase(it, updateList.end());

			//This sort needs to happen because if a state has parents that get submitted instead it should not have those be higher or lower than due.
			std::sort(updateList.begin(), updateList.end(), [](auto& lhs, auto& rhs) {return lhs->priority() > rhs->priority(); });

			activeStates = updateList;

			for (auto state : activeStates){
				state->SetAllInputsEnabled(false);
			}
		}

		void CheckUpdate()
		{
			bool absolute = !updateTimestamp && activeStates.empty();

			if (absolute || SecondsSinceLastUpdate(updateTimestamp) >= Settings::updateTime) {
				Update(absolute ? RefreshCode::Absolute : RefreshCode::Update, updateTimestamp);//We are updating under the event of OnTick.
				updateTimestamp = RE::GetDurationOfApplicationRunTime();
				
			}
		}

		bool PrepVisitorList(detail::VisitorList& list, RE::InputEvent* event, Input input, Input control)
		{
			CheckUpdate();
			
			bool result = true;

			//Now that I think about it, the way this is set up, any state that can run

			InputState* winner = nullptr;

			for (auto state : activeStates)
			{
				//If we aren't collecting, we're just shutting down, primarily so multipress commands don't end up causing undue waiters that may jam functionality

				state->LoadVisitorList(list, input, control, winner);


				if (state->CanInputPass(event) == false)
					result = false;
			}

			return result;
		}

	};


	inline StateMap StateManager::CreateMap()
	{
		StateMap result;
		result.manager = this;
		
		//Here you'd pull the proper manager from a context list or something.

		return result;
	}



	namespace
	{
		std::unordered_map<Input::Hash, std::vector<std::string>> tmp_Controls;

		std::vector<std::string>* GetControls(Input input)
		{
			//This shit kinda temp ngl
			return nullptr;
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




	inline void ChangeMeSlowly(EventData&& data, EventFlag& flags, bool& result, const Argument& param1, const Argument& param2)
	{
		auto axis = data.event.GetAxis();

		auto player = RE::PlayerCharacter::GetSingleton()->AsActorValueOwner();

		player->RestoreActorValue(RE::ACTOR_VALUE_MODIFIER::kDamage,
			RE::ActorValue::kStamina, 30 * axis.x * RE::GetSecondsSinceLastFrame());

		player->RestoreActorValue(RE::ACTOR_VALUE_MODIFIER::kDamage,
			RE::ActorValue::kMagicka, 30 * axis.y * RE::GetSecondsSinceLastFrame());

		if (data.stage == EventStage::Finish)
			RE::DebugMessageBox("Finished change");
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

		uint16_t highestRank = 0;

		std::vector<ActiveCommand> sharedCommands;

		//At a later point, make this a pointer. Not all commands would ever need this.
		// I have no idea if I may use a ref later.
		//std::unordered_map<ActiveCommand::ID, uint16_t> delayCommands;
		//If I could make some sort of struct that default creates a targeted object when accessed, that would be cool.
		std::set<ActiveCommand::ID> delayCommands;


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
			highestRank = 0;
		}

		void FillBlockStages()
		{
			ClearBlockStages();

			//This prevents other items of precedence from taking place.
			//EventStage reqs = EventStage::None;

			//Shouldn't this be searching delay commands?
			for (auto& command : sharedCommands)
			{
				//if (reqs == EventStage::All)
				//	continue;

				if (command.IsRunning() && command->ShouldBlockTriggers() == true)
					EmplaceBlockStages(command);
				
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
			auto rank = cmd->Rank();
			if (pred > highestPrecedence || pred == highestPrecedence && rank > highestRank){
				ClearBlockStages();
				highestPrecedence = pred;
				highestRank = rank;
			}
			else if (pred < highestPrecedence || rank < highestRank) {
				return false;
			}

			auto emplace_array = blockCommands;

			auto stages = cmd->GetBlockingFilter();

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
		ActiveCommand& AddCommand(ActiveCommand& command, EventStage stage)
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

			//I just realized ordering this like this might make higher layers not handle properly.
			// Still gonna keep this around in case I need to remember it exists.
			
			//auto& result = *sharedCommands.insert(std::upper_bound(sharedCommands.begin(), sharedCommands.end(), command, [](const auto& lhs, const auto& rhs)
			//	{return lhs->priority() > rhs->priority(); }),
			//	std::move(command));

			auto& result = sharedCommands.emplace_back(std::move(command));


			//if (result->IsDelayable() == true) {
			if (auto delay_state = result->GetDelayState(data, stage); delay_state != DelayState::None) {
				//TODO: This may have issues suppressing other non-delayed commands. I think should be added regardless if it's actually running or not, ...
				// otherwise it may not suppress events it my normally depending on the order of inputs
				if (delay_state == DelayState::Advancing)
					result->SetDelayed(true);
				
				//Do this is precedence is greater than 1
				if (result->UsesPrecedence() == true)
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
					//if (other.id() == act.id())
						continue;

					if (act.entry->tmpname_ShouldWaitOnMe(*other.entry) == true) {
						other.tempname_IncWaiters();

						RemoveBlockStages(other);
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


							//EmplaceCommand(other.entry, stage, dump, &other);
							UpdateCommand(other, stage);
						}


					}
				}

				//A good check is if I actually find this within the thing.
				//assert(found_self);

				act.SetDelayUndone();
			}
			act.Deactivate();
		}


		//TODO: Need a function to fail a command, which clears out the waiters.

	

		void UpdateCommand(ActiveCommand& act, EventStage stage)

		{//TODO: Make a different version of emplace function. I don't want this option exposed.


			DelayState delay_state = act->GetDelayState(data, stage);

			
			//if (act.entry->IsDelayable() == true)

			if (delay_state != DelayState::None)
			{
				if (act.IsFailing() == true) {
					delay_state = DelayState::Failure;
				}

				if (delay_state == DelayState::Success) {
					//While this delay condition is off, it needs to be running by this point.
					act->SetDelayed(false);
					//DelayState t = (DelayState)-1;
				}
				else
				{
					bool is_running = act.IsRunning();

					if (!is_running && stage == EventStage::Finish) {
						act.Deactivate();
						delay_state = DelayState::Failure;
					}


					if (!is_running && !act.IsDelayUndone() && delay_state == DelayState::Failure)
					{
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


			//To the blocking features here, I'd like to disable them at a later point, handle this business elsewhere. Namble


			//*NOTE: I guess I can probably handle this later, but this seems to not care about precedence so I'll just add the things anyhow
			//if (cmd->ShouldBeBlocked() && IsStagesBlocked(cmd->GetTriggerFilter()) == true) {
			//	return;
			//}
			

			ActiveCommand act{ cmd };

			
			if (cmd->ShouldBlockTriggers() && EmplaceBlockStages(act) == false)
				return;

			AddCommand(act, stage);
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

				it = std::find_if(it, end, [id](const ActiveCommand& entry) {return entry.id() == id; });

				if (it != end)
					return std::addressof(*it);
			}
			
			return nullptr;
		}
		const ActiveCommand* GetCommandFromID(ActiveCommand::ID id) const 
		{
			if (id) {
				auto it = sharedCommands.begin();
				auto end = sharedCommands.end();

				it = std::find_if(it, end, [id](const ActiveCommand& entry) {return entry.id() == id; });

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

		inline bool IsStagesBlocked(EventStage stage, MatrixType type = MatrixType::Total) const
		{
			if (stage)
			{
				for (auto x = 1, y = 0; x < EventStage::Total; x <<= 1, y++)
				{
					if (stage & x && blockCommands[y]) {
						//if (type != MatrixType::Total)
						//{
						//	auto command = GetCommandFromID(blockCommands[y]);
						//	if (!command || command->entry->)
						//}

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


		std::array<ActiveCommand*, EventStage::Total> GetAllBlockCommands()
		{
			std::array<ActiveCommand*, EventStage::Total> result;

			for (int i = 0; i < EventStage::Total; i++)
			{
				auto command = GetCommandFromID(blockCommands[i]);

				result[i] = command;
			}

			return result;
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
		bool IsStageBlocked(EventStage stage)
		{
			auto inch = std::countr_zero(std::to_underlying(stage));
			
			
			return blockCommands[inch];
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
				previous_value = IsStageBlocked(stage);//This has no need to ask for not trigger. it will already do non-triggers.
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
			assert(!h.IsUserEvent());

			if (h.IsUserEvent() == true)
				throw std::exception("Cannot obtain active input for control");

			auto it = map.find(h);
			auto end = map.end();

			if (it != end) {
				input = it->second.get();
			}

		}

	};

	inline LayerMatrix* testMode = new LayerMatrix;





	inline StateManager testManager;







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

		StateMap stateMap;

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


		std::vector<InputMode::Instance*> modeMaps;//The last mode is most important.

		RE::InputContextID context;


		void EmplaceMode(InputMode* mode, CommandEntryPtr& command, bool strengthen) 
		{
			auto it = modeMaps.begin();
			auto end = modeMaps.end();

			it = std::find_if(it, end, [&](InputMode::Instance* i) {return i->source == command && i->GetBaseObject() == mode; });

			if (it == end) {
				auto& config = modeMaps.emplace_back(mode->ObtainContextInstance(k_gameplayContext));
				config->source = command;
				config->isStrong = strengthen;

				return;
			}

			if (strengthen)//This needs to do other things than just declare strength BTW, but this works for now.
				(*it)->isStrong = true;

		}

		void RemoveMode(LayerMatrix* mode, CommandEntryPtr& command)
		{
			auto it = modeMaps.begin();
			auto end = modeMaps.end();

			it = std::find_if(it, end, [&](InputMode::Instance* i) {return i->source == command && i->GetBaseObject() == mode; });

			if (it != end) {
				modeMaps.erase(it);
			}
		}

		InputMode::Instance* GetCurrentMode()
		{
			return modeMaps.size() ? modeMaps.back() : nullptr;
		}





		bool PrepVisitorList(detail::VisitorList& list, RE::InputEvent* event, Input input, Input control, MatrixType type)
		{
			//Returns if it should be allowed to continue on down the line.

			//std::vector<CommandEntryPtr>& 
			CommandMap* map;
			Matrix* matrix = nullptr;

			switch (type)
			{
			case MatrixType::Selected:

			default:
				return true;
			
			case MatrixType::State:
				return stateMap.PrepVisitorList(list, event, input, control);

			case MatrixType::Mode: {
				InputMode::Instance* mode = GetCurrentMode();
				if (!mode) {
					return true;
				}

				map = &mode->storage;
				matrix = mode->GetBaseObject();

				break;
			}
			case MatrixType::Dynamic:
				map = &dynamicMap;
				break;
			}

			if (!map)
				return true;
			

			auto ctrl_it = map->find(control.hash());
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
				commandA->parent = testMode;
				commandB->parent = testMode;

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
			trigger6.conflict = ConflictLevel::Following;
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
			//trigger4.type = TriggerType::OnButton;
			trigger4.conflict = ConflictLevel::Defending;
			trigger4.stageFilter = EventStage::StartFinish;
			trigger4.args.emplace_back(std::make_unique<Argument[]>(1))[OnControl::CONTROL_ID] = "Left Attack/Block";
			trigger4.args.emplace_back(std::make_unique<Argument[]>(1))[OnControl::CONTROL_ID] = "Right Attack/Block";
			//trigger4.args.emplace_back(std::make_unique<Argument[]>(1))[OnButton::BUTTON_ID] = Input{ RE::INPUT_DEVICE::kMouse, 0 };
			//trigger4.args.emplace_back(std::make_unique<Argument[]>(1))[OnButton::BUTTON_ID] = Input{ RE::INPUT_DEVICE::kMouse, 1 };



			InputCommand* command7 = new InputCommand;


			auto& action7 = command7->actions.emplace_back();
			action7.type = ActionType::InvokeFunction;
			auto& args7 = action7.args = std::make_unique<Argument[]>(3);
			args7[InvokeFunction::FUNCTION_PTR] = ChangeMeSlowly;
			args7[InvokeFunction::CUST_PARAM_1] = 10;

			auto& trigger7 = command7->triggers.emplace_back();
			trigger7.priority = 10;
			//trigger7.type = TriggerType::OnMouseMove;
			trigger7.type = TriggerType::OnAxis;
			trigger7.conflict = ConflictLevel::Defending;
			trigger7.stageFilter = EventStage::All;
			//Needs no arguments, can really only activate here.
			trigger7.args.emplace_back(std::make_unique<Argument[]>(1))[OnAxis::AXIS_ID] = "Move";

			std::vector<InputCommand*> something{ command1, command2, command3, command4, command5, command6, command7 };




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

			//*
			InputCommand* command2621 = new InputCommand; {
				auto& action2620 = command2621->actions.emplace_back();
				action2620.type = ActionType::InvokeFunction;
				auto& args2620 = action2620.args = std::make_unique<Argument[]>(3);
				args2620[InvokeFunction::FUNCTION_PTR] = tmp_YELL_a_nameNEW;
				args2620[InvokeFunction::CUST_PARAM_1] = 220;


				auto& trigger2620 = command2621->triggers.emplace_back();
				trigger2620.priority = 20;
				trigger2620.type = TriggerType::OnControl;
				trigger2620.conflict = ConflictLevel::Blocking;
				trigger2620.stageFilter = EventStage::Start;
				trigger2620.args.emplace_back(std::make_unique<Argument[]>(1))[OnControl::CONTROL_ID] = "Left Attack/Block";
				//trigger2620.canPendCommands = false;
				auto& delayArgs = trigger2620.delayArgs = std::make_unique<Argument[]>(1);
				delayArgs[OnControl::HOLD_TIME] = 1.5f;
				something.push_back(command2621);
			};
			//*/


		
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
			if (input.IsUserEvent() == true)
				throw std::exception("Cannot obtain active input for control");

			auto& ptr = activeMap[input];

			if (!ptr)
				ptr = std::make_unique<ActiveInput>();

			return *ptr;
		}

		//Takes input because it is genuinely easier to do it like this.
		bool ClearActiveInput(Input input)
		{
			if (input.IsUserEvent() == true)
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

		void CheckRelease(RE::PlayerControls* controls, InputInterface& event, Input input)
		{
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

			HandleEvent(controls, event);

			CommandEntry::DecStaticTimestamp();

			event.SetEventValues(pair.first, pair.second);
		}



		bool IsLayerBlocked(std::array<ActiveCommand*, EventStage::Total> blocks, ActiveCommand& other, EventStage stages)
		{
			//Ease of use function so I don't have to search for the active command every time

			if (stages)
			{
				for (auto x = 1, y = 0; x < EventStage::Total; x <<= 1, y++)
				{
					auto block = blocks[y];

					if (!block)
						continue;

					if (stages & x && 
						block != &other && 
						block->entry->command->CompareOrder(other->command)._Value >= std::strong_ordering::equal._Value
						//blocks[y]->entry->GetParentType() <= other->GetParentType()
						) {
						return true;
					}

				}
			}
			return false;
		}



		bool HandleEvent(RE::PlayerControls* controls, InputInterface event)
		{
			//HandleEvent is basically the core driving function. It takes the player controls, and the given input event, as well as an
			// InputEvent that is mutated to be refired.
			// it will return true if it intends to fire the original function, and will do so with the out function if present.
			// if it's true and the out function isn't there, it will fire as normal.

			//I think I'll put the interface object here, along with the event data.

			
			EventStage stage = event.GetEventStage();

			if (stage == EventStage::None)
				return true;


			Input input = Input::CreateInput(event);
			Input control = Input::CreateUserEvent(event);

			//This isn't good to use, stop using it.
			if (stage == EventStage::Start) {
				CheckRelease(controls, event, input);
			}



			ActiveInputHandle active_input{ input, activeMap, event.GetEventValues() };


			bool allow_execute = true;


			

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
					//TODO: If I can make PrepVisitorList be able to visit lists SEPERATELY for each MatrixType that would be ideal.
					// The current issue with this machination is states they still have plenty of data that can't be visited one at a time.
					// actually this isn't quite true. If I just packed the function for VisitingLists in there I could accomplish this.
					if (PrepVisitorList(list, event.event, input, control, i) == false) {
						//If one of these blocks further inputs, it needs an active command as a reminder.
						active_input->SetBasicFailure();
						break;
					}
				}

				
				VisitLists([&](CommandEntryPtr entry, bool& should_continue)
				{
					if (entry->GetRealInputRef() > 1) {
						entry->IsActive();
					}

					if (!entry->IsActive() && entry->IsEnabled() && entry->CanHandleEvent(event) == true) {
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
				std::array<ActiveCommand*, EventStage::Total> blocks = active_input->GetAllBlockCommands();


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

						if (!act.entry->ShouldBeBlocked() || IsLayerBlocked(blocks, act, trigger_stages) == false)
						{
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


									{

										EventData data{ this, act.entry, &active_input->data, event, i };

										act.entry->RepeatExecute(data, flags, executed);
									}
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


							ExecuteInput(controls, event);
							
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

		void ReleaseInput(RE::PlayerControls* a_controls, decltype(activeMap)::iterator& it)
		{

			VirtualEvent virtual_input;

			//InputInterface event{ button.get(), a_controls };
			InputInterface event{ &virtual_input };


			bool purge = true;

			auto dump = it->first;
			auto& active = it->second;

			EventFlag flags = EventFlag::None;

			Input input = dump;

			virtual_input.device = input.device;
			virtual_input.heldDownSecs = active->data.SecondsHeld();

			//button->device = input.device;
			//button->heldDownSecs = active->data.SecondsHeld();

			auto blocks = active->GetAllBlockCommands();

			active->VisitActiveCommands([&](ActiveCommand& act)
				{
					if (act->GetTriggerFilter() & EventStage::Finish)
					{
						if (!act.entry->ShouldBeBlocked() || IsLayerBlocked(blocks, act, EventStage::Finish) == false)
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
								logger::info("Retaining at success level {} {:X}", act->GetSuccess(), (uintptr_t)act.entry.get());
								return;
							}
							logger::info("Releasing at success level {} {:X}", act->GetSuccess(), (uintptr_t)act.entry.get());


							//As before but even more so, I'm REALLY digging the idea of putting this in active inputs.
							// Doing so would prevent these from ever forming as an activeCommand, and thus cutdown on the amount of
							// computing needed to process things that will never come into success.

							//if (!active->IsStageBlockedHashed(block_, hash, act.id(), EventStage::Finish) || act.entry->ShouldBeBlocked() == false) 
							{
								//if (!block_ || act.entry->ShouldBeBlocked() == false) {

								EventData data{ this, act.entry, &active->data, event, EventStage::Finish };

								if (!act.entry->Execute(data, flags)) {
									//This should do something, but currently I'm unsure what exactly.
									//allow_execute
								}
							}
						}
					}
				});


			if (!purge || ClearActiveInput(it) == false) {
				++it;
			}
		}

		void HandleRelease(RE::PlayerControls* a_controls)
		{
			//Emplaces can be handled here. Please handle them.
			
			//TODO: HandleRelease has a small issue in that when it happens it happens regardless if it's actually been updated or not.

			bool block_;

			size_t hash = 0;

			std::unique_ptr<RE::ButtonEvent> button{ RE::ButtonEvent::Create(RE::INPUT_DEVICES::kNone, "", 0, 0, 0) };

			VirtualEvent virtual_input;

			//InputInterface event{ button.get(), a_controls };
			InputInterface event{ &virtual_input };


			//PLEASE note, delay event refiring cannot happen here as proper, because emplace command is not happening. So, 
			// I need to divide that function in such a way that I can use it's components.

			for (auto it = activeMap.begin(); it != activeMap.end(); ) {
				ReleaseInput(a_controls, it);
				continue;
				
				bool purge = true;
				
				auto dump = it->first;
				auto& active = it->second;
				
				EventFlag flags = EventFlag::None;

				Input input = dump;

				virtual_input.device = input.device;
				virtual_input.heldDownSecs = active->data.SecondsHeld();

				//button->device = input.device;
				//button->heldDownSecs = active->data.SecondsHeld();

				auto blocks = active->GetAllBlockCommands();

				active->VisitActiveCommands([&](ActiveCommand& act)
				{
					if (act->GetTriggerFilter() & EventStage::Finish)
					{
						if (!act.entry->ShouldBeBlocked() || IsLayerBlocked(blocks, act, EventStage::Finish) == false)
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
								logger::info("Retaining at success level {} {:X}", act->GetSuccess(), (uintptr_t)act.entry.get());
								return;
							}
							logger::info("Releasing at success level {} {:X}", act->GetSuccess(), (uintptr_t)act.entry.get());


							//As before but even more so, I'm REALLY digging the idea of putting this in active inputs.
							// Doing so would prevent these from ever forming as an activeCommand, and thus cutdown on the amount of
							// computing needed to process things that will never come into success.

							{
								EventData data{ this, act.entry, &active->data, event, EventStage::Finish };

								if (!act.entry->Execute(data, flags)) {
									//This should do something, but currently I'm unsure what exactly.
									//allow_execute
								}
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



		void QueueAxisRelease()
		{//I want to make a release version just for Axis so I can save myself the trouble. For now? fuck it.

			auto control_map = RE::ControlMap::GetSingleton();

			//I just remembered, we don't actually care about the control, it's the input we're trying to clear.
			constexpr Input axisInputs[]{
				{ RE::INPUT_DEVICE::kMouse, 0xA }, //mouseMove
				{ RE::INPUT_DEVICE::kGamepad, 0xB },//thumbMoveL
				{ RE::INPUT_DEVICE::kGamepad, 0xC },//thumbMoveR
			};
			
			for (auto input : axisInputs)
			{
				auto end = activeMap.end();
				auto it = activeMap.find(input);

				if (it == end)
					continue;

				auto& active = it->second;

				active->VisitActiveCommands([&](ActiveCommand& act)
					{
						//This actually needs to work for the individual active entry.

						//For each entry active, try to mark it for release.

						//act->ResetExecute();
						act.SetEarlyExit(true);
					});
			}
		}

		void ReleaseAxis(RE::PlayerControls* a_controls)
		{//I want to make a release version just for Axis so I can save myself the trouble. For now? fuck it.

			//TODO: Generalize this pls
			constexpr Input axisInputs[]{
				{ RE::INPUT_DEVICE::kMouse, 0xA }, //mouseMove
				{ RE::INPUT_DEVICE::kGamepad, 0xB },//thumbMoveL
				{ RE::INPUT_DEVICE::kGamepad, 0xC },//thumbMoveR
			};

			for (auto input : axisInputs)
			{
				auto end = activeMap.end();
				auto it = activeMap.find(input);

				if (it == end)
					continue;

				ReleaseInput(a_controls, it);
			}
		}


		//The axis release version of Handle Release should have a "ReleaseInput" function, it should use the iterator to do it's business. Also being
		// able to push it.

	};
	

	inline MatrixController* testController = new MatrixController;
	inline std::array<MatrixController*, (int)RE::InputContextID::kTotal> Controllers;


	inline void LoadTestManager()
	{
		auto isCrouched = [](RE::PlayerCharacter* player)
			{
				return player->AsActorState()->IsSneaking();
			};

		auto isArmed = [](RE::PlayerCharacter* player)
			{
				return player->AsActorState()->IsWeaponDrawn();
			};

		//Armed is a bit jank because of the disonance between the events I'm using and the actual states being queried
		auto armedEvent1 = RefreshEvent{ RefreshCode::GraphOutputEvent, "WeapEquip_Out"_ih };
		auto armedEvent2 = RefreshEvent{ RefreshCode::GraphOutputEvent, "Unequip_Out"_ih };

		
		
		auto crouchEvent1 = RefreshEvent{ RefreshCode::GraphOutputEvent, "tailSneakIdle"_ih };
		auto crouchEvent2 = RefreshEvent{ RefreshCode::GraphOutputEvent, "tailSneakLocomotion"_ih };
		auto crouchEvent3 = RefreshEvent{ RefreshCode::GraphOutputEvent, "tailMTIdle"_ih };
		auto crouchEvent4 = RefreshEvent{ RefreshCode::GraphOutputEvent, "tailMTLocomotion"_ih };
		auto crouchEvent5 = RefreshEvent{ RefreshCode::GraphOutputEvent, "tailCombatIdle"_ih };
		auto crouchEvent6 = RefreshEvent{ RefreshCode::GraphOutputEvent, "tailCombatLocomotion"_ih };
		

		auto stateArmed = testManager.CreateState();
		{
			stateArmed->refreshEvents = {
				armedEvent1,
				armedEvent2,
			};

			stateArmed->condition = isArmed;
			stateArmed->debugName = "Armed";
			stateArmed->_priority = 27;
			stateArmed->level = StateLevel::Smother;
			stateArmed->blockedInputs = {
					Input { RE::INPUT_DEVICE::kNone, "Jump"_h },
			};

			stateArmed->commands.reserve(10);


			/*
			
			InputCommand* commandA = &stateArmed->commands.emplace_back();
			commandA->parent = stateArmed;

			auto& actionA = commandA->actions.emplace_back();
			actionA.type = ActionType::InvokeFunction;
			auto& argsA = actionA.args = std::make_unique<Argument[]>(3);
			argsA[InvokeFunction::FUNCTION_PTR] = tmp_YELL_a_nameNEW;
			argsA[InvokeFunction::CUST_PARAM_1] = 26;

			auto& triggerA = commandA->triggers.emplace_back();
			triggerA.priority = 26;
			triggerA.type = TriggerType::OnControl;
			triggerA.conflict = ConflictLevel::Defending;
			triggerA.stageFilter = EventStage::Start;
			triggerA.args.emplace_back(std::make_unique<Argument[]>(1))[OnControl::CONTROL_ID] = "Jump";
			//*/


			InputCommand* commandB = &stateArmed->commands.emplace_back();

			commandB->parent = stateArmed;

			auto& actionB = commandB->actions.emplace_back();
			actionB.type = ActionType::InvokeFunction;
			auto& argsB = actionB.args = std::make_unique<Argument[]>(3);
			argsB[InvokeFunction::FUNCTION_PTR] = tmp_YELL_a_nameNEW;
			argsB[InvokeFunction::CUST_PARAM_1] = 16;

			auto& triggerB = commandB->triggers.emplace_back();
			triggerB.priority = 16;
			triggerB.type = TriggerType::OnControl;
			triggerB.conflict = ConflictLevel::Blocking;
			triggerB.stageFilter = EventStage::Start;
			triggerB.args.emplace_back(std::make_unique<Argument[]>(1))[OnControl::CONTROL_ID] = "Left Attack/Block";


		}


		auto stateCrouch = testManager.CreateState();
		{
			stateCrouch->refreshEvents = {
				crouchEvent1,
				crouchEvent2,
				crouchEvent3,
				crouchEvent4,
				crouchEvent5,
				crouchEvent6,
			};

			stateCrouch->condition = isCrouched;
			stateCrouch->debugName = "Crouching";
			stateCrouch->_priority = 26;
			stateCrouch->blockedInputs;
			stateCrouch->blockedInputs = {
					Input { RE::INPUT_DEVICE::kNone, "Jump"_h },
			};

			stateCrouch->commands.reserve(10);

			InputCommand* commandA = &stateCrouch->commands.emplace_back();
			commandA->parent = stateCrouch;



			auto& actionA = commandA->actions.emplace_back();
			actionA.type = ActionType::InvokeFunction;
			auto& argsA = actionA.args = std::make_unique<Argument[]>(3);
			argsA[InvokeFunction::FUNCTION_PTR] = tmp_YELL_a_nameNEW;
			argsA[InvokeFunction::CUST_PARAM_1] = 26;

			auto& triggerA = commandA->triggers.emplace_back();
			triggerA.priority = 26;
			triggerA.type = TriggerType::OnControl;
			triggerA.conflict = ConflictLevel::Defending;
			triggerA.stageFilter = EventStage::Start;
			triggerA.args.emplace_back(std::make_unique<Argument[]>(1))[OnControl::CONTROL_ID] = "TEST_B";

			InputCommand* commandB = &stateCrouch->commands.emplace_back();
			
			commandB->parent = stateCrouch;



			auto& actionB = commandB->actions.emplace_back();
			actionB.type = ActionType::InvokeFunction;
			auto& argsB = actionB.args = std::make_unique<Argument[]>(3);
			argsB[InvokeFunction::FUNCTION_PTR] = tmp_say_a_nameNEW;
			argsB[InvokeFunction::CUST_PARAM_1] = 18;

			auto& triggerB = commandB->triggers.emplace_back();
			triggerB.priority = 18;
			triggerB.type = TriggerType::OnControl;
			triggerB.conflict = ConflictLevel::Blocking;
			triggerB.stageFilter = EventStage::Start;
			triggerB.args.emplace_back(std::make_unique<Argument[]>(1))[OnControl::CONTROL_ID] = "Left Attack/Block";
		}

		InputState* stateArmedAndCrouching = testManager.CreateState({ stateArmed, stateCrouch });
		if (stateArmedAndCrouching)
		{


			stateArmedAndCrouching->condition = nullptr;
			stateArmedAndCrouching->debugName = "ArmedAndDangerous";
			stateArmedAndCrouching->_priority = 27;
			stateArmedAndCrouching->level = StateLevel::Merge;
			//stateArmedAndCrouching->blockedInputs = {
			//		Input { RE::INPUT_DEVICE::kNone, "Jump"_h },
			//};

			stateArmedAndCrouching->commands.reserve(10);

			//InputCommand* commandA = &stateArmedAndCrouching->commands.emplace_back();
			//InputCommand* commandB = &stateCrouch->commands.emplace_back();


			/*
			
			commandA->parent = stateArmedAndCrouching;

			auto& actionA = commandA->actions.emplace_back();
			actionA.type = ActionType::InvokeFunction;
			auto& argsA = actionA.args = std::make_unique<Argument[]>(3);
			argsA[InvokeFunction::FUNCTION_PTR] = tmp_YELL_a_nameNEW;
			argsA[InvokeFunction::CUST_PARAM_1] = 26;

			auto& triggerA = commandA->triggers.emplace_back();
			triggerA.priority = 26;
			triggerA.type = TriggerType::OnControl;
			triggerA.conflict = ConflictLevel::Defending;
			triggerA.stageFilter = EventStage::Start;
			triggerA.args.emplace_back(std::make_unique<Argument[]>(1))[OnControl::CONTROL_ID] = "Jump";
			//*/


			InputCommand* commandB = &stateArmedAndCrouching->commands.emplace_back();


			commandB->parent = stateArmedAndCrouching;

			auto& actionB = commandB->actions.emplace_back();
			actionB.type = ActionType::InvokeFunction;
			auto& argsB = actionB.args = std::make_unique<Argument[]>(3);
			argsB[InvokeFunction::FUNCTION_PTR] = tmp_YELL_a_nameNEW;
			argsB[InvokeFunction::CUST_PARAM_1] = 14;

			auto& triggerB = commandB->triggers.emplace_back();
			triggerB.priority = 14;
			triggerB.type = TriggerType::OnControl;
			triggerB.conflict = ConflictLevel::Defending;
			triggerB.stageFilter = EventStage::Start;
			triggerB.args.emplace_back(std::make_unique<Argument[]>(1))[OnControl::CONTROL_ID] = "Left Attack/Block";
		}


		testController->stateMap = testManager.CreateMap();
	}




	namespace IMV2
	{
		using CommandMap = std::unordered_map<Input::Hash, std::vector<std::shared_ptr<CommandEntry>>>;


		

		struct Matrix
		{
			struct Instance
			{
				virtual ~Instance() = default;

				Matrix* base = nullptr;

				RE::InputContextID context = RE::InputContextID::kNone;

				CommandMap storage;

				//Utilize covariance with this
				Matrix* GetBaseObject()
				{
					return base;
				}
				const Matrix* GetBaseObject() const
				{
					return base;
				}

				//Ensure that an IMatrix makes this.
			};
			using InstancePtr = std::unique_ptr<Instance>;




			virtual ~Matrix() = default;



			virtual bool CanInputPass(RE::InputEvent* event) const
			{
				return true;
			}


			virtual MatrixType GetMatrixType() const = 0;


			//This can be overriden with a custom entry, such as for states.
			virtual Instance* GetContextInstance(RE::InputContextID context, bool create_if_required = false) = 0;
			
			virtual void DestroyInstance(RE::InputContextID context) = 0;


			virtual std::strong_ordering CompareOrder(const Matrix* other) const
			{
				return std::strong_ordering::equal;
			}


			virtual CommandMap CreateMap() = 0;


			template <class Self>
			auto ObtainContextInstance(this Self&& a_this, RE::InputContextID context)// -> Self::Instance*
			{
				using Thing = std::remove_pointer_t<std::remove_cvref_t<Self>>::Instance;
				return static_cast<Thing*>(a_this.GetContextInstance(context, true));
			}
			
			//Instance* ObtainContextInstance(RE::InputContextID context)
			//{
			//	return GetContextInstance(context, true);
			//}
		};


		struct InputMatrix : public Matrix
		{

			std::map<RE::InputContextID, InstancePtr> entries;

			std::vector<InputCommand> commands;//Once this is finalized, this cannot have it's values changed. so it should be private.

			MatrixType type = MatrixType::Total;



			
		protected:
			

			template<std::derived_from<Instance> IType>
			IType* LoadInstance(RE::InputContextID context, bool create_if_required = false)
			{
				//The idea with this is making a setup where the item pocket, and the thing gotten are one in the same.
				auto& slot = entries[context];

				if (create_if_required && !slot) {
					slot = std::make_unique<IType>();
					slot->base = this;
					slot->context = context;
					slot->storage = CreateMap();
					//TODO: Need to make context right here.
				}

				return static_cast<IType*>(slot.get());
			}
		public:

			Instance* GetContextInstance(RE::InputContextID context, bool create_if_required = false) override
			{
				return LoadInstance<Instance>(context, create_if_required);
			}

			void DestroyInstance(RE::InputContextID context) override
			{
				entries.erase(context);
			}

			MatrixType GetMatrixType() const override
			{
				return type;
			}


			MatrixType FetchMatrixType() const
			{
				return this ? GetMatrixType() : MatrixType::Dynamic;
			}

			CommandMap CreateMap() override
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



		struct LayerMatrix : public InputMatrix
		{
			//Blocked user events are stored as device less id codes.
			std::set<Input> blockedInputs;

			//I would like some extra settings for this. If you have a device that has -1 it means that device is entirely disabled.
			// if you have one that has a none device and is fully negative that means ALL controls are prevented from passing through.
			// Selected will do this sometimes, fully preventing passing through to completely reinvent the bindings.

			bool CanInputPass(RE::InputEvent* event) const override
			{
				auto& event_name = event->QUserEvent();
				Input input = event;

				Input userEvent{ RE::INPUT_DEVICE::kNone, Hash(event_name.c_str(), event_name.length()) };

				return !blockedInputs.contains(input) && !blockedInputs.contains(userEvent);
			}
		};








		struct InputState : public LayerMatrix
		{
			struct Instance : public LayerMatrix::Instance
			{
			public:
				

				InputState* GetBaseObject()
				{
					return static_cast<DIMS::IMV2::InputState*>(__super::GetBaseObject());
				}
				const InputState* GetBaseObject() const
				{
					return static_cast<const DIMS::IMV2::InputState*>(__super::GetBaseObject());
				}


				auto priority() const
				{
					return  GetBaseObject()->_priority;
				}


				auto level() const
				{
					return GetBaseObject()->level;
				}


				bool IsInConflict(Instance* other) const
				{
					return GetBaseObject()->IsInConflict(other->GetBaseObject());
				}


				void SetInputEnabled(Input input, bool value)
				{

					auto it = storage.find(input.hash());

					auto end = storage.end();

					if (it == end) {
						return;
					}

					for (auto& entry : it->second)
					{
						entry->SetEnabled(value);
					}

					for (auto parent : GetBaseObject()->parents)
					{
						parent->ObtainContextInstance(context)->SetInputEnabled(input, value);
					}
				}

				//This is called on all activeStates at the end of a successful update.
				void SetAllInputsEnabled(bool value = true)
				{
					for (auto& [key, lists] : storage)
					{
						for (auto& entry : lists)
						{
							entry->SetEnabled(value);
						}

					}

					for (auto parent : GetBaseObject()->parents)
					{
						parent->ObtainContextInstance(context)->SetAllInputsEnabled(value);
					}
				}

				bool CanInputPass(RE::InputEvent* event) const
				{
					return GetBaseObject()->CanInputPass(event);
				}

				bool DerivesFrom(Instance* other)
				{
					if (!other)
						return false;

					return GetBaseObject()->DerivesFrom(other->GetBaseObject());

				}


				std::vector<Instance*> GetViableStates(RefreshCode code, double data1, double data2, std::span<Instance*>& limit, InputState*& winner, bool& change, bool inner_change = false)
				{
					//TODO: GetViableStates needs to be changed pretty badly. It should only add "this" if it experiences complete success with it's parents.
					// or if it lacks parents. Basically, it must maintain the expectations of it's previous as well. That, or it must exist in the limit list.


					//It should be noted that having activated this frame is not grounds for 

					RefreshCode k_fakeUpdateCode = RefreshCode::Update;
					RefreshCode k_fakeExpectedCode = RefreshCode::Update;

					Instance* self = nullptr;

					auto end = limit.end();
					bool in_previous = std::find(limit.begin(), end, this) != end;

					if (!inner_change && GetBaseObject()->CheckEvent(code, data1, data2) == false) {
						//Given update either isn't within here or doesn't match parameters.

						//If it's not within the expected update but it's currently active, it will just put it in there.
						// I may actually just make a setting for this specifically to make it faster. For now, this will work.


						if (in_previous) {
							self = this;
						}
						else
							return {};
					}
					//The rule is that the states in question must remain above
					else {

						//What this actually should want to do is check the parents before checking itself so if it fails its children it doesn't take place.
						// But that won't be needed for a while even if this isn't a very smart way of doing this.
						if (GetBaseObject()->tmpCheckParentCondition() == true && GetBaseObject()->tmpCheckCondition() == true) {
							self = this;
						}


						if (self && !in_previous || !self && in_previous) {
							inner_change = change = true;

						}
						else {
							inner_change = false;
						}
					}



					if (self) {

						//Here a question about whether this should be viable based on the winner is cast forth.

						if (!winner) {
							winner = GetBaseObject();
							return { self };
						}
						else {
							bool collapse = GetBaseObject()->ShouldCollapse() || winner->ShouldSmash() && (!winner->IsInputRelevant() || winner->IsInConflict(GetBaseObject()));

							if (collapse)
								return {};
						}
					}



					//if (in_previous) {
					//	return {};
					//}



					std::vector<Instance*> result;


					for (auto parent : GetBaseObject()->parents)
					{
						auto add = parent->ObtainContextInstance(context)->GetViableStates(code, data1, data2, limit, winner, change, inner_change);

						if (add.size() != 0)
							result.append_range(add);
					}

					return result;
				}

				//bool CheckConditions(std::span<StateEntry>& end){}

				void LoadVisitorList(detail::VisitorList& list, Input input, Input control, InputState*& winner)
				{
					auto ctrl_it = storage.find(control.hash());
					auto input_it = storage.find(input.hash());

					auto end = storage.end();


					bool add = true;

					bool c_find = ctrl_it != end;
					bool i_find = input_it != end;

					auto settings = GetBaseObject();

					if  constexpr (0)
					{
						//I'm doing this real crude like for the visuals
						if (c_find || i_find)
						{
							//Do this bit first if you can, it'd be nice to have the searching not done if it's not viable.
							if (!winner) {
								//Winner only matters here if it does either of these things
								if (settings->ShouldSmother() || settings->ShouldSmash())
									winner = settings;
							}
							else
							{
								if (settings->ShouldCollapse() || winner->ShouldSmash())
								{
									return;
								}
							}


							if (c_find) {
								list.push_back(std::ref(ctrl_it->second));
								SetInputEnabled(control, true);
							}

							if (i_find) {
								list.push_back(std::ref(input_it->second));
								SetInputEnabled(input, true);
							}
						}
					}
					else
					{
						if (ctrl_it != end) {
							list.push_back(std::ref(ctrl_it->second));
							SetInputEnabled(control, true);
						}

						if (input_it != end) {
							list.push_back(std::ref(input_it->second));
							SetInputEnabled(input, true);
						}
					}


					//Basically if it's already taken, take it back
					InputState* _winner = winner;

					for (auto& parent : settings->parents)
					{
						parent->ObtainContextInstance(context)->LoadVisitorList(list, input, control, _winner);
					}


				}
			};
			

			InputState()
			{
				type = MatrixType::State;
			}

			Instance* GetContextInstance(RE::InputContextID context, bool create_if_required = false) override
			{
				return LoadInstance<Instance>(context, create_if_required);
			}




			static constexpr RefreshEvent genericUpdateEvent{ RefreshCode::Update, 0.0, CompareType::kGreaterOrEqual };



			//This is a matrix setting that creates the setting
			std::vector<InputState*> parents;

			StateLevel level = StateLevel::Smother;
			int16_t _priority = 1;
			tmp_Condition* condition = nullptr;

			std::string_view debugName;

			std::set<Input> conflictList;

			//If refresh events exist, then the default update is used.
			std::set<RefreshEvent> refreshEvents;


			bool CheckEvent(RefreshCode a_code, double data1, double data2)
			{
				//returns 1 if true, 0 if false, -1 if it fails the basic state update and doesn't have any entries.

				if (a_code == RefreshCode::Absolute) {
					return true;
				}


				if (refreshEvents.empty() && parents.empty() == true) {
					return genericUpdateEvent.CheckEvent(a_code, data1, data2);
				}


				for (auto& event : refreshEvents)
				{
					if (event.CheckEvent(a_code, data1, data2) == true)
					{
						return true;
					}
				}

				for (auto parent : parents)
				{
					if (parent->CheckEvent(a_code, data1, data2) == true)
					{
						return true;
					}
				}

				return false;
			}



			MatrixType GetMatrixType() const override { return MatrixType::State; }

			std::strong_ordering CompareOrder(const Matrix* o) const override
			{
				auto other = dynamic_cast<const InputState*>(o);

				if (DerivesFrom(other) == true) {
					return std::strong_ordering::greater;
				}

				if (other->DerivesFrom(this) == true) {
					return std::strong_ordering::less;
				}

				return std::strong_ordering::equal;
			}

			//If input is relevant to the below
			bool IsInputRelevant() const
			{
				switch (level)
				{
				case StateLevel::Smother:
				case StateLevel::Clobber:
					return true;

				default:
					return false;
				}
			}

			bool ShouldSmash() const
			{
				switch (level)
				{
				case StateLevel::Smash:
				case StateLevel::Clobber:
					return true;

				default:
					return false;
				}
			}

			//Collapse is the act of crumpling despite not being asked to.
			bool ShouldCollapse() const
			{
				switch (level)
				{
				case StateLevel::Smash:
				case StateLevel::Clobber:
				case StateLevel::Collapse:
					return true;

				default:
					return false;
				}
			}

			bool ShouldSmother() const
			{

				switch (level)
				{
				case StateLevel::Smash:
				case StateLevel::Clobber:
				case StateLevel::Smother:
					return true;

				default:
					return false;
				}
			}


			bool ShouldBeSmothered() const
			{
				return level != StateLevel::Merge;
			}




			void tmpBuildConflictList()
			{
				conflictList.clear();

				for (auto& command : commands)
				{
					for (auto& trigger : command.triggers)
					{
						conflictList.insert_range(trigger.GetInputs());
					}
				}

				for (auto parent : parents)
				{
					//This assumes that the previous has built it's own list.
					conflictList.insert_range(parent->conflictList);
				}
			}

			bool IsInConflict(const InputState* other) const
			{
				auto control_map = RE::ControlMap::GetSingleton();


				for (auto input : conflictList) {
					if (other->HasInput(input) == true)
						return true;
				}

				if (other->HasRawInputs() == true)
				{
					for (auto input : other->conflictList) {
						if (HasInputAsUserEvent(input) == true)
							return true;


						continue;

						//saving this just in case
						if (input.IsUserEvent() == true)
							continue;


						if (auto name = control_map->GetUserEventName(input.code, input.device); name.empty() == false) {
							//TODO: When custom controls exist, this will have to be done many times over to check for personal user events.
							Input ctrl = Input::CreateUserEvent(name);

							if (conflictList.contains(ctrl) == true)
								return true;
						}


					}
				}

				return false;
			}


			bool IsInConflict(const InputState* other, Input input) const
			{
				return HasInput(input) && other->HasInput(input);
			}


			//A bool will get flipped on if we even need to look for user events in the conflict list.
			bool HasRawInputs() const
			{
				//When a raw input is added to a state, a boolean flag will be ticked that will start the search from the other side.
				return true;
			}



			bool HasInputAsUserEvent(Input input) const
			{
				if (input.IsUserEvent() == false) {
					auto control_map = RE::ControlMap::GetSingleton();

					if (auto name = control_map->GetUserEventName(input.code, input.device); name.empty() == false) {
						//TODO: When custom controls exist, this will have to be done many times over to check for personal user events.
						Input ctrl = Input::CreateUserEvent(name);

						if (conflictList.contains(ctrl) == true)
							return true;
					}
				}

				return false;
			}


			bool HasInput(Input input) const
			{
				if (conflictList.contains(input) == true)
					return true;

				return HasInputAsUserEvent(input);
			}

			bool IsInConflict(const InputState& other)
			{
				return IsInConflict(&other);
			}


			bool DerivesFrom(const InputState* other) const
			{
				if (!other)
					return false;

				if (this == other)
					return true;

				for (auto parent : parents)
				{
					if (parent->DerivesFrom(other) == true)
						return true;
				}

				return false;

			}

			bool CanInputPass(RE::InputEvent* event) const override
			{
				auto prev = __super::CanInputPass(event);

				if (prev)
				{
					for (auto parent : parents)
					{
						if (parent->CanInputPass(event) == false) {
							return false;
						}
					}
				}

				return prev;
			}

			bool tmpCheckCondition()
			{
				if (condition)
					return condition(RE::PlayerCharacter::GetSingleton());

				return true;
			}

			bool tmpCheckParentCondition()
			{
				for (auto parent : parents)
				{
					if (!parent->tmpCheckCondition() || !parent->tmpCheckParentCondition()) {
						return false;
					}
				}

				return true;
			}

			int16_t priority() const
			{
				return _priority;
			}
		};


		struct InputMode : public LayerMatrix
		{

			struct Instance : public LayerMatrix::Instance
			{
			public:
				CommandEntryPtr source = nullptr;
				
				bool isStrong = true;

				InputMode* GetBaseObject()
				{
					return static_cast<InputMode*>(__super::GetBaseObject());
				}
				const InputMode* GetBaseObject() const
				{
					return static_cast<const InputMode*>(__super::GetBaseObject());
				}

			};

			InputMode()
			{
				type = MatrixType::Mode;
			}

			Instance* GetContextInstance(RE::InputContextID context, bool create_if_required = false) override
			{
				return LoadInstance<Instance>(context, create_if_required);
			}

		};

		inline void Testing()
		{
			InputState state{};

		}


	}

}