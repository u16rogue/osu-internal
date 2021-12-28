#pragma once

#include <Windows.h>
#include "../sdk/osu_vec.hpp"

namespace features
{
	class aim_assist
	{
		enum class method_e : int
		{
			LINEAR,
			DIRECTIONAL_CURVE
		};
		
		inline static bool  enable          = false;
		inline static float fov             = 0.f;
		inline static float dir_fov         = 20.f;
		inline static float safezone        = 20.f;
		inline static float scaleassist     = 1.f;
		inline static float timeoffsetratio = 0.8f;

		inline static method_e method = method_e::LINEAR;

		inline static sdk::vec2 last_tick_point  {};
		inline static sdk::vec2 player_direction {};

		inline static bool vis_fov         = false;
		inline static bool vis_safezonefov = false;

		inline static float velocity { 0.f };

	public:
		aim_assist() = delete;
		static auto on_tab_render() -> void;
		static auto on_wndproc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam, void * reserved) -> bool;
		static auto on_render() -> void;
		static auto on_osu_set_raw_coords(sdk::vec2 * raw_coords) -> void;
	};
}