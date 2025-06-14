#include "idle_state.h"
#include "../cheat/helper.h"
#include "../common/common.h"
#include "player_data.h"

void new_target() {
    if (TargetGuid == 0) {
        unsigned long long get_new_target = TargetGuid();

    }
}


void idle_update(BotState* self) {
    logging("Idle state updating...");
    consoleLogger("Idle state updating...");
}

BotState idleState = { idle_update };
