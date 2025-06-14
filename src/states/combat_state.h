#pragma once
#include "../cheat/bot.h"
#include "../common/common.h"
#include "../cheat/player_data.h"
#include "../cheat/helper.h"

#ifndef COMBAT_STATE_H
#define COMBAT_STATE_H

extern BotState combatState;

void combat_update(BotState* self);
int combat_finished();
int find_closest_unit();

#endif //COMBAT_STATE_H
