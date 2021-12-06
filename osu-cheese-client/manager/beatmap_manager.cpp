#include "beatmap_manager.hpp"

auto manager::beatmap::load(std::filesystem::path & file) -> bool
{
	worker_requests = std::make_optional(file);

	if (!worker_initialized)
		worker_thread = std::thread(worker_load);
	
	return true;
}

auto manager::beatmap::unload() -> void
{
	hitobjects.clear();
	status = status_e::UNLOADED;
}

auto manager::beatmap::loaded() -> bool
{
	return status == status_e::LOADED;
}

auto manager::beatmap::get_coming_hitobject() -> std::pair<const sdk::hit_object *, int>
{
	if (hitobjects.empty())
		return std::make_pair(nullptr, -1);

	// TODO: optimize this to skip past iterated hitobjects
	for (int i = 0; i < hitobjects.size() - 1; ++i)
	{
		const auto & o = hitobjects;
		if (game::p_game_info->beat_time <= o[i].time && (i == 0 || game::p_game_info->beat_time > o[i - 1].time))
			return std::make_pair(&o[i], i);
	}

	return std::make_pair(nullptr, -1);
}

auto manager::beatmap::worker_load() -> void
{
	worker_initialized = true;
	DEBUG_PRINTF("\n[D] Worker initialized!");

	while (worker_initialized)
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(50));
		if (!worker_requests)
			continue;

		DEBUG_PRINTF("\n[D] Worker is now dumping the beatmap...");

		std::filesystem::path file = *worker_requests;
		worker_requests = std::nullopt;
		unload();
		status = status_e::LOADING;

		std::vector<sdk::hit_object> _mm_ho_copy {}; // doing this because the global one isn't being properly initialized when manually mapped

		bool res = utils::beatmap::dump_hitobjects_from_file(file, _mm_ho_copy);
		//worker_requests.pop();

		if (!res)
		{
			status = status_e::UNLOADED;
			continue;
		}

		hitobjects = std::move(_mm_ho_copy);
		status = status_e::LOADED;
		DEBUG_PRINTF("\n[D] Worker finished dumping the beatmap!");
	}
}
