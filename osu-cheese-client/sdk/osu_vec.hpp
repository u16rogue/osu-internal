#pragma once

#include <cstdint>

namespace sdk
{
	template <typename T>
	struct basic_vec2
	{
		T x, y;
	};

	template <typename T>
	struct vec2
	{
		T x, y;
	};

	using vec2i = vec2<std::int32_t>;
}