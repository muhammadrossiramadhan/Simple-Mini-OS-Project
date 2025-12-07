#include "syscall.h"
#include "reboot.h"
#include "scheduler.h"
#include "utils.h"
#include <stdint.h>

// --- FEATURE 1: I/O BENCHMARK ---
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
    sys_print("Write throughput     : "); sys_print_dec(tp_w); sys_print(" kB/s (approx)\n");
    sys_print("Read throughput      : "); sys_print_dec(tp_r); sys_print(" kB/s (approx)\n");

    DeviceStatus st = sys_get_status();
    sys_print("\n=== STAT ===\n");
    sys_print("total_writes : "); sys_print_dec(st.total_writes); sys_print("\n");
    sys_print("total_reads  : "); sys_print_dec(st.total_reads); sys_print("\n");
    sys_print("io_ticks     : "); sys_print_dec(st.total_io_time); sys_print("\n");

    sys_print("\nBenchmark selesai.\n");
    sys_print("Press any key to return to menu...");
    sys_read();
}

// --- HELPER FUNC (FIBONACCI) ---
long long fib(int n) {
    if (n <= 1) return n;
    return fib(n-1) + fib(n-2);
}

// Fungsi Wrapper (Tetap sama seperti sebelumnya)
void fib_task_wrapper(void* arg) {
    int n = (int)arg; 
    sys_print("   [Task] Calculating Fib("); 
    sys_print_dec(n); sys_print(")...\n");
    
    uint64_t t_start = sys_cpu_time_us();
    long long res = fib(n);
    uint64_t t_end = sys_cpu_time_us();

    sys_print("   [Done] Result: "); sys_print_u64((uint64_t)res);
    sys_print(" | Time: "); sys_print_u64(t_end - t_start); 
    sys_print(" us\n");
}

// --- FEATURE 4: MULTI-INPUT FIBONACCI ---
void run_fibonacci(void) {
    sys_clear_screen();
    sys_print("=== Multi-Task Fibonacci Scheduler ===\n");
    
    // 1. Reset Scheduler (Bersihkan antrean lama)
    init_scheduler();

    // 2. Tanya user mau berapa tugas?
    sys_print("Berapa angka yang ingin dihitung? (Max 5): ");
    int count = read_int_from_user();

    // Validasi input
    if (count <= 0) {
        sys_print("\nBatal. Tidak ada tugas yang dibuat.\n");
        sys_read();
        return;
    }
    if (count > 5) {
        count = 5;
        sys_print("\nKebanyakan. Dibatasi max 5 tugas.\n");
    } else {
        sys_print("\n");
    }

    // 3. Loop meminta input angka untuk setiap tugas
    for (int i = 0; i < count; i++) {
        sys_print("Masukkan angka ke-"); 
        sys_print_dec(i + 1); 
        sys_print(": ");
        
        int n = read_int_from_user();
        
        // Validasi angka fibonacci agar tidak terlalu berat/nol
        if (n <= 0) n = 10; // Default jika user asal enter
        
        // Masukkan ke Scheduler
        // Kita kirim 'n' sebagai void* data
        int id = create_task(fib_task_wrapper, (void*)n);
        
        if (id == -1) {
            sys_print("   (Gagal: Antrean Penuh!)\n");
        }
    }

    sys_print("\nSemua tugas sudah masuk antrean.\n");
    sys_print("Tekan Enter untuk menjalankan Scheduler...");
    sys_read();

    // 4. Jalankan Scheduler
    sys_print("\n\n=== STARTING SCHEDULER ===\n");
    scheduler_run();

    sys_print("\nSemua perhitungan selesai.\n");
    sys_print("Press any key to return to menu...");
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
        sys_print("4. Fibonacci Scheduler\n"); // Menu Updated
        sys_print("5. Restart/Reboot\n");
        sys_print("6. Quit\n");
        sys_print("\nSelect command [1-6]: ");

        char choice = sys_read();

        switch (choice) {
            case '1':
                run_io_benchmark(); // Pastikan fungsi ini ada
                break;
            case '2':
                sys_print("\n(Not Implemented)\n"); sys_read();
                break;
            case '3':
                sys_print("\n(Not Implemented)\n"); sys_read();
                break;
            case '4':
                run_fibonacci(); // Panggil fungsi baru
                break;
            case '5':
                sys_print("\nRebooting...\n");
                reboot_system();
                break;
            case '6':
                sys_print("\nQuitting...\n");
                qemu_shutdown();
                break;
            default:
                break;
        }
    }
}