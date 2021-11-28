#pragma once

#include <cstdint>
#include <cstddef>
#include <sed/macro.hpp>

namespace sdk
{
	// Credits! https://github.com/VacCatGT/undercover-osu-fork/blob/main/undercover-osu/Sdk/Player/Functions/Player.cpp
	union info_player_t
	{
		OC_UNS_PAD(0x17A, bool, is_replay_mode)
		OC_UNS_PAD(0x182, bool, async_complete)
	};

	class ppinfo_player_t
	{
	public:
		ppinfo_player_t() = default;

		ppinfo_player_t(info_player_t ** ppinfo)
			: pp_info_player(ppinfo) {};

		auto operator*() -> info_player_t *
		{
			if (this->pp_info_player && *this->pp_info_player)
				return *this->pp_info_player;

			return nullptr;
		}

		auto operator->() -> info_player_t *
		{
			if (this->pp_info_player && *this->pp_info_player)
				return *this->pp_info_player;

			return &this->info_player_dummy;
		}

		operator bool() const noexcept
		{
			return this->pp_info_player && *this->pp_info_player != nullptr;
		}

	private:
		info_player_t ** pp_info_player    { nullptr };

		info_player_t info_player_dummy
		{
			.is_replay_mode = false,
			.async_complete = false
		};
	};

}