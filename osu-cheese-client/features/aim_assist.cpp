#include "aim_assist.hpp"

#include <imgui.h>
#include <sed/macro.hpp>

#include "../game.hpp"
#include "../sdk/osu_vec.hpp"

#include "../manager/gamefield_manager.hpp"
#include "../manager/beatmap_manager.hpp"


auto features::aim_assist::on_tab_render() -> void
{
	if (!ImGui::BeginTabItem("Aim assist"))
		return;

	ImGui::Checkbox("Aim assist", &enable);
	OC_IMGUI_HOVER_TXT("Enable aim assistance - Corrects your aim to the neareast hit object when moving your cursor.");
	ImGui::SliderFloat("FOV", &fov, 0.f, 800.f);
	OC_IMGUI_HOVER_TXT("Distance between your cursor and the hit object required before aim assistance activates. (0 = Global)");
	ImGui::SliderFloat("Safezone FOV", &safezone, 0.f, 800.f);
	OC_IMGUI_HOVER_TXT("Disables the aim assist when the player cursor is within the safezone. (0 = Never)");
	ImGui::SliderFloat("Assist strength", &strength, 0.f, 60.f);
	OC_IMGUI_HOVER_TXT("Strength of the aim assist. (0 = Instant lock)");
	ImGui::SliderFloat("Target time offset ratio", &timeoffsetratio, 0.f, 1.f);
	OC_IMGUI_HOVER_TXT("Amount of time ahead on recognizing a hit object as active.");

	ImGui::Separator();

	ImGui::Checkbox("Visualize Aim FOV", &vis_fov);
	ImGui::Checkbox("Visualize Safezone FOV", &vis_safezonefov);

	ImGui::EndTabItem();
}

auto features::aim_assist::on_wndproc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam, void * reserved) -> bool
{
	static const sdk::hit_object * ho_filter = nullptr;

	if (Msg == WM_MOUSEMOVE || !enable || !manager::beatmap::loaded() || !game::pp_info_player->async_complete || game::pp_info_player->is_replay_mode || !game::p_game_info->is_playing)
		return false;
	
	auto [ho, i] = manager::beatmap::get_coming_hitobject();
	if (!ho || ho == ho_filter)
		return false;

	// Check time offset
	if (timeoffsetratio != 0.f && i != 0)
	{
		auto prev = ho - 1;
		auto time_sub = ho->time - ((ho->time - prev->time) * (1.f - timeoffsetratio));
		if (game::p_game_info->beat_time < time_sub)
			return false;
	}

	auto player_field_pos = game::pp_pos_info->pos;
	auto dist_to_ho = player_field_pos.distance(ho->coords);

	// Check fov
	if (fov != 0.f && dist_to_ho > fov)
		return false;

	// Safezone override
	if (safezone != 0.f && dist_to_ho <= safezone)
	{
		ho_filter = ho;
		return false;
	}
	
	POINT pscr = strength == 0.f ? ho->coords.field_to_view() : player_field_pos.forward(ho->coords, std::clamp(strength, 0.f, dist_to_ho)).field_to_view();
	ClientToScreen(hWnd, &pscr);
	SetCursorPos(pscr.x, pscr.y);
	return true;
}

auto features::aim_assist::on_render() -> void
{
	if (!enable || !manager::beatmap::loaded() || !game::pp_info_player->async_complete || game::pp_info_player->is_replay_mode)
		return;

	auto draw = ImGui::GetBackgroundDrawList();

	if (vis_fov)
		draw->AddCircle(game::pp_pos_info->pos, fov, 0xFFFFFFFF);

	if (vis_safezonefov)
	{
		auto [ho, i] = manager::beatmap::get_coming_hitobject();
		if (!ho)
			return;

		draw->AddCircle(ho->coords.field_to_view(), safezone, 0xFFFFFFFF);
	}

}

auto features::aim_assist::on_osu_set_raw_coords(sdk::vec2 * raw_coords) -> void
{
}
