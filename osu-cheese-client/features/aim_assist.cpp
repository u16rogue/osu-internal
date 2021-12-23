#include "aim_assist.hpp"

#include <imgui.h>
#include <sed/macro.hpp>

#include "../game.hpp"
#include "../sdk/osu_vec.hpp"

#include "../manager/gamefield_manager.hpp"
#include "../manager/beatmap_manager.hpp"

#if 0
auto features::aim_assist::get_velocity() -> float
{
	int   sample_count { 0 };
	float result      { 0.f };

	auto tick = GetTickCount();

	for (const auto & v : move_samples)
	{
		if (v.tick < tick)
			continue;

		++sample_count;
		//result += v.
	}

	return 0.0f;
}

auto features::aim_assist::push_sample(sdk::vec2 & pos) -> void
{
}
#endif

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
	return false;
}

auto features::aim_assist::on_render() -> void
{
	// HACK: DEBUG CODE! REMOVE!
	auto _draw = ImGui::GetBackgroundDrawList();
	// Visualize player direction
	_draw->AddLine(game::pp_viewpos_info->pos, (game::pp_viewpos_info->pos + (player_direction * 80.f)), 0xFFFFFFFF, 4.f);
	// Velocity
	std::string _dbg_txt_info = "Sample velocity: " + std::to_string(velocity) + "\nDegrees: " + std::to_string(player_direction.norm2rad2deg());
	_draw->AddText(game::pp_viewpos_info->pos + 1.f, 0xFF000000, _dbg_txt_info.c_str());
	_draw->AddText(game::pp_viewpos_info->pos, 0xFFFFFFFF, _dbg_txt_info.c_str());
	// Visualize directional FOV
	float cur_ang = player_direction.norm2rad2deg();
	float min_b_ang = cur_ang - dir_fov;
	float max_b_ang = cur_ang + dir_fov;
	auto min_dir = sdk::vec2::from_deg(min_b_ang);
	auto max_dir = sdk::vec2::from_deg(max_b_ang);
	auto min_point = min_dir * 80.f;
	auto max_point = max_dir * 80.f;

	_draw->AddLine(game::pp_viewpos_info->pos, game::pp_viewpos_info->pos + min_point, 0xFFFFFFFF, 4.f);
	_draw->AddLine(game::pp_viewpos_info->pos, game::pp_viewpos_info->pos + max_point, 0xFFFFFFFF, 4.f);

	if (!enable || !manager::beatmap::loaded() || !game::pp_info_player->async_complete || game::pp_info_player->is_replay_mode)
		return;

	auto draw = ImGui::GetBackgroundDrawList();

	if (vis_fov)
		draw->AddCircle(game::pp_viewpos_info->pos, fov, 0xFFFFFFFF);

	auto [ho, i] = manager::beatmap::get_coming_hitobject();
	if (!ho)
		return;

	if (vis_safezonefov)
		draw->AddCircle(ho->coords.field_to_view(), safezone, 0xFFFFFFFF);

}

auto features::aim_assist::on_osu_set_raw_coords(sdk::vec2 * raw_coords) -> void
{
	static const sdk::hit_object * ho_filter = nullptr;

	if (auto _velocity = last_tick_point.distance(*raw_coords); _velocity != 0.f)
	{
		velocity = _velocity;
		player_direction = last_tick_point.normalize(*raw_coords);
		last_tick_point = *raw_coords;
	}

	if (!enable || !manager::beatmap::loaded() || !game::pp_info_player->async_complete || game::pp_info_player->is_replay_mode || !game::p_game_info->is_playing)
		return;

	auto [ho, i] = manager::beatmap::get_coming_hitobject();
	if (!ho || ho == ho_filter)
		return;

	// Check time offset
	if (timeoffsetratio != 0.f && i != 0)
	{
		auto prev = ho - 1;
		auto time_sub = ho->time - ((ho->time - prev->time) * (1.f - timeoffsetratio));
		if (game::p_game_info->beat_time < time_sub)
			return;
	}

	auto player_field_pos = game::pp_viewpos_info->pos.view_to_field();
	auto dist_to_ho = player_field_pos.distance(ho->coords);

	// Check fov
	if (fov != 0.f && dist_to_ho > fov)
		return;

	// Safezone override
	if (safezone != 0.f && dist_to_ho <= safezone)
	{
		ho_filter = ho;
		return;
	}

	auto new_coords = strength == 0.f ? ho->coords.field_to_view() : player_field_pos.forward_towards(ho->coords, std::clamp(strength, 0.f, dist_to_ho)).field_to_view();

	*raw_coords = new_coords;

	if (!game::pp_raw_mode_info->is_raw)
	{
		POINT pscr = new_coords;
		ClientToScreen(game::pp_wnd_info->handle, &pscr);
		SetCursorPos(pscr.x, pscr.y);
	}

	return;
}
