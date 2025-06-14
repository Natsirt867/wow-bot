#include "bot.h"
#include "../common/common.h"
#include "helper.h"
#include "../states/idle_state.h"
#include "../states/lua_state.h"
#include "../states/combat_state.h"
#include "../common/background_threads.h"

Bot g_bot;

void stack_init(BotStateStack* stack) {
    stack->top = -1;
}

int stack_push(BotStateStack* stack, BotState* state) {
    if (stack->top < MAX_STATES - 1) {
        stack->states[++stack->top] = state;
        return 1;
    }
    return 0;
}

BotState* stack_pop(BotStateStack* stack) {
    if (stack->top >= 0) {
        return stack->states[stack->top--];
    }
    return NULL;
}

BotState* stack_peek(BotStateStack* stack) {
    if (stack->top >= 0) {
        return stack->states[stack->top];
    }
    return NULL;
}

void bot_init(Bot* bot) {
    stack_init(&bot->stateStack);
    bot->running = 0;
}


void setup_bot() {
    bot_init(&g_bot);
    stack_push(&g_bot.stateStack, &idleState); // <-- Push the state onto the stack
    initialize_mutex(); // Initialize the mutex
    start_background_enumerator(); // Start the background thread
    stack_push(&g_bot.stateStack, &combatState);
    bot_start(&g_bot);
}

DWORD WINAPI StateThread(LPVOID lpParam) {
    Bot* bot = (Bot*)lpParam;
    logging("BOT: StateThread Started!\n");
    consoleLogger("BOT: StateThread Started!\n");

    while (bot->running) {
        if (bot->stateStack.top == -1) {
            logging("BOT: Bot currently has no state.\n");
            consoleLogger("BOT: Bot currently has no state.\n");
            Sleep(1000);
        } else {
            BotState* state = stack_peek(&bot->stateStack);
            if (state && state->update) {
                state->update(state);
                logging("BOT: Updating State.\n");
                consoleLogger("BOT: Updating State.\n");
            }
            Sleep(1000);
        }
    }
    logging("BOT: StateThread exiting.\n");
    consoleLogger("BOT: StateThread exiting.\n");
    return 0;
}

void bot_start(Bot* bot) {
    if (bot->running) return;
    logging("DLL: ---- STARTING BOT ----\n");
    consoleLogger("---- STARTING BOT ----\n");
    bot->running = 1;
    CreateThread(NULL, 0, StateThread, bot, 0, NULL);
}

void bot_stop(Bot* bot) {
    logging("DLL: ---- STOPPING BOT ----\n");
    consoleLogger("---- STOPPING BOT ----\n");
    bot->running = 0;
    while (bot->stateStack.top != -1)
        stack_pop(&bot->stateStack);
}

void push_lua_state(const char* command) {
    // Allocate a new LuaState on the heap
    LuaState* state = (LuaState*)malloc(sizeof(LuaState));
    state->update = lua_state_update;
    strncpy(state->command, command, sizeof(state->command) - 1);
    state->command[sizeof(state->command) - 1] = '\0';

    stack_push(&g_bot.stateStack, (BotState*)state);
}