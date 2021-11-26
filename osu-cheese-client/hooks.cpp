#include "hooks.hpp"

#include <Windows.h>
#include <windowsx.h>
#include <intrin.h>
#include <sed/console.hpp>
#include <sed/memory.hpp>

static auto CALLBACK hk_CallWindowProc(WNDPROC lpPrevWndFunc, HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam) -> void
{
	if (Msg == WM_LBUTTONDOWN)
	{
		printf("\n[D] Click -> [%d, %d]", GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
	}
}

void * target_CallWindowProc = CallWindowProcW;

static auto __attribute__((naked)) trampoline_CallWindowProc(WNDPROC lpPrevWndFunc, HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam) -> LRESULT
{
	__asm
	{
		push ebp
		mov ebp, esp
		push [ebp+24]
		push [ebp+20]
		push [ebp+16]
		push [ebp+12]
		push [ebp+8] // forgot the return lol
		call hk_CallWindowProc
		mov eax, target_CallWindowProc
		lea eax, [eax + 5]
		jmp eax
	}
}

auto hooks::install() -> bool
{
	printf("\n[+] Installing hooks...");
	
	std::uint8_t cwpa_sc[] = { 0xE9, 0x00, 0x00, 0x00, 0x00 };

	printf("\n[+] Changing protection...");
	DWORD oprot { 0 };
	VirtualProtect(target_CallWindowProc, sizeof(cwpa_sc), PAGE_EXECUTE_READWRITE, &oprot);

	auto rel = sed::abs2rel32(target_CallWindowProc, sizeof(cwpa_sc), trampoline_CallWindowProc);
	printf("\n[+] Calculating relative address... ABS: 0x%p REL: 0x%p", trampoline_CallWindowProc, rel);
	*reinterpret_cast<std::uintptr_t*>(cwpa_sc + 0x1) = rel;

	printf("\n[+] Patching jump...");
	std::memcpy(target_CallWindowProc, cwpa_sc, sizeof(cwpa_sc));
	
	return true;
}
