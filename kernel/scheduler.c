#include "scheduler.h"
#include "syscall.h" 
#include <stdint.h>

static Task tasks[MAX_PROCESSES];
static int task_count = 0;
static int current = -1;

static void print_dec(uint32_t val) {
    sys_print_dec(val);
}

void init_scheduler() {
    for (int i = 0; i < MAX_PROCESSES; i++) {
        tasks[i].id = -1;
        tasks[i].state = TASK_FINISHED;
        tasks[i].entry = 0;
        tasks[i].data = 0; // Reset data
    }
    task_count = 0;
    current = -1;
    sys_print("[SCHED] Initialized.\n");
}

// PERBAIKAN: Terima parameter data dan simpan ke struct
int create_task(TaskFunction func, void* data) {
    if (task_count >= MAX_PROCESSES) {
        sys_print("[SCHED] ERROR: Max task limit reached.\n");
        return -1;
    }

    tasks[task_count].id = task_count;
    tasks[task_count].state = TASK_READY;
    tasks[task_count].entry = func;
    tasks[task_count].data = data; // SIMPAN DATA DI SINI

    sys_print("[SCHED] Created task ");
    print_dec(task_count);
    sys_print(".\n");

    return task_count++;
}

static int pick_next_task() {
    if (task_count == 0) return -1;
    for (int i = 1; i <= task_count; i++) {
        int idx = (current + i) % task_count;
        if (tasks[idx].state == TASK_READY)
            return idx;
    }
    return -1;
}

void scheduler_tick() {
    int next = pick_next_task();
    if (next < 0) return;

    current = next;
    tasks[current].state = TASK_RUNNING;

    sys_print("[SCHED] Running task ");
    print_dec(current);
    sys_print("...\n");

    // PERBAIKAN: Jalankan fungsi sambil melempar datanya
    tasks[current].entry(tasks[current].data); 

    tasks[current].state = TASK_FINISHED;
}

void scheduler_run() {
    int unfinished = 1;
    while (unfinished) {
        unfinished = 0;
        for (int i = 0; i < task_count; i++) {
            if (tasks[i].state != TASK_FINISHED) {
                scheduler_tick();
                unfinished = 1;
            }
        }
    }
    sys_print("[SCHED] All tasks completed.\n");
}

int get_running_task_id() {
    return current;
}