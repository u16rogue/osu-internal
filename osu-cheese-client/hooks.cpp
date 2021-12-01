#include "hooks.hpp"

#include <Windows.h>
#include <windowsx.h>
#include <sed/console.hpp>
#include <sed/memory.hpp>
#include <gl/GL.h>
#include <gl/GLU.h>
#include "game.hpp"
#include "sdk/gamefield.hpp"
#include <sed/strings.hpp>
#include "utils/beatmap.hpp"
#include "features/assist.hpp"

// TODO: BUG! proxy hook for CWP A causing characters to get corrupted

enum class CallWindowProc_variant : int
{
	A     = 0,
	MOUSE = A,
	W     = 1,
	KEY   = W
};

static auto CALLBACK CallWindowProc_hook(CallWindowProc_variant variant, HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam) -> bool
{
	if (variant == CallWindowProc_variant::MOUSE && Msg == WM_MOUSEMOVE)
		features::assist::run_aimassist(hWnd, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));

	#ifdef OSU_CHEESE_DEBUG_BUILD
		if (variant == CallWindowProc_variant::MOUSE && Msg == WM_LBUTTONDOWN)
		{
			auto [px, py] = sdk::game_field::v2f(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
			DEBUG_PRINTF("\n[D] Click -> [X: %d (%.2f), Y: %d (%.2f), TIME: %d, INGAME: %d, PLAYER: 0x%p] @ 0x%p", GET_X_LPARAM(lParam), px, GET_Y_LPARAM(lParam), py, game::p_game_info->beat_time, game::pp_info_player->async_complete, *game::pp_info_player, hWnd);
		}
	#endif
	
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
	"   jz LBL_VARIANT_A      \n"
	);

	// Call W variant
	__asm__("LBL_VARIANT_W:");
	__asm lea eax, [CallWindowProcW + 5]
	__asm__("jmp LBL_CALL_ORIGINAL");

	// Call A variant
	__asm__("LBL_VARIANT_A:");
	__asm lea eax, [CallWindowProcA + 5]
	__asm__("jmp LBL_CALL_ORIGINAL");

	// Call original
	__asm__(
	".intel_syntax noprefix \n"
	"LBL_SKIP_ORIGINAL:     \n"
	"   pop eax             \n"
	"	pop ebp             \n"
	"   mov eax, 1          \n"
	"	ret 0x14            \n"
	"LBL_CALL_ORIGINAL:     \n"
	// "	lea eax, [eax+5]    \n"
	"	jmp eax             \n"
	);
}

static auto __attribute__((naked)) CallWindowProcW_proxy(WNDPROC lpPrevWndFunc, HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam) -> LRESULT
{
	__asm
	{
		mov eax, CallWindowProc_variant::W
		jmp CallWindowProc_proxy
	};
}

static auto __attribute__((naked)) CallWindowProcA_proxy(WNDPROC lpPrevWndFunc, HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam) -> LRESULT
{
	__asm
	{
		mov eax, CallWindowProc_variant::A // can just be xor eax, eax but this looks more verbose...
		jmp CallWindowProc_proxy
	};
}

static auto WINAPI gdi32full_SwapBuffers_hook(HDC hdc) -> void
{

}

static volatile decltype(SwapBuffers) * gdi32full_SwapBuffers_target { nullptr };
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

static auto WINAPI SetWindowTextW_hook(HWND hWnd, LPCWSTR lpString) -> void
{
	auto beatmap = sed::str_starts_with(lpString, "osu!  - ");
	if (!beatmap)
		return;

	auto bm_file = utils::beatmap::find_file_by_title(beatmap);
	if (!bm_file)
	{
		DEBUG_PRINTF("\n[!] Failed to load beatmap!");
		return;
	}

	features::assist::load_beatmap(*bm_file);
}

static volatile decltype(SetWindowTextW) * SetWindowTextW_target = SetWindowTextW;
static auto __attribute__((naked)) SetWindowTextW_proxy(HWND hWnd, LPCWSTR lpString) -> BOOL
{
	__asm
	{
		push ebp
		mov ebp, esp

		push [ebp+12]
		push [ebp+8]
		call SetWindowTextW_hook
		
		mov eax, SetWindowTextW_target // we cant do load effective address because clang (msvc?) does some funny things like using the ecx register causing the ctx to get corrupted
		lea eax, [eax + 5]
		jmp eax
	}
}

// TODO: hook clipcursor

auto hooks::install() -> bool
{
	DEBUG_PRINTF("\n[+] Installing hooks..."
	             "\n[+] Importing gdi32full.SwapBuffers...");
	
	gdi32full_SwapBuffers_target = reinterpret_cast<decltype(gdi32full_SwapBuffers_target)>(GetProcAddress(GetModuleHandleW(L"gdi32full.dll"), "SwapBuffers"));
	if (!gdi32full_SwapBuffers_target)
	{
		DEBUG_PRINTF("\n[!] Failed to import gdi32full.SwapBuffers");
		return false;
	}
	DEBUG_PRINTF(" 0x%p", gdi32full_SwapBuffers_target);

	if (!sed::jmprel32_apply(CallWindowProcA, CallWindowProcA_proxy)
	||  !sed::jmprel32_apply(CallWindowProcW, CallWindowProcW_proxy)
	||  !sed::jmprel32_apply(SetWindowTextW, SetWindowTextW_proxy)
	||  !sed::jmprel32_apply(gdi32full_SwapBuffers_target, gdi32full_SwapBuffers_proxy)
	) {
		DEBUG_PRINTF("\n[!] Failed to install hooks!");
		return false;
	}
	
	

	return true;
}
