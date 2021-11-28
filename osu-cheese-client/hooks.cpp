#include "hooks.hpp"

#include <Windows.h>
#include <windowsx.h>
#include <sed/console.hpp>
#include <sed/memory.hpp>
#include <gl/GL.h>
#include <gl/GLU.h>
#include "game.hpp"
#include "sdk/gamefield.hpp"

enum class CallWindowProc_variant : int
{
	A     = 0,
	MOUSE = A,
	W     = 1,
	KEY   = W
};

static auto CALLBACK CallWindowProc_hook(CallWindowProc_variant variant, HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam) -> bool
{
	static int reso_mode = 0;
	static float resos[][2] =
	{
		{  800.f, 600.f },
		{ 1024.f, 768.f },
		{ 1024.f, 600.f },
		{ 1280.f, 720.f },
		{ 1280.f, 768.f },
		{ 1360.f, 768.f },
		{ 1600.f, 900.f }
	};

	if (variant == CallWindowProc_variant::KEY && Msg == WM_KEYDOWN)
	{
		printf("\n[D] Key -> 0x%x", wParam);
		switch (wParam)
		{
			case VK_HOME:
				reso_mode = (reso_mode + 1) % (sizeof(resos) / sizeof(resos[0]));
				sdk::game_field::resize(resos[reso_mode][0], resos[reso_mode][1]);
				printf("\n[D] Force change resolution mode to: %.0f x %.0f", resos[reso_mode][0], resos[reso_mode][1]);
				break;
		};
	}
	else if (variant == CallWindowProc_variant::MOUSE && Msg == WM_LBUTTONDOWN)
	{
		auto [px, py] = sdk::game_field::s2f(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
		printf("\n[D] Click -> [X: %d (%.2f), Y: %d (%.2f), TIME: %d, INGAME: %d, PLAYER: 0x%p]", GET_X_LPARAM(lParam), px, GET_Y_LPARAM(lParam), py, game::p_game_info->beat_time, game::pp_info_player->async_complete, *game::pp_info_player);
	}

	return false;
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
	__asm lea eax, [CallWindowProcA + 5];
	__asm__("jmp LBL_CALL_ORIGINAL");

	// Call W variant
	__asm__("LBL_VARIANT_W:");
	__asm lea eax, [CallWindowProcW + 5];
	__asm__("jmp LBL_CALL_ORIGINAL");

	// Call original
	__asm__(
	".intel_syntax noprefix \n"
	"LBL_SKIP_ORIGINAL:     \n"
	"   pop eax             \n"
	"	pop ebp             \n"
	"	ret 0x14            \n"
	"LBL_CALL_ORIGINAL:     \n"
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

static auto WINAPI gdi32full_SwapBuffers_hook(HDC hdc) -> void
{

}

static decltype(SwapBuffers) * gdi32full_SwapBuffers_target { nullptr };

static auto __attribute__((naked)) gdi32full_SwapBuffers_proxy(HDC hdc) -> BOOL
{
	__asm
	{
		push ebp
		mov ebp, esp

		push [ebp+8]
		call gdi32full_SwapBuffers_hook

		mov eax, gdi32full_SwapBuffers_target
		lea eax, [eax + 5]
		jmp eax
	}
}

auto hooks::install() -> bool
{
	printf("\n[+] Installing hooks..."
	       "\n[+] Importing gdi32full.SwapBuffers...");
	
	gdi32full_SwapBuffers_target = reinterpret_cast<decltype(gdi32full_SwapBuffers_target)>(GetProcAddress(GetModuleHandleW(L"gdi32full.dll"), "SwapBuffers"));
	if (!gdi32full_SwapBuffers_target)
	{
		printf("\n[!] Failed to import gdi32full.SwapBuffers");
		return false;
	}
	printf(" 0x%p", gdi32full_SwapBuffers_target);

	if (!sed::jmprel32_apply(CallWindowProcA, CallWindowProcA_proxy)
	||  !sed::jmprel32_apply(CallWindowProcW, CallWindowProcW_proxy)
	||  !sed::jmprel32_apply(gdi32full_SwapBuffers_target, gdi32full_SwapBuffers_proxy)
	) {
		printf("\n[!] Failed to install hooks!");
		return false;
	}
	
	

	return true;
}
