#include "memory_manager.h"
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

// Global memory arrays
static uint8_t physical_memory[MEMORY_SIZE];
static memory_block_t block_table[MAX_BLOCKS];
static segment_info_t segments[SEGMENT_COUNT];

// Current heap pointer
static void* heap_current = NULL;

// Initialize memory manager
void memory_init(void) {
    printf("Initializing Memory Manager...\n");
    
    // Initialize physical memory to zero
    memset(physical_memory, 0, MEMORY_SIZE);
    
    // Initialize block table
    for (int i = 0; i < MAX_BLOCKS; i++) {
        block_table[i].status = BLOCK_FREE;
        block_table[i].size = 0;
        block_table[i].physical_addr = &physical_memory[i * BLOCK_SIZE];
        block_table[i].pid = 0;
    }
    
    // Define memory segments
    // Kernel segment (first 128KB)
    segments[SEGMENT_KERNEL].start_addr = &physical_memory[0];
    segments[SEGMENT_KERNEL].end_addr = &physical_memory[128 * 1024 - 1];
    segments[SEGMENT_KERNEL].total_size = 128 * 1024;
    segments[SEGMENT_KERNEL].used_size = 0;
    segments[SEGMENT_KERNEL].name = "Kernel";
    
    // Heap segment (next 512KB)
    segments[SEGMENT_HEAP].start_addr = &physical_memory[128 * 1024];
    segments[SEGMENT_HEAP].end_addr = &physical_memory[640 * 1024 - 1];
    segments[SEGMENT_HEAP].total_size = 512 * 1024;
    segments[SEGMENT_HEAP].used_size = 0;
    segments[SEGMENT_HEAP].name = "Heap";
    
    // User segment (next 256KB)
    segments[SEGMENT_USER].start_addr = &physical_memory[640 * 1024];
    segments[SEGMENT_USER].end_addr = &physical_memory[896 * 1024 - 1];
    segments[SEGMENT_USER].total_size = 256 * 1024;
    segments[SEGMENT_USER].used_size = 0;
    segments[SEGMENT_USER].name = "User";
    
    // Stack segment (last 128KB)
    segments[SEGMENT_STACK].start_addr = &physical_memory[896 * 1024];
    segments[SEGMENT_STACK].end_addr = &physical_memory[MEMORY_SIZE - 1];
    segments[SEGMENT_STACK].total_size = 128 * 1024;
    segments[SEGMENT_STACK].used_size = 0;
    segments[SEGMENT_STACK].name = "Stack";
    
    // Set initial heap pointer
    heap_current = segments[SEGMENT_HEAP].start_addr;
    
    // Mark kernel blocks as system
    size_t kernel_blocks = segments[SEGMENT_KERNEL].total_size / BLOCK_SIZE;
    for (size_t i = 0; i < kernel_blocks; i++) {
        block_table[i].status = BLOCK_SYSTEM;
        block_table[i].pid = 0; // Kernel PID
    }
    
    printf("Memory Manager initialized with %d KB total memory\n", MEMORY_SIZE / 1024);
}

// Allocate memory
void* kmalloc(size_t size) {
    if (size == 0) return NULL;
    
    // Calculate required blocks
    size_t required_blocks = (size + sizeof(size_t) + BLOCK_SIZE - 1) / BLOCK_SIZE;
    
    // Find contiguous free blocks
    int start_block = -1;
    int consecutive_free = 0;
    
    for (int i = 0; i < MAX_BLOCKS; i++) {
        if (block_table[i].status == BLOCK_FREE) {
            if (start_block == -1) {
                start_block = i;
            }
            consecutive_free++;
            
            if (consecutive_free >= required_blocks) {
                break;
            }
        } else {
            start_block = -1;
            consecutive_free = 0;
        }
    }
    
    if (consecutive_free < required_blocks) {
        printf("Memory allocation failed: Not enough contiguous blocks\n");
        return NULL;
    }
    
    // Mark blocks as allocated
    for (size_t i = 0; i < required_blocks; i++) {
        block_table[start_block + i].status = BLOCK_ALLOCATED;
        block_table[start_block + i].size = size;
        block_table[start_block + i].pid = 1; // Default PID
    }
    
    void* allocated_addr = block_table[start_block].physical_addr;
    
    // Store size at beginning of allocation
    size_t* size_ptr = (size_t*)allocated_addr;
    *size_ptr = size;
    
    // Update segment usage
    segments[SEGMENT_HEAP].used_size += required_blocks * BLOCK_SIZE;
    
    printf("Allocated %zu bytes at %p (blocks: %d-%d)\n", 
           size, allocated_addr, start_block, start_block + required_blocks - 1);
    
    return (void*)((uint8_t*)allocated_addr + sizeof(size_t));
}

// Allocate and zero memory
void* kcalloc(size_t num, size_t size) {
    size_t total_size = num * size;
    void* ptr = kmalloc(total_size);
    
    if (ptr != NULL) {
        memset(ptr, 0, total_size);
    }
    
    return ptr;
}

// Free allocated memory
void kfree(void* ptr) {
    if (ptr == NULL) return;
    
    if (!memory_validate_ptr(ptr)) {
        printf("Invalid pointer for free: %p\n", ptr);
        return;
    }
    
    // Get the actual start of allocation (where size is stored)
    void* actual_start = (uint8_t*)ptr - sizeof(size_t);
    size_t block_index = ((uint8_t*)actual_start - physical_memory) / BLOCK_SIZE;
    
    // Get size from allocation header
    size_t* size_ptr = (size_t*)actual_start;
    size_t size = *size_ptr;
    size_t allocated_blocks = (size + sizeof(size_t) + BLOCK_SIZE - 1) / BLOCK_SIZE;
    
    // Mark blocks as free
    for (size_t i = 0; i < allocated_blocks; i++) {
        if (block_table[block_index + i].status == BLOCK_ALLOCATED) {
            block_table[block_index + i].status = BLOCK_FREE;
            block_table[block_index + i].size = 0;
            block_table[block_index + i].pid = 0;
        }
    }
    
    // Update segment usage
    segments[SEGMENT_HEAP].used_size -= allocated_blocks * BLOCK_SIZE;
    
    printf("Freed %zu bytes at %p (blocks: %zu-%zu)\n", 
           size, ptr, block_index, block_index + allocated_blocks - 1);
}

// Reallocate memory
void* krealloc(void* ptr, size_t size) {
    if (ptr == NULL) return kmalloc(size);
    if (size == 0) {
        kfree(ptr);
        return NULL;
    }
    
    if (!memory_validate_ptr(ptr)) {
        printf("Invalid pointer for realloc: %p\n", ptr);
        return NULL;
    }
    
    // Get current size
    void* actual_start = (uint8_t*)ptr - sizeof(size_t);
    size_t* size_ptr = (size_t*)actual_start;
    size_t old_size = *size_ptr;
    
    if (old_size >= size) {
        // Shrink allocation - for simplicity, we don't actually shrink
        // Just update the size and return same pointer
        *size_ptr = size;
        return ptr;
    }
    
    // Need more space - allocate new, copy, free old
    void* new_ptr = kmalloc(size);
    if (new_ptr != NULL) {
        memcpy(new_ptr, ptr, old_size);
        kfree(ptr);
    }
    
    return new_ptr;
}

// Get memory statistics
void memory_get_stats(memory_stats_t* stats) {
    if (stats == NULL) return;
    
    stats->total_memory = MEMORY_SIZE;
    stats->used_memory = 0;
    stats->free_memory = 0;
    stats->allocated_blocks = 0;
    stats->free_blocks = 0;
    
    for (int i = 0; i < MAX_BLOCKS; i++) {
        if (block_table[i].status == BLOCK_ALLOCATED) {
            stats->used_memory += BLOCK_SIZE;
            stats->allocated_blocks++;
        } else if (block_table[i].status == BLOCK_FREE) {
            stats->free_memory += BLOCK_SIZE;
            stats->free_blocks++;
        }
    }
    
    // Calculate fragmentation (simplified)
    int free_blocks_count = 0;
    int free_blocks_groups = 0;
    bool in_free_group = false;
    
    for (int i = 0; i < MAX_BLOCKS; i++) {
        if (block_table[i].status == BLOCK_FREE) {
            free_blocks_count++;
            if (!in_free_group) {
                free_blocks_groups++;
                in_free_group = true;
            }
        } else {
            in_free_group = false;
        }
    }
    
    if (free_blocks_count > 0) {
        stats->fragmentation = (free_blocks_groups * 100) / free_blocks_count;
    } else {
        stats->fragmentation = 0;
    }
}

// Print memory layout
void memory_print_layout(void) {
    printf("\n=== Memory Layout ===\n");
    printf("Total Memory: %d KB\n", MEMORY_SIZE / 1024);
    printf("Block Size: %d bytes\n", BLOCK_SIZE);
    printf("Total Blocks: %d\n\n", MAX_BLOCKS);
    
    for (int i = 0; i < SEGMENT_COUNT; i++) {
        printf("Segment: %s\n", segments[i].name);
        printf("  Address: %p - %p\n", segments[i].start_addr, segments[i].end_addr);
        printf("  Size: %zu bytes (%zu KB)\n", 
               segments[i].total_size, segments[i].total_size / 1024);
        printf("  Used: %zu bytes (%zu%%)\n", 
               segments[i].used_size, (segments[i].used_size * 100) / segments[i].total_size);
        printf("\n");
    }
}

// Print block information
void memory_print_blocks(void) {
    printf("\n=== Memory Blocks ===\n");
    
    memory_stats_t stats;
    memory_get_stats(&stats);
    
    printf("Allocated Blocks: %zu\n", stats.allocated_blocks);
    printf("Free Blocks: %zu\n", stats.free_blocks);
    printf("System Blocks: %zu\n", MAX_BLOCKS - stats.allocated_blocks - stats.free_blocks);
    printf("Fragmentation: %zu%%\n", stats.fragmentation);
    
    printf("\nDetailed Block Map:\n");
    for (int i = 0; i < MAX_BLOCKS; i++) {
        if (i % 32 == 0) {
            printf("\n%04d: ", i);
        }
        
        switch (block_table[i].status) {
            case BLOCK_FREE:
                printf(".");
                break;
            case BLOCK_ALLOCATED:
                printf("A");
                break;
            case BLOCK_SYSTEM:
                printf("S");
                break;
        }
    }
    printf("\n\nLegend: . = Free, A = Allocated, S = System\n");
}

// Get available memory
size_t memory_get_available(void) {
    memory_stats_t stats;
    memory_get_stats(&stats);
    return stats.free_memory;
}

// Validate pointer
int memory_validate_ptr(void* ptr) {
    if (ptr == NULL) return 0;
    
    // Check if pointer is within heap segment
    if (ptr < segments[SEGMENT_HEAP].start_addr || ptr > segments[SEGMENT_HEAP].end_addr) {
        return 0;
    }
    
    // Check if pointer points to a valid allocation
    void* actual_start = (uint8_t*)ptr - sizeof(size_t);
    size_t block_index = ((uint8_t*)actual_start - physical_memory) / BLOCK_SIZE;
    
    if (block_index >= MAX_BLOCKS) {
        return 0;
    }
    
    return block_table[block_index].status == BLOCK_ALLOCATED;
}

// Simple defragmentation
void memory_defragment(void) {
    printf("Defragmenting memory...\n");
    // This is a simplified defragmentation
    // In a real OS, this would be much more complex
    printf("Defragmentation completed (simulated)\n");
}