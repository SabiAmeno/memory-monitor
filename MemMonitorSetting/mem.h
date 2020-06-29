#pragma once

#include "mem.h"
#include <thread>
#include <cassert>
#include <Windows.h>
#include <Psapi.h>
#include <tchar.h>
#include <vector>
#include <fstream>
#include <map>
#include <ShlObj.h>
#include <QPixmap>
#include <QtWinExtras/qwinfunctions.h>

std::vector<char> wchars2string(TCHAR* str);
BOOL EnableDebugPrivilege();

class Proc {
public:
    Proc() {}
    Proc(DWORD procid)
    {
        _initial(procid);
    }

    ~Proc()
    {
        if (IsValid())
            CloseHandle(_handle);
    }

    bool IsValid() { return _handle != NULL; }
    std::string GetProcName()
    {
        HMODULE hMod;
        DWORD need;
        TCHAR szProcessName[MAX_PATH * 2] = TEXT("<unknown>");

        if (!EnumProcessModulesEx(_handle, &hMod, sizeof(hMod), &need, LIST_MODULES_ALL)) {
            DWORD err = GetLastError();
            return "<unknown error>";
        }

        GetModuleBaseName(_handle, hMod, szProcessName, sizeof(szProcessName) / sizeof(TCHAR));

        std::string name(&wchars2string(szProcessName)[0]);
        size_t pos = name.find_last_of('.');

        return name.substr(0, pos);
    }
    double GetProcMemory()
    {
        PROCESS_MEMORY_COUNTERS mems;

        GetProcessMemoryInfo(_handle, &mems, sizeof(mems));

        return mems.WorkingSetSize / 1024.0 / 1024.0;
    }

    static std::map<DWORD, std::string> GetProcs()
    {
        std::map<DWORD, std::string> pros;
        DWORD process[1024], needed;
        if (!EnumProcesses(process, sizeof(process), &needed))
            return pros;

        DWORD cprocess = needed / sizeof(DWORD);

        for (int i = 0; i < cprocess; ++i) {
            int pos = -1;
            if (process[i] != 0)
            {
                Proc proc(process[i]);
                if (proc.IsValid()) {
                    pros.insert(std::make_pair(process[i], proc.GetProcName()));
                }
            }
        }

        return pros;
    }

    void Terminate()
    {
        EnableDebugPrivilege();
        BOOL r = TerminateProcess(_handle, 0);
        if (!r) {
            auto err = GetLastError();
        }
    }

    QPixmap getIcon()
    {
        QPixmap pix;

        WCHAR path[1024] = { 0 };
        DWORD psize = 1024;
        BOOL r = QueryFullProcessImageName(_handle, 0, path, &psize);
        if (r)
        {
            SHFILEINFOW sfi = { 0 };
            HRESULT hr = SHGetFileInfo(path,
                -1,
                &sfi,
                sizeof(sfi),
                SHGFI_ICON | SHGFI_ADDOVERLAYS | SHGFI_EXETYPE | SHGFI_SMALLICON | SHGFI_TYPENAME);
            if (SUCCEEDED(hr))
            {
                pix = QtWin::fromHICON(sfi.hIcon);
            }
        }

        return pix;
    }
private:
    void _initial(DWORD procid)
    {
        _handle = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ | PROCESS_TERMINATE, FALSE, procid);

        if (NULL == _handle) {
            DWORD err = GetLastError();
            return;
        }

    }
private:

    HANDLE _handle;
};

struct ProcInfo
{
    DWORD procid;
    std::string procname;
    double memory;
};

class PrintHelper
{
public:
    virtual void print(const std::string&) = 0;
protected:
    PrintHelper() {}
};

class FilePrinter : public PrintHelper
{
public:
    virtual void print(const std::string& s)
    {
        std::ofstream ofs;
        ofs.open("proc-record.txt", std::ios::out | std::ios::binary | std::ios::trunc);
        if (!ofs.is_open())
            return;

        ofs << s;
        ofs.close();
    }
};

typedef std::vector<ProcInfo> ProcInfos;

class ProcMonitorManage {
public:
    ProcMonitorManage() : _procs(), _print_helper(0) {}

    void AddProc(const ProcInfo& pi) { _procs.push_back(pi); }
    ProcInfos Procs() const { return _procs; }

    void RemoveProc(DWORD pid)
    {
        auto it = _procs.begin();
        for (; it != _procs.end(); ++it) {
            if (it->procid == pid) {
                it = _procs.erase(it);
                break;
            }
        }
    }
    void SetPrinter(PrintHelper* ph) { _print_helper = ph; }
    void Save()
    {
        std::string str;
        for (auto pi : _procs)
        {
            char buffer[1024] = { 0 };
            sprintf_s(buffer, "%5d[%s] => [%f]\n", pi.procid, pi.procname.c_str(), pi.memory);
            str += buffer;
        }
        _print_helper->print(str);
    }

    bool InMonitor(DWORD pid, int& pos)
    {
        int i = 0;
        for (int i = 0; i < _procs.size(); ++i) {
            if (_procs[i].procid == pid) {
                pos = i;
                return true;
            }
        }

        return false;
    }

    ProcInfo& operator[](int pos) {
        return _procs[pos];
    }

    void Reset() { _procs.clear(); }
private:
    ProcInfos _procs;
    PrintHelper* _print_helper;
};