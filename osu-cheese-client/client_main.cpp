#include "client_main.hpp"
#include <Windows.h>

#include <sed/windows/smart_handle.hpp>
#include <sed/console.hpp>
#include <sed/memory.hpp>

#include "game.hpp"
#include "hooks.hpp"

auto WINAPI DllMain(HINSTANCE inst, DWORD reason, LPVOID res0) -> BOOL
{
	if (reason == DLL_PROCESS_ATTACH) sed::smart_handle(CreateThread(nullptr, NULL, [](LPVOID inst) -> DWORD
	{
		sed::console::init();
		printf("\n[+] Initializing...");
		if (!game::initialize() || !hooks::install())
		{
			printf("\n[!] Initialization failed");
			FreeLibraryAndExitThread(reinterpret_cast<HMODULE>(inst), 0);
		}

		return 0;
	}, inst, NULL, nullptr));

	return 0;
}