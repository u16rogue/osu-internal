#pragma once

#include <cstdint>
#include "osu_vec.hpp"
#include <sed/memory.hpp>
#include <sed/macro.hpp>

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
	private:
		// The 0xC is from a register but seems to be constant, perhaps this is due
		// to not being able to encode 0x10 into the move instruction while only being
		// able to encode 0x4.
		// mov esi, [eax+ecx+0x04] where EAX is 0xC
		char pad[0x4 + 0xC];

	public:
		struct
		{
			std::int32_t start; // 00
			std::int32_t end; // 04
		} time;

		hit_type        type; // 08
		sound_type      sound_type; // 12
		std::int32_t    segment_count; // 16
		std::int32_t    segment_length; // 20
		/*double*/ char spatial_length[4]; // 24 // internally a double but it's only 4 bytes so it's padded instead since double is 8 bytes
		std::int32_t    combo_color_offset; // 28
		std::int32_t    combo_color_index; // 32
		std::uint32_t   raw_color; // 36
		sdk::vec2       position; // 40
		// not sure with the rest below
		std::int32_t    stack_count; // 48
		std::int32_t    last_in_combo; // 52
	private:
		char pad1[0x3C];
	public:
		std::int32_t is_hit;
	};

	// ?? internal array (_items)
	struct ho_array
	{
	private:
		char pad[0x8];
	public:
		// Array of 4 byte pointers
		// mov ecx,[eax+ebx*4+08]
		hitobject * hitobjects[];
	};


	// this is List<HitObject>
	struct ho_vector
	{
	private:
		char pad[0x4]; // _defaultCapacity or maybe the vtable idk since this one is const
	public:
		ho_array * container; // _items
	private:
		char pad1[0x4];
	public:
		std::uint32_t count; // _size

	public:
		auto get_coming_hitobject(std::uint32_t time) -> std::pair<hitobject *, int>;
		auto begin() -> hitobject **;
		auto end() -> hitobject **;
	};

	class beatmapbase
	{
	public:
		int8_t pad[44]; //0x0000
		float DifficultyApproachRate; //0x002C
		float DifficultyCircleSize; //0x0030
		float DifficultyHpDrainRate; //0x0034
		float DifficultyOverall; //0x0038
		double DifficultySliderMultiplier; //0x003C
		double DifficultySliderTickRate; //0x0044
	}; //Size: 0x004C

	// actually hitobjectmanager
	union ho_1
	{
		OC_UNS_PAD(0x30, beatmapbase *, beatmap);

		// 0x44 is in register EAX but seems to be constant
		// mov ecx,[eax+ecx+0x04]
		OC_UNS_PAD(0x04 + 0x44, ho_vector *, ho_vec); // internal List<HitObject> hitObjects = new List<HitObject>();
	};

	// ~hitobject manager... or this could actually be a Ruleset~
	// yes this is actually a rule set
	struct ho_2
	{
	private:
		char pad[0x3C];
	public:
		ho_1 * ho1;
	};

	// this is actually the Player class lol
	union hitobject_pointer
	{
		// actual hitobject manager is at 0x40
		OC_UNS_PAD(0x40, ho_1 *, hitobjectmanager);
		OC_UNS_PAD(0x60, ho_2 *, ho2);// this is a rule set!!
	};

	class pp_phitobject_t : public sed::basic_ptrptr<hitobject_pointer>
	{
	public:
		auto begin() -> hitobject **;
		auto end() -> hitobject **;

		auto get_coming_hitobject(std::uint32_t time) -> std::pair<hitobject *, int>;
		auto count() -> std::uint32_t;

		auto operator[](int index) -> hitobject *;
		operator bool() const noexcept;
	};
}