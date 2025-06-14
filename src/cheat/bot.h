#ifndef BOT_H
#define BOT_H

#define MAX_STATES 16



typedef struct BotState {
    void (*update)(struct BotState* self);

} BotState;

typedef struct {
    BotState* states[MAX_STATES];
    int top; // index of top element
} BotStateStack;

typedef struct {
    BotStateStack stateStack;
    int running; // 1 on 0 off
    int inCombat; // 1 in combat, 0 for idle
} Bot;

extern Bot g_bot;

void stack_init(BotStateStack* stack);
int stack_push(BotStateStack* stack, BotState* state);
BotState* stack_pop(BotStateStack* stacK);
BotState* stack_peek(BotStateStack* stack);

void bot_init(Bot* bot);
void bot_start(Bot* bot);
void bot_stop(Bot* bot);
void bot_push_state(/**/);
void push_lua_state();
void bot_pop_state(void);
void setup_bot();

#endif //BOT_H
