#pragma once

#include <cstdint>
#include "osu_vec.hpp"

// TODO: cleanup

namespace sdk
{
	// osu!master\osu!\GameplayElements\HitObjects\HitObjectBase.cs
	struct hit_object
	{
		vec2 coords;
		int time;
		int type;
		int hitsound;

		//union _dummytype_objectParams
		//{
		//} object_params;
		//
		//union _dummytype_hitSample
		//{
		//} hit_sample;
	};
}