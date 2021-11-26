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

static auto CallWindowProc_install(void * target, void * tramp, char variant) -> bool
{
	std::uint8_t CallWindowProc_jmp_shellcode[] = { 0xE9, 0x00, 0x00, 0x00, 0x00 };

	printf("\n[+] Installing CallWindowProc%c", variant);

	printf("\n[+] Changing CallWindowProc%c_target protection...", variant);
	DWORD oprot { 0 };
	if (!VirtualProtect(target, sizeof(CallWindowProc_jmp_shellcode), PAGE_EXECUTE_READWRITE, &oprot))
		return false;

	auto rel = sed::abs2rel32(target, sizeof(CallWindowProc_jmp_shellcode), tramp);
	printf("\n[+] Calculating CWP%c_target -> CWP%c_trampoline relative address... ABS: 0x%p REL: 0x%p", variant, variant, tramp, rel);
	*reinterpret_cast<std::uintptr_t*>(CallWindowProc_jmp_shellcode + 0x1) = rel;

	printf("\n[+] Patching CallWindowProc%c_trampoline jump...", variant);
	std::memcpy(target, CallWindowProc_jmp_shellcode, sizeof(CallWindowProc_jmp_shellcode));

	printf("\n[+] Verifying patch...");
	if (std::memcmp(target, CallWindowProc_jmp_shellcode, sizeof(CallWindowProc_jmp_shellcode)))
		return false;

	printf("\n[+] Restoring CallWindowProc%c_target protection...", variant);
	if (!VirtualProtect(target, sizeof(CallWindowProc_jmp_shellcode), oprot, &oprot))
		return false;

	return true;
}

auto hooks::install() -> bool
{
	printf("\n[+] Installing hooks...");
	
	if (!CallWindowProc_install(CallWindowProcA, CallWindowProcA_trampoline, 'A')
	||  !CallWindowProc_install(CallWindowProcW, CallWindowProcW_trampoline, 'W'))
	{
		return false;
	}

	return true;
}
