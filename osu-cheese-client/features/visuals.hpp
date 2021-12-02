#pragma once

namespace features
{
	class visuals
	{
	public:
		visuals() = delete;

		inline static bool ho_timer = false;
		inline static bool ho_tracer = false;
		inline static bool ho_distance = false;

		static auto render() -> void;
	};
}