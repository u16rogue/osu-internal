#pragma once

#include <Windows.h>

namespace features
{
	class aim_assist
	{
		inline static bool  enable = false;
		inline static float fov = 0.f;
		inline static float safezone = 20.f;
		inline static float strength = 20.f;
		inline static float timeoffsetratio = 0.8f;

	public:
		aim_assist() = delete;
		static auto on_tab_render() -> void;
		static auto on_wndproc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam, void * reserved) -> bool;
		static auto on_render() -> void;
	};
}