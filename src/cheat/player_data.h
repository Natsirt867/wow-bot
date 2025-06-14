#ifndef PLAYER_DATA_H 
#define PLAYER_DATA_H

#include "../common/common.h"

// Fastcall typedef for the game's enumerate function
typedef void (__fastcall *EnumerateVisibleObjectsFastcall_t)(void* callback, int filter);

// Fastcall typedef for the game's lua call function
typedef void (__fastcall *LuaCallFastcall_t)(const char* code, const char* unused);

// clickToMove def (Pointer to player, clickType (4 types I believe), Position struct x,y,z
void clickToMove(void* playerPtr, clickType clickType, Position position);

void clickToMoveStop(void *playerPtr, clickType clickType, Position position);

// SetTarget function
void setTarget(unsigned long long guid);

unsigned long long TargetGuid();

// The enumeration and logging function
void enumerate_visible_objects_and_log();

// The callback signature (adjust if needed)
int __stdcall enumerate_callback(unsigned long long guid);

void DLL_EXPORT __stdcall EnumerateVisibleObjects(unsigned int callback, int filter, unsigned int parPtr);

void DLL_EXPORT __stdcall LuaCall(char* code, unsigned int ptr);

typedef void (*ClickToMoveFuncPtr)(void* playerPtr, clickType clickType, unsigned long long* interactGuidPtr, Position* positionPtr, float precision);

typedef void (__fastcall *SetTargetFuncPtr)(unsigned long long guid);

#endif // PLAYER_DATA_H

