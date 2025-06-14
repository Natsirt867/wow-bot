#include <stdio.h>
#include <stdarg.h>
#include <direct.h>
#include <time.h>
#include "../common/logging.h"
#include "../common/common.h"

static FILE* log_file_handle = NULL;

// Set up a way to gracefully close console while keeping game open? or no need as the console wouldn't be run unless cheating
BOOL WINAPI ConsoleHandler(DWORD ctrlType) {
    switch (ctrlType) {
        case CTRL_C_EVENT:
        case CTRL_BREAK_EVENT:
        case CTRL_CLOSE_EVENT:
        case CTRL_LOGOFF_EVENT:
        case CTRL_SHUTDOWN_EVENT:
            return TRUE;
        default:
            return FALSE;
    }
}

void consoleSetup(const char* msg) {
    AllocConsole();
    FILE* fp = NULL;
    if (freopen_s(&fp, "CONOUT$", "w", stdout) != 0) {
        logging("CONSOLE: Failed to redirect console output\n");
        return;
    }
    SetConsoleCtrlHandler(ConsoleHandler, TRUE);
    printf(".: ~~ Welcome to Natsirt's WoWBot ~~ :.\n\n");
    logging("CONSOLE: .: ~~ Welcome to Natsirt's WoWBot ~~ :.");
}

void logToConsole(const char* msg) {
    FILE* fp = freopen("CONOUT$", "w", stdout);
    if (fp) {
        printf("%s\n", msg);
        fflush(stdout);
    }
}

void create_logs_dir(const char *path) {
    if (_mkdir(path) == 0) {
        MessageBox(NULL, TEXT("Logs directory created: %s\n", path), TEXT("Success!"), 0);
    } else if (errno != EEXIST) {
        MessageBox(NULL, TEXT("Failure creating Logs directory."), TEXT("Error."), 0);
    }
}

void get_unique_log_file_path(char *buffer, size_t buffer_size) {
    time_t now = time(NULL);
    struct tm *local_time = localtime(&now);

    snprintf(buffer, buffer_size, "%s\\log_%04d-%02d-%02d_%02d-%02d-02d.txt",
        BASE_LOG_DIR,
        local_time->tm_year + 1900,
        local_time->tm_mon + 1,
        local_time->tm_mday,
        local_time->tm_hour,
        local_time->tm_min,
        local_time->tm_sec);

}

void initializeLogging() {
    create_logs_dir(BASE_LOG_DIR);

    char log_file_path[512];
    get_unique_log_file_path(log_file_path, sizeof(log_file_path));

    if (fopen_s(&log_file_handle, log_file_path, "w") != 0) {
        MessageBox(NULL, TEXT("Failed to open log file for writing!"), TEXT("Error."), 0);
        return;
    }
}

void shutdownLogging() {
    if (log_file_handle != NULL) {
        if (fclose(log_file_handle) != 0) {
            logging("DLL: fclose() error");
        }
        log_file_handle = NULL;
    }
}



void logging(const char* format, ...) {
    // Check if the file was opened
    if (log_file_handle == NULL) {
        return;
    }

    va_list args;
    va_start(args, format);

    // Write the formatted message to the file
    vfprintf(log_file_handle, format, args);

    va_end(args);

    fprintf(log_file_handle, "\n");

    fflush(log_file_handle);
}

void logMemory(void* ptr, size_t size) {
    uint8_t* p = (uint8_t*)ptr; // Cast the pointer to a byte array
    char buffer[256];

    for (size_t i = 0; i < size; i++) {
        snprintf(buffer, sizeof(buffer), ("Byte %zu: 0x%02X", i, p[i]));
        logToConsole(buffer);
        logging(buffer);

    }
}