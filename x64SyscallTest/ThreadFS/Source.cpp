#include <cstdio>

#include <Windows.h>

DWORD WINAPI ThreadEntry(LPVOID lpParameter)
{
    WaitForSingleObject((HANDLE)lpParameter, INFINITE);

    return 0;
}

int main(int argc, char *argv[])
{
    HANDLE hEvent = CreateEvent(nullptr, FALSE, FALSE, L"Useless event");

    HANDLE hThread1 = CreateThread(nullptr, 0, &ThreadEntry, hEvent, 0, nullptr);
    HANDLE hThread2 = CreateThread(nullptr, 0, &ThreadEntry, hEvent, 0, nullptr);
    HANDLE hThread3 = CreateThread(nullptr, 0, &ThreadEntry, hEvent, 0, nullptr);

    CONTEXT ctxThread1 = { CONTEXT_ALL };
    (void)GetThreadContext(hThread1, &ctxThread1);

    CONTEXT ctxThread2 = { CONTEXT_ALL };
    (void)GetThreadContext(hThread2, &ctxThread2);

    CONTEXT ctxThread3 = { CONTEXT_ALL };
    (void)GetThreadContext(hThread3, &ctxThread3);

    LDT_ENTRY ldtThread1 = { 0 };
    LDT_ENTRY ldtThread2 = { 0 };
    LDT_ENTRY ldtThread3 = { 0 };

    (void)GetThreadSelectorEntry(hThread1, ctxThread1.SegFs, &ldtThread1);
    (void)GetThreadSelectorEntry(hThread2, ctxThread2.SegFs, &ldtThread2);
    (void)GetThreadSelectorEntry(hThread3, ctxThread3.SegFs, &ldtThread3);

    NT_TIB *pTibMain = (NT_TIB *)__readfsdword(0x18);

    DWORD_PTR dwFSBase1 = (ldtThread1.HighWord.Bits.BaseHi << 24) |
        (ldtThread1.HighWord.Bits.BaseMid << 16) |
        ldtThread1.BaseLow;

    DWORD_PTR dwFSBase2 = (ldtThread2.HighWord.Bits.BaseHi << 24) |
        (ldtThread2.HighWord.Bits.BaseMid << 16) |
        ldtThread2.BaseLow;

    DWORD_PTR dwFSBase3 = (ldtThread3.HighWord.Bits.BaseHi << 24) |
        (ldtThread3.HighWord.Bits.BaseMid << 16) |
        ldtThread3.BaseLow;

    fprintf(stderr, "Thread 1 FS Segment base address: %X\n"
        "Thread 2 FS Segment base address : %X\n"
        "Thread 3 FS Segment base address : %X\n",
        dwFSBase1, dwFSBase2, dwFSBase3);

    DWORD_PTR dwWOW64Address1 = *(DWORD_PTR *)((unsigned char *)dwFSBase1 + 0xC0);
    DWORD_PTR dwWOW64Address2 = *(DWORD_PTR *)((unsigned char *)dwFSBase2 + 0xC0);
    DWORD_PTR dwWOW64Address3 = *(DWORD_PTR *)((unsigned char *)dwFSBase3 + 0xC0);

    fprintf(stderr, "Thread 1 FS:[0xC0] : %X\n"
        "Thread 2 FS:[0xC0] : %X\n"
        "Thread 3 FS:[0xC0] : %X\n",
        dwWOW64Address1, dwWOW64Address2, dwWOW64Address3);

    return 0;
}