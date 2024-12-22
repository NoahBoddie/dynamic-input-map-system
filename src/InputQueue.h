#pragma once

#include "Input.h"
#include "InputNumber.h"

namespace DIMS
{
	struct InputQueue
	{
		void emplace_vtable_steal()
		{
			//i'm too lazy to gitcommit so this is what I'm doing for now.
			/*
			auto address = T::VTABLE[0].address();
			if (!address) {
				return false;
			}
			reinterpret_cast<std::uintptr_t*>(a_ptr)[0] = address;
			return true;
			//*/
		}

		//At a later point, I'd actually like these to be a personal set of buttons that are created at personal
		// disgression.

		//Also later, they have the option of not being gameplay focused.
		inline static std::vector<std::unique_ptr<RE::InputEvent>> queuedEvents;

		static void AddEvent(Input input, const std::string& userEvent, InputNumber value1, InputNumber value2)
		{
			//This function needs to have another function that tells it whether it's supposed to make an button/thumb/mouse event from a given input.
			// for now, only keyboard is allowed.

			if (input.device != RE::INPUT_DEVICE::kKeyboard)
				return;

			//Something like this is then done to it.
			//*
			auto buttonEvent = RE::malloc<RE::ButtonEvent>(sizeof(RE::ButtonEvent));
			std::memset(reinterpret_cast<void*>(buttonEvent), 0, sizeof(RE::ButtonEvent));
			if (buttonEvent) {
				reinterpret_cast<std::uintptr_t*>(buttonEvent)[0] = RE::VTABLE_ButtonEvent[0].address();

				buttonEvent->device = input.device;
				buttonEvent->eventType = RE::INPUT_EVENT_TYPE::kButton;
				buttonEvent->next = nullptr;
				buttonEvent->userEvent = userEvent.c_str();
				buttonEvent->idCode = input.code;
				buttonEvent->value = value1;
				buttonEvent->heldDownSecs = value2;
			}
			
			queuedEvents.push_back(std::unique_ptr<RE::InputEvent>{buttonEvent});

			//*/
		}

		static bool HasQueue()
		{
			return !queuedEvents.empty();
		}

		static auto MoveQueue()
		{
			auto end = queuedEvents.end();

			for (auto it = queuedEvents.begin(); it != end; it++)
			{
				auto next = it + 1;

				if (next != end)
					it->get()->next = next->get();
			}

			return std::move(queuedEvents);
		}
	};
}