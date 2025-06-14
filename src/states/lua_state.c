#include "lua_state.h"
#include "../cheat/helper.h"
#include "../common/common.h"
#include "player_data.h"


void lua_state_update(BotState* self) {
    LuaState* luaState = (LuaState*)self;
    logging("LuaState updating: running command: %s", luaState->command);
    consoleLogger(luaState->command);

}