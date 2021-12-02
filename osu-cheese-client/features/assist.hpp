#pragma once

#include <Windows.h>

// TODO: separate module as beatmap manager

namespace features
{
	class assist
	{
	public:
		inline static bool  aa_enable = false;
		inline static float aa_fov = 0.f;
		inline static float aa_safezone = 80.f;
		inline static float aa_strength = 20.f;
		inline static float aa_timeoffsetratio = 0.8f;

		assist() = delete;
		static auto run_aimassist(HWND osu_wnd, int vx, int vy) -> void;
		static auto run_relax(int vx, int vy) -> void;
	};
}