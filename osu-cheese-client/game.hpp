#pragma once

// Used for interfacing with the game's internals

#include "sdk/info_player.hpp"
#include "sdk/info_struct.hpp"
#include "sdk/position_info.hpp"

namespace game
{
	inline sdk::ppinfo_player_t    pp_info_player;
	inline sdk::pp_position_info_t pp_pos_info;
	inline sdk::unk_game_info_a  * p_game_info { nullptr };
	inline bool                  * is_raw_input;

	auto initialize() -> bool;
}