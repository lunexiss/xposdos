ENTRY(_start)

SECTIONS {
    . = 1M;
    .multiboot : { *(.multiboot) }
    .text : { *(.text*) }

    .text : {
        *(.multiboot)
        *(.text*)
    }

    .rodata : { *(.rodata*) }

    .data : { *(.data*) }

    .bss : {
        *(.bss*)
        *(COMMON)
    }

    _end = .;
}
