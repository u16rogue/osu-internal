#pragma once

#include "../utils/beatmap.hpp"
#include "gamefield_manager.hpp"
#include "../game.hpp"
#include <vector>
#include <filesystem>
#include <tuple>
#include <thread>
#include <optional>

namespace manager
{
	class beatmap
	{
	public:
		
		enum class status_e
		{
			LOADED,
			LOADING,
			UNLOADED,
		};

		using hitobjects_t = std::vector<sdk::hit_object>;

		beatmap() = delete;

		static auto load(std::filesystem::path & file) -> bool;
		static auto unload() -> void;

		static auto loaded() -> bool;
		static auto get_coming_hitobject() -> std::pair<const sdk::hit_object *, int>;

	public:
		inline static status_e status { status_e::UNLOADED };

	private:
		inline static std::optional<std::filesystem::path> worker_requests; // this should be a stack to properly model a fifo
		static auto worker_load() -> void;
		inline static bool worker_initialized { false };
		inline static std::thread worker_thread;

	private:
		inline static hitobjects_t hitobjects;
	};
}