#pragma once

#include <Windows.h>
#include "sdk/osu_vec.hpp"

class menu
{
public:
	menu() = delete;

	inline static bool      visible = false;
	inline static sdk::vec2 freeze_raw_point;
	

	static auto render() -> void;
	static auto wndproc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam) -> bool;
};