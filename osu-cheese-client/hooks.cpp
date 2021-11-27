#include "hooks.hpp"

#include <Windows.h>
#include <windowsx.h>
#include <sed/console.hpp>
#include <sed/memory.hpp>
#include <gl/GL.h>
#include <gl/GLU.h>

#include "game.hpp"

enum class CallWindowProc_variant : int
{
	A     = 0,
	MOUSE = A,
	W     = 1,
	KEY   = W
};

static auto CALLBACK CallWindowProc_hook(CallWindowProc_variant variant, HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam) -> bool
{
	static bool hold = false;

	if (variant == CallWindowProc_variant::KEY && Msg == WM_KEYDOWN)
	{
		printf("\n[D] Key -> 0x%x", wParam);
	}
	else if (variant == CallWindowProc_variant::MOUSE && Msg == WM_LBUTTONDOWN)
	{
		printf("\n[D] Click -> [X: %d, Y: %d, TIME: %d, INGAME: %d, PLAYER: 0x%p]", GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam), game::p_game_info->beat_time, game::pp_info_player->async_complete, *game::pp_info_player);
	}

	return hold;
}

static auto __attribute__((naked)) CallWindowProc_proxy(WNDPROC lpPrevWndFunc, HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam) -> LRESULT
{
	__asm__(
	".intel_syntax noprefix \n"
	"	push ebp	        \n"
	"	mov ebp, esp        \n"
	"	push eax            \n"
	"	push [ebp+24]       \n"
	"	push [ebp+20]       \n"
	"	push [ebp+16]       \n"
	"	push [ebp+12]       \n"
	"	push eax	        \n"
	);

	// TODO: figure out how to do this in clang
	// Call hook function and check return
	__asm call CallWindowProc_hook;
	__asm__(
	".intel_syntax noprefix   \n"
	"	test al, al           \n"
	"	jnz LBL_SKIP_ORIGINAL \n"
	"	pop eax               \n"
	"   test al, al           \n"
	"   jz LBL_VARIANT_W      \n"
	);

	// Call A variant
	__asm__("LBL_VARIANT_A:");
	__asm lea eax, CallWindowProcA;
	__asm__("jmp LBL_CALL_ORIGINAL");

	// Call W variant
	__asm__("LBL_VARIANT_W:");
	__asm lea eax, CallWindowProcW;
	__asm__("jmp LBL_CALL_ORIGINAL");

	// Call original
	__asm__(
	".intel_syntax noprefix \n"
	"LBL_SKIP_ORIGINAL:     \n"
	"   pop eax             \n"
	"	pop ebp             \n"
	"	ret 0x14            \n"
	"LBL_CALL_ORIGINAL:     \n"
	"	lea eax, [eax + 5]  \n"
	"	jmp eax             \n"
	);
}

static auto __attribute__((naked)) CallWindowProcW_proxy(WNDPROC lpPrevWndFunc, HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam) -> LRESULT
{
	__asm
	{
		mov eax, 1
		jmp CallWindowProc_proxy
	};
}

static auto __attribute__((naked)) CallWindowProcA_proxy(WNDPROC lpPrevWndFunc, HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam) -> LRESULT
{
	__asm
	{
		xor eax, eax
		jmp CallWindowProc_proxy
	};
}

auto hooks::install() -> bool
{
	printf("\n[+] Installing hooks...");
	
	if (!sed::jmprel32_apply(CallWindowProcA, CallWindowProcA_proxy)
	||  !sed::jmprel32_apply(CallWindowProcW, CallWindowProcW_proxy)
	) {
		printf("\n[!] Failed to install hooks!");
		return false;
	}
	
	

	return true;
}
