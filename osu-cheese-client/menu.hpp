#pragma once

#include <Windows.h>


class menu
{
public:
	menu() = delete;

	inline static bool visible = false;

	static auto render() -> void;
	static auto wndproc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam) -> bool;
};