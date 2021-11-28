#include "gamefield.hpp"

auto sdk::game_field::resize(int vw, int vh, float size) -> void
{
	float window_ratio = static_cast<float>(vh) / osu_native_h;

	float w = osu_field_w * window_ratio * size;
	float h = osu_field_h * window_ratio * size;

	float offset = -16.f * window_ratio;

	game_field::offset_x = (static_cast<float>(vw) - w) / 2.f;
	game_field::offset_y = (static_cast<float>(vh) - h) / 4.f * 3.f + offset;

	game_field::field_ratio = h / osu_field_h;
}

auto sdk::game_field::s2f(int x, int y) -> std::pair<float, float>
{
	return std::make_pair(
		(static_cast<float>(x) - game_field::offset_x) / field_ratio,
		(static_cast<float>(y) - game_field::offset_y) / field_ratio
	);
}

auto sdk::game_field::f2s(int x, int y) -> std::pair<float, float>
{
	return std::make_pair(
		static_cast<float>(x) * field_ratio + game_field::offset_x,
		static_cast<float>(y) * field_ratio + game_field::offset_y
	);
}

