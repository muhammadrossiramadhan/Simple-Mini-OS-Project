ENTRY(_start)

SECTIONS
{
/* Kernel dimuat di 1MB */
. = 0x00100000;

.multiboot :
{
	KEEP(*(.multiboot))
}

.text :
{
	*(.text)
	*(.text.*)
}

.rodata :
{
	*(.rodata)
	*(.rodata.*)
}

.data :
{
	*(.data)
	*(.data.*)
}

.bss :
{
	*(COMMON)
	*(.bss)
	*(.bss.*)
}


}
