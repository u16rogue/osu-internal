#include "memory.hpp"

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
	
	for (std::size_t rva = 0; rva < size - len; ++rva)
	{
		for (int pat_i = 0; pat_i < len; ++pat_i)
		{
			if (mask[pat_i] == '?')
				continue;

			if (mask[pat_i] != 'x' || pattern[pat_i] != reinterpret_cast<const char *>(start)[rva + pat_i])
				break;

			if (pat_i == len - 1)
				return reinterpret_cast<std::uintptr_t>(start_) + rva;
		}
	}

	return 0;
}

auto sed::pattern_scan_exec_region(void * start_, std::size_t size, const char * pattern, const char * mask) -> std::uintptr_t
{
	std::uint8_t * current = reinterpret_cast<decltype(current)>(start_);
	std::uint8_t * end     = size == -1 ? reinterpret_cast<std::uint8_t *>(-1) : current + size;

	MEMORY_BASIC_INFORMATION mbi { 0 };
	while (VirtualQuery(current, &mbi, sizeof(mbi)) && current < end)
	{
		constexpr DWORD any_execute = PAGE_EXECUTE | PAGE_EXECUTE_READ | PAGE_EXECUTE_READWRITE | PAGE_EXECUTE_WRITECOPY;
		if (mbi.State != MEM_COMMIT || !(mbi.Protect & any_execute))
		{
			current += mbi.RegionSize;
			continue;
		}

		auto match = sed::pattern_scan(current, mbi.RegionSize, pattern, mask);
		if (match)
			return match;

		current += mbi.RegionSize;
	}

	return 0;
}