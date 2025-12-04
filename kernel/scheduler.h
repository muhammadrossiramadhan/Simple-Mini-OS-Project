#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <stdint.h>

#define MAX_PROCESSES 16

typedef enum {
    TASK_READY,
    TASK_RUNNING,
    TASK_FINISHED
} TaskState;

typedef void (*TaskFunction)();

typedef struct {
    int id;
    TaskState state;
    TaskFunction entry;
} Task;

void init_scheduler();
int create_task(TaskFunction func);
void scheduler_run();
void scheduler_tick();
int get_running_task_id();

#endif
