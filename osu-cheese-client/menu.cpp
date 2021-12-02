#include "menu.hpp"

#include <imgui.h>
#include <imgui_impl_win32.h>
#include <sed/macro.hpp>

#include "features/assist.hpp"
#include "features/visuals.hpp"

#define OC_IMGUI_HOVER_TXT(fmt, ...) \
	if (ImGui::IsItemHovered()) \
		ImGui::SetTooltip(fmt, __VA_ARGS__)

extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);

auto menu::render() -> void
{
	if (!menu::visible)
		return;

	ImGui::Begin("papaya!bass");
	ImGui::Text("[PAUSE] Key to toggle menu.");

	ImGui::Separator();

	ImGui::Checkbox("Aim assist", &features::assist::aa_enable);
	OC_IMGUI_HOVER_TXT("Enable aim assistance - Corrects your aim to the neareast hit object when moving your cursor.");
	
	ImGui::SliderFloat("FOV:", &features::assist::aa_fov, 0.f, 600.f);
	OC_IMGUI_HOVER_TXT("Distance between your cursor and the hit object required before aim assistance activates.");
	
	ImGui::SliderInt("Target time offset", &features::assist::aa_timeoffset, 0, 10000);
	OC_IMGUI_HOVER_TXT("Amount of time ahead on recognizing a hit object as active. (curtime >= ho.time - time_offset)");

	ImGui::Separator();
	ImGui::Checkbox("Hit object timer", &features::visuals::ho_timer);
	OC_IMGUI_HOVER_TXT("Shows a countdown timer towards the next hit object.");

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