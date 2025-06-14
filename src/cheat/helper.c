#include "helper.h"

void consoleLogger(const char* msg) {
    char buffer[128];
    snprintf(buffer, sizeof(buffer), msg);
    logToConsole(msg);
}

void fileLogger(const char* msg) {
    char fileBuffer[128];
    snprintf(fileBuffer, sizeof(fileBuffer), msg);
    logging(msg);
}

float distance_to(Position a, Position b) {
    float dx = a.x - b.x;
    float dy = a.y - b.y;
    float dz = a.z - b.z;
    return (float)sqrt(dx * dx + dy * dy + dz * dz);
}
