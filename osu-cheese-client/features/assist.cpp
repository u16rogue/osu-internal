#include "assist.hpp"

#include "../utils/beatmap.hpp"
#include "../sdk/gamefield.hpp"
#include <sed/macro.hpp>
#include "../game.hpp"

auto features::assist::load_beatmap(std::filesystem::path & file) -> void
{
	std::vector<sdk::hit_object> _mm_ho_copy {}; // doing this because the global one isn't being properly initialized when manually mapped

	if (!utils::beatmap::dump_hitobjects_from_file(file, _mm_ho_copy))
		features::assist::hitobjects.clear();

	features::assist::hitobjects = std::move(_mm_ho_copy);

	#ifdef OSU_CHEESE_DEBUG_BUILD
	{
		for (const auto & o : features::assist::hitobjects)
			DEBUG_PRINTF("\n[D] X: %.0f, Y: %.0f, TIME: %d, TYPE: %d", o.x, o.y, o.time, o.type);

		DEBUG_PRINTF("\n[D] Dumped %d HitObjects!", features::assist::hitobjects.size());
	}
	#endif

	// TODO: find a way to load reso
	sdk::game_field::resize(800, 600);
	DEBUG_PRINTF("\n[+] Loaded game field as 800x600!");
}

auto features::assist::run_aimassist(HWND osu_wnd, int vx, int vy) -> void
{
	static const sdk::hit_object * last_hitobj = nullptr;

	if (!game::pp_info_player->async_complete || game::pp_info_player->is_replay_mode || !game::p_game_info->is_playing)
		return;

	const auto curr = features::assist::get_coming_hitobject();
	if (!curr || curr == last_hitobj || curr->time - 4 > game::p_game_info->beat_time)
		return;

	DEBUG_PRINTF("\n[D] CURR -> %.0f, %.0f", curr->x, curr->y);
	auto [nx, ny] = sdk::game_field::f2v(curr->x, curr->y);
	POINT pscr { .x = nx, .y = ny };
	ClientToScreen(osu_wnd, &pscr);
	SetCursorPos(pscr.x, pscr.y);
	last_hitobj = curr;
}

auto features::assist::run_relax(int vx, int vy) -> void
{
}

auto features::assist::get_coming_hitobject() -> const sdk::hit_object *
{
	// TODO: optimize this to skip past iterated hitobjects
	for (int i = 0; i < features::assist::hitobjects.size() - 1; ++i)
	{
		const auto & o = features::assist::hitobjects;
		if (game::p_game_info->beat_time <= o[i].time && (i == 0 || game::p_game_info->beat_time > o[i - 1].time))
			return &o[i];
	}

	return nullptr;
}
