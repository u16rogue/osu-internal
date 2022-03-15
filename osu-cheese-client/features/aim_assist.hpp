#pragma once

#include <Windows.h>
#include "../sdk/osu_vec.hpp"

namespace features
{
	class aim_assist
	{

		// Generic settings
		inline static bool  enable          = false;

		// Visuals settings

		// Directional curve settings

		// Internal calculations and tracking

	public:
		aim_assist() = delete;
		static auto on_tab_render() -> void;
		static auto on_wndproc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam, void * reserved) -> bool;
		static auto on_render() -> void;
		static auto on_osu_set_raw_coords(sdk::vec2 * raw_coords) -> void;
		static auto osu_set_field_coords_rebuilt(sdk::vec2 * out_coords) -> void;

	private:
		static auto run_aim_assist(sdk::vec2 * pcoords) -> void;
	};
}