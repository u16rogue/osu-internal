#include "beatmap_manager.hpp"

auto manager::beatmap::load(std::filesystem::path & file) -> bool
{
	std::vector<sdk::hit_object> _mm_ho_copy {}; // doing this because the global one isn't being properly initialized when manually mapped

	if (!utils::beatmap::dump_hitobjects_from_file(file, _mm_ho_copy))
		hitobjects.clear();

	hitobjects = std::move(_mm_ho_copy);

	#ifdef OSU_CHEESE_DEBUG_BUILD
	{
		for (const auto & o : hitobjects)
			DEBUG_PRINTF("\n[D] X: %.0f, Y: %.0f, TIME: %d, TYPE: %d", o.x, o.y, o.time, o.type);

		DEBUG_PRINTF("\n[D] Dumped %d HitObjects!", hitobjects.size());
	}
	#endif

	return true;
}

auto manager::beatmap::unload() -> void
{
}

auto manager::beatmap::get_coming_hitobject() -> const sdk::hit_object *
{
	// TODO: optimize this to skip past iterated hitobjects
	for (int i = 0; i < hitobjects.size() - 1; ++i)
	{
		const auto & o = hitobjects;
		if (game::p_game_info->beat_time <= o[i].time && (i == 0 || game::p_game_info->beat_time > o[i - 1].time))
			return &o[i];
	}

	return nullptr;
}
