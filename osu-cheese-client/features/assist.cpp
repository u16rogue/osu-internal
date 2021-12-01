#include "assist.hpp"

#include "../utils/beatmap.hpp"
#include "../sdk/gamefield.hpp"
#include <sed/macro.hpp>

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

auto features::assist::run_aimassist(int vx, int vy) -> void
{

}

auto features::assist::run_relax(int vx, int vy) -> void
{
}
