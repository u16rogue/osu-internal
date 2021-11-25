#include "loader_main.hpp"
#include <Windows.h>
#include <cstdio>
#include <TlHelp32.h>
#include <tuple>
#include <Psapi.h>
#include <winternl.h>
#include <filesystem>
#include <iostream>
#include <sed/windows/smart_handle.hpp>
#include <sed/windows/suspend_guard.hpp>

#if __has_include("client_path.hpp")
	#include "client_path.hpp"
#else
	#define DEBUG_HARDCODE_CLIENT_PATH L""
#endif

using info_t = std::pair<sed::smart_handle, DWORD>;

decltype(NtQueryInformationThread) * _ntquerythread = nullptr;

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

			auto tt = sed::suspend_guard(te.th32ThreadID);
			if (!tt)
				continue;

			DWORD staddr { 0 };
			// https://www.geoffchappell.com/studies/windows/km/ntoskrnl/api/ps/psquery/class.htm
			_ntquerythread(tt, THREADINFOCLASS(0x09), &staddr, sizeof(staddr), nullptr);

			if (staddr >= start && staddr <= end)
				return std::move(tt);

		} while (Thread32Next(thread_snap, &te));
	}

	return 0;
}

// TODO: clean up, merge checks as one iteration
auto main(int argc, char ** argv) -> int
{
	const char * client_path = nullptr;

	if (argc < 2)
	{
		if (!DEBUG_HARDCODE_CLIENT_PATH[0])
		{
			printf("[!] ERROR: Missing path for client!");
			return 1;
		}
		
		client_path = DEBUG_HARDCODE_CLIENT_PATH;
	}
	else
	{
		client_path = argv[1];
	}

	printf("\n[+] Client path: %s", client_path);

	// NtQueryInformationThread import
	printf("\n[+] Importing NtQueryInformationThread... ");
	_ntquerythread = reinterpret_cast<decltype(_ntquerythread)>(GetProcAddress(GetModuleHandleW(L"ntdll.dll"), "NtQueryInformationThread"));
	if (!_ntquerythread)
	{
		printf("Failed!");
		return 1;
	}
	printf("0x%p", _ntquerythread);

	// Process query
	info_t osu_proc {};
	printf("\n[+] Attaching to osu...");
	while (!(osu_proc = find_osu_proc()).first)
		Sleep(800);

	// Module query
	printf("\n[+] Looking for osu!auth...");
	std::uintptr_t start, end;
	while (!find_osu_auth(osu_proc, start, end))
		Sleep(800);
	printf(" 0x%p - 0x%p", reinterpret_cast<void *>(start), reinterpret_cast<void *>(end));

	// Thread query
	printf("\n[+] Enumerating for osu auth thread...");
	sed::suspend_guard auth_thread;
	while (!(auth_thread = find_osu_auth_thread(osu_proc, start, end)))
		Sleep(800);

	printf("[+] Suspended auth thread... 0x%p - ID: %lu", static_cast<HANDLE>(auth_thread), auth_thread.get_id());
	std::cin.get();



	return 0;
}
