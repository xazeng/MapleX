#pragma once


namespace hk
{
    namespace log
    {
        void Write(const char* fmt, ...);

        void WriteTxt(const char* prefix, const char* fmt, ...);
        void WriteBin(const char* prefix, const void* data, int count);
    }
}

