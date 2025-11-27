.section .text.entry
    .globl _start
_start:
    # We're in S-mode now (OpenSBI boots us here)
    # Disable interrupts
    csrw sie, zero

    la t0, trap_entry
    csrw stvec, t0

    # Set up stack pointer
    la sp, __stack_top

    # Hart ID is passed in register a0 by OpenSBI
    # Save it for later if needed
    mv s0, a0

    # Only hart 0 continues, others park
    bnez a0, park_hart

    # Clear BSS section
    la t0, __bss_start
    la t1, __bss_end
clear_bss:
    bgeu t0, t1, bss_done
    sd zero, 0(t0)
    addi t0, t0, 8
    j clear_bss
bss_done:

    # Jump to kernel (a0 still contains hart ID)
    call kernel_main

trap_entry:
    call trap_handler
    sret

park_hart:
    wfi
    j park_hart