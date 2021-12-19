#include "beatmap.hpp"

#include <fstream>
#include <memory>
#include <sed/strings.hpp>
#include <sed/macro.hpp>

auto utils::beatmap::find_file_by_title(std::wstring_view title) -> std::optional<std::filesystem::path>
{
	// TODO: implement better beatmap detection, this is really ghetto...

	auto diff_tok = title.find_last_of('[');
	auto song_title = title.substr(0, diff_tok - 1);
	auto difficulty = title.substr(diff_tok);

	for (auto & songsfolder : std::filesystem::directory_iterator { "Songs" })
	{
		if (!songsfolder.is_directory())
			continue;

		for (auto & songfile : std::filesystem::directory_iterator { songsfolder })
		{
			if (songfile.path().extension() != ".osu")
				continue;

			std::wstring file = songfile.path().filename();
			if (file.find(song_title) == std::wstring::npos || file.find(difficulty) == std::wstring::npos)
				continue;

			return std::make_optional(songfile);
		}
	}

	return std::nullopt;
}

static auto beatmap_traverse_tag(const char * tag, char * buffer, char * end) -> char *
{
	const auto tag_len = strlen(tag);

	do
	{
		if (*buffer != '[')
			continue;
			
		for (int i = 0; i < tag_len; ++i)
		{
			if (buffer[i + 1] != tag[i])
				break;

			if (i == tag_len - 1 && buffer[i + 2] == ']')
				return &buffer[i + 4];
		}
	} while (++buffer + tag_len < end);

	return nullptr;
}

static auto beatmap_next_object(char *& buffer, char * end) -> bool
{
	constexpr auto match = "\r\n";
	constexpr auto len   = sed::str_len(match);

	while (buffer + 3 < end)
	{
		if (*buffer == '\n')
		{
			buffer = &buffer[1];
			return true;
		}

		++buffer;
	}

	return false;
}

// WARNING: only used for parsing HitObjects, not meant for anything else
static auto beatmap_parse_item(char *& buffer, char * end, int & out_value) -> bool
{
	for (; buffer < end; ++buffer)
	{
		if (*buffer < '0' || *buffer > '9')
		{
			++buffer;
			return true;
		}

		out_value *= 10;
		out_value += *buffer - '0';
	}

	return false;
}

auto utils::beatmap::dump_hitobjects_from_file(std::filesystem::path file, std::vector<sdk::hit_object> & out_objects) -> bool
{
	out_objects.clear();

	if (!std::filesystem::exists(file))
		return false;

	std::ifstream fstr(file, std::ifstream::in);
	if (!fstr.is_open())
		return false;

	fstr.seekg(0, fstr.end);
	auto len = fstr.tellg();
	fstr.seekg(0, fstr.beg);

	auto buffer = std::make_unique<char[]>(std::size_t(len) + 1);
	auto eob = buffer.get() + len;

	fstr.read(buffer.get(), len);

	auto current = beatmap_traverse_tag("HitObjects", buffer.get(), eob);
	if (!current)
		return false;

	do
	{
		int x { 0 }, y { 0 }, time { 0 }, type { 0 };

		beatmap_parse_item(current, eob, x);
		beatmap_parse_item(current, eob, y);
		beatmap_parse_item(current, eob, time);
		beatmap_parse_item(current, eob, type);
		
		out_objects.emplace_back(sdk::hit_object
		{
			sdk::vec2(x, y),
			.time = time,
			.type = sdk::hit_type(type)
		});

	} while(beatmap_next_object(current, eob));

	return true;
}
