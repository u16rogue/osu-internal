#pragma once

#include <cstdint>
#include "osu_vec.hpp"

// TODO: cleanup

namespace sdk
{
	// osu!master\osu!\GameplayElements\HitObjects\HitObjectBase.cs
	enum class hit_type : std::uint8_t
	{
		Normal = 1,
		Slider = 2,
		NewCombo = 4,
		NormalNewCombo = 5,
		SliderNewCombo = 6,
		Spinner = 8,
		ColourHax = 112,
		Hold = 128,
		ManiaLong = 128
	};

	enum class sound_type : std::uint8_t
	{
		None = 0,
		Normal = 1,
		Whistle = 2,
		Finish = 4,
		Clap = 8
	};

	struct hit_object
	{
		vec2 coords;
		int time;
		hit_type type;
		sound_type hitsound;

		//union _dummytype_objectParams
		//{
		//} object_params;
		//
		//union _dummytype_hitSample
		//{
		//} hit_sample;
	};
}