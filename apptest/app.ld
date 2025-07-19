/* app.ld - Linker script for applications */

ENTRY(main)

SECTIONS
{
    . = 0x100000;  /* Load address - adjust for your OS */
    
    .text : {
        *(.text)
    }
    
    .data : {
        *(.data)
    }
    
    .bss : {
        *(.bss)
    }
}