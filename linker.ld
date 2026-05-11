ENTRY(_start)

SECTIONS
{
    . = 0x1000;

    .text : AT(0x1000)
    {
        *(.text._start)
        *(.text)
        *(.rodata*)
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

    _end = .;
}
