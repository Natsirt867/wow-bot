#include "direct_x_mgr.h"
#include "helper.h"
#include "../common/common.h"
#include <d3d9.h>
#include "MinHook.h"
#include <stdio.h>
#include "player_data.h"
#include "../server/pipe_server.h"
#include "command_queue.h"

typedef HRESULT (WINAPI *EndScene_t)(LPDIRECT3DDEVICE9 pDevice);
static EndScene_t original_endscene = NULL;

static DWORD last_frame_tick = 0;

void** GetD3D9DeviceTable() {
    static void* vtable[119];
    static bool initialized = FALSE;

    if (initialized) {
        return vtable;
    }

    logging("DIRECTXMGR: Initializing D3D9 vtable...\n");

    // Create D3D9 interface
    IDirect3D9* d3d = Direct3DCreate9(D3D_SDK_VERSION);
    if (!d3d) {
        logging("DIRECTXMGR: Direct3DCreate9 failed!\n");
        return NULL;
    }

    // Create dummy window
    HWND tempHwnd = CreateWindowA("STATIC", "DummyD3D", WS_DISABLED,
        0, 0, 1, 1, NULL, NULL, GetModuleHandle(NULL), NULL);
    if (!tempHwnd) {
        logging("DIRECTXMGR: CreateWindowA failed! Error: %d\n", GetLastError());
        d3d->lpVtbl->Release;
        return NULL;
    }

    // Create device
    D3DPRESENT_PARAMETERS params = {0};
    params.Windowed = TRUE;
    params.SwapEffect = D3DSWAPEFFECT_DISCARD;
    params.hDeviceWindow = tempHwnd;

    IDirect3DDevice9* device = NULL;
    HRESULT result = d3d->lpVtbl->CreateDevice(
        d3d, D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, tempHwnd,
        D3DCREATE_SOFTWARE_VERTEXPROCESSING, &params, &device);
    if (FAILED(result)) {
        logging("DIRECTXMGR: CreateDevice failed! HRESULT: 0x%08X\n", result);
        d3d->lpVtbl->Release;
        DestroyWindow(tempHwnd);
        return NULL;
    }

    // Copy vtable
    memcpy(vtable, *(void***)device, sizeof(vtable));
    logging("DIRECTXMGR: vtable initialized successfully\n");

    device->lpVtbl->Release;
    d3d->lpVtbl->Release;
    DestroyWindow(tempHwnd);

    initialized = TRUE;
    return vtable;
}


HRESULT WINAPI MyEndScene(LPDIRECT3DDEVICE9 pDevice) {
    DirectX_ThrottleFPS();

    char buffer[128];
    static int frameCount = 0;
    if (++frameCount % 60 == 0) {

        //snprintf(buffer, sizeof(buffer), "EndScene called 60 times!");
        //logToConsole(buffer);
    }


    //LuaCallFastcall_t luaExec = (LuaCallFastcall_t)LUA_CALL_FUN_PTR;
    //luaExec("Jump()", "Unused");
    //luaExec("SendChatMessage(\"Hello from bot!\")", "Unused");
    char cmd[MAX_COMMAND_LEN];
    while (dequeue_command(cmd)) {
        if (strncmp(cmd, "LUA:", 4) == 0) {
            const char* lua_code = cmd + 4;
            LuaCallFastcall_t luaExec = (LuaCallFastcall_t)LUA_CALL_FUN_PTR;
            luaExec(lua_code, "Unused");
        } else {
            handle_command(cmd); // For other commands
        }
    }


    // TODO
    // ...overlay drawing or other logic ...
    return original_endscene(pDevice);
}


bool DirectX_InstallHooks(void) {
    // Initialize MinHook
    if (MH_Initialize() != MH_OK) {
        logging("DIRECTXMGR: MinHook initialization failed!\n");
        return FALSE;
    }

    // Get the address of EndScene from a real device
    void** vtable = GetD3D9DeviceTable();
    if (!vtable) {
        logging("DIRECTXMGR: Failed to retrieve D3D9 vtable!\n");
        return FALSE;
    }

    void* end_scene_addr = vtable[42]; // EndScene is at index 42 apparently

    if (MH_CreateHook(end_scene_addr, &MyEndScene, (LPVOID*)&original_endscene) != MH_OK) {
        logging("DIRECTXMGR: MH_CreateHook failed!\n");
        return false;
    }

    if (MH_EnableHook(end_scene_addr) != MH_OK) {
        logging("DIRECTXMGR: MH_EnableHook failed!\n");
        return false;
    }

    // to patch click-to-move
    BYTE patch[4] = {0x00, 0x00, 0x00, 0x00};
    memcpy((void*)0x860A90, patch, 4);

    // lua unlocker patch
    DWORD oldProtect;
    VirtualProtect((void*)LUA_UNLOCK, 6, PAGE_EXECUTE_READWRITE, &oldProtect);
    BYTE patch2[6] = {0xB8, 0x01, 0x00, 0x00, 0x00, 0xc3};
    memcpy((void*)LUA_UNLOCK, patch2, 6);
    VirtualProtect((void*)LUA_UNLOCK, 6, oldProtect, &oldProtect);

    logging("DIRECTXMGR: Hooks installed successfully!\n");
    consoleLogger("DirectX9 hooks successfully installed, now have access to main thread.\n");
    return TRUE;
}


void DirectX_RemoveHooks(void) {
    // Get the address of the EndScene from a real device
    void** vtable = GetD3D9DeviceTable();
    if (!vtable)
        return;

    void* end_scene_addr = vtable[42];

    logging("DIRECTXMGR: Removing installed hooks...\n");
    MH_DisableHook(end_scene_addr);
    MH_Uninitialize();
    logging("DIRECTXMGR: Successfully removed installed hooks!\n");
}

void DirectX_ThrottleFPS(void) {
    DWORD const now = GetTickCount();

    if (last_frame_tick != 0) {
        DWORD const time_between_frame = now - last_frame_tick;
        if (time_between_frame < 15) {
            Sleep(15 - time_between_frame);
        }
    }
    last_frame_tick = GetTickCount();
    //logging("DIRECTXMGR: FPS successfully throttled %d\n", last_frame_tick);
}