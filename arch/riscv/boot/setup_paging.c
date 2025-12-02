#include "memory.h"
#include "types.h"

__attribute__((section(".boot.data"), aligned(4096)))
pte_t kernel_l2_table[512];

__attribute__((section(".boot.data"), aligned(4096)))
pte_t kernel_l1_table_boot[512];

__attribute__((section(".boot.data"), aligned(4096)))
pte_t kernel_l1_table_high[512];

__attribute__((section(".boot.data"), aligned(4096)))
pte_t kernel_l1_direct[512];

__attribute__((section(".boot.data"), aligned(4096)))
pte_t kernel_l0_table[512];

__attribute__((section(".boot.text")))
void setup_boot_paging(void) {

  for (int i = 0; i < 512; i++) {
    kernel_l2_table[i].value = 0;
    kernel_l1_table_boot[i].value = 0;
    kernel_l0_table[i].value = 0;
    kernel_l1_table_high[i].value = 0;
  }

  // Manual paging setup for now, its not pretty but at least it works!  
  // Shared Top Level Page Table for boot and higher half kernel

  // map addresses from 0x80000000
  kernel_l2_table[2].value = (((uint64_t)kernel_l1_table_boot >> 12) << 10) | 0x01;
  // map addressed from 0xFFFFFFFF80000000
  kernel_l2_table[510].value = (((uint64_t)kernel_l1_table_high >> 12) << 10) | 0x01;
  // Setup direct mapping
  kernel_l2_table[256].value = (((uint64_t)kernel_l1_direct >> 12) << 10) | 0x01;
  
  // Boot code starts at 0x80200000 so we map the kernel code at offset 1
  kernel_l1_table_boot[1].value = (((uint64_t)kernel_l0_table >> 12) << 10) | 0x01;
  // Kernel start at 0xFFFFFFFF80000000 so no offset
  kernel_l1_table_high[0].value = (((uint64_t)kernel_l0_table >> 12) << 10) | 0x01;
  
  // Map 2MB starting at physical 0x80200000
  // The same code is mapped to boot and higher half kernel
  // otherwise enable_paging_and_jump would die right after the sfence.vma
  for (int i = 0; i < 512; i++) {
    uint64_t phys_addr = 0x80200000 + (i * 4096);
    uint64_t ppn = phys_addr >> 12;
    kernel_l0_table[i].value = (ppn << 10) | 0x0F;
  }

  // Create direct mapping with huge pages, so each l1 entry is a leaf entry
  // and covers 2MB each
  for (int i = 0; i < 64; i++) {
    uint64_t phys_addr = 0x80000000 + (i * 2 * 1024 * 1024);
    uint64_t ppn = phys_addr >> 12;
    kernel_l1_direct[i].value = (ppn << 10) | 0x0F;
  }
}

__attribute__((section(".boot.text")))
void enable_paging_and_jump(void) {
  uint64_t satp = (8ULL << 60) | (((uint64_t)kernel_l2_table) >> 12);
  
  asm volatile("csrw satp, %0" :: "r"(satp));
  asm volatile("sfence.vma");
  
  asm volatile(
    "la sp, __stack_top\n"
    "la t0, kernel_main\n"
    "jalr t0\n"
);
}