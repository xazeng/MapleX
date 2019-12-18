// MapleX.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "MapleX.h"
#include <assert.h>
#include <stdio.h>
#include <commdlg.h>
#include <io.h>
#include "hklib/ApiHook.h"
#include "hklib/Log.h"

static HANDLE _instance = NULL;
static HWND _mainDlg = NULL;
static HWND _pathEdit = NULL;
static const char* _path_cache = "./MapleX.path";

void MessageBoxErr(const char* title, const char* content = nullptr);
INT_PTR CALLBACK WinProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
void OnInitDialog(HWND hDlg);
INT_PTR OnCommand(WPARAM wParam, LPARAM lParam);
void OnClickPathBtn();
void OnClickStartBtn();
//////////////////////////////////////////////////////////////////////////

int APIENTRY _tWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPTSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

    _instance = hInstance;
    DialogBox(hInstance, MAKEINTRESOURCE(IDD_MAIN), NULL, WinProc);
    return 0;
}

void MessageBoxErr(const char* title, const char* content)
{
    assert(_mainDlg != NULL);

    if (content != nullptr)
    {
        MessageBox(_mainDlg, (LPCTSTR)content, title, MB_OK | MB_ICONERROR);
    }
    else
    {
        auto err = GetLastError();
        LPVOID bufPtr;
        FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
            NULL, err, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR)&bufPtr, 0, NULL);
        MessageBox(_mainDlg, (LPCTSTR)bufPtr, title, MB_OK | MB_ICONERROR);
        LocalFree(bufPtr);
    }

    return;
}

INT_PTR CALLBACK WinProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
        case WM_INITDIALOG:
            OnInitDialog(hDlg);
            return (INT_PTR)TRUE;

        case WM_COMMAND:
            if (LOWORD(wParam) == IDCANCEL)
            {
                EndDialog(hDlg, LOWORD(wParam));
                return (INT_PTR)TRUE;
            }
            else
            {
                return OnCommand(wParam, lParam);
            }
            break;
    }
    return (INT_PTR)FALSE;
}

void OnInitDialog(HWND hDlg)
{
    _mainDlg = hDlg;
    _pathEdit = GetDlgItem(hDlg, IDC_EDIT_PATH);

    {
        char buf[MAX_PATH] = "";
        GetCurrentDirectory(MAX_PATH, buf);
        FILE* file = nullptr;
        if (0 == fopen_s(&file, _path_cache, "r"))
        {
            char mpPath[MAX_PATH] = "";
            if (nullptr != fgets(mpPath, MAX_PATH, file))
            {
                SetWindowText(_pathEdit, mpPath);
            }
        }
    }

    return;
}

INT_PTR OnCommand(WPARAM wParam, LPARAM lParam)
{
    switch (LOWORD(wParam))
    {
        case IDC_BTN_PATH:
            OnClickPathBtn();
            break;

        case IDC_BTN_START:
            OnClickStartBtn();
            break;

        default:
            return (INT_PTR)FALSE;
    }

    return (INT_PTR)TRUE;
}

void OnClickPathBtn()
{
    OPENFILENAME ofn = { 0 };
    TCHAR strFilename[MAX_PATH] = { 0 };
    ofn.lStructSize = sizeof(OPENFILENAME);
    ofn.hwndOwner = NULL;
    ofn.lpstrFilter = "exe flie\0*.exe\0\0";
    ofn.nFilterIndex = 1;
    ofn.lpstrFile = strFilename;
    ofn.nMaxFile = sizeof(strFilename);
    ofn.lpstrInitialDir = NULL;
    ofn.lpstrTitle = TEXT("");
    ofn.Flags = OFN_NOCHANGEDIR | OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_HIDEREADONLY;
    if (GetOpenFileName(&ofn))
    {
        SetWindowText(_pathEdit, strFilename);
        FILE* file = nullptr;
        if (0 == fopen_s(&file, _path_cache, "w+"))
        {
            fwrite(strFilename, strlen(strFilename) + 1, 1, file);
            fclose(file);
        }
    }
}

void OnClickStartBtn()
{
    TCHAR mpPath[MAX_PATH] = { 0 };
    GetWindowText(_pathEdit, mpPath, sizeof(mpPath));

    TCHAR dllPath[MAX_PATH] = { 0 };
    GetModuleFileName(NULL, dllPath, sizeof(dllPath));

    char* mp = mpPath;
    while (*mp != 0) { ++mp; }; while (*mp != '\\') { --mp; }; ++mp;
    char* dll = dllPath;
    while (*dll != 0) { ++dll; }; while (*dll != '\\') { --dll; }; ++dll;
    while (*mp != '.') { *dll++ = *mp++; }
    memcpy(dll, ".dll", 5);

    if (0 != _access_s(mpPath, 4))
    {
        MessageBoxErr("Error", "exe file not found.");
        return;
    }

    if (0 != _access_s(dllPath, 4))
    {
        MessageBoxErr("Error", "dll file not found.");
        return;
    }

    if (!hk::CreateProcessWithDll(mpPath, dllPath))
    {
        MessageBoxErr("Error");
    }
    else
    {
        ShowWindow(_mainDlg, SW_MINIMIZE);
    }

    return;
}



