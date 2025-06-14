#include "combat_state.h"
#include "idle_state.h"

extern HANDLE g_context_mutex;

static unitObject local_units[MAX_OBJECTS];
static int local_unit_count;

//static wowPlayer player[MAX_OBJECTS]; -- not needed right now

static int local_player_found = 0;
static void* local_player_ptr = NULL;
static Position local_player_pos = {0,0,0};

void deep_copy_objects(unitObject* dest, const unitObject* src) {
    dest->base = src->base;
    dest->health = src->health;
    dest->pos = src->pos;
    dest->targetGuid = src->targetGuid;

    if (src->name) {
        dest->name = malloc(strlen(src->name) + 1);
        strcpy(dest->name, src->name);
    } else {
        dest->name = NULL;
    }
}

int combat_finished() {
    unsigned long long current_target_guid = TargetGuid();
    if (current_target_guid == 0) {
        // No target means combat is finished
        return 1;
    }


    // Check if the target is dead
    for (int i = 0; i < local_unit_count; ++i) {
        if (local_units[i].base.guid == current_target_guid) {
            if (local_units[i].health <= 0) {
                logging("CombatState: Current target GUID=%llu is dead.", current_target_guid);

                // Attempt to find and target the next closest unit
                unsigned long long closest_guid = 0;
                Position closest_pos = {0,0,0};
                int closest_health = 0;

                if (find_closest_unit(&closest_guid, &closest_pos, &closest_health)) {
                    if (closest_health > 0) {
                        setTarget(closest_guid);
                        logging("CombatState: Combat finished. New target set to GUID=%llu  at position (%.2f, %.2f, %.2f) with health %d",
                        closest_guid, closest_pos.x, closest_pos.y, closest_pos.z, closest_health);
                        return 0;
                    }
                }

                // No valid units, combat is done
                logging("CombatState: No valid units found. Combat is finished.");
                return 1; // Target is dead, combat is finished
            }
        }
    }

    return 0; // Combat is not finished
}

int find_closest_unit(unsigned long long* out_guid, Position* out_position, int* health) {
    float min_dist = FLT_MAX;
    unsigned long long closest_guid = 0;
    Position closest_pos = {0, 0, 0};
    int closest_health = 0;

    for (int i = 0; i < local_unit_count; ++i) {
        unitObject* unit = &local_units[i];

        // Check if the unit matches the criteria (e.g., name)
        if (unit->name && strcmp(unit->name, "Ragged Young Wolf") == 0 && unit->health > 0) {
            float dist = distance_to(local_player_pos, unit->pos);
            if (dist < min_dist) {
                min_dist = dist;
                closest_guid = unit->base.guid;
                closest_pos.x = unit->pos.x;
                closest_pos.y = unit->pos.y;
                closest_pos.z = unit->pos.z;
                closest_health = unit->health;
            }
        }
        free(unit->name);
    }

    if (closest_guid != 0) {
        *out_guid = closest_guid;
        *out_position = closest_pos;
        *health = closest_health;
        return 1; // Found a valid unit
    }

    return 0; // No valid units found
}

void auto_target_unit() {
    unsigned long long current_target_guid = TargetGuid();
    if (current_target_guid != 0) {
        logging("auto_target_unit: Player already has a target: GUID=%llu", current_target_guid);
        return; // No need to find a new target
    }

    unsigned long long closest_guid = 0;
    Position closest_pos = {0, 0, 0};
    int closest_health = 0;

    if (find_closest_unit(&closest_guid, &closest_pos, &closest_health)) {
        if (closest_health > 0) {
            setTarget(closest_guid);
            logging("auto_target_unit: Target set to GUID=%llu at position (%.2f, %.2f, %.2f) with health %d",
                    closest_guid, closest_pos.x, closest_pos.y, closest_pos.z, closest_health);
        } else {
            logging("auto_target_unit: Target GUID=%llu is dead with %d health.", closest_guid, closest_health);
        }
    } else {
        logging("auto_target_unit: No eligible units found.");
    }
}

void combat_update(BotState* bot) {
    WaitForSingleObject(g_context_mutex, INFINITE);

    local_player_ptr = g_context.local_player_ptr;
    local_player_pos = g_context.local_player_pos;
    local_player_found = g_context.local_player_found;

    local_unit_count = g_context.unit_count;
    for (int i = 0; i < local_unit_count; i++) {
        deep_copy_objects(&local_units[i], &g_context.units[i]);
    }

    ReleaseMutex(g_context_mutex);

    // check for local player
    if (!local_player_found) {
        logging("CombatState: Local player not found.");
        g_bot.inCombat = 0;
        return;
    }

    if (combat_finished()) {
        g_bot.inCombat = 0;
        stack_pop(&g_bot.stateStack);
        stack_push(&g_bot.stateStack, &idleState);
        logging("CombatState: Combat finished. Transitioning to IdleState");
    }

    auto_target_unit();

    // Check for player target
    unsigned long long current_target_guid = TargetGuid();
    Position target_pos = { 0,0,0 };

    for (int i = 0; i < local_unit_count; ++i) {
        if (local_units[i].base.guid == current_target_guid) {
            target_pos.x = local_units[i].pos.x;
            target_pos.y = local_units[i].pos.y;
            target_pos.z = local_units[i].pos.z;
            break;
        }
    }

    /*
    if (current_target_guid == 0) {
        logging("CombatState: No eligible target found after auto-targeting.");
            g_bot.inCombat = 0;

            return;
        }*/

    float dist = distance_to(local_player_pos, target_pos);
    if (dist > 3.0f) {
        clickToMove(local_player_ptr, Move, target_pos);
        g_bot.inCombat = 1;
        logging("CombatState: Moving to target at position (%.2f, %.2f, %.2f)",
            target_pos.x, target_pos.y, target_pos.z);
    } else {
        // Stop moving and interact
        clickToMoveStop(local_player_ptr, None, local_player_pos);
        g_bot.inCombat = 1;
        logging("CombatState: Stopped moving. Interacting with target");

        // lua call to attack the target
        LuaCallFastcall_t luaExec = (LuaCallFastcall_t)LUA_CALL_FUN_PTR;
        luaExec("AttackTarget()", "Unused");
    }
}

BotState combatState = { combat_update };