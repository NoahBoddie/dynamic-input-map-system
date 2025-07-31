#pragma once

namespace DIMS
{
	struct CommandEntry;

	using CommandEntryPtr = std::shared_ptr<CommandEntry>;

	using tmp_Condition = bool(RE::PlayerCharacter*);

	//Should actually be voidable
	using Condition = LEX::Formula<LEX::Voidable()>;

	using InputCount = int8_t;

}


namespace RE
{
	using UserEventFlag = UserEvents::USER_EVENT_FLAG;
	using UserEventMapping = ControlMap::UserEventMapping;
	using InputContextID = RE::UserEvents::INPUT_CONTEXT_IDS::INPUT_CONTEXT_ID;
	using InputEventType = INPUT_EVENT_TYPE;
	using InputDevice = INPUT_DEVICE;

	using GamepadInput = REX::W32::XINPUT_GAMEPAD_BUTTON;
	using Keyboard = REX::W32::DIK;
}

namespace DIMS
{
	constexpr auto k_gameplayContext = RE::InputContextID::kGameplay;

}
