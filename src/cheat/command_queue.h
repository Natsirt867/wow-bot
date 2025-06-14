#ifndef COMMAND_QUEUE_H
#define COMMAND_QUEUE_H

#define MAX_COMMANDS 16
#define MAX_COMMAND_LEN 128

void command_queue_init(void);
void enqueue_command(const char* cmd);
int dequeue_command(char* outCmd);

#endif // COMMAND_QUEUE_H