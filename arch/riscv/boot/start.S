.section .boot.text
.globl _start
_start:
    # Set up boot stack (physical address)
    la sp, __boot_stack_top
    
    # Save hartid and dtb
    mv s0, a0
    mv s1, a1
    
    # Only hart 0 continues
    bnez a0, park_hart
    
    la t0, __bss_start
    la t1, __bss_end
clear_bss:
    bgeu t0, t1, bss_done
    sd zero, 0(t0)
    addi t0, t0, 8
    j clear_bss
bss_done:
    
    # Set up paging
    call setup_boot_paging
    
    # Enable paging and jump to high kernel
    call enable_paging_and_jump
    
    # Should never return
    j hang

park_hart:
    wfi
    j park_hart

hang:
    wfi
    j hang

.section .boot.data
.align 12
__boot_stack:
    .space 16384
__boot_stack_top: