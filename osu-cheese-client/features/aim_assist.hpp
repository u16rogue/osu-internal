#pragma once

#include <Windows.h>
#include "../sdk/osu_vec.hpp"

#include <optional>
#include <deque>

namespace features
{
	class aim_assist
	{

		enum class PATHING : int
		{
			LINEAR,
			CURVED
		};

		struct point_record
		{
			sdk::vec2 point {};
			DWORD tick {};
		};

		// Generic settings
		inline static bool enable = false;
		inline static int max_tick_sample = 200;
		inline static bool do_prediction = false;
		inline static PATHING path_mode = PATHING::LINEAR;
		inline static float t_val = .5f;

		inline static bool ds_use_count = false;
		inline static int count_direction_sampling = 4;
		inline static float dst_direction_sampling = 12.f;

		inline static bool silent = true;
		inline static int max_reach_time_offset = 500;
		inline static float distance_fov = 30.f; 
		inline static float directional_fov = 80.f;

		inline static float time_offset_ratio = 0.8f;

		// Visuals settings

		// Directional curve settings

		enum class TARGETTING
		{
			NONE,
			GOING_TO,
			TO,
			GOING_HOME,
			HOME
		};

		// Internal calculations and tracking
		inline static bool      use_set = false;
		inline static void *    last_lock = nullptr;
		inline static sdk::vec2 set_point {};
		inline static sdk::vec2 aa_end_point {};
		inline static sdk::vec2 aa_start_point {};
		inline static int       aa_start_time {};
		inline static int       aa_end_time {};

		inline static sdk::vec2 aa_home_point {}; // start point to home
		inline static int aa_home_start {};
		inline static TARGETTING tmode = TARGETTING::TO;

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
		static auto extrap_to_point(const sdk::vec2 & start, const sdk::vec2 & end, const float & t, const float & rate) -> sdk::vec2;
		static auto predict_time_to_point(const sdk::vec2 start, const sdk::vec2 end) -> float;
		static auto check_aim_assist() -> void;
		static auto move_aim_assist() -> void;
		static auto collect_sampling (const sdk::vec2 & cpoint) -> void;
		static auto run_sampling() -> void;
	};
}