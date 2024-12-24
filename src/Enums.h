#pragma once

namespace DIMS
{
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

	enum struct ParameterType
	{
		None,
		Bool,
		Int,			//32 bit
		Float,			//32 bits
		String,			//Stores a hash. if one wants to recover the string I may make something for that.
		Form,			//A form of some kind.
		Input,			//A raw input, including device stuff.
		Function,
		Total,			//Total. Also used as an accept all key for arguments
	};



	ENUM(TriggerType, uint8_t)
	{
		OnButton,
		OnControl,

		Total,
		None = TriggerType::Total,
	};


	ENUM(ActionType, uint8_t)
	{

		InvokeFunction,			//Invokes a C++ function. This one is personal, may make API for register
		InvokeInput,			//Queues a virtual input for SKSE's papyrus to intercept (but not PlayerControls or MenuControls)
		InvokeUserEvent,		//Queues a virtual user event for SKSE's papyrus to intercept (but not PlayerControls or MenuControls)
		InvokeButton,			//Invokes an existing button
		InvokeMouse,			//Invokes the mouse move event. Cannot use finish stage. Doing so will result in failure.
		InvokeThumbstick,		//Invokes the thumbstick move event and calls basic. Cannot use finish events. Doing so will result in failure.

		InvokeControl,			//Invokes a specific control by name.
		InvokeAxis,				//Invokes a specific axis by name. If the original function is called and is a finish event, it will result in failure

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
		
		_Last,
		Total = (EventStage::_Last - 1) << 1,


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
	};

	enum struct ConflictLevel : int8_t
	{
		None,			//Technically the same as sharing, but if a trigger has this, the action takes preference.
		Sharing,		//Triggers registered as sharing will not block, or be stopped by blocking
		Obliging,		//Obliging will not block other triggers, but is stopped by blocking
		Guarding,		//Guarding will block original or basic triggers, but not most others. Will be blocked by capturing and blocking.
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

	

}