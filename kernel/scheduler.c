#include "scheduler.h"
#include "syscall.h" 
#include "memory_manager.h"
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
        tasks[i].data = 0;
        tasks[i].priority = PRIORITY_LOW;
    }
    task_count = 0;
    current = -1;
    sys_print("[SCHED] Initialized.\n");
}

// Update fungsi create_task menerima 'prio'
int create_task(TaskFunction func, void* data, TaskPriority prio, const char* name) {
    if (task_count >= MAX_PROCESSES) {
        sys_print("[SCHED] ERROR: Max task limit reached.\n");
        return -1;
    }

    tasks[task_count].id = task_count;
    tasks[task_count].state = TASK_READY;
    tasks[task_count].entry = func;
    tasks[task_count].data = data;
    tasks[task_count].priority = prio;
    
    // COPY NAMA (Manual loop atau k_memcpy)
    // Kita pakai manual loop sederhana untuk aman, atau k_memcpy jika sudah yakin
    int i = 0;
    while(name[i] != 0 && i < 15) {
        tasks[task_count].name[i] = name[i];
        i++;
    }
    tasks[task_count].name[i] = 0; // Null terminate

    sys_print("[SCHED] Created: \"");
    sys_print(tasks[task_count].name);
    sys_print("\"");
    
    if (prio == PRIORITY_HIGH) sys_print(" [HIGH]");
    else if (prio == PRIORITY_LOW) sys_print(" [LOW]");
    sys_print("\n");

    return task_count++;
}

// LOGIKA BARU: PRIORITY SCHEDULING
static int pick_next_task() {
    int best_candidate = -1;
    int highest_prio_found = -1; // -1 lebih kecil dari PRIORITY_LOW (0)

    // Cek semua task, cari yang READY dengan prioritas TERTINGGI
    for (int i = 0; i < task_count; i++) {
        if (tasks[i].state == TASK_READY) {
            // Jika ketemu task dengan prioritas lebih tinggi dari kandidat sebelumnya
            if ((int)tasks[i].priority > highest_prio_found) {
                highest_prio_found = tasks[i].priority;
                best_candidate = i;
            }
        }
    }

    return best_candidate;
}

void scheduler_tick() {
    int next = pick_next_task();
    if (next < 0) return;

    current = next;
    tasks[current].state = TASK_RUNNING;

    // TAMPILKAN NAMA TASK SAAT RUNNING
    sys_print("[RUN] \""); 
    sys_print(tasks[current].name);
    sys_print("\"");

    if (tasks[current].priority == PRIORITY_HIGH) sys_print(" [HIGH]...\n");
    else if (tasks[current].priority == PRIORITY_LOW) sys_print(" [LOW]...\n");
    else sys_print("...\n");

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