#include "memory.hpp"

#include <Windows.h>

auto sed::abs2rel32(void * from, std::size_t size, void * to) -> std::uintptr_t
{
	return reinterpret_cast<std::uintptr_t>(to) - (reinterpret_cast<std::uintptr_t>(from) + size);
}

auto sed::jmprel32_apply(void * from, void * to) -> bool
{
	std::uint8_t shell[] = { 0xE9, 0x00, 0x00, 0x00, 0x00 };
	*reinterpret_cast<std::uintptr_t *>(shell + 1) = sed::abs2rel32(from, sizeof(shell), to);

	DWORD oprot { 0 };
	if (!VirtualProtect(from, sizeof(shell), PAGE_EXECUTE_READWRITE, &oprot))
		return false;

	for (int i = 0; i < sizeof(shell); ++i)
		reinterpret_cast<std::uint8_t *>(from)[i] = shell[i];

	if (!VirtualProtect(from, sizeof(shell), oprot, &oprot))
		return false;

	return true;
}
