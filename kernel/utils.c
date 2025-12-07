#include "utils.h"
#include "syscall.h"
#include <stdint.h>

// --- HELPER FUNCTIONS ---

// Membaca input angka dari user
int read_int_from_user(void) {
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

// Fungsi membaca string dari user
void read_str_from_user(char* buffer, int max_len) {
    int idx = 0;
    char c;
    while (1) {
        c = sys_read();
        
        // Handle Backspace
        if (c == '\b') { 
            if (idx > 0) { idx--; sys_print("\b \b"); }
            continue; 
        }
        // Handle Enter
        if (c == '\n') break;
        
        // Simpan karakter jika buffer muat
        if (idx < max_len - 1) {
            buffer[idx++] = c;
            char tmp[2] = {c, 0};
            sys_print(tmp); // echo ke layar
        }
    }
    buffer[idx] = 0; // Null terminator
}

// Menunggu user menekan tombol
void pause_system(void) {
    sys_read();
}
