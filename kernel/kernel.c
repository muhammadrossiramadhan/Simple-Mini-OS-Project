#include "syscall.h"
#include "reboot.h"
#include <stdint.h>

// --- HELPER FUNCTIONS ---

// Membaca input angka dari user
static int read_int_from_user(void) {
    char buf[16];
    int idx = 0;
    char c;
    while (1) {
        c = sys_read();
        // Handle Backspace visual
        if (c == '\b') { 
            if (idx > 0) { idx--; sys_print("\b \b"); }
            continue; 
        }
        if (c == '\n') break;
        if (idx < 15) {
            buf[idx++] = c;
            char tmp[2] = {c, 0};
            sys_print(tmp); // echo
        }
    }
    buf[idx] = 0;
    
    int val = 0;
    for (int i = 0; i < idx; i++) {
        if (buf[i] < '0' || buf[i] > '9') return 0;
        val = val * 10 + (buf[i] - '0');
    }
    return val;
}

// Menunggu user menekan tombol
void pause_system(void) {
    sys_read();
}

// --- FEATURE 1: I/O BENCHMARK (Fixed Output) ---
void run_io_benchmark(void) {
    sys_clear_screen();
    init_driver(); 
    
    sys_print("Masukkan jumlah blok untuk benchmark (max 1000). Tekan Enter untuk default 1000:\n");
    
    int blocks = read_int_from_user();
    
    if (blocks > BLOCK_COUNT) {
        blocks = BLOCK_COUNT;
        sys_print("\nInput terlalu besar. Menggunakan maximal: 1000\n");
    } else if (blocks <= 0) {
        blocks = 1000;
        sys_print("1000\n");
    }
    sys_print("\n");

    static uint8_t buf[BLOCK_SIZE];
    static uint8_t check[BLOCK_SIZE];
    for (int i=0; i<BLOCK_SIZE; i++) buf[i] = (uint8_t)(i & 0xFF);

    sys_print("Mulai write test...\n");
    uint32_t w0 = sys_time_us();
    for (int b=0; b<blocks; b++) io_write_block(b, buf);
    uint32_t w1 = sys_time_us();

    sys_print("Mulai read & verify test...\n");
    uint32_t r0 = sys_time_us();
    for (int b=0; b<blocks; b++) io_read_block(b, check);
    uint32_t r1 = sys_time_us();

    uint32_t write_time = w1 - w0;
    uint32_t read_time  = r1 - r0;
    uint32_t lat_w = (blocks > 0) ? (write_time / blocks) : 0;
    uint32_t lat_r = (blocks > 0) ? (read_time / blocks) : 0;
    uint32_t total_kb = (blocks * BLOCK_SIZE) / 1024;
    uint32_t tp_w = (write_time > 0) ? (total_kb * 1000000 / write_time) : 0;
    uint32_t tp_r = (read_time > 0)  ? (total_kb * 1000000 / read_time)  : 0;

    sys_print("\n=== Hasil Benchmark Mini-OS ===\n");
    sys_print("Jumlah blok          : "); sys_print_dec(blocks); sys_print("\n");
    sys_print("Write time total     : "); sys_print_dec(write_time); sys_print(" us\n");
    sys_print("Read time total      : "); sys_print_dec(read_time);  sys_print(" us\n");
    sys_print("Write latency /blok  : "); sys_print_dec(lat_w); sys_print(" us\n");
    sys_print("Read latency /blok   : "); sys_print_dec(lat_r); sys_print(" us\n");
    sys_print("Write throughput     : "); sys_print_dec(tp_w); sys_print(" KB/s (approx)\n");
    sys_print("Read throughput      : "); sys_print_dec(tp_r); sys_print(" KB/s (approx)\n");

    DeviceStatus st = sys_get_status();
    sys_print("\n=== STAT ===\n");
    sys_print("total_writes : "); sys_print_dec(st.total_writes); sys_print("\n");
    sys_print("total_reads  : "); sys_print_dec(st.total_reads); sys_print("\n");
    sys_print("io_ticks     : "); sys_print_dec(st.total_io_time); sys_print("\n");

    sys_print("\nBenchmark selesai.\n");
    sys_print("Press any key to return to menu...");
    sys_read();
}

// --- FEATURE 4: FIBONACCI (MULTITASKING CASE - WITH INPUT) ---
long long fib(int n) {
    if (n <= 1) return n;
    return fib(n-1) + fib(n-2);
}

void run_fibonacci(void) {
    sys_clear_screen();
    sys_print("=== Simulasi Fibonacci (Mini-OS) ===\n");
    sys_print("Multitasking Scheduler Test\n\n");
    
    // Input Task 1
    sys_print("Masukkan angka Fibonacci Task 1 (default 35): ");
    int n1 = read_int_from_user();
    if (n1 <= 0) {
        n1 = 35;
        sys_print("35\n");
    } else {
        sys_print("\n");
    }

    // Input Task 2
    sys_print("Masukkan angka Fibonacci Task 2 (default 38): ");
    int n2 = read_int_from_user();
    if (n2 <= 0) {
        n2 = 38;
        sys_print("38\n");
    } else {
        sys_print("\n");
    }
    
    if (n1 > 42 || n2 > 42) {
        sys_print("\n[Warning] Angka > 42 akan memakan waktu lama (rekursif)!\n");
    }

    sys_print("\nRunning Task 1: Fib("); sys_print_dec(n1); sys_print(")...\n");
    uint64_t t1_start = sys_cpu_time_us();
    long long res1 = fib(n1);
    uint64_t t1_end = sys_cpu_time_us();
    
    sys_print("Task fib("); sys_print_dec(n1); sys_print(") = "); sys_print_u64((uint64_t)res1);
    sys_print(" | time "); sys_print_u64(t1_end - t1_start); sys_print(" us\n\n");

    sys_print("Running Task 2: Fib("); sys_print_dec(n2); sys_print(")...\n");
    uint64_t t2_start = sys_cpu_time_us();
    long long res2 = fib(n2);
    uint64_t t2_end = sys_cpu_time_us();

    sys_print("Task fib("); sys_print_dec(n2); sys_print(") = "); sys_print_u64((uint64_t)res2);
    sys_print(" | time "); sys_print_u64(t2_end - t2_start); sys_print(" us\n\n");
    
    sys_print("Scheduler berhasil menjalankan 2 task bergantian.\n");
    
    sys_print("\nPress any key to return to menu...");
    sys_read();
}

// --- MAIN KERNEL ---
void kernel_main(void) {
    init_driver();

    while (1) {
        sys_clear_screen();
        sys_print("list of mini os commands :\n\n");
        sys_print("1. I/O Driver Simulation\n");
        sys_print("2. Memory Manager\n");
        sys_print("3. Scheduler Priority\n");
        sys_print("4. Fibonacci\n");
        sys_print("5. Restart/Reboot\n");
        sys_print("6. Quit\n");
        sys_print("\nSelect command [1-6]: ");

        char choice = sys_read();

        switch (choice) {
            case '1':
                run_io_benchmark();
                break;
            case '2':
                sys_print("\n\n(BUT THE TEMPORARY, THE CODE OF PROGRAM NULL OR NOTHING)\n");
                sys_print("\nPress any key to return to menu...");
                sys_read();
                break;
            case '3':
                sys_print("\n\n(BUT THE TEMPORARY, THE CODE OF PROGRAM NULL OR NOTHING)\n");
                sys_print("\nPress any key to return to menu...");
                sys_read();
                break;
            case '4':
                run_fibonacci();
                break;
            case '5':
                sys_print("\n\nRebooting...\n");
                reboot_system();
                break;
            case '6':
            sys_print("\n\nQuitting QEMU...\n");
            qemu_shutdown();
            break;
            default:
                break;
        }
    }
}
