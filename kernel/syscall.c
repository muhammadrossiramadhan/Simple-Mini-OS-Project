#include "syscall.h"

#define VGA_ADDRESS 0xB8000
#define VGA_WIDTH 80
#define VGA_HEIGHT 25
#define WHITE_ON_BLACK 0x0F

int cursor_x = 0;
int cursor_y = 0;

void clear_screen() {
    volatile char* vga = (volatile char*)VGA_ADDRESS;
    for (int i = 0; i < VGA_WIDTH * VGA_HEIGHT; i++) {
        vga[i*2] = ' ';
        vga[i*2+1] = WHITE_ON_BLACK;
    }
    cursor_x = 0;
    cursor_y = 0;
}

void kprint(const char* str) {
    volatile char* vga = (volatile char*)VGA_ADDRESS;

    for (int i = 0; str[i]; i++) {
        if (str[i] == '\n') {
            cursor_x = 0;
            cursor_y++;
        } else {
            int index = (cursor_y * VGA_WIDTH + cursor_x) * 2;
            vga[index] = str[i];
            vga[index + 1] = WHITE_ON_BLACK;
            cursor_x++;
        }

        if (cursor_x >= VGA_WIDTH) {
            cursor_x = 0;
            cursor_y++;
        }

        if (cursor_y >= VGA_HEIGHT) {
            cursor_y = 0; // nanti bisa dikembangkan jadi scroll
        }
    }
}

void sys_print(const char* str) {
    kprint(str);
}

extern char keyboard_getchar();

char sys_read() {
    return keyboard_getchar();
}