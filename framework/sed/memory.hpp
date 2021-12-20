#pragma once

#include <cstdint>
#include <cstddef>
#include <vector>
#include <Windows.h>
#include <Psapi.h>
#include <memory>
#include "windows/protect_guard.hpp"

namespace sed
{
	auto abs2rel32(void * from, std::size_t size, void * to) -> std::int32_t;
	auto rel2abs32(void * instruction, std::size_t size) -> std::uintptr_t;
	auto abs32(void * instruction, std::size_t size) -> std::uintptr_t;

	auto op1rel32_apply(std::uint8_t opcode, void * from, void * to) -> bool;
	auto jmprel32_apply(void * from, void * to) -> bool;
	auto callrel32_apply(void * from, void * to) -> bool;

	class mempatch_interface
	{
	public:
		virtual bool patch() = 0;
		virtual bool restore() = 0;
		virtual operator bool() const noexcept = 0;
	};

	template <std::uint8_t opcode>
	class basic_mempatch_op1r32 : public mempatch_interface
	{
	public:
		basic_mempatch_op1r32(void * from, void * to)
			: from(from), to(to) {}

		bool patch() override
		{
			auto prot = sed::protect_guard(this->from, 0x5);
			if (!prot)
				return false;

			// backup
			std::memcpy(this->restore_buffer, this->from, 0x5);

			// patch
			std::uint8_t shell[] = { opcode, 0x00, 0x00, 0x00, 0x00 };
			*reinterpret_cast<std::uintptr_t *>(shell + 1) = sed::abs2rel32(this->from, sizeof(shell), this->to);
			std::memcpy(this->from, shell, sizeof(shell));

			this->patched = true;
			return true;
		}

		bool restore() override
		{
			if (!this->patched)
				return true;

			auto prot = sed::protect_guard(this->from, 0x5);
			if (!prot)
				return false;

			std::memcpy(this->from, this->restore_buffer, 0x5);
			this->patched = false;
			return true;
		}

		operator bool() const noexcept override
		{
			return this->patched;
		}

	private:
		bool   patched { false   };
		void * from    { nullptr },
			 * to      { nullptr };

		std::uint8_t restore_buffer[5] { 0x00 };
	};

	using mempatch_jmpr32  = basic_mempatch_op1r32<0xE9>;
	using mempatch_callr32 = basic_mempatch_op1r32<0xE8>; 

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