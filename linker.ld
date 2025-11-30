OUTPUT_ARCH(riscv)
ENTRY(_start)

KERNEL_PHYS_BASE = 0x80200000;
KERNEL_VIRT_BASE = 0xFFFFFFFF80000000;

SECTIONS {
    /* Boot section at physical address */
    . = KERNEL_PHYS_BASE;
    
    __boot_start = .;
    
    .boot.text : {
        *(.boot.text)
    }
    
    .boot.data : {
        *(.boot.data)
    }
    
    . = ALIGN(4K);
    __boot_end = .;
    
    /* Main kernel at virtual address, loaded at physical */
    . = KERNEL_VIRT_BASE + (__boot_end - KERNEL_PHYS_BASE);
    __kernel_virt_start = .;
    
    .text : AT(__boot_end) {
        *(.text*)
    }
    
    .rodata : AT(ADDR(.rodata) - KERNEL_VIRT_BASE + KERNEL_PHYS_BASE) {
        *(.rodata*)
    }
    
    .data : AT(ADDR(.data) - KERNEL_VIRT_BASE + KERNEL_PHYS_BASE) {
        *(.data*)
    }
    
    .bss : AT(ADDR(.bss) - KERNEL_VIRT_BASE + KERNEL_PHYS_BASE) {
        __bss_start = .;
        *(.bss*)
        *(.sbss*)
        __bss_end = .;
    }
    
    . = ALIGN(4K);
    __kernel_virt_end = .;
    
    __kernel_phys_start = KERNEL_PHYS_BASE;
    __kernel_phys_end = __boot_end + (__kernel_virt_end - __kernel_virt_start);
    
    /* Stack */
    . = ALIGN(4K);
    . = . + 0x10000;
    __stack_top = .;
}