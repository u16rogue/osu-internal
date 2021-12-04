#include "assist.hpp"

#include "../manager/gamefield_manager.hpp"
#include <sed/macro.hpp>
#include "../game.hpp"
#include <numbers>
#include "../manager/beatmap_manager.hpp"

auto features::assist::run_aimassist(HWND osu_wnd, sdk::vec2 mousepos) -> void
{
	static const sdk::hit_object * ho_filter = nullptr;

	if (!aa_enable || !game::pp_info_player->async_complete || game::pp_info_player->is_replay_mode || !game::p_game_info->is_playing)
		return;

	auto [ho, i] = manager::beatmap::get_coming_hitobject();
	if (!ho || ho == ho_filter)
		return;

	// Check time offset
	if (aa_timeoffsetratio != 0.f && i != 0)
	{
		auto prev = ho - 1;
		auto time_sub = ho->time - ((ho->time - prev->time) * (1.f - aa_timeoffsetratio));
		if (game::p_game_info->beat_time < time_sub)
			return;
	}
	
	auto player_field_pos = mousepos.view_to_field();
	auto dist_to_ho = player_field_pos.distance(ho->coords);

	// Check fov
	if (aa_fov != 0.f && dist_to_ho > aa_fov)
		return;

	// Safezone override
	if (aa_safezone != 0.f && dist_to_ho <= aa_safezone)
	{
		ho_filter = ho;
		return;
	}
	
	auto aa_new_pos = player_field_pos.forward(ho->coords, std::clamp(aa_strength, 0.f, dist_to_ho)).field_to_view();
	POINT pscr { .x = LONG(aa_new_pos.x), .y = LONG(aa_new_pos.y) };
	ClientToScreen(osu_wnd, &pscr);
	SetCursorPos(pscr.x, pscr.y);
}

auto features::assist::run_relax(int vx, int vy) -> void
{
	static const sdk::hit_object * filter_ho = nullptr;

	if (!rx_enable || !game::pp_info_player->async_complete || game::pp_info_player->is_replay_mode || !game::p_game_info->is_playing)
		return;

	auto [ho, i] = manager::beatmap::get_coming_hitobject();
	if (!ho)
		return;
	
	if (rx_offset >= 0)
		--ho;

	if (ho->time + rx_offset <= game::p_game_info->beat_time || ho == filter_ho)
		return;

	filter_ho = ho;

	INPUT inp[2] { 0 };
	inp[0].type = INPUT_MOUSE;
	inp[0].mi.dwFlags = MOUSEEVENTF_LEFTDOWN;

	inp[1].type = INPUT_MOUSE;
	inp[1].mi.dwFlags = MOUSEEVENTF_LEFTUP;

	SendInput(2, inp, sizeof(INPUT));
}