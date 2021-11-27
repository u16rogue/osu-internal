#include "memory.hpp"

#include <Windows.h>
#include <Psapi.h>

auto sed::abs2rel32(void * from, std::size_t size, void * to) -> std::int32_t
{
	return static_cast<std::int32_t>(reinterpret_cast<std::uintptr_t>(to) - (reinterpret_cast<std::uintptr_t>(from) + size));
}

auto rel2abs32(void * instruction, std::size_t size) -> std::uintptr_t
{
	std::uintptr_t next = reinterpret_cast<std::uintptr_t>(instruction) + size;
	return next + *reinterpret_cast<std::int32_t *>(next - sizeof(std::int32_t));
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

auto sed::pattern_scan(void * start_, std::size_t size, const char * pattern, const char * mask) -> std::uintptr_t
{
	std::uint8_t * start = reinterpret_cast<decltype(start)>(start_);
	const auto     len   = strlen(mask);
	const auto   * end   = start + size - len;

	for (auto current = start; current < end; ++current)
	{
		for (int i = 0; i < len; ++i)
		{
			if (mask[i] == '?')
				continue;

			if (mask[i] != 'x' || pattern[i] != current[i])
				break;

			if (i == len - 1)
				return reinterpret_cast<std::uintptr_t>(current);
		}
	}

	return 0;
}

auto sed::pattern_scan(const wchar_t * modname, const char * pattern, const char * mask) -> std::uintptr_t
{
	HMODULE    mod = GetModuleHandleW(modname);
	MODULEINFO mi { 0 };

	if (!mod || !GetModuleInformation(GetCurrentProcess(), mod, &mi, sizeof(mi)))
		return 0;

	return sed::pattern_scan(mi.lpBaseOfDll, mi.SizeOfImage, pattern, mask);
}