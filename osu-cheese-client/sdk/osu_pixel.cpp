#include "osu_pixel.hpp"

static constexpr float osu_native_w = 640.f;
static constexpr float osu_native_h = 480.f;
static constexpr float osu_field_w = osu_native_w * 0.8f;
static constexpr float osu_field_h = osu_native_h * 0.8f;
static constexpr float osu_margin_ratio = 0.1f;
static constexpr float osu_scale_ratio = 0.28f;

// 80 x 70 - top
// 80 x 50 - bottom

auto sdk::screen2osupixel(int x, int y, int vw, int vh, float size) -> sdk::vec2i
{
	// From osu! source leak...
	float window_ratio = static_cast<float>(vh) / osu_native_h;

	float w = osu_field_w * window_ratio * size;
	float h = osu_field_h * window_ratio * size;

	float offset = -16.f * window_ratio;

	float offx = (static_cast<float>(vw) - w) / 2.f;
	float offy = (static_cast<float>(vh) - h) / 4.f * 3.f + offset;

	float field_ratio = h / osu_field_h;

	float px = (static_cast<float>(x) - offx) / field_ratio;
	float py = (static_cast<float>(y) - offy) / field_ratio;

	return sdk::vec2i {
		.x = static_cast<int>(px),
		.y = static_cast<int>(py)
	};
}

auto sdk::osupixel2screen(int x, int y, int vw, int vh) -> sdk::vec2i
{
	return { 0, 0 };
}
