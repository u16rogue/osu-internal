#pragma once

#include <string>
#include <string_view>
#include <filesystem>
#include <optional>
#include <vector>

#include "../sdk/osu_file.hpp"

namespace utils::beatmap
{
	auto find_file_by_title(std::wstring_view title) -> std::optional<std::filesystem::path>;
	auto dump_hitobjects_from_file(std::filesystem::path file, std::vector<sdk::hit_object> & out_objects) -> bool;
}