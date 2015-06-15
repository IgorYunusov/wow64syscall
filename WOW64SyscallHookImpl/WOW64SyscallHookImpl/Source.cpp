#include "Funcs.h"

#include <cstdio>

#include <Windows.h>

const int PAGE_SIZE = 0x1000;
const int SYSCALL_INTERCEPT = 0x39;
const int NUM_WOW64_BYTES = 0x9;

using pNtWriteVirtualMemory = NTSTATUS (NTAPI *)(HANDLE ProcessHandle,
    PVOID BaseAddress, PVOID Buffer, SIZE_T NumberOfBytesToWrite, PSIZE_T NumberOfBytesWritten);
pNtWriteVirtualMemory NtWriteVirtualMemory = nullptr;

DWORD_PTR dwWow64Address = 0;
LPVOID lpJmpRealloc = nullptr;

void __declspec(naked) NtWriteVirtualMemoryHook()
{
    __asm pushad
    
    fprintf(stderr, "NtWriteVirtualMemory called.\n");

    __asm
    {
        popad
        jmp lpJmpRealloc
    }
}

const DWORD_PTR __declspec(naked) GetWow64Address()
{
    __asm
    {
        mov eax, dword ptr fs:[0xC0]
        ret
    }
}

void __declspec(naked) Wow64Trampoline()
{
    __asm
    {
        cmp eax, SYSCALL_INTERCEPT
        jz NtWriteVirtualMemoryHook
        jmp lpJmpRealloc
    }
}

const LPVOID CreateNewJump(const DWORD_PTR dwWow64Address)
{
    lpJmpRealloc = VirtualAlloc(nullptr, PAGE_SIZE, MEM_RESERVE | MEM_COMMIT,
        PAGE_EXECUTE_READWRITE);

    (void)memcpy(lpJmpRealloc, (const void *)dwWow64Address, NUM_WOW64_BYTES);

    return lpJmpRealloc;
}

const void EnableWow64Redirect(const DWORD_PTR dwWow64Address, const LPVOID lpNewJumpLocation)
{
    unsigned char trampolineBytes[] =
    {
        0x68, 0xDD, 0xCC, 0xBB, 0xAA,       /*push 0xAABBCCDD*/
        0xC3,                               /*ret*/
        0xCC, 0xCC, 0xCC                    /*padding*/
    };
    memcpy(&trampolineBytes[1], &lpNewJumpLocation, sizeof(DWORD_PTR));

    WriteJump(dwWow64Address, trampolineBytes, sizeof(trampolineBytes));
}

const void WriteJump(const DWORD_PTR dwWow64Address, const void *pBuffer, size_t ulSize)
{
    DWORD dwOldProtect = 0;
    (void)VirtualProtect((LPVOID)dwWow64Address, PAGE_SIZE, PAGE_EXECUTE_READWRITE, &dwOldProtect);
    (void)memcpy((void *)dwWow64Address, pBuffer, ulSize);
    (void)VirtualProtect((LPVOID)dwWow64Address, PAGE_SIZE, dwOldProtect, &dwOldProtect);
}

int main(int argc, char *argv[])
{
    HMODULE hModule = GetModuleHandle(L"ntdll.dll");
    NtWriteVirtualMemory =
        (pNtWriteVirtualMemory)GetProcAddress(hModule, "NtWriteVirtualMemory");

    dwWow64Address = GetWow64Address();

    const LPVOID lpNewJumpLocation = CreateNewJump(dwWow64Address);
    EnableWow64Redirect(dwWow64Address, (LPVOID)Wow64Trampoline);

    //Test syscall
    int i = 0x123;
    int j = 0x678;
    SIZE_T ulBytesWritten = 0;

    fprintf(stderr, "i = 0x%X\n", i);
    NtWriteVirtualMemory(GetCurrentProcess(), &i, &j, sizeof(int), &ulBytesWritten);
    fprintf(stderr, "i = 0x%X\n", i);

    return 0;
}