#pragma once

#include <cstdint>
#include <cstddef>
#include <sed/macro.hpp>
#include <sed/memory.hpp>

namespace sdk
{
	// Credits! https://github.com/VacCatGT/undercover-osu-fork/blob/main/undercover-osu/Sdk/Player/Functions/Player.cpp
	union info_player_t
	{
		OC_UNS_PAD(0x17A, bool, is_replay_mode)
		OC_UNS_PAD(0x182, bool, async_complete)
	};

	class pp_info_player_t : public sed::basic_ptrptr<info_player_t>
	{
	};

}