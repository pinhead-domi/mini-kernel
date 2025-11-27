OUTPUT_ARCH(riscv)
ENTRY(_start)

SECTIONS {
    /* QEMU virt machine loads kernel at 0x80200000 */
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

    /* Stack grows downward */
    . = . + 0x10000;  /* 64 KB stack */
    __stack_top = .;
}