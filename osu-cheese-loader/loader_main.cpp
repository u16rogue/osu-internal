#include "loader_main.hpp"
#include <Windows.h>
#include <cstdio>
#include <TlHelp32.h>
#include <tuple>
#include <Psapi.h>
#include <sed/windows/smart_handle.hpp>
#include <sed/windows/suspend_guard.hpp>

using info_t = std::pair<sed::smart_handle, DWORD>;

static auto find_osu_proc() -> info_t
{
	sed::smart_handle proc_snap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);
	if (!proc_snap)
	{
		printf("Failed.");
		return std::make_pair(nullptr, 0);
	}
	
	PROCESSENTRY32W pe { .dwSize = sizeof(pe) };
	if (Process32FirstW(proc_snap, &pe))
	{
		do
		{
			if (wcscmp(pe.szExeFile, L"osu!.exe"))
				continue;

			return std::make_pair(sed::smart_handle(OpenProcess(PROCESS_ALL_ACCESS, FALSE, pe.th32ProcessID)), pe.th32ProcessID);

		} while (Process32NextW(proc_snap, &pe));
	}

	return std::make_pair(nullptr, 0);
}

static auto find_osu_auth(info_t & proc, std::uintptr_t & start, std::uintptr_t & end) -> bool
{
	sed::smart_handle mod_snap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE32 | TH32CS_SNAPMODULE, proc.second);
	if (!mod_snap)
		return false;

	MODULEENTRY32W me { .dwSize = sizeof(me) };
	if (Module32FirstW(mod_snap, &me))
	{
		do
		{
			if (wcscmp(me.szModule, L"osu!auth.dll"))
				continue;

			start = reinterpret_cast<std::uintptr_t>(me.modBaseAddr);
			end = start + me.modBaseSize;
			return true;
		} while (Module32NextW(mod_snap, &me));
	}

	return false;
}

static auto find_osu_auth_thread(info_t & proc, std::uintptr_t start, std::uintptr_t end) -> sed::suspend_guard
{
	sed::smart_handle thread_snap = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, NULL);
	if (!thread_snap)
		return 0;

	THREADENTRY32 te { .dwSize = sizeof(te) };
	if (Thread32First(thread_snap, &te))
	{
		do
		{
			if (te.th32OwnerProcessID != proc.second)
				continue;
			
			sed::suspend_guard tt (te.th32ThreadID);
			if (!tt)
				continue;

			CONTEXT tctx { };
			if (!GetThreadContext(tt, &tctx))
				continue;

			printf("\n0x%x - %d", tctx.Eip, tt.get_id());

			if (tctx.Eip >= start && tctx.Eip <= end)
				return std::move(tt);

		} while (Thread32Next(thread_snap, &te));
	}

	return 0;
}

// TODO: clean up, merge checks as one iteration
auto main() -> int
{
	info_t osu_proc {};
	printf("[+] Attaching to osu...");
	while (!(osu_proc = find_osu_proc()).first)
		Sleep(800);

	printf("\n[+] Looking for osu!auth...");
	std::uintptr_t start, end;
	while (!find_osu_auth(osu_proc, start, end))
		Sleep(800);
	printf(" 0x%p - 0x%p", reinterpret_cast<void *>(start), reinterpret_cast<void *>(end));

	printf("\n[+] Enumerating for osu auth thread...");
	sed::suspend_guard auth_thread;
	while (!(auth_thread = find_osu_auth_thread(osu_proc, start, end)))
		Sleep(800);
	printf(" 0x%p - %d", static_cast<HANDLE>(auth_thread), auth_thread.get_id());

	return 0;
}
