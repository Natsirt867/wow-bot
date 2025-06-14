#include "common.h"

// Global dynamic g_context array ( probably a bad thing to do )
GameContext g_context = {
    .descriptor_ptr = 0,
    .local_player_pos = {0, 0, 0},
    .local_player_ptr = NULL,
    .local_player_found = 0,
    .player_count = 0,
    .unit_count = 0,
};


// convertor for objectType to string
const char* typeToString(objectType const type) {
    switch (type) {
    case NONE:            return "NONE";
    case ITEM:            return "ITEM";
    case CONTAINER:       return "CONTAINER";
    case UNIT:            return "UNIT";
    case PLAYER:          return "PLAYER";
    case GAME_OBJECT:     return "CONTAINER";
    case DYNAMIC_OBJECT:  return "UNIT";
    case CORPSE:          return "CORPSE";
    default:              return "UNKNOWN";
    }
}