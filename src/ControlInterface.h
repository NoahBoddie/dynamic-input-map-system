#pragma once

#include "InputInterface.h"
#include "RE/Functions.h"

namespace DIMS
{
	struct InputInterface;


	struct ControlInterface
	{
		enum State
		{
			kInvalid,
			kGameMode,
			kMenuMode,
		};

		union
		{
			uintptr_t raw = 0;
			RE::BSTEventSink<RE::InputEvent*>* sink;
			RE::PlayerControls* gameCtrl;
			RE::MenuControls* menuCtrl;
		};

		State state = kInvalid;

		//TODO: This needs to learn how to deal with virtual events
		constexpr ControlInterface(RE::PlayerControls* ctrl) noexcept : gameCtrl{ ctrl }, state{ kGameMode } {}
		constexpr ControlInterface(RE::MenuControls* ctrl) noexcept : menuCtrl{ ctrl }, state{ kMenuMode } {}


		ControlState GetState() const
		{
			switch (state)
			{
			case State::kGameMode:
				return ControlState::Gameplay;
			case State::kMenuMode:
				return ControlState::MenuMode;

			default:
				return ControlState::Total;
			}
		}

		void ExecuteInput(InputInterface& event)
		{
			switch (state)
			{
			case kGameMode:
				RE::ExecuteInput(gameCtrl, event);
				RE::UnkFunc01(gameCtrl);
				break;

			case kMenuMode: {
				RE::InputEvent* input = event;
				ExecuteMenuInput(menuCtrl->handlers, input);
				break;
			}

			}
		}

	};

}