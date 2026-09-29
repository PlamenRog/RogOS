ENTRY(_start)

PHDRS
{
    text   PT_LOAD FLAGS(5); /* R-X */
    rodata PT_LOAD FLAGS(4); /* R-- */
    data   PT_LOAD FLAGS(6); /* RW- */
}

SECTIONS
{
    . = 0x08000000;

    .text :
    {
        *(.text*)
    } :text

    . = ALIGN(0x1000);

    .rodata :
    {
        *(.rodata*)
    } :rodata

    . = ALIGN(0x1000);

    .data :
    {
        *(.data*)
    } :data

    .bss :
    {
        *(COMMON)
        *(.bss*)
    } :data
}
