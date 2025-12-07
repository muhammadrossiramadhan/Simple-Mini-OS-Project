#ifndef SCHEDULER_H
#define SCHEDULER_H

#define MAX_PROCESSES 10

// 1. Definisikan Level Prioritas
typedef enum {
    PRIORITY_LOW    = 0,
    PRIORITY_NORMAL = 1,
    PRIORITY_HIGH   = 2
} TaskPriority;

typedef void (*TaskFunction)(void* data);

typedef enum {
    TASK_READY,
    TASK_RUNNING,
    TASK_FINISHED
} TaskState;

typedef struct {
    int id;
    TaskState state;
    TaskFunction entry;
    void* data;
    TaskPriority priority; // 2. Tambahkan kolom prioritas
} Task;

void init_scheduler();

// 3. Update create_task untuk menerima parameter priority
int create_task(TaskFunction func, void* data, TaskPriority prio);

void scheduler_run();
int get_running_task_id();

#endif