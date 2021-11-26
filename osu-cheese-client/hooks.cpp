#include "hooks.hpp"

#include <Windows.h>
#include <windowsx.h>
#include <sed/console.hpp>
#include <sed/memory.hpp>

static auto CALLBACK CallWindowProc_hk(WNDPROC lpPrevWndFunc, HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam) -> void
{
	if (Msg == WM_LBUTTONDOWN)
	{
		printf("\n[D] Click -> [%d, %d]", GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
	}
}

void * CallWindowProc_target = CallWindowProcW;

// osu! detects hooks by checking the return address, this little trampoline prevents that.
static auto __attribute__((naked)) CallWindowProc_trampoline(WNDPROC lpPrevWndFunc, HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam) -> LRESULT
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
		call CallWindowProc_hk
		mov eax, CallWindowProc_target
		lea eax, [eax + 5]
		jmp eax
	}
}

static auto CallWindowProc_install() -> bool
{
	std::uint8_t CallWindowProc_jmp_shellcode[] = { 0xE9, 0x00, 0x00, 0x00, 0x00 };

	printf("\n[+] Installing CallWindowProc");

	printf("\n[+] Changing CallWindowProc_target protection...");
	DWORD oprot { 0 };
	if (!VirtualProtect(CallWindowProc_target, sizeof(CallWindowProc_jmp_shellcode), PAGE_EXECUTE_READWRITE, &oprot))
		return false;

	auto rel = sed::abs2rel32(CallWindowProc_target, sizeof(CallWindowProc_jmp_shellcode), CallWindowProc_trampoline);
	printf("\n[+] Calculating CallWindowProc_target -> CallWindowProc_trampoline relative address... ABS: 0x%p REL: 0x%p", CallWindowProc_trampoline, rel);
	*reinterpret_cast<std::uintptr_t*>(CallWindowProc_jmp_shellcode + 0x1) = rel;

	printf("\n[+] Patching CallWindowProc_trampoline jump...");
	std::memcpy(CallWindowProc_target, CallWindowProc_jmp_shellcode, sizeof(CallWindowProc_jmp_shellcode));

	printf("\n[+] Verifying patch...");
	if (std::memcmp(CallWindowProc_target, CallWindowProc_jmp_shellcode, sizeof(CallWindowProc_jmp_shellcode)))
		return false;

	printf("\n[+] Restoring CallWindowProc_target protection...");
	if (!VirtualProtect(CallWindowProc_target, sizeof(CallWindowProc_jmp_shellcode), oprot, &oprot))
		return false;

	return true;
}

auto hooks::install() -> bool
{
	printf("\n[+] Installing hooks...");
	
	if (!CallWindowProc_install())
		return false;

	return true;
}
