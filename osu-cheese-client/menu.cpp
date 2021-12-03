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
	
	ImGui::Begin("osu!");
	ImGui::Text("[PAUSE] Key to toggle menu.");

	ImGui::Separator();

	ImGui::Checkbox("Aim assist", &features::assist::aa_enable);
	OC_IMGUI_HOVER_TXT("Enable aim assistance - Corrects your aim to the neareast hit object when moving your cursor.");
	ImGui::SliderFloat("FOV", &features::assist::aa_fov, 0.f, 800.f);
	OC_IMGUI_HOVER_TXT("Distance between your cursor and the hit object required before aim assistance activates. (0 = Global)");
	ImGui::SliderFloat("Safezone FOV", &features::assist::aa_safezone, 0.f, 800.f);
	OC_IMGUI_HOVER_TXT("Disables the aim assist when the player cursor is within the safezone. (0 = Never)");
	ImGui::SliderFloat("Assist strength", &features::assist::aa_strength, 0.f, 60.f);
	OC_IMGUI_HOVER_TXT("Strength of the aim assist. (0 = Instant lock)");
	ImGui::SliderFloat("Target time offset ratio", &features::assist::aa_timeoffsetratio, 0.f, 1.f);
	OC_IMGUI_HOVER_TXT("Amount of time ahead on recognizing a hit object as active.");

	ImGui::Separator();

	ImGui::Checkbox("Relax", &features::assist::rx_enable);
	OC_IMGUI_HOVER_TXT("Auto click hit objects.");
	ImGui::SliderFloat("Edge field", &features::assist::rx_edge, 0.f, 300.f);
	OC_IMGUI_HOVER_TXT("Edge field area where relax is enabled. (0 = Global)");
	ImGui::SliderInt("Timing offset", &features::assist::rx_offset, -10000, 10000);
	OC_IMGUI_HOVER_TXT("Hit timing offset from the hit object's time.");
	ImGui::Checkbox("Randomize timing offset", &features::assist::rx_offsetrand);
	OC_IMGUI_HOVER_TXT("Randomizes the timing offset and uses the current timing offset as the range.");

	ImGui::Separator();

	ImGui::Checkbox("Hit object timer", &features::visuals::ho_timer);
	OC_IMGUI_HOVER_TXT("Shows a countdown timer towards the next hit object.");
	ImGui::Checkbox("Hit object distance", &features::visuals::ho_distance);
	OC_IMGUI_HOVER_TXT("Shows the distance between the player cursor and the next hit object (also your current FOV value).");
	ImGui::Checkbox("Hit object tracer", &features::visuals::ho_tracer);
	OC_IMGUI_HOVER_TXT("Draws a line from the players cursor to the next hit object.");

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