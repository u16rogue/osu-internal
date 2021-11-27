#include "game.hpp"

#include <sed/console.hpp>
#include <sed/memory.hpp>

template <typename T>
static auto pattern_scan_helper(const char * name, T & out, const char * pattern, const char * mask, int rel32sz = 0) -> bool
{
	printf("\n[+] Searching for %s...", name);
	auto res = sed::pattern_scan_exec_region(nullptr, -1, pattern, mask);
	printf("0x%p", res);

	if (!res)
		return false;

	out = reinterpret_cast<T>(res);

	if (rel32sz)
	{
		out = reinterpret_cast<T>(sed::rel2abs32(static_cast<T>(out), rel32sz));
		printf(" -> 0x%p", out);
	}
	
	return true;
}

auto game::initialize() -> bool
{
	printf("\n[+] Loading game information...");
	
	if (!pattern_scan_helper("game::p_game_info", game::p_game_info, "\xDB\x05\x00\x00\x00\x00\xD9\x5D\xF8", "xx????xxx", 0x6)
	||  !pattern_scan_helper("game::pp_info_player", game::pp_info_player, "\x8B\x15\x00\x00\x00\x00\x8B\x72", "xx????xx", 0x6)
	) {
		return false;
	}

	return true;
}
