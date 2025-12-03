BITS 32

%define MULTIBOOT_MAGIC  0x1BADB002
%define MULTIBOOT_FLAGS  0x00000003
%define MULTIBOOT_CHECKSUM -(MULTIBOOT_MAGIC + MULTIBOOT_FLAGS)

section .multiboot
align 4
    dd MULTIBOOT_MAGIC
    dd MULTIBOOT_FLAGS
    dd MULTIBOOT_CHECKSUM

section .text
global _start
extern kernel_main

_start:
    call kernel_main
    cli
.hang:
    hlt
    jmp .hang
