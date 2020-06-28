#include "mem.h"

std::vector<char> wchars2string(TCHAR* str)
{
    std::vector<char> buffer;
    int size = WideCharToMultiByte(CP_UTF8, 0, str, -1, NULL, 0, NULL, NULL);
    if (size > 0) {
        buffer.resize(size);
        WideCharToMultiByte(CP_UTF8, 0, str, -1, (LPSTR)(&buffer[0]), buffer.size(), NULL, NULL);
    }

    return buffer;
}