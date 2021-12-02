#pragma once

#include <filesystem>
#include <vector>
#include "../sdk/osu_file.hpp"
#include <Windows.h>

// TODO: separate module as beatmap manager

namespace features
{
	class assist
	{
	public:
		inline static bool  aa_enable = true;
		inline static float aa_fov = 60.f;
		inline static int   aa_timeoffset = 20;

		assist() = delete;
		static auto run_aimassist(HWND osu_wnd, int vx, int vy) -> void;
		static auto run_relax(int vx, int vy) -> void;
	};
}