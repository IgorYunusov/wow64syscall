#pragma once

#include <Windows.h>

void NtWriteVirtualMemoryHook();
const DWORD_PTR GetWow64Address();
void Wow64Trampoline();
const LPVOID CreateNewJump(const DWORD_PTR dwWow64Address);
const void WriteJump(const DWORD_PTR dwWow64Address, const void *pBuffer, size_t ulSize);
const void EnableWow64Redirect(const DWORD_PTR dwWow64Address, const LPVOID lpNewJumpLocation);
