all:
	mkdir -p build iso/boot/grub

	nasm -f elf32 kernel/boot.s -o build/boot.o
	gcc -m32 -c kernel/kernel.c -o build/kernel.o -ffreestanding -nostdlib -fno-pie
	gcc -m32 -c kernel/syscall.c -o build/syscall.o -ffreestanding -nostdlib -fno-pie
	gcc -m32 -c kernel/keyboard.c -o build/keyboard.o -ffreestanding -nostdlib -fno-pie

	ld -m elf_i386 -T kernel/linker.ld -o build/kernel.bin build/boot.o build/kernel.o build/syscall.o build/keyboard.o

	cp build/kernel.bin iso/boot/
	cp boot/grub/grub.cfg iso/boot/grub/
	
	grub-mkrescue -o minios.iso iso

run:
	qemu-system-i386 -cdrom minios.iso -vga std

clean:
	rm -rf build iso minios.iso
