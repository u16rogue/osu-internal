#include "oc_client.hpp"

#include <Windows.h>
#include <sed/console.hpp>
#include "game.hpp"
#include "hooks.hpp"

auto oc::load() -> bool
{
	#ifdef OSU_CHEESE_DEBUG_BUILD
	sed::console::init();
	#endif

	DEBUG_PRINTF("\n[+] Initializing...");
	if (!game::initialize() || !hooks::install())
	{
		DEBUG_PRINTF("\n[!] Initialization failed");
		// FreeLibraryAndExitThread(reinterpret_cast<HMODULE>(inst), 0);
	}

	DEBUG_PRINTF("\n[+] Ready!");
	return true;
}

auto oc::unload() -> bool
{
	return false;
}