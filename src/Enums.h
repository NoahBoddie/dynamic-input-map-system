#pragma once

namespace DIMS
{

	constexpr RE::INPUT_EVENT_TYPE virtualEventType = (RE::INPUT_EVENT_TYPE)-1;


	//Move to RGL
	//Returns the highest bit position of an enum or integer
	template <typename T> requires(std::is_enum_v<T> || std::is_integral_v<T>)
	constexpr int8_t GetBitPosition(const T v) noexcept
	{
		size_t value = static_cast<size_t>(v);

		if (value){
			int8_t i = 1;
			while (value != 1) i++, value >>= 1;
			return i;
		}

		return -1;
	}

	constexpr int strong_ordering_to_int(std::strong_ordering o)
	{
		if (o == std::strong_ordering::less)    return -1;
		if (o == std::strong_ordering::greater) return 1;
		return 0;
	}

	enum struct ParameterType
	{
		None,
		Bool,
		Int,			//32 bit
		Float,			//32 bits
		String,			
		Form,			//A form of some kind.
		//Input,			//A raw input, including device stuff.
		//Function,
		Total,			//Total. Also used as an accept all key for arguments
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
		kOr = kSecondary,
	};


	enum struct MatrixType : uint8_t
	{//I wanted to rename this, I don't remember what.


		//PrefixState,
		Mode,			//The latest mode in the controller.
		State,			//
		//SuffixState,
		//The relationship between ^ these is gonna have to be manual in terms of states blocking modes and such.

		Dynamic,			//A matrix that is selected from the controls menu. Should be serialized outside of control map.
		Selected,		//The default state. Default configurations attach to this, rather than the default made config

		Total,
	};


	enum struct DelayState
	{
		None,			//This command has no state of delay
		Failure,		//This command is unable to run due to failing delay conditions
		Success,		//This command is now able to run passing delay conditions
		Listening,		//This command is can continue querying for an end to its delay state
		Advancing		//This command can both continue querying its delay state and innate based delay conditions
	};


	ENUM(TriggerType, uint8_t)
	{
		OnButton,
		OnControl,
		OnMouseMove,
		OnThumbstick,
		OnAxis,
		Total,
		None = TriggerType::Total,
	};


	ENUM(ActionType, uint8_t)
	{

		InvokeFunction,			//Invokes a C++ function. This one is personal, may make API for register
		InvokeInput,			//Queues a virtual input for SKSE's papyrus to intercept (but not PlayerControls or MenuControls)
		//InvokeUserEvent,		//Queues a virtual user event for SKSE's papyrus to intercept (but not PlayerControls or MenuControls)
		//InvokeButton,			//Invokes an existing button
		//InvokeMouse,			//Invokes the mouse move event. Cannot use finish stage. Doing so will result in failure.
		//InvokeThumbstick,		//Invokes the thumbstick move event and calls basic. Cannot use finish events. Doing so will result in failure.
	
		//InvokeControl,			//Invokes a specific control by name.
		//InvokeAxis,				//Invokes a specific axis by name. If the original function is called and is a finish event, it will result in failure
		//InvokeFormula,			//Calls a lexicon function
		//InvokeModEvent,			//Sends a mod event to papyrus
		InvokeMode,

		Total,
	};

	//Controls how an action recovers from failure
	ENUM(ActionRecovery, uint8_t)
	{
		//These names are NOT good. I get their functionality mixed a lot. Might as well have numbers.
		Persist,		//Persists through the action list, but will return failure, on actions bets failure on the next one. Default of Commands.
		Break,			//Breaks on the first sign of failure. On Actions it will send a failure the moment it fails. Default of action.
		Continue,		//Ignores failure and continues to return success.
	};
	

	ENUM(EventStage, uint8_t)
	{
		None = 0,
		Start = 1 << 0,
		Repeating = 1 << 1,
		Finish = 1 << 2,
		
		Last,
		Total = GetBitPosition(EventStage::Last),


		Preface = 1 << 3, //This serves as the action for tenative actions. When prefacing an execution, it will not allow multiple executions.

		UntilFinish = EventStage::Start | EventStage::Repeating,
		AfterStart = EventStage::Repeating | EventStage::Finish,

		StartFinish = EventStage::Start | EventStage::Finish,
		All = EventStage::Start | EventStage::Repeating | EventStage::Finish,
	};

	ENUM(EventFlag)
	{
		None = 0,
		Continue = 1 << 0,	//Continues original, regardless of blocking.
		Refired = 1 << 1,
	};

	ENUM(ActionFlag, uint8_t)
	{
		None = 0,
		Tentative = 1 << 0,
		Reprisal = 1 << 1,
	};

	ENUM(TriggerFlag, uint8_t)
	{
		None = 0,
		Reprise = 1 << 0,	//It is allowed to handle reprisal events
		KeepCheckingCondition = 1 << 1,
	};

	enum struct ConflictLevel : int8_t
	{
		None,			//Technically the same as sharing, but if a trigger has this, the action takes preference.
		Following,		//Does not block, cannot be blocked, cannot be delayed, and will not pend other commands.
		Sharing,		//Triggers registered as sharing will not block, or be stopped by blocking
		Obliging,		//Obliging will not block other triggers, but is stopped by blocking
		
		Guarding,		//Guarding will not block triggers but will block basic events. Cannot be blocked.
		Defending,		//Defending will block all basic events and triggers, but not most others. Will be blocked by capturing and blocking.
		Blocking,		//Blocking will block the trigger events choosen, while also being stopped by other blocks
		Capturing,		//Capturing will block all events in a trigger, regardless if it actually runs them, and will be blocked.
	};


	enum struct BlockingState : uint8_t
	{
		None,
		Weak,
		Strong,
	};



	ENUM(ActiveState, uint8_t)
	{
		Inactive,		//Active command has not been initialized yet.
		Managing,		//Active command is managing an input, but is incapable of running a command.
		Running,		//Active command has confirmed all input requirements have been met and is allowed to run commands.
		Failing,		//Active command was managing but has now failed, and will cease to run commands.

		EarlyExit = 1 << 5,
		DelayUndo = 1 << 6,
		Waited = 1 << 7,
		
		Flags = ActiveState::EarlyExit | ActiveState::DelayUndo | ActiveState::Waited,
	};

	
	//This type has the chance to hold personalized flags
	ENUM(RefreshCode, uint32_t)
	{
		Update,			//Code sent when a sufficient amount of time has passed since the state last updated.

		GraphOutputEvent,
		GraphInputEvent,
		GraphVarChange,


		Custom = 1 << 31,	//This code allows for custom refresh points.
							// when using a custom event, one needs to specify it with a character of some kind. After that
							// the string afterwards is turned into a hash, and this hash (with the custom bit move) is what
							// triggers a custom event code. This code can be helpful for scripted updates, or external state changes.

		

		Absolute = ~RefreshCode::Custom, //Runs even if the expected refresh code doesn't matches. Using this should also reset update's timestamp.
	};

	//TODO: A map of default checks for refresh code should exist. Something with very literal checks mashed together.
	//So if there was a state that updates on crouch and weapon draw, it will have the default condition of happening when you draw your weapon and crouch.
	// of course, it would need space for parameters, so it would start when you crouch and end when you stop crouching. But all that comes later.


	enum struct StateLevel : uint8_t
	{
		Smother,		//Smothers commands that have the same inputs, while keeping the rest of the inputs intact.
		Merge,			//Processes all commands in both states
		Clobber,		//Smashes lesser commands if there's an input clash only
		Smash,			//Smashes lesser regardless of input clash
		Collapse,		//Collapses the lesser regardless of clash
	};




	//I think I'll move the flags to this
	ENUM(CommandFlag, uint8_t)
	{
		None		= 0,
		Started		= 1 << 0,
		Finished	= 1 << 1,
		Complete	= 1 << 2,
		Canceled	= 1 << 3,
		Inactive	= 1 << 4,		//CommandEntries with inactive 
		Disabled	= 1 << 5,
		Delayed		= 1 << 6,
		
		StageFlags = CommandFlag::Started | CommandFlag::Finished,
		RunningFlags = CommandFlag::Complete | CommandFlag::Canceled | CommandFlag::Inactive,
		Instance = CommandFlag::Started | CommandFlag::Finished | CommandFlag::Complete | CommandFlag::Canceled | CommandFlag::Inactive | CommandFlag::Delayed,
	};



	ENUM(ControlState, uint8_t)
	{
		Gameplay,
		MenuMode,
		Total,
	};

}