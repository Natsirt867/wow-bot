#include "cry_state.h"

#include "../cheat/helper.h"



void cry_update(BotState* self) {
    logging("Cry :( state updating...");
    consoleLogger("Cry :( state updating...");
}

BotState cryState = { cry_update };