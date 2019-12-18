#pragma once

namespace hk
{
    void* HookApi(void* api, void* func);
    void UnhookApi(void* api);
    bool CreateProcessWithDll(const char* exe, const char* dll);
}



