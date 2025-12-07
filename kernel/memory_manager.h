#ifndef MEMORY_MANAGER_H
#define MEMORY_MANAGER_H

#include <stdint.h>

// Definisi Tipe Data
typedef uint32_t size_t;
#define NULL ((void*)0)

// UBAH NAMA agar tidak bentrok dengan driver.h
#define MEM_SIZE        (1024 * 1024)  // 1MB
#define MEM_BLOCK_SIZE  256            // Ganti nama dari BLOCK_SIZE
#define MAX_MEM_BLOCKS  (MEM_SIZE / MEM_BLOCK_SIZE)

// --- DEFINISI STRUCT (HARUS DI ATAS FUNCTION PROTOTYPES) ---

// Memory block status
typedef enum {
    BLOCK_FREE = 0,
    BLOCK_ALLOCATED = 1,
    BLOCK_SYSTEM = 2
} block_status_t;

// Memory block descriptor
typedef struct {
    block_status_t status;
    size_t size;         
    void* physical_addr; 
    uint32_t pid;        
} memory_block_t;

// Memory layout segments
typedef enum {
    SEGMENT_KERNEL = 0,
    SEGMENT_HEAP,
    SEGMENT_USER,
    SEGMENT_STACK,
    SEGMENT_COUNT
} memory_segment_t;

// Memory segment information
typedef struct {
    void* start_addr;
    void* end_addr;
    size_t total_size;
    size_t used_size;
    const char* name;
} segment_info_t;

// Memory manager statistics
typedef struct {
    size_t total_memory;
    size_t used_memory;
    size_t free_memory;
    size_t allocated_blocks;
    size_t free_blocks;
    size_t fragmentation;
} memory_stats_t;

// --- FUNCTION PROTOTYPES ---

void memory_init(void);
void* kmalloc(size_t size);
void* kcalloc(size_t num, size_t size);
void* krealloc(void* ptr, size_t size);
void kfree(void* ptr);

void memory_get_stats(memory_stats_t* stats);

// Utilities (Tambahkan ini agar kernel.c bisa pakai)
void* k_memset(void* ptr, int value, size_t num);
void* k_memcpy(void* dest, const void* src, size_t n);

#endif // MEMORY_MANAGER_H