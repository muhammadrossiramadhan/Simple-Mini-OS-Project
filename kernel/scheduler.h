#ifndef SCHEDULER_H
#define SCHEDULER_H

#define MAX_PROCESSES 10

// PERBAIKAN 1: Tambah parameter (void* data)
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
    void* data; // PERBAIKAN 2: Tempat simpan data
} Task;

void init_scheduler();

// PERBAIKAN 3: create_task menerima argument data
int create_task(TaskFunction func, void* data);

void scheduler_run();
int get_running_task_id();

#endif