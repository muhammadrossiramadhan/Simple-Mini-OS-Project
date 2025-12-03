#ifndef REBOOT_H
#define REBOOT_H

#include <stdint.h>

// Fitur Shutdown QEMU (Quit)
static inline void qemu_shutdown(void) {
    unsigned short data = 0x2000;
    unsigned short port = 0x604;
    __asm__ volatile ("outw %0, %1" : : "a"(data), "Nd"(port));
}

// Fitur Reboot Standard x86 (Restart)
static inline void reboot_system(void) {
    uint8_t good = 0x02;
    while (good & 0x02) {
        unsigned char result;
        __asm__ volatile ("inb %1, %0" : "=a"(result) : "Nd"(0x64));
        good = result;
    }
    __asm__ volatile ("outb %0, %1" : : "a"((uint8_t)0xFE), "Nd"(0x64));
    __asm__ volatile ("hlt");
}

#endif
