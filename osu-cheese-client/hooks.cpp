#include "hooks.hpp"

#include <Windows.h>
#include <windowsx.h>
#include <sed/macro.hpp>
#include <sed/memory.hpp>
#include <sed/strings.hpp>
#include "utils/beatmap.hpp"

#include "manager/gamefield_manager.hpp"
#include "manager/beatmap_manager.hpp"

#include <GL/gl3w.h>
#include <imgui_impl_opengl3.h>
#include <imgui_impl_win32.h>
#include <imgui.h>

#include "game.hpp"
#include "menu.hpp"

#include "features/features.hpp"

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

	// HACK: tesing coord system, remove later!
	if (variant == CallWindowProc_variant::KEY && Msg == WM_KEYDOWN && wParam == VK_NUMPAD0)
	{
		game::pp_pos_info->pos.x += 20.f;
		DEBUG_PRINTF("\n[D] sent move!");
	}

	if (variant == CallWindowProc_variant::MOUSE)
	{
		if (features::feature::on_wndproc(hWnd, Msg, wParam, lParam, nullptr))
			return true;
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
	if (static bool init = true; init)
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

	features::feature::on_render();
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
	auto beatmap = sed::str_starts_with(lpString, "osu!");
	if (!beatmap)
		return;

	beatmap = sed::str_starts_with(beatmap, "  - ");
	if (!beatmap)
	{
		manager::beatmap::unload();
		DEBUG_PRINTF("\n[D] Unloaded beatmap!");
		return;
	}

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

#if 0
static volatile decltype(ShowCursor) * ShowCursor_target = ShowCursor;
static auto __attribute__((naked)) ShowCursor_trampoline(BOOL bShow) -> int
{
	_asm
	{
		push ebp
		mov ebp, esp
		mov eax, ShowCursor_target
		lea eax, [eax+5]
		jmp eax
	}
}



static auto WINAPI ShowCursor_hook(BOOL bShow) -> int
{
	if (!menu::visible)
		return ShowCursor_trampoline(bShow);

	menu::last_show_cursor_request = bShow;
	return bShow ? 0 : -1;
}
#endif

// Name: #=zP4nKUSUPOssQxNF6$g==::#=z9UGmDcmwjvbl ( very useful :))) !!! )
// rebuilt from assembly, due to clr being jitted this might get outdated soon!
static auto __fastcall osu_set_field_coords_rebuilt(void * ecx, sdk::vec2 * out_coords) -> void
{
	*out_coords = game::pp_pos_info->pos.view_to_field();
}

static auto __attribute__((naked)) osu_set_field_coords_proxy(void * ecx, sdk::vec2 * out_coords) -> void
{
	__asm
	{
		push esi
		sub esp, 8
		//push esp
		call osu_set_field_coords_rebuilt
		//pop esp
		add esp, 8
		pop esi
		ret 8
	}
}

static auto __stdcall osu_set_raw_coords_rebuilt() -> void
{

}

auto hooks::install() -> bool
{
	DEBUG_PRINTF("\n[+] Installing hooks..."
	             "\n[+] Importing gdi32full.SwapBuffers...");
	
	// Swap buffers
	gdi32full_SwapBuffers_target = reinterpret_cast<decltype(gdi32full_SwapBuffers_target)>(GetProcAddress(GetModuleHandleW(L"gdi32full.dll"), "SwapBuffers"));
	if (!gdi32full_SwapBuffers_target)
	{
		DEBUG_PRINTF("\n[!] Failed to import gdi32full.SwapBuffers");
		return false;
	}
	DEBUG_PRINTF(" 0x%p", gdi32full_SwapBuffers_target);

	// Set Field coordinates
	DEBUG_PRINTF("\n[+] Searching for osu_set_field_coords... ");
	void * osu_set_field_coords_target = reinterpret_cast<void *>(sed::pattern_scan_exec_region(nullptr, -1, "\x56\x83\xec\x00\x8b\xf2", "xxx?xx"));
	if (!osu_set_field_coords_target)
	{
		DEBUG_PRINTF("\n[!] Failed to look for osu_set_field_coords!");
		return false;
	}
	DEBUG_PRINTF(" 0x%p", osu_set_field_coords_target);
	
	// Set raw input coordinates
	DEBUG_PRINTF("\n[+] Searching for osu_set_raw_coords...");
	auto cond_raw_coords = sed::pattern_scan_exec_region(nullptr, -1, "\x74\x00\x8b\x75\x00\x83\xc6", "x?xx?xx");
	if (!cond_raw_coords)
	{
		DEBUG_PRINTF("\n[!] Failed to look for osu_set_raw_coords!");
		return false;
	}
	DEBUG_PRINTF(" 0x%p", cond_raw_coords);

	// Calculate relative
	auto cond_raw_rel8 = *reinterpret_cast<std::uint8_t *>(cond_raw_coords + 1);
	DEBUG_PRINTF("\n[+] raw coords rel8 and abs -> 0x%x", cond_raw_rel8);
	// Calculate absolute from rel8
	auto cond_raw_abs = cond_raw_coords + 2 + cond_raw_rel8;
	DEBUG_PRINTF(" -> 0x%p", cond_raw_abs);

	if (!sed::jmprel32_apply(CallWindowProcA, CallWindowProcA_proxy)
	||  !sed::jmprel32_apply(CallWindowProcW, CallWindowProcW_proxy)
	||  !sed::jmprel32_apply(SetWindowTextW, SetWindowTextW_proxy)
	||  !sed::jmprel32_apply(gdi32full_SwapBuffers_target, gdi32full_SwapBuffers_proxy)
	||  !sed::jmprel32_apply(osu_set_field_coords_target, osu_set_field_coords_proxy)
	||  !sed::callrel32_apply(reinterpret_cast<void *>(cond_raw_coords), osu_set_raw_coords_rebuilt)
	||  !sed::jmprel32_apply(reinterpret_cast<void *>(cond_raw_coords + 5), reinterpret_cast<void *>(cond_raw_abs))
	) {
		DEBUG_PRINTF("\n[!] Failed to install hooks!");
		return false;
	}
	
	return true;
}
