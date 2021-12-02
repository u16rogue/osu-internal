#include "assist.hpp"

#include "../manager/gamefield_manager.hpp"
#include <sed/macro.hpp>
#include "../game.hpp"
#include "../manager/beatmap_manager.hpp"

auto features::assist::run_aimassist(HWND osu_wnd, int vx, int vy) -> void
{
	static const sdk::hit_object * last_hitobj = nullptr;

	if (!game::pp_info_player->async_complete || game::pp_info_player->is_replay_mode || !game::p_game_info->is_playing)
		return;

	const auto curr = manager::beatmap::get_coming_hitobject();
	if (!curr || curr == last_hitobj || curr->time - 4 > game::p_game_info->beat_time)
		return;

	DEBUG_PRINTF("\n[D] CURR -> %.0f, %.0f", curr->x, curr->y);
	auto [nx, ny] = manager::game_field::f2v(curr->x, curr->y);
	POINT pscr { .x = nx, .y = ny };
	ClientToScreen(osu_wnd, &pscr);
	SetCursorPos(pscr.x, pscr.y);
	last_hitobj = curr;
}

auto features::assist::run_relax(int vx, int vy) -> void
{
}