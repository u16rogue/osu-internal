#pragma once

#include "osu_vec.hpp"
#include <tuple>

namespace sdk
{
	auto screen2osupixel(int x, int y, int vw, int vh, float size = 1.f) -> sdk::vec2i;
	auto osupixel2screen(int x, int y, int vw, int vh) -> sdk::vec2i;
}