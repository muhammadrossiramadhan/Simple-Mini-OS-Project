#include <stdio.h>
#include <string.h>
#include "memory_manager.h"

void demo_memory_manager(void) {
    printf("=== MiniOS Memory Manager Demo ===\n\n");
    
    // Initialize memory manager
    memory_init();
    
    // Print initial layout
    memory_print_layout();
    
    // Allocate some memory
    printf("=== Allocation Tests ===\n");
    int* numbers = (int*)kmalloc(10 * sizeof(int));
    char* text = (char*)kmalloc(100);
    char* zeros = (char*)kcalloc(50, sizeof(char));
    
    if (numbers) {
        for (int i = 0; i < 10; i++) {
            numbers[i] = i * i;
        }
        printf("Numbers array filled with squares\n");
    }
    
    if (text) {
        strcpy(text, "Hello, MiniOS Memory Manager!");
        printf("Text: %s\n", text);
    }
    
    if (zeros) {
        printf("Zero-initialized array created\n");
    }
    
    // Print memory status
    memory_print_blocks();
    
    // Get statistics
    memory_stats_t stats;
    memory_get_stats(&stats);
    printf("\n=== Memory Statistics ===\n");
    printf("Total Memory: %zu bytes\n", stats.total_memory);
    printf("Used Memory: %zu bytes\n", stats.used_memory);
    printf("Free Memory: %zu bytes\n", stats.free_memory);
    printf("Available: %zu bytes\n", memory_get_available());
    
    // Test realloc
    printf("\n=== Reallocation Test ===\n");
    char* dynamic = (char*)kmalloc(20);
    if (dynamic) {
        strcpy(dynamic, "Small buffer");
        printf("Before realloc: %s\n", dynamic);
        
        dynamic = (char*)krealloc(dynamic, 50);
        if (dynamic) {
            strcat(dynamic, " - now larger!");
            printf("After realloc: %s\n", dynamic);
        }
        kfree(dynamic);
    }
    
    // Free memory
    printf("\n=== Freeing Memory ===\n");
    kfree(numbers);
    kfree(text);
    kfree(zeros);
    
    // Final status
    memory_print_blocks();
    
    printf("\nDemo completed!\n");
}

int main() {
    demo_memory_manager();
    return 0;
}