#include "game.hpp"

#include <sed/console.hpp>
#include <sed/memory.hpp>

template <typename T>
static auto pattern_scan_helper(const char * name, T & out, const char * pattern, const char * mask, int sz32 = 0, bool is_abs = true) -> bool
{
	DEBUG_PRINTF("\n[+] Searching for %s...", name);
	auto res = sed::pattern_scan_exec_region(nullptr, -1, pattern, mask);
	DEBUG_PRINTF(" 0x%p", res);

	if (!res)
		return false;

	out = reinterpret_cast<T>(res);

	if (sz32)
	{
		out = reinterpret_cast<T>(is_abs ? sed::abs32(out, sz32) : sed::rel2abs32(out, sz32));
		DEBUG_PRINTF(" -> 0x%p", out);
	}
	
	return true;
}

auto game::initialize() -> bool
{
	DEBUG_PRINTF("\n[+] Loading game information...");

	if (!pattern_scan_helper("game::p_game_info",    game::p_game_info,        "\xDB\x05\x00\x00\x00\x00\xD9\x5D\xF8",         "xx????xxx",   0x6)
	||  !pattern_scan_helper("game::pos_info",       game::pp_pos_info.ptr,    "\x8b\x05\x00\x00\x00\x00\xd9\x40\x00\x8b\x15", "xx????xx?xx", 0x6)
	||  !pattern_scan_helper("game::pp_info_player", game::pp_info_player.ptr, "\xFF\x50\x0C\x8B\xD8\x8B\x15",                 "xxxxxxx",     0xB) //  /*"\x8B\x15\x00\x00\x00\x00\x8B\x72", "xx????xx"*/ TODO: find better sig
	||  !pattern_scan_helper("game::is_raw_input",   game::is_raw_input,       "\x80\x3d\x00\x00\x00\x00\x00\x74\x00\x8b\x75", "xx?????x?xx", 0x6)
	) {
		return false;
	}

	return true;
}
