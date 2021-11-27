#pragma once

#include "sdk/info_player.hpp"
#include "sdk/info_struct.hpp"

namespace game
{
	inline sdk::ppinfo_player_t   pp_info_player;
	inline sdk::unk_game_info_a * p_game_info { nullptr };

	auto initialize() -> bool;
}