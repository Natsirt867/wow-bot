#pragma once
#include "../common/common.h"
#include "../common/logging.h"
#include <tlhelp32.h>

#ifndef HELPER_H
#define HELPER_H


void consoleLogger(const char* msg);
void fileLogger(const char* msg);
float distance_to(Position a, Position b);

// Helper function to get process id by name
static inline DWORD GetProcessIdByName(const char* process_name) {
    PROCESSENTRY32 entry;
    entry.dwSize = sizeof(PROCESSENTRY32);

    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (Process32First(snapshot, &entry)) {
        do {
            if (_stricmp(entry.szExeFile, process_name) == 0) {
                DWORD const pid = entry.th32ProcessID;
                CloseHandle(snapshot);
                return pid;
            }
        } while (Process32Next(snapshot, &entry));
    }
    CloseHandle(snapshot);
    return 0;
}
/*
// Write byte function
static inline BOOL WriteBytes(void* address, const BYTE* bytes, SIZE_T size) {
    DWORD const pid = GetProcessIdByName("WoW.exe");

    if (pid == 0) {
        puts("Process not found");
        return FALSE;
    }

    HANDLE const hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
    if (!hProcess) {
        puts("Failed to open process");
        return FALSE;
    }

    SIZE_T bytes_written = 0;
    BOOL result = WriteProcessMemory(hProcess, address, bytes, size, &bytes_written);

    CloseHandle(hProcess);
    if (!result || bytes_written != size) {
        puts("WriteProcessMemory failed");
        return FALSE;
    }

    return TRUE;
}*/

#endif //HELPER_H
