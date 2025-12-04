#ifndef DRIVER_H
#define DRIVER_H

#include <stdint.h>

#define BLOCK_SIZE 4096
#define BLOCK_COUNT 1000

typedef struct {
    uint32_t total_reads;
    uint32_t total_writes;
    uint32_t total_io_time;
} DeviceStatus;

/* Driver API */
void init_driver(void);
void driver_clear_screen(void);
void driver_write(const char *s);
void driver_write_dec(uint32_t v);
void driver_write_u64(uint64_t v); 

/* Block I/O */
int io_write_block(int block, const uint8_t *buf);
int io_read_block(int block, uint8_t *buf);

/* Timer & Status */
uint32_t driver_time_us(void);   // Disk Time
uint64_t get_cpu_time_us(void);  // CPU Time
DeviceStatus driver_status(void);

#endif
