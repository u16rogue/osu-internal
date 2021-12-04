// Main entry point of the client

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
		#ifdef OSU_CHEESE_DEBUG_BUILD
			sed::console::init();
		#endif
		
		DEBUG_PRINTF("\n[+] Initializing...");
		if (!game::initialize() || !hooks::install())
		{
			DEBUG_PRINTF("\n[!] Initialization failed");
			FreeLibraryAndExitThread(reinterpret_cast<HMODULE>(inst), 0);
		}

		DEBUG_PRINTF("\n[+] Ready!");

		return 0;
	}, inst, NULL, nullptr));

	return 0;
}