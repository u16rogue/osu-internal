#include "loader_main.hpp"
#include <Windows.h>
#include <cstdio>
#include <TlHelp32.h>
#include <sed/windows/smart_handle.hpp>

auto find_osu_proc(sed::smart_handle & open_out) -> bool
{
	// Find osu! process
	printf("\n[+] Creating process snapshot...");
	sed::smart_handle proc_snap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);
	if (!proc_snap)
	{
		printf("Failed.");
		return 1;
	}

	PROCESSENTRY32W pe { .dwSize = sizeof(pe) };
	if (Process32FirstW(proc_snap, &pe))
	{

	}
}

auto main() -> int
{
	sed::smart_handle osu_proc;
	if (!find_osu_proc(osu_proc))
		return 1;


	return 0;
}
