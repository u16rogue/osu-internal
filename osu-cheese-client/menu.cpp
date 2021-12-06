#include "menu.hpp"

#include <imgui.h>
#include <imgui_impl_win32.h>
#include <sed/macro.hpp>

#include "features/features.hpp"

extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);

auto menu::render() -> void
{
	if (!menu::visible)
		return;
	
	ImGui::Begin("osu!");
	ImGui::Text("[PAUSE] Key to toggle menu.");

	if (ImGui::BeginTabBar("##oc_tabs"))
	{
		features::feature::on_tab_render();
		ImGui::EndTabBar();	
	}
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

	switch (Msg)
	{
		case WM_KEYDOWN:
		case WM_KEYUP:
		case WM_LBUTTONDBLCLK:
		case WM_LBUTTONDOWN:
		case WM_LBUTTONUP:
		case WM_RBUTTONDBLCLK:
		case WM_RBUTTONDOWN:
		case WM_RBUTTONUP:
		case WM_XBUTTONDBLCLK:
		case WM_XBUTTONDOWN:
		case WM_XBUTTONUP:
			return true;
	}

	return false;
}