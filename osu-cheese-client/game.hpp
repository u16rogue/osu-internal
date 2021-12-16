#pragma once

// Used for interfacing with the game's internals

#include "sdk/info_player.hpp"
#include "sdk/info_struct.hpp"
#include "sdk/position_info.hpp"
#include "sdk/raw_info.hpp"

namespace game
{
	inline sdk::pp_info_player_t   pp_info_player;
	inline sdk::pp_viewpos_info_t  pp_viewpos_info;
	inline sdk::unk_game_info_a  * p_game_info { nullptr };
	inline sdk::pp_raw_mode_info_t pp_raw_mode_info;

	inline HWND                    osu_wnd { nullptr }; // See comment on SetWindowTextW hook

	auto initialize() -> bool;
}