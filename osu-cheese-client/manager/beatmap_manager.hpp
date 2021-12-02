#pragma once

#include "../utils/beatmap.hpp"
#include "gamefield_manager.hpp"
#include "../game.hpp"
#include <vector>
#include <filesystem>

namespace manager
{
	class beatmap
	{
	public:
		using hitobjects_t = std::vector<sdk::hit_object>;

		beatmap() = delete;

		static auto load(std::filesystem::path & file) -> bool;
		static auto unload() -> void;

		static auto get_coming_hitobject() -> const sdk::hit_object *;

	private:
		inline static hitobjects_t hitobjects;
	};
}