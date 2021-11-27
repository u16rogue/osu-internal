#pragma once

#include "sdk/info_player.hpp"
#include "sdk/info_struct.hpp"

namespace game
{
	inline sdk::info_player_t   ** pp_info_player { nullptr };
	inline sdk::unk_game_info_a *  p_game_info    { nullptr };

	auto initialize() -> bool;
}