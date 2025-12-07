#include <stdint.h>
#include "driver.h"

/* --- FIX: 64-BIT MATH HELPERS FOR 32-BIT SYSTEM --- */
// GCC membutuhkan fungsi ini untuk pembagian/modulus uint64_t di mode 32-bit tanpa stdlib
// Wajib ada untuk kalkulasi waktu Fibonacci
uint64_t __udivdi3(uint64_t n, uint64_t d) {
    uint64_t q = 0, r = 0;
    for (int i = 63; i >= 0; i--) {
        r <<= 1;
        if ((n >> i) & 1) r |= 1;
        if (r >= d) {
            r -= d;
            q |= (1ULL << i);
        }
    }
    return q;
}

uint64_t __umoddi3(uint64_t n, uint64_t d) {
    uint64_t r = 0;
    for (int i = 63; i >= 0; i--) {
        r <<= 1;
        if ((n >> i) & 1) r |= 1;
        if (r >= d) {
            r -= d;
        }
    }
    return r;
}
/* -------------------------------------------------- */

/* VGA TEXT MODE */
static uint16_t *const VGA_BUFFER = (uint16_t *)0xB8000;
static int cursor_row = 0;
static int cursor_col = 0;
static const uint8_t VGA_ATTR = 0x0F; // White text on Black background

static void scroll() {
    for (int row = 1; row < 25; row++) {
        for (int col = 0; col < 80; col++) {
            VGA_BUFFER[(row - 1) * 80 + col] = VGA_BUFFER[row * 80 + col];
        }
    }
    for (int col = 0; col < 80; col++) {
        VGA_BUFFER[(24 * 80) + col] = ((uint16_t)VGA_ATTR << 8) | ' ';
    }
}

static void putchar_at(char c, int row, int col) {
    VGA_BUFFER[row * 80 + col] = ((uint16_t)VGA_ATTR << 8) | (uint8_t)c;
}

static void vga_putc(char c) {
    if (c == '\n') {
        cursor_row++;
        cursor_col = 0;
    } else if (c == '\r') {
        cursor_col = 0;
    } else if (c == '\b') {
        if (cursor_col > 0) cursor_col--;
        putchar_at(' ', cursor_row, cursor_col);
    } else {
        putchar_at(c, cursor_row, cursor_col);
        cursor_col++;
        if (cursor_col >= 80) {
            cursor_col = 0;
            cursor_row++;
        }
    }
    if (cursor_row >= 25) {
        scroll();
        cursor_row = 24;
    }
}

void driver_clear_screen(void) {
    for (int i = 0; i < 80*25; i++) {
        VGA_BUFFER[i] = ((uint16_t)VGA_ATTR << 8) | (uint8_t)' ';
    }
    cursor_row = 0;
    cursor_col = 0;
}

void driver_write(const char *s) {
    while (*s) vga_putc(*s++);
}

void driver_write_at(const char *str, int col, int row, uint8_t color) {
    int offset = (row * 80) + col;
    while (*str) {
        VGA_BUFFER[offset] = (uint16_t)color << 8 | (uint8_t)*str;
        str++;
        offset++;
    }
}

void driver_write_dec(uint32_t v) {
    if (v == 0) { driver_write("0"); return; }
    char buf[16];
    int i = 0;
    while (v > 0) {
        buf[i++] = '0' + (v % 10);
        v /= 10;
    }
    while (i > 0) vga_putc(buf[--i]);
}

void driver_write_u64(uint64_t v) {
    if (v == 0) { driver_write("0"); return; }
    char buf[32];
    int i = 0;
    while (v > 0) {
        buf[i++] = '0' + (v % 10);
        v /= 10;
    }
    while (i > 0) vga_putc(buf[--i]);
}

/* VIRTUAL BLOCK DEVICE SIMULATION */
static uint8_t vdisk[BLOCK_SIZE * BLOCK_COUNT];
static DeviceStatus dev_stat;
static uint32_t fake_time_us = 0;
#define IO_TIME_PER_BLOCK_US 200

void init_driver(void) {
    for (int i = 0; i < BLOCK_SIZE * BLOCK_COUNT; i++) vdisk[i] = 0;
    dev_stat.total_reads = 0;
    dev_stat.total_writes = 0;
    dev_stat.total_io_time = 0;
    fake_time_us = 0;
}

int io_write_block(int block, const uint8_t *data) {
    if (block < 0 || block >= BLOCK_COUNT) return -1;
    uint32_t base = (uint32_t)block * BLOCK_SIZE;
    for (int i = 0; i < BLOCK_SIZE; i++) vdisk[base + i] = data[i];
    dev_stat.total_writes++;
    dev_stat.total_io_time++;
    fake_time_us += IO_TIME_PER_BLOCK_US;
    return 0;
}

int io_read_block(int block, uint8_t *data) {
    if (block < 0 || block >= BLOCK_COUNT) return -1;
    uint32_t base = (uint32_t)block * BLOCK_SIZE;
    for (int i = 0; i < BLOCK_SIZE; i++) data[i] = vdisk[base + i];
    dev_stat.total_reads++;
    dev_stat.total_io_time++;
    fake_time_us += IO_TIME_PER_BLOCK_US;
    return 0;
}

uint32_t driver_time_us(void) {
    return fake_time_us;
}

DeviceStatus driver_status(void) {
    return dev_stat;
}

/* REAL CPU TIMER (RDTSC) */
uint64_t get_cpu_time_us(void) {
    uint32_t lo, hi;
    __asm__ volatile ("rdtsc" : "=a" (lo), "=d" (hi));
    uint64_t cycles = ((uint64_t)hi << 32) | lo;
    // Estimasi: Dibagi 2000 untuk konversi kasar tick ke microsecond pada QEMU TCG
    return cycles / 2000; 
}
