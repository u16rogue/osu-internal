#pragma once

#include <Windows.h>

namespace features
{
	class relax
	{
		inline static bool  enable = false;
		inline static float edge = 80.f;
		inline static int   offset = -50;
		inline static bool  offsetrand = false;

	public:
		relax() = delete;
		static auto on_tab_render() -> void;
		static auto on_wndproc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam, void * reserved) -> bool;
		static auto on_render() -> void;
	};
}