/* Region 1 image: SYNINFO first, everything (incl. .bss) as file data,
   because the slave does not clear the engine window. BASE = run address. */
ENTRY(plugout_init)
SECTIONS
{
    . = BASE;
    .image : {
        KEEP(*(.syninfo))
        *(.text*)
        *(.rodata*)
        *(.data*)
        *(.bss*) *(COMMON)
        . = ALIGN(16);
    }
    /DISCARD/ : { *(.ARM.exidx*) *(.ARM.attributes) *(.comment) }
}
