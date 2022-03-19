#pragma once

#include <Windows.h>
#include "../sdk/osu_vec.hpp"

#include <optional>
#include <deque>

namespace features
{
	class aim_assist
	{

		struct point_record
		{
			sdk::vec2 point {};
			DWORD tick {};
		};

		// Generic settings
		inline static bool enable = false;
		inline static int max_tick_sample = 200;
		inline static int count_direction_sampling = 4;
		inline static bool silent = true;
		inline static int max_reach_time_offset = 250;
		inline static float distance_fov = 30.f; 
		inline static float directional_fov = 50.f;

		// Visuals settings

		// Directional curve settings

		// Internal calculations and tracking
		inline static bool locking = false;
		inline static bool use_set = false;
		inline static sdk::vec2 set_point {};
		inline static sdk::vec2 target_point {};

		inline static float velocity {};
		inline static sdk::vec2 direction {};
		inline static std::optional<std::deque<point_record>> point_records;

	public:
		aim_assist() = delete;
		static auto on_tab_render() -> void;
		static auto on_wndproc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam, void * reserved) -> bool;
		static auto on_render() -> void;
		static auto on_osu_set_raw_coords(sdk::vec2 * raw_coords) -> void;
		static auto osu_set_field_coords_rebuilt(sdk::vec2 * out_coords) -> void;

	private:
		static auto check_aim_assist() -> void;
		static auto move_aim_assist() -> void;
		static auto collect_sampling (const sdk::vec2 & cpoint) -> void;
		static auto run_sampling() -> void;
	};
}