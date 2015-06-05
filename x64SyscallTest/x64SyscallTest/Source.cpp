#include <cstdio>
#include <Windows.h>

using pNtWriteVirtualMemory = NTSTATUS (NTAPI *)(HANDLE ProcessHandle,
    PVOID BaseAddress, PVOID Buffer, ULONG NumberOfBytesToWrite,
    PULONG NumberOfBytesWritten);

pNtWriteVirtualMemory NtWriteVirtualMemory = nullptr;

int main(int argc, char *argv[])
{
    HMODULE hModule = GetModuleHandle(L"ntdll.dll");
    NtWriteVirtualMemory = (pNtWriteVirtualMemory)GetProcAddress(hModule,
        "NtWriteVirtualMemory");

    int i = 0x321;
    int j = 0x123;

    fprintf(stderr, "j = %X\n", j);

    ULONG numBytesWritten = 0;
    NTSTATUS success = NtWriteVirtualMemory(GetCurrentProcess(), &j, &i,
        sizeof(int), &numBytesWritten);

    fprintf(stderr, "j = %X\n", j);

    return 0;
}