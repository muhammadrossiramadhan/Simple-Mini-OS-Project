#include "memory_manager.h"
#include "syscall.h"
#include <stdint.h>
#include <stdbool.h>

#define MEM_BLOCK_SIZE 256

static uint8_t memory_pool[MEM_SIZE];
static uint32_t total_allocated_memory = 0;

typedef struct {
    char label[32];
    uint32_t mem_used;
} history_entry_t;

static history_entry_t g_history[MAX_HISTORY];
static uint32_t g_history_count = 0;

void history_log(const char* label, uint32_t mem_used) {
    if (g_history_count >= MAX_HISTORY) return;
    history_entry_t* e = &g_history[g_history_count];
    for (int i = 0; i < 31; i++) {
        if (label[i] == 0) {
            e->label[i] = 0;
            break;
        }
        e->label[i] = label[i];
    }
    e->label[31] = 0;
    e->mem_used = mem_used;
    g_history_count++;
}

void history_show(void) {
    if (g_history_count == 0) {
        sys_print("(History kosong)\n");
        return;
    }
    for (uint32_t i = 0; i < g_history_count; i++) {
        sys_print_dec(i + 1);
        sys_print(". ");
        sys_print(g_history[i].label);
        sys_print(" | ");
        sys_print_dec(g_history[i].mem_used / 1024);
        sys_print(" KB\n");
    }
}

void history_clear(void) {
    g_history_count = 0;
}

uint32_t history_available(void) {
    return MAX_HISTORY - g_history_count;
}

void memory_init(void) {
    total_allocated_memory = 0;
}

void* kmalloc(size_t size) {
    if (total_allocated_memory + size > 900 * 1024) {
        return NULL;
    }
    void* ptr = &memory_pool[total_allocated_memory];
    total_allocated_memory += size;
    return ptr;
}

void memory_get_stats(memorystats_t* stats) {
    stats->total_memory = MEM_SIZE;
    stats->used_memory = total_allocated_memory;
    stats->free_memory = MEM_SIZE - total_allocated_memory;
    stats->allocated_blocks = total_allocated_memory / MEM_BLOCK_SIZE;
}
