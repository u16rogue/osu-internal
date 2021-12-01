#pragma once

#include <filesystem>
#include <vector>
#include "../sdk/osu_file.hpp"

namespace features
{
	class assist
	{
		using hitobjects_t = std::vector<sdk::hit_object>;

	public:
		inline static bool  aa_enable = true;
		inline static float aa_fov = 20.f;

		assist() = delete;
		static auto load_beatmap(std::filesystem::path & file) -> void;
		static auto run_aimassist(int vx, int vy) -> void;
		static auto run_relax(int vx, int vy) -> void;

	private:
		inline static hitobjects_t hitobjects;
	};
}