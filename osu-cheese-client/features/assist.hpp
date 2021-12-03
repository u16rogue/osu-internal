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
		inline static float aa_safezone = 20.f;
		inline static float aa_strength = 20.f;
		inline static float aa_timeoffsetratio = 0.8f;

		inline static bool rx_enable = false;
		inline static float rx_edge = 80.f;
		inline static int  rx_offset = -50;
		inline static bool rx_offsetrand = false;

		assist() = delete;
		static auto run_aimassist(HWND osu_wnd, int vx, int vy) -> void;
		static auto run_relax(int vx, int vy) -> void;
	};
}