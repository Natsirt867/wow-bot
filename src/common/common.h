#ifndef COMMON_H
#define COMMON_H

#include <windows.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include "math.h"
#include "float.h"




#define DLL_EXPORT __declspec(dllexport)
// USING DWORD_PTR for flexibility between 32-bit and 64-bit
typedef DWORD_PTR PlayerDataPointer;
typedef DWORD_PTR EnumerateVisibleObjectsFunPointer;
typedef DWORD_PTR ObjectDataPointer;
typedef DWORD_PTR SetTargetPointer;

/* DEFINTIONS */
#define PLAYER_DATA_ADDRESS 0x00B41414
#define ENUMERATE_VISIBLE_OBJECTS_FUN_PTR 0x00468380
#define GET_OBJECT_PTR_FUN_PTR 0x00464870
#define I_SCENE_END_FUN_PTR 0x005A17A0
#define OBJECT_TYPE_OFFSET 0x14
#define CLICK_TO_MOVE_FUN_PTR ((ClickToMoveFuncPtr)0x00611130)
#define LUA_CALL_FUN_PTR 0x00704CD0
#define LUA_UNLOCK 0x494A50
#define TARGET_FUN_PTR 0x00493540

#define POS_X_OFFSET 0x9B8
#define POS_Y_OFFSET 0x9BC
#define POS_Z_OFFSET 0x9C0

/* DESCRIPTORS */
#define DESCRIPTOR_OFFSET 0x8
#define HEALTH_OFFSET 0x58
#define NAME_OFFSET 0xB30
#define NAME_BASE_OFFSET 0xC0E230
#define NEXT_NAME_OFFSET 0xC
#define PLAYER_NAME_OFFSET 0x14
#define TARGET_GUID_OFFSET 0x40

#define MAX_OBJECTS 1024

/* ENUM */
typedef enum {
    NONE, // Equivalent of the C# walk-through I believe, struct starts 0 ends 7 for total 8 object types.
    ITEM,
    CONTAINER,
    UNIT,
    PLAYER,
    GAME_OBJECT,
    DYNAMIC_OBJECT,
    CORPSE
} objectType;

typedef enum {
    Move = 0x4,
    None = 0xD
} clickType;

typedef struct {
    unsigned long long guid;
    void* pointer;
    objectType object_type;
} wowObject;

typedef struct {
    float x;
    float y;
    float z;
} Position;

typedef struct {
    wowObject base; // base class
    int health;
    char* name;
    Position pos;
    unsigned long long targetGuid;
} unitObject;

typedef struct {
    unitObject base;
} wowPlayer;

typedef struct {
    wowPlayer base;
} localPlayer;

typedef struct {
    wowObject objects[MAX_OBJECTS];
    int object_count;

    wowPlayer players[MAX_OBJECTS];
    int player_count;

    unitObject units[MAX_OBJECTS];
    int unit_count;

    uint32_t descriptor_ptr; // store the descriptor_ptr for global access -- local player only
    Position local_player_pos; // store the local players position for global access
    void* local_player_ptr; // store the pointer to teh local player object for global access
    int local_player_found; // store flag to indicate if local player found
} GameContext;

// Global extern
extern GameContext g_context;

/* FUNCTION PROTOTYPES */
const char* typeToString(objectType type);

#endif // COMMON_H