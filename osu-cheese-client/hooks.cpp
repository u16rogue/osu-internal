#include "hooks.hpp"

#include <Windows.h>
#include <windowsx.h>
#include <sed/console.hpp>

WNDPROC o_WindowProc { nullptr };
static auto CALLBACK hk_WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) -> LRESULT
{
	if (uMsg == WM_LBUTTONDOWN)
	{
		printf("\n[D] Click -> [%d, %d]", GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
	}

	return o_WindowProc(hwnd, uMsg, wParam, lParam);
}

auto hooks::install() -> bool
{
	printf("\n[+] Installing hooks...");

	// TODO: Use the game's window handle instead of grabbing it from FindWindow
	printf("\n[+] Obtaining window handle... ");
	HWND gamewnd = FindWindowW(L"WindowsForms10.Window.2b.app.0.1a0e24_r30_ad1", L"osu!");
	if (!gamewnd)
		return false;
	printf("0x%p", gamewnd);

	auto lol = GetWindowLongPtrW(gamewnd, GWLP_WNDPROC);
	printf("\n[+] Hooking WindowProc...");
	o_WindowProc = reinterpret_cast<decltype(o_WindowProc)>(SetWindowLongPtrW(gamewnd, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(hk_WindowProc)));
	if (!o_WindowProc)
	{
		printf("ERR: %lu", GetLastError());
		return false;
	}
	printf("0x%p - 0x%p", o_WindowProc, lol);

	return true;
}
