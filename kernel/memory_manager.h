#ifndef MEMORYMANAGER_H
#define MEMORYMANAGER_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#define MEM_SIZE        (1024 * 1024)
#define MAX_HISTORY     32

typedef uint32_t size_t;

typedef struct {
    size_t total_memory;
    size_t used_memory;
    size_t free_memory;
    size_t allocated_blocks;
} memorystats_t;

// History log API
void history_log(const char* label, uint32_t mem_used);
void history_show(void);
void history_clear(void);
uint32_t history_available(void);

// Memory manager API
void memory_init(void);
void* kmalloc(size_t size);
void* kcalloc(size_t num, size_t size);
void* krealloc(void* ptr, size_t size);
void kfree(void* ptr);
void memory_get_stats(memorystats_t* stats);

#endif
