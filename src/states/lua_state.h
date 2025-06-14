#ifndef LUA_STATE_H
#define LUA_STATE_H
#include "../cheat/bot.h"

typedef struct LuaState {
    void (*update)(struct BotState* self);
    char command[128];
} LuaState;

void lua_state_update(BotState* self);
void push_lua_state(const char* command);

#endif //LUA_STATE_H
