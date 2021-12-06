#pragma once

#include <sed/macro.hpp>
#include <sed/memory.hpp>
#include "osu_vec.hpp"

namespace sdk
{
	union position_info_t
	{
		OC_UNS_PAD(0x4, sdk::vec2, pos);
	};

	class pp_position_info_t : public sed::basic_ptrptr<position_info_t>
	{
	};

}