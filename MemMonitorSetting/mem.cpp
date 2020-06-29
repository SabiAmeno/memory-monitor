#include "mem.h"
#include <iostream>

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

BOOL EnableDebugPrivilege()
{
    HANDLE hToken;
    BOOL fOk = FALSE;
    if (OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES, &hToken))
    {
        TOKEN_PRIVILEGES tp;
        tp.PrivilegeCount = 1;
        LookupPrivilegeValue(NULL, SE_DEBUG_NAME, &tp.Privileges[0].Luid);
        tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

        AdjustTokenPrivileges(hToken, FALSE, &tp, sizeof(tp), NULL, NULL);

        fOk = (GetLastError() == ERROR_SUCCESS);
        CloseHandle(hToken);
    }
    return fOk;
}
