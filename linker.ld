ENTRY(_start)

SECTIONS
{
    . = 0x1000;

    .text : AT(0x1000)
    {
        *(.text._start)
        *(.text)
        *(.rodata*) /* Include strings and constants! */
    }

    .data : 
    {
        *(.data)
    }

    .bss :
    {
        *(COMMON)
        *(.bss)
    }

    /* This creates a marker for the end of the kernel */
    _end = .;
}