#include "ApiHook.h"
#include "Log.h"
#include <windows.h>
#include <string>


static unsigned char _func_head_x86[5] = { 0x8b, 0xff, 0x55, 0x8b, 0xec };

void* hk::HookApi(void* oldFunc, void* myFunc)
{
    unsigned char* oldPtr = (unsigned char*)oldFunc;
    unsigned char* myPtr = (unsigned char*)myFunc;

    // check old function head opcode;
    if (memcmp(oldPtr, _func_head_x86, 5) != 0)
    {
        log::Write("function head opcode not match.\n");
        return nullptr;
    }

    // new function
    void* newFunc = VirtualAlloc(NULL, 5 + 5,
        MEM_COMMIT, PAGE_EXECUTE_READWRITE);
    unsigned char* newPtr = (unsigned char*)newFunc;

    // check jmp 32
    auto toMyDt = myPtr - oldPtr - 5;
    auto toOldDt = (oldPtr + 5) - (newPtr + 5) - 5;

    // new function opcode
    memcpy(newPtr, oldPtr, 5);
    newPtr[5] = 0xe9;
    memcpy(newPtr + 6, &toOldDt, 4);

    // old function opcode
    DWORD dwOldProtect = 0;
    VirtualProtect(oldFunc, 5, PAGE_EXECUTE_READWRITE, &dwOldProtect);
    oldPtr[0] = 0xe9;
    memcpy(oldPtr + 1, &toMyDt, 4);
    DWORD dwBuf = 0;	// nessary othewrise the function fails
    VirtualProtect(oldFunc, 5, dwOldProtect, &dwBuf);

    return newFunc;
}

void hk::UnhookApi(void* oldFunc)
{
    DWORD dwOldProtect = 0;
    VirtualProtect(oldFunc, 5, PAGE_EXECUTE_READWRITE, &dwOldProtect);
    memcpy(oldFunc, _func_head_x86, 5);
    DWORD dwBuf = 0;	// nessary othewrise the function fails
    VirtualProtect(oldFunc, 5, dwOldProtect, &dwBuf);
    return;
}
    
bool hk::CreateProcessWithDll(const char* exe, const char* dll)
{
    std::string exePath(exe);
    std::string dllPath(dll);
    std::string dir = exePath.substr(0, exePath.find_last_of("\\/"));

    STARTUPINFO si;
    PROCESS_INFORMATION pi;
    ZeroMemory(&si, sizeof(STARTUPINFO));
    ZeroMemory(&pi, sizeof(PROCESS_INFORMATION));
    si.cb = sizeof(STARTUPINFO);
    if (!CreateProcess(NULL, (LPSTR)exe, NULL,
        NULL, FALSE, CREATE_DEFAULT_ERROR_MODE | CREATE_SUSPENDED, NULL, 
        (LPCTSTR)dir.c_str(), &si, &pi))
    {
        return false;
    }

    void* remoteMemory = VirtualAllocEx(pi.hProcess, NULL, dllPath.size()+1, MEM_COMMIT, PAGE_EXECUTE_READWRITE);
    if (remoteMemory == NULL)
    {
        return false;
    }

    if (!WriteProcessMemory(pi.hProcess, remoteMemory, dllPath.c_str(), dllPath.size()+1, NULL))
    {
        return false;
    }

    HANDLE remoteThread = CreateRemoteThread(pi.hProcess, NULL, 0, (LPTHREAD_START_ROUTINE)LoadLibrary, remoteMemory, 0, NULL);
    if (remoteThread == NULL)
    {
        return false;
    }

    WaitForSingleObject(remoteThread, INFINITE);
    DWORD remoteModule;
    GetExitCodeThread(remoteThread, &remoteModule);
    ResumeThread(pi.hThread);
    CloseHandle(remoteThread);
    VirtualFreeEx(pi.hProcess, remoteMemory, dllPath.size()+1, MEM_DECOMMIT);
    return true;
}
