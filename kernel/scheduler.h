#ifndef SCHEDULER_H
#define SCHEDULER_H

#include "memory_manager.h" // Agar bisa pakai size_t/defs jika perlu

#define MAX_PROCESSES 10

typedef void (*TaskFunction)(void* data);

// ... Enum TaskPriority dan TaskState TETAP SAMA ...
typedef enum {
    PRIORITY_LOW    = 0,
    PRIORITY_NORMAL = 1,
    PRIORITY_HIGH   = 2
} TaskPriority;

typedef enum {
    TASK_READY,
    TASK_RUNNING,
    TASK_FINISHED
} TaskState;

typedef struct {
    int id;
    char name[16]; // <--- KOLOM BARU: Nama Task (Max 15 chars)
    TaskState state;
    TaskFunction entry;
    void* data;
    TaskPriority priority;
} Task;

void init_scheduler();

// UBAH: Tambah parameter 'const char* name'
int create_task(TaskFunction func, void* data, TaskPriority prio, const char* name);

void scheduler_run();
int get_running_task_id();

#endif