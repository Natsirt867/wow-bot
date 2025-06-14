#include "pipe_server.h"
#include "../states/idle_state.h"
#include "../states/cry_state.h"
#include "../states/yell_state.h"
#include "../states/pop_state.h"
#include "../common/common.h"
#include "../common/logging.h"
#include "object_enumerator.h"
#include "bot.h"
#include "../cheat/command_queue.h"



void handle_command(const char* cmd) {
    if (strcmp(cmd, "START") == 0) {
        setup_bot();
        bot_start(&g_bot);
    } else if (strcmp(cmd, "STOP") == 0) {
        bot_stop(&g_bot);
    } else if (strcmp(cmd, CLICK_TO_MOVE_CMD) == 0) {
        click_to_move_func();
    } else if (strcmp(cmd, "PUSHYELL") == 0) {
        stack_push(&g_bot.stateStack, &yellState);
    } else if (strcmp(cmd, "PUSHCRY") == 0) {
        stack_push(&g_bot.stateStack, &cryState);
    } else if (strcmp(cmd, "POPSTATE") == 0) {
        stack_pop(&g_bot.stateStack);
    } else if (strncmp(cmd, "LUA:", 4) == 0) {
        enqueue_command(cmd);;
    } else if (strcmp(cmd, "SetTarget") == 0) {
        set_target_func();
    } else if (strcmp(cmd, "ClickToMoveStop") == 0) {
        click_to_move_stop_func();
    }
    // TODO:
    // MORE COMMANDS
}

int pipe_init() {
    char buffer[128];
    char buffer2[128];

    logging("SERVER: Initialization process started...\n");
    snprintf(buffer, sizeof(buffer), "SERVER: Initializing server pipe...\n");
    logToConsole(buffer);

    DWORD bytesRead;


    logging("SERVER: Waiting for client message...\n");
    snprintf(buffer, sizeof(buffer), "SERVER: Waiting for client message...\n");
    logToConsole(buffer);
    while (1) {
        HANDLE hPipe = CreateNamedPipeA(
            "\\\\.\\pipe\\NatsirtBotPipe",
            PIPE_ACCESS_INBOUND,
            PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT,
            1, 128, 128, 0, NULL);

        if (hPipe == INVALID_HANDLE_VALUE) {
            logging("SERVER: Failed to create pipe. Error: %lu\n", GetLastError());
            return 1;
        }

        BOOL connected = ConnectNamedPipe(hPipe, NULL) ? TRUE : ERROR_PIPE_CONNECTED;

        if (connected) {
            if (ReadFile(hPipe, buffer2, sizeof(buffer2) - 1, &bytesRead, NULL)) {
                buffer2[bytesRead] = '\0';
                handle_command(buffer2);
                logging("CLIENT: %s\n", buffer2);
                snprintf(buffer, sizeof(buffer), ("CLIENT: %s\n", buffer2));
                logToConsole(buffer);

            } else {

                logging("SERVER: Readfile failed\n", GetLastError());
                snprintf(buffer, sizeof(buffer), ("SERVER: Readfile failed\n", GetLastError()));
                logToConsole(buffer);
            }
        }
        CloseHandle(hPipe);
    }
    return 0;
}