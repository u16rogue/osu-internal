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
	auto jmprel32_apply(void * from, void * to) -> bool;

	// TODO: consteval ida style pattern generator
	auto pattern_scan(void * start_, std::size_t size, const char * pattern, const char * mask) -> std::uintptr_t;
	auto pattern_scan_exec_region(void * start_, std::size_t size, const char * pattern, const char * mask) -> std::uintptr_t;
}