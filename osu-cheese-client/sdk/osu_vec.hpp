#pragma once

#include <imgui.h>
#include <Windows.h>

namespace sdk
{
	struct vec2
	{
		enum class rangestat
		{
			INSIDE,
			OUTSIDE
		};

		float x, y;

		vec2()
			: x(0.f), y(0.f) {}

		vec2(float x_, float y_)
			: x(x_), y(y_) {}

		vec2(int x_, int y_)
			: x(float(x_)), y(float(y_)) {}

		auto distance(const vec2 & to) const -> float;

		auto view_to_field() const -> vec2;
		static auto view_to_field(vec2 & v) -> void;
		auto field_to_view() const -> vec2;
		static auto field_to_view(vec2 & v) -> void;
		
		auto normalize(const vec2 & to) const -> vec2;
		auto forward(const vec2 & to, float fwd_distance = 1.f) const -> vec2;

		auto range(const vec2 & to, float field) const -> rangestat;

		auto operator ==(const vec2 & rhs) const noexcept -> bool;

		operator ImVec2() const;
		operator POINT() const;
	};
}