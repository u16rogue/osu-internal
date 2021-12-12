#pragma once

#include <cstdint>
#include <cstddef>
#include <vector>
#include <Windows.h>
#include <Psapi.h>
#include <memory>

namespace sed
{
	auto abs2rel32(void * from, std::size_t size, void * to) -> std::int32_t;
	auto rel2abs32(void * instruction, std::size_t size) -> std::uintptr_t;
	auto abs32(void * instruction, std::size_t size) -> std::uintptr_t;

	auto op1rel32_apply(std::uint8_t opcode, void * from, void * to) -> bool;
	auto jmprel32_apply(void * from, void * to) -> bool;
	auto callrel32_apply(void * from, void * to) -> bool;

	// TODO: consteval ida style pattern generator
	auto pattern_scan(void * start_, std::size_t size, const char * pattern, const char * mask) -> std::uintptr_t;
	auto pattern_scan_exec_region(void * start_, std::size_t size, const char * pattern, const char * mask) -> std::uintptr_t;

	template <class data, typename default_t = int, default_t default_v = 0>
	class basic_ptrptr
	{
	public:
		auto operator*() -> data *
		{
			if (this->ptr && *this->ptr)
				return *this->ptr;

			return nullptr;
		}

		auto operator->() -> data *
		{
			if (this->ptr && *this->ptr)
				return *this->ptr;

			return &this->dummy;
		}

		operator bool() const noexcept
		{
			return this->ptr && *this->ptr != nullptr;
		}

	public:
		data ** ptr    { nullptr   };
		data dummy     { default_v };
	};

}