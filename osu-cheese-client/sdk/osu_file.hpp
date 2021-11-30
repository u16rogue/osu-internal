#pragma once

#include <cstdint>

namespace sdk
{
	enum class hit_type : std::uint8_t
	{
		HIT_CIRCLE = 1 << 0,
		SLIDER     = 1 << 1,
		NEW_COMBO  = 1 << 2,
		SPINNER    = 1 << 3,
		COMBOSPEC  = 0b01110000,
		MANIA_HOLD = 1 << 7,

		MASK_SHOULD_AIMASSIST = HIT_CIRCLE | SLIDER
	};

	struct hit_object
	{
		float x, y; // actually an int, but just using float so we don't have to cast later
		int time;
		hit_type type;
		int hitsound; // will prolly leave this out cause not needed
		// objectParams
		// hitSamples
	};
}