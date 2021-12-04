#include "osu_vec.hpp"

#include <cstdint>
#include <Windows.h>
#include <cmath>
#include <numbers>

#include "../manager/gamefield_manager.hpp"

auto sdk::vec2::distance(const vec2 & to) const -> float
{
	return std::sqrt(std::pow(to.x - this->x, 2) + std::pow(to.y - this->y, 2));
}

auto sdk::vec2::view_to_field() const -> vec2
{
	vec2 v = *this;
	vec2::view_to_field(v);
	return v;
}

auto sdk::vec2::view_to_field(vec2 & v) -> void
{
	v.x = (v.x - manager::game_field::offset_x) / manager::game_field::field_ratio;
	v.y = (v.y - manager::game_field::offset_y) / manager::game_field::field_ratio;
}

auto sdk::vec2::field_to_view() const -> vec2
{
	vec2 v = *this;
	vec2::field_to_view(v);
	return v;
}

auto sdk::vec2::field_to_view(vec2 & v) -> void
{
	v.x = v.x * manager::game_field::field_ratio + manager::game_field::offset_x;
	v.y = v.y * manager::game_field::field_ratio + manager::game_field::offset_y;
}

auto sdk::vec2::normalize(const vec2 & to) const -> vec2
{
	auto dist = this->distance(to);
	return vec2(
		(to.x - this->x) / dist,
		(to.y - this->y) / dist
	);
}

auto sdk::vec2::forward(const vec2 & to, float fwd_distance) const -> vec2
{
	auto norm = this->normalize(to);
	return vec2(
		this->x + fwd_distance * norm.x,
		this->y + fwd_distance * norm.y
	);
}

auto sdk::vec2::range(const vec2 & to, float field) const -> rangestat
{
	return this->distance(to) <= field ? rangestat::INSIDE : rangestat::OUTSIDE;
}

sdk::vec2::operator ImVec2() const
{
	return ImVec2(this->x, this->y);
}

sdk::vec2::operator POINT() const
{
	return POINT { .x = LONG(this->x), .y = LONG(this->y) };
}
