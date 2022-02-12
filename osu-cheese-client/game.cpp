#include "game.hpp"

#include <sed/console.hpp>
#include <sed/memory.hpp>
#include <sed/strings.hpp>

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
	DEBUG_PRINTF("\n[+] Loading game information..."
				 "\n[+] Obtaining handle to window...");

	while (!game::hwnd)
	{
		EnumWindows([](HWND hwnd, LPARAM lparam) -> BOOL
		{
			DWORD pid { 0 };
			GetWindowThreadProcessId(hwnd, &pid);
			wchar_t text_buffer[MAX_PATH] { 0 };
			constexpr auto text_buffer_sz = ARRAYSIZE(text_buffer);

			if (pid != GetCurrentProcessId()
			|| !GetWindowTextW(hwnd, text_buffer, text_buffer_sz)
			|| !sed::str_starts_with(text_buffer, L"osu!")
			// dumb fix for debug console
			|| !RealGetWindowClassW(hwnd, text_buffer, text_buffer_sz)
			|| sed::str_starts_with(text_buffer, L"Console")
			) {
				return TRUE;
			}
			
			game::hwnd = hwnd;
			return FALSE;
		}, NULL);

		Sleep(800);
	}
	DEBUG_WPRINTF(L"0x%p", game::hwnd);
	
	if (!pattern_scan_helper("game::p_game_info",      game::p_game_info,          "\xDB\x05\x00\x00\x00\x00\xD9\x5D\xF8",                                                             "xx????xxx",                0x6)
	||  !pattern_scan_helper("game::pp_viewpos_info",  game::pp_viewpos_info.ptr,  "\x8b\x05\x00\x00\x00\x00\xd9\x40\x00\x8b\x15",                                                     "xx????xx?xx",              0x6)
	||  !pattern_scan_helper("game::pp_info_player",   game::pp_info_player.ptr,   "\xFF\x50\x0C\x8B\xD8\x8B\x15",                                                                     "xxxxxxx",                  0xB)
	||  !pattern_scan_helper("game::pp_raw_mode_info", game::pp_raw_mode_info.ptr, "\x8b\xec\x83\xec\x00\xa1\x00\x00\x00\x00\x85\xc0\x74",                                             "xxxx?x????xxx",            0xA)
	||  !pattern_scan_helper("game::pp_pplayer_keys",  game::pp_pplayer_keys.ptr,  "\x8b\x0d\x00\x00\x00\x00\x8b\x55\x00\x39\x09\xff\x15\x00\x00\x00\x00\x85\xc0\x74",                 "xx????xx?xxxx????xxx",     0x6)
	||  !pattern_scan_helper("game::pp_phitobject",    game::pp_phitobject.ptr,    "\x8B\x0D\x00\x00\x00\x00\x85\xC9\x75\x07\xB8\x01\x00\x00\x00\xEB\x08\x8B\x01\x8B\x40\x2C\xFF\x50", "xx????xxx?x????x?xxxx?xx", 0x6)
	// ||  !pattern_scan_helper("game::pp_wnd_info",      game::pp_wnd_info.ptr,        "\x8b\x0d\x00\x00\x00\x00\x89\x45\x00\x89\x7d",         "xx????xx?xx",   0x6)
	) {
		return false;
	}
	
	return true;
}
