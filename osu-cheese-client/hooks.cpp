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

#if 0
static auto CALLBACK WindowProc_hook( _In_ HWND hwnd, _In_ UINT uMsg, _In_ WPARAM wParam, _In_ LPARAM lParam) -> LRESULT
{
	return TRUE;
}
#endif

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
		if (features::dispatcher::on_wndproc(hWnd, Msg, wParam, lParam, nullptr))
			return true;
	}

	return false;
}

static decltype(CallWindowProcA) * CallWindowProcA_target = CallWindowProcA;
static decltype(CallWindowProcW) * CallWindowProcW_target = CallWindowProcW;
static auto __attribute__((naked)) CallWindowProc_proxy(WNDPROC lpPrevWndFunc, HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam) -> LRESULT
{
	__asm
	{
		push ebp	 
		mov ebp, esp 
		push eax     
		push [ebp+24]
		push [ebp+20]
		push [ebp+16]
		push [ebp+12]
		push eax	 
		call CallWindowProc_hook;
		test al, al
		jnz LBL_CWP_SKIP_ORIGINAL
		pop eax
		test al, al
		jnz LBL_CWP_VARIANT_A

	LBL_CWP_VARIANT_W:
		mov eax, CallWindowProcA_target
		jmp LBL_CWP_CALL_ORIGINAL

		// Call A variant
	LBL_CWP_VARIANT_A:
		mov eax, CallWindowProcW_target
		jmp LBL_CWP_CALL_ORIGINAL

		// Call original
	LBL_CWP_SKIP_ORIGINAL:
		pop eax   
		pop ebp   
		mov eax, 1
		ret 0x14  
	LBL_CWP_CALL_ORIGINAL:
		lea eax, [eax+5]
		jmp eax
	}
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

	features::dispatcher::on_render();
	oc::menu::render();

	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
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

static decltype(SetWindowTextW) * SetWindowTextW_target = SetWindowTextW;
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
	{
		*raw_coords = oc::menu::freeze_view_point;
		return;
	}

	// TODO: actually rebuild this function from assembly
	// but seems like there are other functions that does our
	// job for us so we don't have to worry about it but it's
	// a better idea to actually rebuild it and restore functionality
	features::dispatcher::on_osu_set_raw_coords(raw_coords);
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

// TODO: we can just hook the function that handles this instead so we don't have to check the return address
static decltype(GetCursorPos) * GetCursorPos_target = GetCursorPos;
static auto __stdcall GetCursorPos_hook(LPPOINT lpPoint) -> bool
{
	if (!oc::menu::visible)
		return false;

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
	
	if (real_return_address >= wnform_start && real_return_address <= wnform_end)
	{
		POINT p = oc::menu::freeze_view_point;
		ClientToScreen(game::hwnd, &p);
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
		test al, al
		jz LBL_GETCURSORPOS_CALL_ORIGINAL

		// Skip original and fake return
	LBL_GETCURSORPOS_SKIP_ORIGINAL:
		// "mov eax, 1                      \n" unecessary since our hook will be setting the eax (or al) register to 1 anyway
		pop ebp
		ret 4

		// Call original
	LBL_GETCURSORPOS_CALL_ORIGINAL:
		mov eax, GetCursorPos_target
		lea eax, [eax + 0x5]
		jmp eax
	}
}

void * osu_ac_flag_original { nullptr };
static auto __stdcall osu_ac_flag() -> void
{
	DEBUG_PRINTF("\n[!] Anti-cheat flag triggered!");
}

static auto __attribute__((naked)) osu_ac_flag_proxy() -> void
{
	__asm
	{
		call osu_ac_flag
		jmp osu_ac_flag_original
	};
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
	auto cond_raw_rel8 = *reinterpret_cast<std::uint8_t *>(cond_raw_coords + 1);
	DEBUG_PRINTF("\n[+] raw coords rel8 and abs -> 0x%x", cond_raw_rel8);
	auto cond_raw_abs = cond_raw_coords + 2 + cond_raw_rel8;
	DEBUG_PRINTF(" -> 0x%p", cond_raw_abs);

	// Anticheat flag - credits! https://github.com/SweetDreamzZz/osuauth-denbai-checker
	DEBUG_PRINTF("\n[+] Searching for ac_flag_call...");
	auto ac_flag_call = sed::pattern_scan_exec_region(nullptr, - 1, "\xE8\x00\x00\x00\x00\x83\xC4\x00\x89\x45\x00\x8B\x4D\x00\x8B\x11\x8B\x42\x00\x89\x45\x00\x0F\xB6\x4D", "x????xx?xx?xx?xxxx?xx?xxx"); // TODO: this doesn't necessarily need to be scanned through regions
	if (!ac_flag_call)
	{
		DEBUG_PRINTF("\n[!] Failed to look for ac_flag_call");
		return false;
	}
	DEBUG_PRINTF(" 0x%p", ac_flag_call);
	osu_ac_flag_original = reinterpret_cast<void *>(sed::rel2abs32(reinterpret_cast<void *>(ac_flag_call), 0x5));
	DEBUG_PRINTF(" -> 0x%p", osu_ac_flag_original);

	#define _OC_ADD_HOOK_INSTANCE(patchtype, from, to) \
		_instances.push_back(std::make_unique<sed::mempatch_##patchtype##r32>(reinterpret_cast<void *>(from), reinterpret_cast<void *>(to)))

	hook_instances_t _instances;

	_OC_ADD_HOOK_INSTANCE(jmp,  CallWindowProcA_target,       CallWindowProcA_proxy);
	_OC_ADD_HOOK_INSTANCE(jmp,  CallWindowProcW_target,       CallWindowProcW_proxy);
	_OC_ADD_HOOK_INSTANCE(jmp,  SetWindowTextW,               SetWindowTextW_proxy);
	_OC_ADD_HOOK_INSTANCE(jmp,  gdi32full_SwapBuffers_target, gdi32full_SwapBuffers_proxy);
	_OC_ADD_HOOK_INSTANCE(jmp,  osu_set_field_coords_target,  osu_set_field_coords_proxy);
	_OC_ADD_HOOK_INSTANCE(call, cond_raw_coords,              osu_set_raw_coords_proxy);
	_OC_ADD_HOOK_INSTANCE(jmp,  cond_raw_coords + 5,          cond_raw_abs);
	_OC_ADD_HOOK_INSTANCE(jmp,  GetCursorPos,                 GetCursorPos_proxy);
	_OC_ADD_HOOK_INSTANCE(call, ac_flag_call,                 osu_ac_flag_proxy);

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
