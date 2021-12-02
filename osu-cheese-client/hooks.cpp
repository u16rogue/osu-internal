#include "hooks.hpp"

#include <Windows.h>
#include <windowsx.h>
#include <sed/console.hpp>
#include <sed/memory.hpp>
#include "game.hpp"
#include "manager/gamefield_manager.hpp"
#include <sed/strings.hpp>
#include "utils/beatmap.hpp"
#include "features/assist.hpp"
#include "manager/beatmap_manager.hpp"
#include <GL/gl3w.h>
#include <imgui_impl_opengl3.h>
#include <imgui_impl_win32.h>
#include <imgui.h>
#include "menu.hpp"

enum class CallWindowProc_variant : int
{
	A     = 0,
	MOUSE = A,
	W     = 1,
	KEY   = W
};

static auto CALLBACK CallWindowProc_hook(CallWindowProc_variant variant, HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam) -> bool
{
	if (menu::wndproc(hWnd, Msg, wParam, lParam))
		return true;

	if (variant == CallWindowProc_variant::MOUSE)
	{
		if (Msg == WM_MOUSEMOVE)
			features::assist::run_aimassist(hWnd, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
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
	static bool init = true;
	if (init)
	{
		//HGLRC ctx = wglCreateContext(hdc);
		gl3wInit();
		ImGui::CreateContext();
		ImGui::StyleColorsDark();
		ImGui::GetIO().IniFilename = nullptr;
		ImGui_ImplWin32_Init(WindowFromDC(hdc));
		ImGui_ImplOpenGL3_Init();
		init = false;
	}

	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
	menu::render();
	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
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

	RECT wr {};
	GetClientRect(hWnd, &wr);
	
	manager::game_field::resize(wr.right, wr.bottom);
	manager::beatmap::load(*bm_file);
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
