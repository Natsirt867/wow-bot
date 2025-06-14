#include "command_queue.h"
#include "../common/common.h"

static char queue[MAX_COMMANDS][MAX_COMMAND_LEN];
static int head = 0, tail = 0;
static CRITICAL_SECTION queue_cs;
static int initialized = 0;

void command_queue_init(void) {
    if (!initialized) {
        InitializeCriticalSection(&queue_cs);
        initialized = 1;
    }
}

void enqueue_command(const char* cmd) {
    EnterCriticalSection(&queue_cs);
    int next_tail = (tail + 1) % MAX_COMMANDS;
    if (next_tail != head) { // Not full
        strncpy(queue[tail], cmd, MAX_COMMAND_LEN - 1);
        queue[tail][MAX_COMMAND_LEN - 1] = '\0';
        tail = next_tail;
    }
    // TODO
    // Drop command, setup logging?
    LeaveCriticalSection(&queue_cs);
}

int dequeue_command(char* outCmd) {
    int has_command = 0;
    EnterCriticalSection(&queue_cs);
    if (head != tail) { // Not empty
        strncpy(outCmd, queue[head], MAX_COMMAND_LEN - 1);
        outCmd[MAX_COMMAND_LEN - 1] = '\0';
        head = (head + 1) % MAX_COMMANDS;
        has_command = 1;
    }
    LeaveCriticalSection(&queue_cs);
    return has_command;
}