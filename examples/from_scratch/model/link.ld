/* Region 0 image: SYNINFO at 0, slot memory at 0x8000 (the OS addresses it
   as base + 0x8000), code and data from 0x118000 like the stock images.
   .bss is file data: nothing guarantees the window is cleared. */
ENTRY(syninfo)
SECTIONS
{
    . = BASE;
    .image : {
        KEEP(*(.syninfo))
        . = 0x8000;
        KEEP(*(.storage))
        . = 0x118000;
        *(.text*)
        *(.rodata*)
        *(.data*)
        *(.bss*) *(COMMON)
        . = ALIGN(16);
    }
    /DISCARD/ : { *(.ARM.exidx*) *(.ARM.attributes) *(.comment) }
}
