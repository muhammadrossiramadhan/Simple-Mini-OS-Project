#include "memory_manager.h"
#include "../kernel/syscall.h" 

// --- UTILS ---
void* k_memset(void* ptr, int value, size_t num) {
    uint8_t* p = (uint8_t*)ptr;
    for(size_t i=0; i<num; i++) p[i] = (uint8_t)value;
    return ptr;
}

void* k_memcpy(void* dest, const void* src, size_t n) {
    uint8_t* d = (uint8_t*)dest;
    const uint8_t* s = (const uint8_t*)src;
    for(size_t i=0; i<n; i++) d[i] = s[i];
    return dest;
}

// --- GLOBAL VARS ---
static uint8_t physical_memory[MEM_SIZE];
static memory_block_t block_table[MAX_MEM_BLOCKS];
static segment_info_t segments[SEGMENT_COUNT];

// --- IMPLEMENTASI ---

void memory_init(void) {
    sys_print("[MEM] Initializing Memory Manager...\n");
    
    k_memset(physical_memory, 0, MEM_SIZE);
    
    // Init block table
    for (int i = 0; i < MAX_MEM_BLOCKS; i++) {
        block_table[i].status = BLOCK_FREE;
        block_table[i].size = 0;
        block_table[i].physical_addr = &physical_memory[i * MEM_BLOCK_SIZE];
        block_table[i].pid = 0;
    }
    
    // Setup Segments
    // KERNEL (0 - 128KB)
    segments[SEGMENT_KERNEL].start_addr = &physical_memory[0];
    segments[SEGMENT_KERNEL].total_size = 128 * 1024;
    
    // HEAP (128KB - 640KB)
    segments[SEGMENT_HEAP].start_addr = &physical_memory[128 * 1024];
    segments[SEGMENT_HEAP].total_size = 512 * 1024;
    
    sys_print("[MEM] Ready. Total: 1MB\n");
}

void* kmalloc(size_t size) {
    if (size == 0) return NULL;
    
    // Hitung butuh berapa blok
    size_t required_blocks = (size + 4 + MEM_BLOCK_SIZE - 1) / MEM_BLOCK_SIZE;
    
    // Heap starts after Kernel
    int start_index = (128 * 1024) / MEM_BLOCK_SIZE; 
    int max_index = (640 * 1024) / MEM_BLOCK_SIZE; 

    int start_block = -1;
    int consecutive = 0;

    // First Fit Algorithm
    for (int i = start_index; i < max_index; i++) {
        if (block_table[i].status == BLOCK_FREE) {
            if (start_block == -1) start_block = i;
            consecutive++;
            if (consecutive >= required_blocks) break;
        } else {
            start_block = -1;
            consecutive = 0;
        }
    }

    if (consecutive < required_blocks) {
        sys_print("[MEM] Alloc Failed: Full/Fragmented\n");
        return NULL;
    }

    // Mark Allocated
    for (size_t i = 0; i < required_blocks; i++) {
        block_table[start_block + i].status = BLOCK_ALLOCATED;
        if (i == 0) {
            block_table[start_block + i].size = size; 
        } else {
            block_table[start_block + i].size = 0;
        }
    }

    void* ptr = block_table[start_block].physical_addr;
    
    sys_print("[MEM] Alloc "); sys_print_dec(size); 
    sys_print(" bytes at Block "); sys_print_dec(start_block);
    sys_print("\n");
    
    return ptr;
}

void kfree(void* ptr) {
    if (ptr == NULL) return;
    
    uint8_t* p_start = (uint8_t*)physical_memory;
    uint8_t* p_target = (uint8_t*)ptr;
    
    if (p_target < p_start || p_target >= (p_start + MEM_SIZE)) {
        sys_print("[MEM] Invalid Free Pointer\n");
        return;
    }

    int block_idx = (p_target - p_start) / MEM_BLOCK_SIZE;
    
    size_t size = block_table[block_idx].size;
    if (size == 0 && block_table[block_idx].status != BLOCK_ALLOCATED) {
         sys_print("[MEM] Double Free or Invalid\n");
         return;
    }

    size_t blocks_to_free = (size + 4 + MEM_BLOCK_SIZE - 1) / MEM_BLOCK_SIZE;

    for(size_t i=0; i<blocks_to_free; i++) {
        block_table[block_idx + i].status = BLOCK_FREE;
        block_table[block_idx + i].size = 0;
    }
    
    sys_print("[MEM] Freed Block "); sys_print_dec(block_idx); sys_print("\n");
}

void* kcalloc(size_t num, size_t size) {
    size_t total = num * size;
    void* ptr = kmalloc(total);
    if (ptr) k_memset(ptr, 0, total);
    return ptr;
}

void* krealloc(void* ptr, size_t size) {
    if (!ptr) return kmalloc(size);
    
    void* new_ptr = kmalloc(size);
    if (new_ptr) {
        k_memcpy(new_ptr, ptr, size); 
        kfree(ptr);
    }
    return new_ptr;
}

void memory_get_stats(memory_stats_t* stats) {
    if (stats == NULL) return;
    
    stats->total_memory = MEM_SIZE;
    stats->used_memory = 0;
    stats->free_memory = 0;
    stats->allocated_blocks = 0;
    
    for(int i=0; i<MAX_MEM_BLOCKS; i++) {
        if(block_table[i].status == BLOCK_ALLOCATED) {
            stats->allocated_blocks++;
            stats->used_memory += MEM_BLOCK_SIZE;
        } else {
            stats->free_memory += MEM_BLOCK_SIZE;
        }
    }
}