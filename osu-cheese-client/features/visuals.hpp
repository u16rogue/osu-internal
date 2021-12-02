#pragma once

namespace features
{
	class visuals
	{
	public:
		visuals() = delete;

		inline static bool ho_timer = false;

		static auto render() -> void;
	};
}