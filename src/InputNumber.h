#pragma once

namespace DIMS
{
	//TODO: input number should just be a double instead. Not saving any space with this.
	union InputNumber
	{
		float	floating;
		int32_t integer = 0;


		constexpr operator float() const noexcept
		{
			return floating;
		}

		constexpr operator int32_t() const noexcept
		{
			return integer;
		}

		constexpr InputNumber() noexcept = default;

		constexpr InputNumber(float value) noexcept : floating{ value }
		{

		}

		constexpr InputNumber(int32_t value) noexcept : integer{ value }
		{

		}
	};

}