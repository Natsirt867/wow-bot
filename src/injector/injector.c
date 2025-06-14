#include <windows.h>
#include <tlhelp32.h>
#include <stdio.h>
#include "../common/logging.h"

#pragma comment(lib, "user32.lib")

DWORD GetTargetProcessId(const char *processName)
{
    DWORD pid = 0;
	
	// 32 or 64 bit unsigned int variable snapshot
	// calls function to take a snapshot of the specified processes, as well as heaps, modules, and threads used by the processes
	// TH32CS_SNAPPROCESS flag includes all processes in the system in the snapshot.
	// 0 is the PID
    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	
	// if snapshot is not equal to 'null handle' (this is -1 as defined by macro)
    if (snapshot != INVALID_HANDLE_VALUE)
    {
		// Contains info about process running
		// must initialize dwSize to size of structure itself
        PROCESSENTRY32 pe;
        pe.dwSize = sizeof(PROCESSENTRY32);
		
		// Retrieve the first process in snapshot
		// Must pass snapshot handle and a pointer to PROCESSENTRY32 structure
        if (Process32First(snapshot, &pe))
        {
			// loop to compare PROCESSENTRY32 member szExeFile
			// if processName (WoW.exe) matches, its evaluates true (0)
            // sets pid variable to process ID of PROCESSENTRY32 as member name equal to processName passed to function
			do
            {
                if (strcmp(pe.szExeFile, processName) == 0)
                {
                    pid = pe.th32ProcessID;
                    break;
                }
			// This is true while there is a next process in the system snapshot (next entry in buffer), else false
            } while (Process32Next(snapshot, &pe));
        }
    }
    CloseHandle(snapshot);
    return pid;
}

// New function to check if DLL is loaded
BOOL IsDllLoaded(HANDLE hProcess, const char *dllName)
{
    HANDLE hModuleSnap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, GetProcessId(hProcess));
    if (hModuleSnap == INVALID_HANDLE_VALUE)
        return FALSE;

    MODULEENTRY32 me32;
    me32.dwSize = sizeof(MODULEENTRY32);

    if (Module32First(hModuleSnap, &me32))
    {
        do
        {
            if (_stricmp(me32.szModule, dllName) == 0 || _stricmp(me32.szExePath, dllName) == 0)
            {
                CloseHandle(hModuleSnap);
                return TRUE;
            }
        } while (Module32Next(hModuleSnap, &me32));
    }

    CloseHandle(hModuleSnap);
    return FALSE;
}

void InjectDLL(DWORD processId, const char *dllPath)
{
	
    HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, processId);
    if (hProcess == NULL)
    {
        printf("Failed to open process for check. Error: %d\n", GetLastError());
        return;
    }

    // Check if DLL is already loaded
    if (IsDllLoaded(hProcess, "cppcheat.dll"))
    {
        MessageBox(NULL, "DLL 'cppcheat.dll' is already injected into the process.", "Success", MB_OK);
        printf("DLL 'cppcheat.dll' is already injected into the process.\n");
        CloseHandle(hProcess);
        return;
    }

    CloseHandle(hProcess); // Close this handle since we're reopening it with more permissions

    // Now, reopen the process with necessary permissions for injection
    hProcess = OpenProcess(PROCESS_CREATE_THREAD | PROCESS_QUERY_INFORMATION | PROCESS_VM_OPERATION | PROCESS_VM_WRITE | PROCESS_VM_READ, FALSE, processId);
    if (hProcess == NULL)
    {
        printf("Failed to open process for injection. Error code: %d\n", GetLastError());
        return;
    }

    LPVOID pRemoteBuffer = VirtualAllocEx(hProcess, NULL, strlen(dllPath) + 1, MEM_COMMIT, PAGE_READWRITE);
    if (pRemoteBuffer == NULL)
    {
        printf("Failed to allocate memory in the process.\n");
        CloseHandle(hProcess);
        return;
    }

    WriteProcessMemory(hProcess, pRemoteBuffer, (LPVOID)dllPath, strlen(dllPath) + 1, NULL);

    HANDLE hThread = CreateRemoteThread(hProcess, NULL, 0, (LPTHREAD_START_ROUTINE)LoadLibraryA, pRemoteBuffer, 0, NULL);
    if (hThread == NULL)
    {
        printf("Failed to create remote thread. Error: %lu\n", GetLastError());
    }

    WaitForSingleObject(hThread, INFINITE);

    DWORD dwRemoteDllBase = 0;
    GetExitCodeThread(hThread, &dwRemoteDllBase); // Get HMODULE of DLL in target
    CloseHandle(hThread);
    VirtualFreeEx(hProcess, pRemoteBuffer, 0, MEM_RELEASE);

    if (dwRemoteDllBase == 0) {
        printf("Failed to load DLL into remote process or get its base address\n");
        CloseHandle(hProcess);
        return;
    }

    printf("DLL loaded at remote address: 0x%1X\n", dwRemoteDllBase);

    // Get address of the exported function to call
    HMODULE hLocalDll = LoadLibrary(dllPath);
    if (!hLocalDll) {
        printf("Could not load DLL\n");
    }

    FARPROC pfnLocalExport = GetProcAddress(hLocalDll, "StartInjectedThread");
    if (!pfnLocalExport) {
        printf("Could not GetProcAddress");
        FreeLibrary(hLocalDll);
    }

    DWORD_PTR rvaExport = (DWORD_PTR)pfnLocalExport - (DWORD_PTR)hLocalDll;
    FreeLibrary(hLocalDll); // Unloads from injector

    LPVOID pRemoteExportFunc = (LPVOID)(dwRemoteDllBase + rvaExport);
    printf("Calculated remote export function address: 0x%p\n", pRemoteExportFunc);

    HANDLE hExecThread = CreateRemoteThread(hProcess, NULL, 0,
        (LPTHREAD_START_ROUTINE)pRemoteExportFunc,
        NULL, // no arg for this example
        0, NULL);

    if (!hExecThread) {
        printf("Failed to create remote thread for exported function. Error: %lu", GetLastError());

    } else {
        printf("Successfully called exported function via remote thread.\n");
        WaitForSingleObject(hExecThread, INFINITE);
        CloseHandle(hExecThread);
    }
    CloseHandle(hProcess);
}

int main()
{
    const char *processName = "WoW.exe";
    const char *dllPath = "F:\\C Projects\\WoWBotD3D9\\cppcheat.dll";

    DWORD pid = GetTargetProcessId(processName);
    if (pid != 0)
    {
        printf("Found process with PID: %d\n", pid);
        InjectDLL(pid, dllPath);
    }
    else
    {
        printf("Could not find process.\n");
    }
    getchar();
    return 0;
}