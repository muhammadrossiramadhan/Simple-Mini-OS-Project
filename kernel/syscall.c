#include "syscall.h"
#include "driver.h"
#include "keyboard.h"

void sys_clear_screen(void) { driver_clear_screen(); }
void sys_print(const char *s) { driver_write(s); }
void sys_print_dec(uint32_t v) { driver_write_dec(v); }
void sys_print_u64(uint64_t v) { driver_write_u64(v); }

char sys_read(void) { return keyboard_getchar(); }

int sys_write_block(int block_index, const uint8_t *buf, uint32_t size) {
    if (size != BLOCK_SIZE) return -1;
    return io_write_block(block_index, buf);
}

int sys_read_block(int block_index, uint8_t *buf, uint32_t size) {
    if (size != BLOCK_SIZE) return -1;
    return io_read_block(block_index, buf);
}

uint32_t sys_time_us(void) { return driver_time_us(); }
uint64_t sys_cpu_time_us(void) { return get_cpu_time_us(); }
DeviceStatus sys_get_status(void) { return driver_status(); }
