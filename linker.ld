OUTPUT_FORMAT("elf64-x86-64")
OUTPUT_ARCH(i386:x86-64)

ENTRY(_start)

SECTIONS
{
    . = 0x1000;

    .text BLOCK(4K) : AT(ADDR(.text))
    {
        *(.text._start)
        *(.text)
    }

    .rodata BLOCK(4K) : AT(ADDR(.rodata))
    {
        *(.rodata*)
    }

    .data BLOCK(4K) : AT(ADDR(.data))
    {
        *(.data)
    }

    .bss BLOCK(4K) : AT(ADDR(.bss))
    {
        *(COMMON)
        *(.bss)
    }

    _end = .;
}