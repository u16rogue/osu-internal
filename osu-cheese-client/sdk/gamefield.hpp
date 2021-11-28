#pragma once

#include <tuple>

namespace sdk
{
	class game_field
	{
		static constexpr float osu_native_w = 640.f;
		static constexpr float osu_native_h = 480.f;
		static constexpr float osu_field_w = osu_native_w * 0.8f;
		static constexpr float osu_field_h = osu_native_h * 0.8f;
		static constexpr float osu_margin_ratio = 0.1f;
		static constexpr float osu_scale_ratio = 0.28f;
		
		inline static float field_ratio = 0.f;
		inline static float offset_x    = 0.f;
		inline static float offset_y    = 0.f;

	public:
		game_field() = delete;

		static auto resize(int vw, int vh, float size = 1.f) -> void;
		
		// Screen to field (/world)
		static auto s2f(int x, int y) -> std::pair<float, float>;

		// Field to screen
		static auto f2s(int x, int y) -> std::pair<float, float>;
	};
}