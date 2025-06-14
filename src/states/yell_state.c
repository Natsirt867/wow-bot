#include "yell_state.h"
#include "../cheat/helper.h"

void yell_update(BotState* self) {
    logging("YELL!!!!! state updating...");
    consoleLogger("YELL!!!!!!!! state updating...");
}

BotState yellState = { yell_update };