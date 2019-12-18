#include "Log.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <atomic>
#include <windows.h>

static std::atomic_flag _write_flag = ATOMIC_FLAG_INIT;

void hk::log::Write(const char* fmt, ...)
{
    static bool path_init = false;
    static char path[MAX_PATH] = "";

    while (_write_flag.test_and_set()) {}

    if (!path_init)
    {
        path_init = true;

        char exe[MAX_PATH];
        GetModuleFileName(NULL, exe, MAX_PATH);
        auto baseName = strrchr(exe, '\\');
        sprintf_s(path, "C:\\hklib\\%s.%u.log", baseName+1, GetCurrentProcessId());
    }

    FILE* h = nullptr;
    if (0 == fopen_s(&h, path, "a+"))
    {
        va_list arg;
        va_start(arg, fmt);
        vfprintf_s(h, fmt, arg);
        va_end(arg);
        fclose(h);
    }

    _write_flag.clear();
}

static std::atomic_flag _write_txt_flag = ATOMIC_FLAG_INIT;
void hk::log::WriteTxt(const char* prefix, const char* fmt, ...)
{
    static bool path_init = false;
    static char path[MAX_PATH] = "";

    while (_write_txt_flag.test_and_set()) {}

    if (!path_init)
    {
        path_init = true;

        char exe[MAX_PATH];
        GetModuleFileName(NULL, exe, MAX_PATH);
        auto baseName = strrchr(exe, '\\');
        sprintf_s(path, "C:\\hklib\\%s.%u.%s.log", baseName+1, GetCurrentProcessId(), prefix);
    }

    FILE* h = nullptr;
    if (0 == fopen_s(&h, path, "a+"))
    {
        va_list arg;
        va_start(arg, fmt);
        vfprintf_s(h, fmt, arg);
        va_end(arg);
        fclose(h);
    }
    _write_txt_flag.clear();
}

static std::atomic_flag _write_bin_flag = ATOMIC_FLAG_INIT;
void hk::log::WriteBin(const char* prefix, const void* data, int count)
{
    static bool path_init = false;
    static char path[MAX_PATH] = "";

    while (_write_bin_flag.test_and_set()) {}

    if (!path_init)
    {
        path_init = true;

        char exe[MAX_PATH];
        GetModuleFileName(NULL, exe, MAX_PATH);
        auto baseName = strrchr(exe, '\\');
        sprintf_s(path, "C:\\hklib\\%s.%u.%s.log", baseName+1, GetCurrentProcessId(), prefix);
    }

    FILE* h = nullptr;
    if (0 == fopen_s(&h, path, "a+"))
    {
        fwrite(data, count, 1, h);
        fclose(h);
    }
    _write_bin_flag.clear();
}

