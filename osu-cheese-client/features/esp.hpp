#pragma once

#include <Windows.h>

namespace features
{
	class esp
	{

		inline static bool timer     = false;
		inline static bool tracer    = false;
		inline static bool distance  = false;

	public:
		esp() = delete;
		static auto on_tab_render() -> void;
		static auto on_wndproc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam, void * reserved) -> bool;
		static auto on_render() -> void;
	};
}