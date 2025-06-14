#include "cheat.h"
#include "direct_x_mgr.h"
#include "../common/logging.h"
#include "../server/pipe_server.h"
#include "bot.c"
#include "../states/idle_state.h"
#include "command_queue.h"




DWORD WINAPI StartUpThread(LPVOID lpParam) {
	UNREFERENCED_PARAMETER(lpParam);
	initializeLogging();

	logging("DLL: Main Worker Thread started!");
	logging("DLL: .: Logging Initialized ~~ Welcome to Natsirt Bot ~~ :.\nDLL: Attempting to setup console...\n");
	consoleSetup();

	logging("DLL: Console setup successfully.\nDLL: Initializing command queue...\n");
	command_queue_init();

	logging("DLL: Command queue initialized.\nDLL: Attempting to install DirectX hooks...");
	DirectX_InstallHooks();
	logging("DLL: DirectX Hooks installed successfully.\n");

	logging("DLL: Initializing server pipe...\n");
	pipe_init();
	logging("DLL: Server pipe initialized. ");

	logging("DLL: Main Worker Thread exiting.");

	return 0;
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved) 
{ 
	switch (ul_reason_for_call) 
	{ 
		case DLL_PROCESS_ATTACH:
		{
				DisableThreadLibraryCalls(hModule);
			break;
		case DLL_THREAD_ATTACH:
		case DLL_THREAD_DETACH:
		case DLL_PROCESS_DETACH:
			break;
		}
	}

	return TRUE;
}


void DLL_EXPORT __stdcall EnumerateVisibleObjects(unsigned int callback, int filter, unsigned int parPtr) {
	typedef void __fastcall func(unsigned int callback, int filter);
	func* function = (func*)parPtr;
	function(callback, filter);
}


__declspec(dllexport) VOID StartInjectedThread() {
	//logging("DLL: Exported StartInjectedThread called.");
	HANDLE hWorker = CreateThread(NULL, 0, StartUpThread, NULL, 0, NULL);
	if (hWorker) {
		//logging("DLL: StartUpThread created by StartInjectedThread.");
		CloseHandle(hWorker);
	} else {
		//logging("DLL: Failed to create StartUpThread from StartInjectedThread.");
	}
}

void __declspec(dllexport) __stdcall LuaCall(char* code, unsigned int ptr)
{
	typedef void (__fastcall *func)(char* code, const char* unused);
	func f = (func)ptr;
	f(code, "Unused");
}