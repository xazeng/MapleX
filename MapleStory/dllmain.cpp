// dllmain.cpp : Defines the entry point for the DLL application.
#include "stdafx.h"
#include <stdio.h>
#include "hklib/ApiHook.h"

typedef HANDLE(WINAPI *CreateMutexA_t)(LPSECURITY_ATTRIBUTES lpMutexAttributes, BOOL bInitialOwner, LPCSTR lpName);
CreateMutexA_t pCreateMutexA = NULL;
HANDLE WINAPI MyCreateMutexA(LPSECURITY_ATTRIBUTES lpMutexAttributes, BOOL bInitialOwner, LPCSTR lpName);

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
        pCreateMutexA = (CreateMutexA_t)hk::HookApi(CreateMutexA, MyCreateMutexA);
        break;
	case DLL_PROCESS_DETACH:
        hk::UnhookApi(CreateMutexA);
        pCreateMutexA = NULL;
		break;
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
        break;
	}
	return TRUE;
}

HANDLE WINAPI MyCreateMutexA(LPSECURITY_ATTRIBUTES lpMutexAttributes, BOOL bInitialOwner, LPCSTR lpName)
{
    if (lpName != NULL && strcmp("WvsClientMtx", lpName) == 0)
    {
        char buf[64] = "";
        sprintf_s(buf, "WvsClientMtx_%u", GetCurrentProcessId());
        return pCreateMutexA(lpMutexAttributes, bInitialOwner, buf);
    }
    return pCreateMutexA(lpMutexAttributes, bInitialOwner, lpName);
}

