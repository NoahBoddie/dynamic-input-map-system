#pragma once

#include "StringTable.h"

namespace DIMS
{
	using ControlID = uint32_t;	


	


	struct Input
	{
	private:
		//static constexpr uint32_t k_nullCode = -1;
		static constexpr uint32_t k_nullCode = 255;

	public:
		using Code = uint32_t;

		using Hash = int64_t;

		static constexpr Hash CONTROL = -1;

	

		constexpr Input(Hash a_hash) noexcept
		{
			//The idea is that this will not take input from literals. No idea how to do that though.
			as_hash() = a_hash;
		}
		constexpr Input(RE::InputDevice d, uint32_t c) noexcept : device{ d }, code{ c }
		{
		}

		constexpr Input(const std::string_view& string) : device{ RE::InputDevice::kNone }, code{ StringTable::Hash(string)}
		{
		}

		//Make create input

		static Input CreateInput(RE::InputEvent* event)
		{
			Input result{ event->GetDevice(), k_nullCode };

			if (auto id = event->AsIDEvent())
			{
				result.code = id->GetIDCode();
			}

			return result;
		}
		static Input CreateUserEvent(const std::string_view& str)
		{
			//If more discriminators are needed for this, I can include the size of the hash to further reduce the chances.
			return Input{ RE::InputDevice::kNone, RGL::Hash<HashFlags::Insensitive>(str) };
		}

		static Input CreateUserEvent(RE::InputEvent* event)
		{
			return CreateUserEvent({ event->QUserEvent().c_str(), event->QUserEvent().length() });
		}


		static Input CreateDynamicInput(StringHash hash, RE::InputDevice device, RE::InputContextID context)
		{
			Input result{ RE::InputDevice::kTotal, hash };

			auto& to = reinterpret_cast<uint32_t&>(result.device);

			to |= (uint8_t)device << 23;
			to |= (uint8_t)context << 15;

			return result;
		}

		



		Input(RE::InputEvent* event) noexcept : Input(event->GetDevice(), k_nullCode)
		{
			if (auto id = event->AsIDEvent())
			{
				code = id->GetIDCode();
			}

		}


		constexpr Input() noexcept = default;

		
		RE::InputDevice device = RE::InputDevice::kNone;
		uint32_t code = k_nullCode;


		constexpr Hash hash() const
		{
			
			return *static_cast<Hash*>((void*)this);//(static_cast<Hash>(device) << 31) | static_cast<Hash>(code);
		}


		constexpr operator Hash() const
		{
			return hash();
		}

		constexpr bool IsDynamicInput() const noexcept
		{
			return IsDevice(RE::InputDevice::kTotal);
		}

		constexpr bool IsUserEvent() const noexcept
		{
			return device >= RE::InputDevice::kTotal;
			return hash() == -1;
		}

		constexpr bool IsDevice(RE::InputDevice a_device) const noexcept
		{
			return device == a_device;
		}

		constexpr auto operator<=>(const Input& other) const = default;

	private:

		constexpr Hash& as_hash() noexcept
		{
			return *static_cast<Hash*>((void*)this);
		}


	};

}