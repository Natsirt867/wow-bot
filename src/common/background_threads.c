#include "background_threads.h"

#include "player_data.h"
#include "../cheat/helper.h"
#include "../cheat/object_enumerator.h"

// global mutex
HANDLE g_context_mutex;

// Background thread function
DWORD WINAPI BackgroundEnumerateThread(LPVOID lpParam) {
    while (1) {
        enumerate_visible_objects_and_log(); // refresh the object list
        Sleep(1000); // wait 50 miliseconds
    }
    return 0;
}

void start_background_enumerator() {
    logging("Creating background thread...");
    HANDLE hThread = CreateThread(
        NULL,
        0,
        BackgroundEnumerateThread,
        NULL,
        0,
        NULL);

    if (hThread) {
        CloseHandle(hThread);
        logging("Background enumerator thread started.");
    } else {
        logging("Failed to create background enumerator thread.");
    }
}

void initialize_mutex() {
    g_context_mutex = CreateMutex(NULL, FALSE, NULL);
    if (!g_context_mutex) {
        logging("Failed to create mutex.");
    }

}
