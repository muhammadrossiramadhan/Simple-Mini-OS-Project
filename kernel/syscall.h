#ifndef SYSCALL_H
#define SYSCALL_H

#include <stdint.h>
#include "driver.h"

void sys_clear_screen(void);
void sys_print(const char *s);
void sys_print_dec(uint32_t v);
void sys_print_u64(uint64_t v); // Tambahan

char sys_read(void);

int sys_write_block(int block_index, const uint8_t *buf, uint32_t size);
int sys_read_block(int block_index, uint8_t *buf, uint32_t size);

uint32_t sys_time_us(void);      // Disk Time
uint64_t sys_cpu_time_us(void);  // CPU Time

DeviceStatus sys_get_status(void);

#endif
