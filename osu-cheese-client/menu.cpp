#include "menu.hpp"

#include <imgui.h>
#include <imgui_impl_win32.h>
#include <sed/macro.hpp>

extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);

auto menu::render() -> void
{
	if (!menu::visible)
		return;

	ImGui::Begin("osu!cheese");
	ImGui::Text("[PAUSE] Key to toggle menu.");
	ImGui::End();
}

auto menu::wndproc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam) -> bool
{
	if (Msg == WM_KEYDOWN && wParam == VK_PAUSE)
	{
		menu::visible = !menu::visible;
		return true;
	}

	if (!menu::visible)
		return false;

	ImGui_ImplWin32_WndProcHandler(hWnd, Msg, wParam, lParam);
	return true;
}