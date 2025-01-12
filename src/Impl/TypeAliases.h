#pragma once

namespace DIMS
{
	struct CommandEntry;

	using CommandEntryPtr = std::shared_ptr<CommandEntry>;

	using tmp_Condition = bool(RE::PlayerCharacter*);

}


namespace RE
{
	using UserEventFlag = UserEvents::USER_EVENT_FLAG;
	using UserEventMapping = ControlMap::UserEventMapping;
	using InputContextID = RE::UserEvents::INPUT_CONTEXT_ID;
}