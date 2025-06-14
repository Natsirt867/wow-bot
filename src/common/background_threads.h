#ifndef BACKGROUND_THREADS_H
#define BACKGROUND_THREADS_H

#include <windows.h>

// function to start background enumerator thread
void start_background_enumerator();

// function to start mutex
void initialize_mutex();

// extern (global) mutex function, protects data from being accessed when not ideal
extern HANDLE g_context_mutex;

#endif //BACKGROUND_THREADS_H
