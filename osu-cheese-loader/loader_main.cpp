#include "loader_main.hpp"
#include <Windows.h>
#include <cstdio>
#include <TlHelp32.h>
#include <sed/windows/smart_handle.hpp>

auto find_osu_proc() -> sed::smart_handle
{
	sed::smart_handle proc_snap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);
	if (!proc_snap)
	{
		printf("Failed.");
		return nullptr;
	}
	
	PROCESSENTRY32W pe { .dwSize = sizeof(pe) };
	if (Process32FirstW(proc_snap, &pe))
	{
		do
		{
			if (wcscmp(pe.szExeFile, L"osu!.exe"))
				continue;

			return sed::smart_handle(OpenProcess(PROCESS_ALL_ACCESS, FALSE, pe.th32ProcessID));

		} while (Process32NextW(proc_snap, &pe));
	}

	return nullptr;
}

auto main() -> int
{
	sed::smart_handle osu_proc = find_osu_proc();
	if (!osu_proc)
		return 1;


	return 0;
}
