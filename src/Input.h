#pragma once

namespace DIMS
{
	using ControlID = uint32_t;
	

	struct Input
	{
	private:
		static constexpr uint32_t k_controlCode = -1;

	public:


		using Hash = int64_t;

		static constexpr Hash CONTROL = -1;



		constexpr Input(Hash a_hash)
		{
			//The idea is that this will not take input from literals. No idea how to do that though.
			as_hash() = a_hash;
		}
		constexpr Input(RE::INPUT_DEVICE d, uint32_t c) noexcept : device{ d }, code{ c }
		{
		}

		static Input CreateControl(RE::InputEvent* event)
		{
			
			return Input{ RE::INPUT_DEVICE::kNone, RGL::Hash<HashFlags::Insensitive>(event->QUserEvent().c_str()) };
		}


		Input(RE::InputEvent* event) noexcept : Input(event->GetDevice(), -1)
		{
			if (auto id = event->AsIDEvent())
			{
				code = id->GetIDCode();
			}

		}


		constexpr Input() noexcept = default;


		RE::INPUT_DEVICE device = RE::INPUT_DEVICE::kNone;
		uint32_t code = k_controlCode;


		constexpr Hash hash() const
		{
			return *static_cast<Hash*>((void*)this);//(static_cast<Hash>(device) << 31) | static_cast<Hash>(code);
		}


		constexpr operator Hash() const
		{
			return hash();
		}


		bool IsControl() const
		{
			return IsDevice(RE::INPUT_DEVICE::kNone);
			return hash() == -1;
		}


		bool IsDevice(RE::INPUT_DEVICE a_device) const
		{
			return device == a_device;
		}

		constexpr auto operator<=>(const Input& other) const = default;

	private:

		constexpr Hash& as_hash()
		{
			return *static_cast<Hash*>((void*)this);
		}


	};

}