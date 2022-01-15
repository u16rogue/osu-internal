#pragma once

#include <cstdint>
#include "osu_vec.hpp"

namespace sdk
{
	// osu!master\osu!\GameplayElements\HitObjects\HitObjectBase.cs
	enum class hit_type : std::uint32_t
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

	enum class sound_type : std::uint32_t
	{
		None = 0,
		Normal = 1,
		Whistle = 2,
		Finish = 4,
		Clap = 8
	};

	struct hitobject
	{
		struct
		{
			std::int32_t start;
			std::int32_t end;
		} time;

		hit_type type;
		sound_type sound_type;
		std::int32_t segment_count;
		std::int32_t segment_length;
		/*double*/ char spatial_length[4]; // internally a double but it's only 4 bytes when reversed so it's padded instead since double is 8 bytes
		std::int32_t combo_color_offset;
		std::int32_t combo_color_index;
		std::uint32_t raw_color;
		sdk::vec2 position;
		// not sure with the rest below
		std::int32_t stack_count;
		std::int32_t last_in_combo;
	};
}