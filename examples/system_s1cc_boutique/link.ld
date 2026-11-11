/* SYSTEM-1 region image: SYNINFO at the start (the OS checks it there), region 0's patch memory at +0x1000 (the OS
   copies NPATCH x 0x200 bytes from there at install), code and data from CODE (an offset, as every '.' inside
   the section: region 0: 0x9000 like the SH-2, region 1: 0x1000). .bss is file data: nothing guarantees the
   region is cleared. */
ENTRY(syninfo)
SECTIONS
{
    . = BASE;
    .image : {
        KEEP(*(.syninfo))
        . = 0x1000;
        KEEP(*(.patches))
        . = CODE;
        *(.text*)
        *(.rodata*)
        *(.data*)
        *(.bss*) *(COMMON)
        . = ALIGN(16);
    }
    /DISCARD/ : { *(.ARM.exidx*) *(.ARM.attributes) *(.comment) }
}
