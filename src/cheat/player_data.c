#include "player_data.h"
#include "bot.h"
#include "helper.h"
#include "object_enumerator.h"

extern HANDLE g_context_mutex;

// Function to retrieve and display player GUID
unsigned long long DisplayPlayerGuid() {
  PlayerDataPointer playerData = *(PlayerDataPointer *)PLAYER_DATA_ADDRESS;

  if (playerData == 0) {
    MessageBox(NULL, "Player not logged in.", "Status", MB_OK | MB_ICONINFORMATION);
    return 0;
  } else {
    unsigned long long guid = *(unsigned long long *)(playerData + 0xC0);
    //char msg[256];
    //sprintf_s(msg, sizeof(msg), "DLL Injected! Player GUID: %llu", guid);
    //logToConsole(msg);
    return guid;
  }
}

unsigned long long TargetGuid() {
    WaitForSingleObject(g_context_mutex, INFINITE);
    PlayerDataPointer playerData = *(PlayerDataPointer *)PLAYER_DATA_ADDRESS;
    char msg[256]; // Buffer for logging

    // Logging playerData itself is tricky as it's a pointer value.
    sprintf_s(msg, sizeof(msg), "TargetGuid(): playerData pointer value: 0x%p", (void*)playerData);
    consoleLogger(msg);
    logging(msg);

    if (playerData == 0) {
        MessageBox(NULL, "Player not logged in.", "Status", MB_OK | MB_ICONINFORMATION);
        return 0; // Return 0 if player data is not accessible
    }

    // Ensure descriptor_ptr is valid before using it
    if (g_context.descriptor_ptr == 0) {
        consoleLogger("TargetGuid(): g_context.descriptor_ptr is NULL!");
        logging("TargetGuid(): g_context.descriptor_ptr is NULL!");
        return 0; // Return 0 if descriptor_ptr is NULL
    }

    // Use the corrected offset
    uintptr_t address_of_target_guid = g_context.descriptor_ptr + TARGET_GUID_OFFSET;

    // Safety check the address before dereferencing
    // (This is a basic check; a more robust check might involve IsBadReadPtr, but that has its own issues)
    // TODO: We already have a list of units and their ptrs, why are we checking arbitrary values?
    if (address_of_target_guid < 0x1000) { // Arbitrary low address check
        sprintf_s(msg, sizeof(msg), "TargetGuid(): Calculated address for target GUID is invalid: 0x%p", (void*)address_of_target_guid);
        consoleLogger(msg);
        logging("TargetGuid(): Calculated address for target GUID is invalid: 0x%p", (void*)address_of_target_guid);
        return 0;
    }

    unsigned long long targetGuidValue = *(unsigned long long *)address_of_target_guid;

    sprintf_s(msg, sizeof(msg), "TargetGuid(): Read Target GUID: %llu from address 0x%p (desc_ptr=0x%X + offset=0x%X)",
            targetGuidValue,
            (void*)address_of_target_guid,
            g_context.descriptor_ptr,
            TARGET_GUID_OFFSET);
    consoleLogger(msg);
    logging("TargetGuid(): Read Target GUID: %llu from address 0x%p (desc_ptr=0x%X + offset=0x%X )",
            targetGuidValue,
            (void*)address_of_target_guid,
            g_context.descriptor_ptr,
            TARGET_GUID_OFFSET);
    ReleaseMutex(g_context_mutex);
    return targetGuidValue;
}

void clickToMove(void* playerPtr, clickType clickType, Position position) {
  unsigned long long interactGuidPtr = 0;
  Position pos = position;
  ClickToMoveFuncPtr func = CLICK_TO_MOVE_FUN_PTR;


  // __thiscall workaround
  __asm {
      mov ecx, playerPtr
      push 2.0f
      lea eax, pos
      push eax
      lea eax, interactGuidPtr
      push eax
      push clickType
      mov eax, func
      call eax
  }
  //CLICK_TO_MOVE_FUN_PTR(playerPtr, clickType, &interactGuidPtr, &pos, 2.0f);
}

void clickToMoveStop(void* playerPtr, clickType clickType, Position position) {
  unsigned long long interactGuidPtr = 0;
  Position pos = position;
  ClickToMoveFuncPtr func = CLICK_TO_MOVE_FUN_PTR;

    __asm {
      mov ecx, playerPtr
      push 2.0f
      lea eax, pos
      push eax
      lea eax, interactGuidPtr
      push eax
      push clickType
      mov eax, func
      call eax
  }
}

void setTarget(unsigned long long guid) {
    char buffer[256];

    // Validate the GUID
    if (guid == 0) {
        sprintf_s(buffer, sizeof(buffer), "setTarget: Invalid GUID: %llu (0)", guid);
        consoleLogger(buffer);
        logging("setTarget: Invalid GUID: %llu (0)", guid);
        return;
    }

    // Validate the function pointer

    if (!g_context.descriptor_ptr) {
        sprintf_s(buffer, sizeof(buffer), "setTarget: TARGET_FUN_PTR is NULL. Cannot call function.");
        consoleLogger(buffer);
        logging("setTarget: TARGET_FUN_PTR is NULL. Cannot call function.");
        return;
    }

    if (&guid != 0) {
        // Log the function call
        sprintf_s(buffer, sizeof(buffer), "setTarget: Calling function at 0x%08X with GUID: %llu", (uintptr_t)TARGET_FUN_PTR, guid);
        consoleLogger(buffer);
        logging("setTarget: Calling function at 0x%08X with GUID: %llu", (uintptr_t)TARGET_FUN_PTR, guid);

        // Call the function
        SetTargetFuncPtr func_to_call = (SetTargetFuncPtr)TARGET_FUN_PTR;
        func_to_call(guid); // Call the game's SetTarget function

        // Log success
        consoleLogger("setTarget: Function call completed");
        logging("setTarget: Function call completed");
    }
}