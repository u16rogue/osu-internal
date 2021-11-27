#include "hooks.hpp"

#include <Windows.h>
#include <windowsx.h>
#include <sed/console.hpp>
#include <sed/memory.hpp>

static auto CALLBACK CallWindowProc_hk(WNDPROC lpPrevWndFunc, HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam) -> bool
{
	static bool hold = false;

	if (Msg == WM_KEYDOWN && wParam == VK_PAUSE)
	{
		printf("\n[D] toggle: %d", wParam);
		hold = !hold;
		return true;
	}

	if (Msg == WM_LBUTTONDOWN)
	{
		printf("\n[D] Click -> [%d, %d]", GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
	}

	return hold;
}

// TODO: refactor and Find a way to Merge A and W tramps

void * CallWindowProcW_target = CallWindowProcW; // TODO: load CallWindowProcW_target from LDR
static auto __attribute__((naked)) CallWindowProcW_trampoline(WNDPROC lpPrevWndFunc, HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam) -> LRESULT
{
	__asm__
	(R"(
		.intel_syntax noprefix
		push ebp
		mov ebp, esp
		push [ebp+24]
		push [ebp+20]
		push [ebp+16]
		push [ebp+12]
		push [ebp+8]
	)");

	// TODO: figure out how to do this in clang
	__asm call CallWindowProc_hk;

	__asm__
	(R"(
		.intel_syntax noprefix
		test al, al
		jz call_CallWindowProcW_target
		pop ebp
		ret 0x14
		call_CallWindowProcW_target:
	)");

	__asm mov eax, CallWindowProcW_target;

	__asm__
	(R"(
		.intel_syntax noprefix
		lea eax, [eax + 5]
		jmp eax
	)");
}

void * CallWindowProcA_target = CallWindowProcA; // TODO: load CallWindowProcA_target from LDR
static auto __attribute__((naked)) CallWindowProcA_trampoline(WNDPROC lpPrevWndFunc, HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam) -> LRESULT
{
	__asm__
	(R"(
		.intel_syntax noprefix
		push ebp
		mov ebp, esp
		push [ebp+24]
		push [ebp+20]
		push [ebp+16]
		push [ebp+12]
		push [ebp+8]
	)");

	// TODO: figure out how to do this in clang
	__asm call CallWindowProc_hk;

	__asm__
	(R"(
		.intel_syntax noprefix
		test al, al
		jz call_CallWindowProcA_target
		pop ebp
		ret 0x14
		call_CallWindowProcA_target:
	)");

	__asm mov eax, CallWindowProcA_target;

	__asm__
	(R"(
		.intel_syntax noprefix
		lea eax, [eax + 5]
		jmp eax
	)");
}

auto hooks::install() -> bool
{
	printf("\n[+] Installing hooks...");
	;
	if (!sed::jmprel32_apply(CallWindowProcA, CallWindowProcA_trampoline)
	||  !sed::jmprel32_apply(CallWindowProcW, CallWindowProcW_trampoline)
	) {
		printf("\n[!] Failed to install hooks!");
		return false;
	}

	return true;
}
