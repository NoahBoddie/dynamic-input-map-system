#pragma once

#include "Enums.h"
#include "Input.h"

namespace DIMS
{
	
	using string_hash = size_t;
	
	struct Argument
	{
	private:
		uint64_t value = 0;
	public:
		template <typename T> requires (std::is_trivially_destructible_v<T> && sizeof(T) <= sizeof(decltype(value)))
			constexpr T& As()
		{
			return reinterpret_cast<T&>(value);
		}

		template <typename T> requires (std::is_trivially_destructible_v<T> && sizeof(T) <= sizeof(decltype(value)))
			constexpr const T& As() const
		{
			return reinterpret_cast<const T&>(value);
		}

		template <typename T> requires (std::is_trivially_destructible_v<T> && sizeof(T) <= sizeof(decltype(value)))
			constexpr void As(T& out) const
		{
			out = As<T>();
		}


		template <typename T> requires (std::is_trivially_destructible_v<T> && sizeof(T) <= sizeof(decltype(value)))
			constexpr void Set(T in)
		{
			As<T>() = in;
		}

	public:
		/*
		bool GetBool(bool& out, ParameterType type) const
		{
			if (type != ParameterType::Bool && type != ParameterType::Total) {
				return false;
			}

			Get(out);

			return true;
		}


		bool GetString(size_t& out, ParameterType type) const
		{
			if (type != ParameterType::String && type != ParameterType::Total) {
				return false;
			}

			Get(out);

			return true;
		}


		bool GetInt(int32_t& out, ParameterType type) const
		{
			if (type != ParameterType::Int && type != ParameterType::Total) {
				return false;
			}

			Get(out);

			return true;
		}

		bool GetFloat(float& out, ParameterType type) const
		{
			if (type != ParameterType::Float && type != ParameterType::Total) {
				return false;
			}

			Get(out);

			return true;
		}

		bool GetForm(RE::TESForm*& out, ParameterType type) const
		{
			if (type != ParameterType::Form && type != ParameterType::Total) {
				return false;
			}

			Get(out);

			return true;
		}

		bool GetInput(Input& out, ParameterType type) const
		{
			if (type != ParameterType::Input && type != ParameterType::Total) {
				return false;
			}

			Get(out);

			return true;
		}


		bool GetFunction(ActionFunction*& out, ParameterType type) const
		{
			if (type != ParameterType::Function && type != ParameterType::Total) {
				return false;
			}

			Get(out);

			return true;
		}

		/////////

		bool GetBool(ParameterType type = ParameterType::Total) const
		{
			bool result;

			if (GetBool(result, type) == false)
			{
				throw std::exception("I'm not supposed to fail ");
			}

			return result;
		}


		size_t GetString(ParameterType type = ParameterType::Total) const
		{
			size_t result;

			if (GetString(result, type) == false)
			{
				throw std::exception("I'm not supposed to fail ");
			}

			return result;
		}


		int32_t GetInt(ParameterType type = ParameterType::Total) const
		{
			int32_t result;

			if (GetInt(result, type) == false)
			{
				throw std::exception("I'm not supposed to fail ");
			}

			return result;
		}

		float GetFloat(ParameterType type = ParameterType::Total) const
		{
			float result;

			if (GetFloat(result, type) == false)
			{
				throw std::exception("I'm not supposed to fail ");
			}

			return result;
		}

		RE::TESForm* GetForm(ParameterType type = ParameterType::Total) const
		{
			RE::TESForm* result;

			if (GetForm(result, type) == false)
			{
				throw std::exception("I'm not supposed to fail ");
			}

			return result;
		}

		Input GetInput(ParameterType type = ParameterType::Total) const
		{
			Input result;

			if (GetInput(result, type) == false)
			{
				throw std::exception("I'm not supposed to fail ");
			}

			return result;
		}
		//*/

		constexpr Argument() = default;

		template <typename T> requires (std::is_trivially_destructible_v<T> && sizeof(T) <= sizeof(decltype(value)))
		constexpr Argument(T val)
		{
			Set(val);
		}

		constexpr Argument(std::string_view str) noexcept
		{
			Set(std::hash<std::string_view>{}(str));
		}
		constexpr Argument(const char* str) noexcept :Argument{ std::string_view{str} }
		{}
		/*
		constexpr Argument(int32_t num) noexcept
		{
			Set(num);
		}
		constexpr Argument(float num) noexcept
		{
			Set(num);
		}
		constexpr Argument(bool boolean) noexcept
		{
			Set(boolean);
		}
		constexpr Argument(RE::TESForm* form) noexcept {
			Set(form);
		}

		constexpr Argument(Input input) noexcept {
			Set(input);
		}
		//*/

	};

}