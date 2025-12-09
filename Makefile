CC = gcc
AS = nasm
LD = ld

CFLAGS  = -m32 -ffreestanding -nostdlib -fno-pie -fno-stack-protector
ASFLAGS = -f elf32

OBJS = build/boot.o build/kernel.o build/driver.o build/syscall.o build/keyboard.o build/memory_manager.o build/scheduler.o build/utils.o

all: mini-os.iso

build:
	mkdir -p build iso/boot/grub

build/boot.o: kernel/boot.s | build
	$(AS) $(ASFLAGS) kernel/boot.s -o build/boot.o

build/kernel.o: kernel/kernel.c kernel/driver.h kernel/syscall.h | build
	$(CC) $(CFLAGS) -c kernel/kernel.c -o build/kernel.o

build/driver.o: kernel/driver.c kernel/driver.h | build
	$(CC) $(CFLAGS) -c kernel/driver.c -o build/driver.o

build/syscall.o: kernel/syscall.c kernel/syscall.h kernel/keyboard.h | build
	$(CC) $(CFLAGS) -c kernel/syscall.c -o build/syscall.o

build/keyboard.o: kernel/keyboard.c kernel/keyboard.h | build
	$(CC) $(CFLAGS) -c kernel/keyboard.c -o build/keyboard.o

build/memory_manager.o: kernel/memory_manager.c kernel/memory_manager.h | build
	$(CC) $(CFLAGS) -c kernel/memory_manager.c -o build/memory_manager.o

build/scheduler.o: kernel/scheduler.c kernel/scheduler.h | build
	$(CC) $(CFLAGS) -c kernel/scheduler.c -o build/scheduler.o

build/utils.o: kernel/utils.c kernel/utils.h | build
	$(CC) $(CFLAGS) -c kernel/utils.c -o build/utils.o	

build/kernel.bin: $(OBJS) kernel/linker.ld
	$(LD) -m elf_i386 -T kernel/linker.ld -o build/kernel.bin $(OBJS)

mini-os.iso: build/kernel.bin
	mkdir -p iso/boot/grub
	cp build/kernel.bin iso/boot/kernel.bin
	cp boot/grub/grub.cfg iso/boot/grub/grub.cfg
	grub-mkrescue -o mini-os-v1.3.0.iso iso || grub2-mkrescue -o mini-os-v1.3.0.iso iso

clean:
	rm -rf build iso mini-os.iso

run: mini-os.iso
	qemu-system-i386 -cdrom mini-os-v1.3.0.iso -m 128M -boot d

.PHONY: all clean

