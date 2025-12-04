#ifndef MEMORY_MANAGER_H
#define MEMORY_MANAGER_H

#include <stddef.h>
#include <stdint.h>

#define MEMORY_SIZE 1024 * 1024  // 1MB total memory
#define BLOCK_SIZE 256           // 256 bytes per block
#define MAX_BLOCKS (MEMORY_SIZE / BLOCK_SIZE)

// Memory block status
typedef enum {
    BLOCK_FREE = 0,
    BLOCK_ALLOCATED = 1,
    BLOCK_SYSTEM = 2
} block_status_t;

// Memory block descriptor
typedef struct {
    block_status_t status;
    size_t size;        // Size in bytes
    void* physical_addr; // Physical address
    uint32_t pid;       // Process ID (0 for system/kernel)
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

// Function prototypes
void memory_init(void);
void* kmalloc(size_t size);
void* kcalloc(size_t num, size_t size);
void kfree(void* ptr);
void* krealloc(void* ptr, size_t size);

void memory_get_stats(memory_stats_t* stats);
void memory_print_layout(void);
void memory_print_blocks(void);
size_t memory_get_available(void);

// Helper functions
int memory_validate_ptr(void* ptr);
void memory_defragment(void);

#endif // MEMORY_MANAGER_H