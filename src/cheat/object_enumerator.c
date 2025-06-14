#include "object_enumerator.h"
#include "player_data.h"
#include "../common/logging.h"
#include <windows.h>
#include "cheat.h"
#include "helper.h"

extern HANDLE g_context_mutex;

static wowObject g_objects[MAX_OBJECTS];
static int g_object_count = 0;

static wowPlayer players[MAX_OBJECTS];
//static int player_count = 0;

static unitObject units[MAX_OBJECTS];
//static int unit_count = 0;

//static Position local_player_pos = {0, 0, 0};
//static void* local_player_ptr = NULL;
//static uint32_t g_descriptor_ptr = 0;
//static int local_player_found = 0;

void* player_ptr = NULL;
Position pos = {0,0,0};

// Helper: Read a byte from process memory
static uint8_t read_byte(void* ptr) {
    return *(uint8_t*)ptr;
}

int read_int(void* ptr) {
    return *(int*)ptr;
}

static uint8_t* read_bytes(void* ptr, size_t count) {
    uint8_t* result = (uint8_t*)malloc(count);

    if (!result) {
        return NULL;
    }

    memcpy(result, ptr, count);
    return result;
}

static float read_float(void* ptr) {
    return *(float*)ptr;
}

char* read_string(void* ptr) {
    // Need to read the name pointer first to get proper health value
    uint8_t* p = (uint8_t*)ptr;
    size_t max_len = 512;
    size_t len = 0;

    while (len < max_len && p[len] != 0) {
            len++;
    }

    // Allocate mem for the string
    char* result = (char*)malloc(len + 1);
    if (!result) {
        return NULL;
    }

    memcpy(result, p, len);
    result[len] = '\0';
    //logging("result %s", result);
    return result;
}

char* name_string(void* ptr) {
    uint32_t name_ptr1 = read_int((uint8_t*)ptr + NAME_OFFSET);
    //logging("name_ptr1: 0x%08X", name_ptr1);
    if (!name_ptr1) {
        return NULL;
    }

    uint32_t name_ptr2 = read_int((uint8_t*)name_ptr1);
    //logging("name_ptr2: 0x%08X", name_ptr2);
    if (!name_ptr2) return NULL;

    // Try reading the string directly from name_ptr2
    char* result = read_string((void*)name_ptr2);
    //logging("name_string result: %s", result);

    return result;
}

char* player_name_from_global_store(unsigned long long guid) {
    uintptr_t name_ptr = *(uintptr_t*)NAME_BASE_OFFSET;
    while (1) {
        unsigned long long next_guid = *(unsigned long long*)((uint8_t*)name_ptr + NEXT_NAME_OFFSET);
        if (next_guid != guid) {
            name_ptr = *(uintptr_t*)name_ptr; // move to next entry
        } else {
            break;
        }
    }
    return read_string((uint8_t*)name_ptr + PLAYER_NAME_OFFSET);
}

/*
unsigned long long getTargetGuid(uintptr_t descriptor_ptr) {
    // Adds the offset to the descriptor ptr and returns the TargetGuid
    return *(unsigned long long*)(descriptor_ptr + TARGET_GUID_OFFSET);
}*/

void clear_units() {
    for (int i = 0; i < g_context.unit_count; ++i) {
        if (g_context.units[i].name) {
            free(g_context.units[i].name);
            g_context.units[i].name = NULL;
        }
        memset(&g_context.units[i], 0, sizeof(unitObject));
    }
    g_context.unit_count = 0;
}

void clear_players() {
    for (int i = 0; i < g_context.player_count; ++i) {
        if (g_context.players[i].base.name) {
            free(g_context.players[i].base.name);
            g_context.players[i].base.name = NULL;
        }
        memset(&g_context.players[i], 0, sizeof(wowPlayer));
    }
    g_context.player_count = 0;
}

void clear_objects () {
    for (int i = 0; i < g_object_count; ++i) {
        memset(&g_objects[i], 0, sizeof(wowObject));
    }
    g_object_count = 0;
}

// The callback called by EnumerateVisibleObjects
int __stdcall enumerate_callback(unsigned long long const guid) {
    // Thread safety
    WaitForSingleObject(g_context_mutex, INFINITE);

    typedef void* (__stdcall *GetObjectPtr_t)(unsigned long long);
    GetObjectPtr_t getObjectPtr = (GetObjectPtr_t)GET_OBJECT_PTR_FUN_PTR;
    void* object_ptr = getObjectPtr(guid);

    if (!object_ptr) return 1;

    uint8_t type_byte = read_byte((uint8_t*)object_ptr + OBJECT_TYPE_OFFSET);
    objectType type = (objectType)type_byte;

    uint32_t descriptor_ptr = read_int((uint8_t*)object_ptr + DESCRIPTOR_OFFSET);

    switch (type) {
    case PLAYER:
        if (guid == DisplayPlayerGuid()) {
            g_context.local_player_pos.x = read_float((uint8_t*)object_ptr + POS_X_OFFSET);
            g_context.local_player_pos.y = read_float((uint8_t*)object_ptr + POS_Y_OFFSET);
            g_context.local_player_pos.z = read_float((uint8_t*)object_ptr + POS_Z_OFFSET);
            //g_context.local_player_pos =  g_context.players[g_context.player_count].base.pos;
            g_context.local_player_ptr = object_ptr;
            g_context.local_player_found = 1;
            g_context.descriptor_ptr = descriptor_ptr; // Only for the local player
        }

        if (g_context.player_count < MAX_OBJECTS) {
                g_context.players[g_context.player_count].base.base.pointer = object_ptr;
                g_context.players[g_context.player_count].base.base.guid = guid;
                g_context.players[g_context.player_count].base.base.object_type = PLAYER;
                g_context.players[g_context.player_count].base.health = read_int((uint8_t*)descriptor_ptr + HEALTH_OFFSET);
                g_context.players[g_context.player_count].base.name = player_name_from_global_store(guid);


                //
                // Read position
                g_context.players[g_context.player_count].base.pos.x = read_float((uint8_t*)object_ptr + POS_X_OFFSET);
                g_context.players[g_context.player_count].base.pos.y = read_float((uint8_t*)object_ptr + POS_Y_OFFSET);
                g_context.players[g_context.player_count].base.pos.z = read_float((uint8_t*)object_ptr + POS_Z_OFFSET);


                //g_context.local_player_ptr = object_ptr;
                //g_context.local_player_found = 1;




                //player_ptr = players[player_count].base.base.pointer;
                //pos = players[player_count].base.pos;

                /*
                char buffer[128];
                snprintf(buffer, sizeof(buffer), "Type: %s, Name: %s, Health: %d, Position: %0.2f, %0.2f, %0.2f",
                        typeToString(g_context.players[g_context.player_count].base.base.object_type),
                        //players[player_count].base.base.guid,
                        //descriptor_ptr,
                        g_context.players[g_context.player_count].base.name,
                        g_context.players[g_context.player_count].base.health,
                        g_context.players[g_context.player_count].base.pos.x,
                        g_context.players[g_context.player_count].base.pos.y,
                        g_context.players[g_context.player_count].base.pos.z
                        );
                logging(buffer);*/
                g_context.player_count++;
            }
        break;
    case UNIT:
        if (g_context.unit_count < MAX_OBJECTS) {
            g_context.units[g_context.unit_count].base.pointer = object_ptr;
            g_context.units[g_context.unit_count].base.guid = guid;
            g_context.units[g_context.unit_count].base.object_type = type;
            g_context.units[g_context.unit_count].health = read_int((uint8_t*)descriptor_ptr + HEALTH_OFFSET);
            g_context.units[g_context.unit_count].name = name_string(object_ptr);

            // Read position
            g_context.units[g_context.unit_count].pos.x = read_float((uint8_t*)object_ptr + POS_X_OFFSET);
            g_context.units[g_context.unit_count].pos.y = read_float((uint8_t*)object_ptr + POS_Y_OFFSET);
            g_context.units[g_context.unit_count].pos.z = read_float((uint8_t*)object_ptr + POS_Z_OFFSET);

            /*
            if (g_context.local_player_found) {
                float dist = distance_to(g_context.local_player_pos, g_context.units[g_context.unit_count].pos);
                char buffer[128];
                snprintf(buffer, sizeof(buffer),
                        "Type: %s, Name: %s, Health: %d, Position: %0.2f, %0.2f, %0.2f, Distance to player: %.2f",
                        typeToString(g_context.units[g_context.unit_count].base.object_type),
                        //units[unit_count].base.guid,
                        //descriptor_ptr,
                        g_context.units[g_context.unit_count].name,
                        g_context.units[g_context.unit_count].health,
                        g_context.units[g_context.unit_count].pos.x,
                        g_context.units[g_context.unit_count].pos.y,
                        g_context.units[g_context.unit_count].pos.z,
                        dist
                        );
                logToConsole(buffer);*/
            //}
            /*logging("Type: %s, GUID: %llu, D_ptr: %d, Health: %d, Name: %s\n Position: %0.6f, %0.6f, %0.6f",
                    typeToString(g_context.units[g_context.unit_count].base.object_type),
                    g_context.units[g_context.unit_count].base.guid,
                    descriptor_ptr,
                    g_context.units[g_context.unit_count].health,
                    g_context.units[g_context.unit_count].name,
                    g_context.units[g_context.unit_count].pos.x,
                    g_context.units[g_context.unit_count].pos.y,
                    g_context.units[g_context.unit_count].pos.z
                    );*/
            g_context.unit_count++;
        }
        break;
    default:
        // wowObject obj;
        if (g_object_count < MAX_OBJECTS) {
            g_objects[g_object_count].pointer = object_ptr;
            g_objects[g_object_count].guid = guid;
            g_objects[g_object_count].object_type = type;
            g_object_count++;
        }
        break;
    }
 /*
    if (g_object_count < MAX_OBJECTS) {
        g_objects[g_object_count].guid = (unsigned long long)guid;
        g_objects[g_object_count].pointer = object_ptr;
        g_objects[g_object_count].object_type = type;
        g_object_count++;
    }
    */
//
    // logging("Object: GUID=%llu, PTR=%p, TYPE=%d", guid, object_ptr, type);

    ReleaseMutex(g_context_mutex);
    return 1;
}

void click_to_move_func() {
    enumerate_visible_objects_and_log();
    float min_dist = FLT_MAX;
    //int closest_index = -1;
    Position closest_unit = {0, 0, 0};
    //float closest_unit_x = 0;
    //float closest_unit_y = 0;
    //float closest_unit_z = 0;

    for (int i = 0; i < g_context.unit_count; ++i) {
        float dist = distance_to(g_context.local_player_pos,  g_context.units[i].pos);
        if (dist < min_dist) {
            min_dist = dist;
            //closest_index = i;
            char buffer_closest[128];
            snprintf(buffer_closest, sizeof(buffer_closest), "Closest Units POS: %0.2f, %0.2f, %0.2f",
                g_context.units[i].pos.x, g_context.units[i].pos.y, g_context.units[i].pos.z);
            //logToConsole(buffer_closest);
            closest_unit.x = g_context.units[i].pos.x;
            closest_unit.y = g_context.units[i].pos.y;
            closest_unit.z = g_context.units[i].pos.z;
        }
    }

    clickToMove(g_context.local_player_ptr, Move, closest_unit);

}

void click_to_move_stop_func() {
    enumerate_visible_objects_and_log();
    Position player = {0, 0, 0};
    player.x = g_context.local_player_pos.x;
    player.y = g_context.local_player_pos.y;
    player.z = g_context.local_player_pos.z;
    logging("click_to_move_stop_func: Stopping movement at position (%.2f, %.2f, %.2f)",
            player.x, player.y, player.z);
    clickToMoveStop(g_context.local_player_ptr, None, player);
}

void set_target_func() {
    if (g_context.local_player_found) {
        unsigned long long target_guid = TargetGuid();
        if (target_guid != 0) {
            setTarget(target_guid);
        } else {
            consoleLogger("No valid target GUID found.");
            logging("No valid target GUID found.");
        }
    } else {
        consoleLogger("Local player not found. Cannot set target");
        logging("Local player not found. Cannot set target");
    }
}

void enumerate_visible_objects_and_log() {
    //g_object_count = 0;

    // Clear objects before calling
    clear_units();
    clear_players();
    clear_objects();

    EnumerateVisibleObjectsFastcall_t enumerateObjects =
        (EnumerateVisibleObjectsFastcall_t)ENUMERATE_VISIBLE_OBJECTS_FUN_PTR;

    if (!enumerateObjects) {
        return;
    }

    // __fastcall: callback in ECX, filter in EDX
    enumerateObjects((void*)enumerate_callback, 0);


    logging("Enumerated %d objects, %d units, %d players", g_object_count, g_context.unit_count, g_context.player_count);
}


