#ifndef INJECTOR_H
#define INJECTOR_H

#include <windows.h>

// Function to inject the DLL into the target process
void InjectDLL(DWORD processId, const char *dllPath);

// Function to get the target process ID
DWORD GetTargetProcessId(const char *processName);

// Function to check if a dll is loaded
BOOL IsDllLoaded(HANDLE hProcess, const char *dllName);

#endif // INJECTOR_H