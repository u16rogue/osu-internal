#pragma once

#include <cstdint>
#include <cstddef>

namespace sed
{
	auto abs2rel32(void * from, std::size_t size, void * to) -> std::uintptr_t;
	auto jmprel32_apply(void * from, void * to) -> bool;
}