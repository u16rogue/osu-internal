#include "hooks.hpp"

#include <Windows.h>
#include <windowsx.h>
#include <tuple>
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
	if (oc::menu::wndproc(hWnd, Msg, wParam, lParam))
		return true;

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
	"	jnz LBL_CWP_SKIP_ORIGINAL \n"
	"	pop eax               \n"
	"   test al, al           \n"
	"   jz LBL_CWP_VARIANT_A      \n"
	);

	// Call W variant
	__asm__("LBL_CWP_VARIANT_W:");
	__asm lea eax, [CallWindowProcW + 5]
	__asm__("jmp LBL_CWP_CALL_ORIGINAL");

	// Call A variant
	__asm__("LBL_CWP_VARIANT_A:");
	__asm lea eax, [CallWindowProcA + 5]
	__asm__("jmp LBL_CWP_CALL_ORIGINAL");

	// Call original
	__asm__(
	".intel_syntax noprefix \n"
	"LBL_CWP_SKIP_ORIGINAL: \n"
	"   pop eax             \n"
	"	pop ebp             \n"
	"   mov eax, 1          \n"
	"	ret 0x14            \n"
	"LBL_CWP_CALL_ORIGINAL: \n"
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
	oc::menu::render();

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
	auto beatmap = sed::str_starts_with(lpString, L"osu!");
	
	if (!beatmap)
		return;

	beatmap = sed::str_starts_with(beatmap, L"  - ");
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

	// TODO: dynamically load these values, we already have the sig stop being lazy.
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

// Name: #=zP4nKUSUPOssQxNF6$g==::#=z9UGmDcmwjvbl
static auto __fastcall osu_set_field_coords_rebuilt(void * ecx, sdk::vec2 * out_coords) -> void
{
	// Can do psilent here by setting the field coordinates
	*out_coords = game::pp_viewpos_info->pos.view_to_field();
}

static auto __attribute__((naked)) osu_set_field_coords_proxy(void * ecx, sdk::vec2 * out_coords) -> void
{
	__asm
	{
		push esi
		sub esp, 8
		call osu_set_field_coords_rebuilt
		add esp, 8
		pop esi
		ret 8
	}
}

static auto __fastcall osu_set_raw_coords_rebuilt(sdk::vec2 * raw_coords) -> void
{
	if (oc::menu::visible && game::pp_raw_mode_info->is_raw)
		*raw_coords = oc::menu::freeze_view_point;

	// TODO: actually rebuild this function from assembly
	// but seems like there are other functions that does our
	// job for us so we don't have to worry about it but it's
	// a better idea to actually rebuild it and restore functionality
	features::feature::on_osu_set_raw_coords(raw_coords);
}

static auto __attribute__((naked)) osu_set_raw_coords_proxy() -> void
{
	__asm
	{
		mov ecx, [ebp - 0x34]
		add ecx, 0x24
		jmp osu_set_raw_coords_rebuilt
	};
}

static volatile decltype(GetCursorPos) * GetCursorPos_target = GetCursorPos;
static auto __stdcall GetCursorPos_hook(LPPOINT lpPoint) -> bool
{
	static void * wnform_start, * wnform_end;
	if (!wnform_start || !wnform_end)
	{
		HMODULE hmod = GetModuleHandleW(L"System.Windows.Forms.ni.dll");
		MODULEINFO mi {};

		if (!hmod || !GetModuleInformation(GetCurrentProcess(), hmod, &mi, sizeof(mi))) // TODO: handle this properly
			TerminateProcess(GetCurrentProcess(), 1);

		wnform_start = mi.lpBaseOfDll;
		wnform_end = reinterpret_cast<void *>(std::uintptr_t(mi.lpBaseOfDll) + mi.SizeOfImage);
	}

	void * real_return_address { nullptr };

	__asm
	{
		push eax
		mov eax, [ebp]
		mov eax, [eax + 4]
		mov real_return_address, eax
		pop eax
	};
	
	if (real_return_address >= wnform_start && real_return_address <= wnform_end && oc::menu::visible)
	{
		POINT p = oc::menu::freeze_view_point;
		ClientToScreen(game::pp_wnd_info->handle, &p);
		*lpPoint = p;
		return true;
	}

	return false;
}

static auto __attribute__((naked)) GetCursorPos_proxy(LPPOINT lpPoint) -> void
{
	__asm
	{
		push ebp
		mov ebp, esp

		push [ebp + 0x8]
		call GetCursorPos_hook
	}

	__asm__(
	".intel_syntax noprefix             \n"
	"test al, al                        \n"
	"jz LBL_GETCURSORPOS_CALL_ORIGINAL  \n"
	);

	// Skip original and fake return
	__asm__(
	".intel_syntax noprefix          \n"
	"LBL_GETCURSORPOS_SKIP_ORIGINAL: \n"
	// "mov eax, 1                      \n" unecessary since our hook will be setting the eax (or al) register to 1 anyway
	"pop ebp                         \n"
	"ret 4                           \n"
	);
	
	// Call original
	__asm__("LBL_GETCURSORPOS_CALL_ORIGINAL:");
	__asm mov eax, GetCursorPos_target;
	__asm__(
	".intel_syntax noprefix \n"
	"lea eax, [eax + 0x5]   \n"
	"jmp eax                \n"
	);
}

using hook_instances_t = std::vector<std::unique_ptr<sed::mempatch_interface>>;
static hook_instances_t hook_instances;

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
	void * osu_set_field_coords_target = reinterpret_cast<void *>(sed::pattern_scan_exec_region(nullptr, -1, "\x56\x83\xEC\x08\x8B\xF2\x8D\x41\x18\xD9\x00\xD9\x40\x04\xd9\x44", "xxx?xxxx?xxxx?xx"));
	if (!osu_set_field_coords_target)
	{
		DEBUG_PRINTF("\n[!] Failed to look for osu_set_field_coords!");
		return false;
	}
	DEBUG_PRINTF(" 0x%p", osu_set_field_coords_target);

	// Set raw input coordinates
	DEBUG_PRINTF("\n[+] Searching for osu_set_raw_coords...");
	auto cond_raw_coords = sed::pattern_scan_exec_region(nullptr, -1, "\x74\x00\x8b\x75\xCC\x83\xc6\x00\x8b\x45", "x?xx?xx?xx");
	if (!cond_raw_coords)
	{
		DEBUG_PRINTF("\n[!] Failed to look for osu_set_raw_coords!");
		return false;
	}
	DEBUG_PRINTF(" 0x%p", cond_raw_coords);

	// Get rel8
	auto cond_raw_rel8 = *reinterpret_cast<std::uint8_t *>(cond_raw_coords + 1);
	DEBUG_PRINTF("\n[+] raw coords rel8 and abs -> 0x%x", cond_raw_rel8);
	// Calculate absolute from rel8
	auto cond_raw_abs = cond_raw_coords + 2 + cond_raw_rel8;
	DEBUG_PRINTF(" -> 0x%p", cond_raw_abs);

	#define _OC_ADD_HOOK_INSTANCE(patchtype, ...) \
		_instances.push_back(std::make_unique<sed::mempatch_##patchtype##r32>(__VA_ARGS__))
	
	hook_instances_t _instances;

	_OC_ADD_HOOK_INSTANCE(jmp,  CallWindowProcA,                               CallWindowProcA_proxy);
	_OC_ADD_HOOK_INSTANCE(jmp,  CallWindowProcW,                               CallWindowProcW_proxy);
	_OC_ADD_HOOK_INSTANCE(jmp,  SetWindowTextW,                                SetWindowTextW_proxy);
	_OC_ADD_HOOK_INSTANCE(jmp,  gdi32full_SwapBuffers_target,                  gdi32full_SwapBuffers_proxy);
	_OC_ADD_HOOK_INSTANCE(jmp,  osu_set_field_coords_target,                   osu_set_field_coords_proxy);
	_OC_ADD_HOOK_INSTANCE(call, reinterpret_cast<void *>(cond_raw_coords),     osu_set_raw_coords_proxy);
	_OC_ADD_HOOK_INSTANCE(jmp,  reinterpret_cast<void *>(cond_raw_coords + 5), reinterpret_cast<void *>(cond_raw_abs));
	_OC_ADD_HOOK_INSTANCE(jmp,  GetCursorPos,                                  GetCursorPos_proxy);

	#undef _OC_ADD_HOOK_INSTANCE

	for (auto & h : _instances)
	{
		if (!h->patch())
		{
			DEBUG_PRINTF("\n[!] Failed to install hooks!");
			return false;
		}
	}
	
	hook_instances = std::move(_instances);
	return true;
}

auto hooks::uninstall() -> bool
{
	if (hook_instances.empty())
		return false;

	for (auto & h : hook_instances)
	{
		if (!h->restore())
		{
			DEBUG_PRINTF("\n[!] Failed to uninstall hook!");
		}
	}

	hook_instances.clear();
	return true;
}
