#ifndef UTILS_H
#define UTILS_H

// --- FUNCTION PROTOTYPES (Tambahkan ini agar tidak error implicit declaration) ---
void run_io_benchmark(void);
void run_fibonacci(void);
void fib_task_wrapper(void* arg);

int read_int_from_user(void);
void read_str_from_user(char* buffer, int max_len);
void pause_system(void);

#endif
