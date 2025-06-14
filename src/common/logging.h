#ifndef LOGGING_H
#define LOGGING_H

#define BASE_LOG_DIR "F:\\C Projects\\WoWBotD3D9\\logs"

void initializeLogging();
void shutdownLogging();
void logging(const char* format, ...);
void logToConsole(const char* msg);
void consoleSetup();
void logMemory(void* ptr, size_t size);

#endif //LOGGING_H
