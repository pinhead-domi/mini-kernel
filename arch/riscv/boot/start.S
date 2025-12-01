.section .boot.text.entry
.global _start
_start:
    # Set up boot stack (physical address)
    la sp, __boot_stack_top
    
    # Save hartid and dtb
    mv s0, a0
    mv s1, a1
    
    # Only hart 0 continues
    bnez a0, park_hart
    
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
