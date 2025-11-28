OUTPUT_ARCH(riscv)
ENTRY(_start)

SECTIONS {
    . = 0x80200000;

    __kernel_start = .;

    .text : {
        *(.text.entry)
        *(.text .text.*)
    }

    .rodata : ALIGN(4K) {
        *(.rodata .rodata.*)
    }

    .data : ALIGN(4K) {
        *(.data .data.*)
        *(.sdata .sdata.*)
    }

    .bss : ALIGN(4K) {
        __bss_start = .;
        *(.bss .bss.*)
        *(.sbss .sbss.*)
        *(COMMON)
        __bss_end = .;
    }

    . = ALIGN(4K);
    __kernel_end = .;

    . = . + 0x10000;
    __stack_top = .;
}