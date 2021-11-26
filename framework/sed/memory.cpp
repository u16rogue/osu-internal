#include "memory.hpp"

auto sed::abs2rel32(void * from, std::size_t size, void * to) -> std::uintptr_t
{
	return reinterpret_cast<std::uintptr_t>(to) - (reinterpret_cast<std::uintptr_t>(from) + size);
}
