#include "driver.h"
#include "keyboard.h"
#include "syscall.h"
#include "reboot.h"
#include "scheduler.h"
#include "memory_manager.h"
extern uint32_t total_allocated_memory;  // ← TAMBAH INI
#include "utils.h"
#include <stdint.h>
#include <stdbool.h>
#define MAX_HISTORY 32

bool g_memory_ready = false;
bool g_memory_full = false;
uint32_t g_last_mem_kb = 0;

// Fungsi utilitas untuk cek prasyarat memori
void check_memory_prereq() {
    if (!g_memory_ready) {
        sys_clear_screen();
        sys_print("======================================\n");
        sys_print("       ERROR: MEMORY NOT INITIALIZED  \n");
        sys_print("  Silakan jalankan Menu 2 dulu!       \n");
        sys_print("======================================\n");
        sys_print("\n[Press any key...]\n");
        sys_read();
        return;
    }
    if (g_memory_full) {
        sys_clear_screen();
        sys_print("======================================\n");
        sys_print("          MEMORY FULL - STOP          \n");
        sys_print("Hanya boleh 1x per cycle!            \n");
        sys_print("Hapus History Log dulu via Menu 5     \n");
        sys_print("======================================\n");
        sys_print("\n[Press any key...]\n");
        sys_read();
        return;
    }
}

// Mini dashboard untuk hasil operasi
void show_operation_complete(const char* op_name) {
    sys_clear_screen();
    sys_print("======================================\n");
    sys_print("          OPERATION COMPLETE          \n");
    sys_print("======================================\n");
    sys_print("Operation   : ");
    sys_print(op_name);
    sys_print("\nMemory Used : ");
    sys_print_dec(g_last_mem_kb);
    sys_print(" KB\n");
    sys_print("History     : ");
    sys_print_dec(MAX_HISTORY - history_available());
    sys_print("/");
    sys_print_dec(MAX_HISTORY);
    sys_print("\nStatus     : MEMORY FULL (1x only)\n");
    sys_print("======================================\n");
    sys_print("[Press any key to History Log]\n");
    sys_read();
}

// --- FEATURE 1: I/O BENCHMARK ---
void run_io_benchmark(void) {
    if (!g_memory_ready) {
        sys_clear_screen();
        sys_print("======================================\n");
        sys_print("ERROR: Memory belum di-set\n");
        sys_print("Jalankan Menu 2 dulu.\n");
        sys_print("======================================\n");
        sys_print("\n[Press any key...]\n");
        sys_read();
        return;
    }
    if (g_memory_full) {
        sys_clear_screen();
        sys_print("======================================\n");
        sys_print("MEMORY FULL! Hapus history dulu.\n");
        sys_print("======================================\n");
        sys_print("\n[Press any key...]\n");
        sys_read();
        return;
    }
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
    history_log("I/O Benchmark", total_kb * 1024);
    if (history_available() <= 4) {
    sys_print("\n[ALERT] History hampir penuh. Clear di HISTORY LOG.\n");
    }

    g_memory_full = true;
    sys_print("MEMORY FULL - 1x only per cycle!\n");
    
    sys_read();



}

// Revisi run_memory_manager dengan dashboard rapi
void run_memory_manager() {
    sys_clear_screen();
    sys_print("======================================\n");
    sys_print("          MEMORY MANAGER SETUP        \n");
    sys_print("======================================\n\n");
    sys_print("Input memory size (KB, max 900): ");

    uint32_t kbsize = read_int_from_user();
    if (kbsize == 0) kbsize = 1;
    if (kbsize > 900) kbsize = 900;

    void* memblock = kmalloc(kbsize * 1024);
    if (memblock) {
        g_memory_ready = true;
        g_memory_full = false;
        g_last_mem_kb = kbsize;

        memorystats_t st;
        memory_get_stats(&st);

        sys_clear_screen();
        sys_print("======================================\n");
        sys_print("        MEMORY ALLOCATION SUCCESS     \n");
        sys_print("======================================\n");
        sys_print("Requested    : ");
        sys_print_dec(kbsize);
        sys_print(" KB (");
        sys_print_dec(kbsize * 1024);
        sys_print(" B)\n");
        sys_print("Total Used   : ");
        sys_print_dec(st.used_memory / 1024);
        sys_print(" KB\n");
        sys_print("Free Space   : ");
        sys_print_dec(st.free_memory / 1024);
        sys_print(" KB\n");
        sys_print("Status      : READY FOR OPERATIONS\n");
        sys_print("======================================\n");
        sys_print("[Press any key to continue...]\n");
        sys_read();
    } else {
        sys_clear_screen();
        sys_print("======================================\n");
        sys_print("        MEMORY ALLOCATION FAILED      \n");
        sys_print("======================================\n");
        sys_print("Possible memory limit or history full.\n");
        sys_print("[Press any key to continue...]\n");
        sys_read();
    }
}




// --- FEATURE 3: PRIORITY SCHEDULER DEMO ---

void task_priority_interactive(void* arg) {
    int id = (int)arg;
    sys_print("      (Executing Task ID #"); sys_print_dec(id); sys_print(")\n");
    for(int i=0; i<300000; i++) { __asm__("nop"); } // Delay
}

void run_priority_test(void) {
    if (!g_memory_ready) {
        sys_clear_screen();
        sys_print("======================================\n");
        sys_print("ERROR: Memory belum di-set\n");
        sys_print("Jalankan Menu 2 dulu.\n");
        sys_print("======================================\n");
        sys_print("\n[Press any key...]\n");
        sys_read();
        return;
    }
    if (g_memory_full) {
        sys_clear_screen();
        sys_print("======================================\n");
        sys_print("MEMORY FULL! Hapus history dulu.\n");
        sys_print("======================================\n");
        sys_print("\n[Press any key...]\n");
        sys_read();
        return;
    }

    sys_clear_screen();
    sys_print("=== Interactive Priority Test ===\n");
    sys_print("0=LOW | 1=NORMAL | 2=HIGH\n\n");
    
    init_scheduler();

    sys_print("Berapa task? (Max 5): ");
    int count = read_int_from_user();
    if (count > 5) count = 5;
    sys_print("\n");

    for (int i = 0; i < count; i++) {
        char name_buf[16];
        int priority_val;
        TaskPriority prio;

        // 1. INPUT NAMA
        sys_print("Task #"); sys_print_dec(i+1); sys_print(" Name: ");
        read_str_from_user(name_buf, 16); // Fungsi baru di utils
        sys_print("\n");

        // 2. INPUT PRIORITY
        sys_print("   Priority [0/1/2]: ");
        priority_val = read_int_from_user();
        sys_print("\n");

        if (priority_val == 2) prio = PRIORITY_HIGH;
        else if (priority_val == 0) prio = PRIORITY_LOW;
        else prio = PRIORITY_NORMAL;

        // 3. CREATE TASK DENGAN NAMA
        create_task(task_priority_interactive, (void*)(i + 1), prio, name_buf);
    }

    sys_print("\nTekan Enter untuk jalankan Scheduler...\n");
    sys_read();

    sys_print("\n=== SCHEDULER START ===\n");
    scheduler_run();

    sys_print("\nSelesai. Press key to return.");
    history_log("Priority Scheduler", 0);
    if (history_available() <= 4) {
    sys_print("\n[ALERT] History hampir penuh. Clear di HISTORY LOG.\n");
    }

    g_memory_full = true;
    sys_print("MEMORY FULL - 1x only per cycle!\n");

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
    if (!g_memory_ready) {
        sys_clear_screen();
        sys_print("======================================\n");
        sys_print("ERROR: Memory belum di-set\n");
        sys_print("Jalankan Menu 2 dulu.\n");
        sys_print("======================================\n");
        sys_print("\n[Press any key...]\n");
        sys_read();
        return;
    }
    if (g_memory_full) {
        sys_clear_screen();
        sys_print("======================================\n");
        sys_print("MEMORY FULL! Hapus history dulu.\n");
        sys_print("======================================\n");
        sys_print("\n[Press any key...]\n");
        sys_read();
        return;
    }

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
        int id = create_task(fib_task_wrapper, (void*)n, PRIORITY_NORMAL, "FibCalc");
        
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
    history_log("Fibonacci Test", 0);
    if (history_available() <= 4) sys_print("\n[ALERT] History hampir penuh. Clear di HISTORY LOG.\n");

    g_memory_full = true;
    sys_print("MEMORY FULL - 1x only per cycle!\n");

    sys_read();

}

// History dashboard dan penghapusan yang terintegrasi
void run_history_dashboard() {
    sys_clear_screen();
    sys_print("======================================\n");
    sys_print("         HISTORY LOG DASHBOARD        \n");
    sys_print("======================================\n\n");

    history_show();

    sys_print("\n--------------------------------------\n");
    sys_print("Slots Used: ");
    sys_print_dec(MAX_HISTORY - history_available());
    sys_print("/");
    sys_print_dec(MAX_HISTORY);
    sys_print("\nClear All History? [Y/N]: ");

    char c = sys_read();
    if (c == 'Y' || c == 'y') {
        history_clear();
        g_memory_full = false;
        g_memory_ready = false;
        memory_init();  // ← GANTI, panggil fungsi init yang sudah ada
        sys_print("\n✓ History cleared! Memory unlocked.\n");
        sys_print("[Press any key to continue...]\n");
        sys_read();
    }
}


// Daftar Menu
const char* menuItems[] = {
    "1. I/O Benchmark",
    "2. Memory Manager", 
    "3. Priority Scheduler",
    "4. Fibonacci Test",
    "5. History Log",
    "6. Reboot",
    "7. Shutdown"
};
#define MENU_COUNT 7


void draw_logo() {
    // Koordinat X=20, Y=2. Warna: Light Green (0x0A)
    // Logo teks "MINI OS"
    uint8_t color = vga_entry_color(VGA_COLOR_GREEN, VGA_COLOR_BLACK);
    
    driver_write_at("  __  __ _       _    ___  ____  ", 22, 2, color);
    driver_write_at(" |  \\/  (_)_ __ (_)  / _ \\/ ___| ", 22, 3, color);
    driver_write_at(" | |\\/| | | '_ \\| | | | | \\___ \\ ", 22, 4, color);
    driver_write_at(" | |  | | | | | | | | |_| |___) |", 22, 5, color);
    driver_write_at(" |_|  |_|_|_| |_|_|  \\___/|____/ ", 22, 6, color);
}

void draw_menu(int selected_index) {
    // Judul
    driver_write_at("=== MINI OS MAIN MENU ===", 28, 5, vga_entry_color(VGA_COLOR_GREEN, VGA_COLOR_BLACK));

    for (int i = 0; i < MENU_COUNT; i++) {
        uint8_t color;
        int x_pos = 25;
        int y_pos = 8 + i; // Mulai di baris 8

        if (i == selected_index) {
            // HIGHLIGHT: Text Hitam, Background Putih/Abu
            color = vga_entry_color(VGA_COLOR_BLACK, VGA_COLOR_LIGHT_GREY);
            // Tambahkan pointer visual
            driver_write_at("-> ", x_pos - 3, y_pos, vga_entry_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK)); 
        } else {
            // NORMAL: Text Putih, Background Hitam
            color = vga_entry_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK);
            driver_write_at("   ", x_pos - 3, y_pos, color); // Hapus pointer lama
        }
        
        driver_write_at(menuItems[i], x_pos, y_pos, color);

    }
    
    // Instruksi di bawah
    driver_write_at("Gunakan UP/DOWN untuk memilih, ENTER untuk ok", 18, 22, vga_entry_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK));
}

void kernel_main(void) {
    init_driver();
    
    int selected = 0;
    int running = 1;

    while (running) {
        // 1. Gambar Menu
        sys_clear_screen(); // Atau optimasi hanya clear jika perlu
        draw_logo();
        draw_menu(selected);

        // 2. Loop Input (Tunggu sampai user tekan tombol navigasi)
        char key = 0;
        while (key != (char)KEY_ENTER) {
            key = sys_read(); // Panggil keyboard_getchar yang sudah diupdate
            
            if (key == (char)KEY_UP) {
                selected--;
                if (selected < 0) selected = MENU_COUNT - 1; // Wrap ke bawah
                draw_menu(selected); // Redraw hanya menu agar tidak flickering
            }
            else if (key == (char)KEY_DOWN) {
                selected++;
                if (selected >= MENU_COUNT) selected = 0; // Wrap ke atas
                draw_menu(selected);
            }
        }

        // 3. Eksekusi Menu (User menekan ENTER)
        sys_clear_screen(); // Bersihkan layar sebelum masuk fitur
        
        switch(selected) {
            case 0: run_io_benchmark(); break;
            case 1: run_memory_manager(); break;
            case 2: run_priority_test(); break;
            case 3: run_fibonacci(); break;
            case 4: run_history_dashboard(); break;
            case 5: reboot_system(); break;
            case 6: qemu_shutdown(); break;
        }



        // Setelah fitur selesai, loop akan kembali ke atas (redraw menu)
    }
}
