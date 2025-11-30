#include "memory.h"
#include "types.h"
#include "kernel.h"

__attribute__((section(".boot.data"), aligned(4096)))
pte_t kernel_l2_table[512];

__attribute__((section(".boot.data"), aligned(4096)))
pte_t kernel_l1_boot[512];

__attribute__((section(".boot.data"), aligned(4096)))
pte_t kernel_l1_high[512];

__attribute__((section(".boot.data"), aligned(4096)))
pte_t kernel_l0_tables[8][512];

__attribute__((section(".boot.text")))
static void clear_table(pte_t *table) {
  for (int i = 0; i < 512; i++) {
    table[i].value = 0;
  }
}

__attribute__((section(".boot.text")))
static void set_pte(pte_t *pte, uint64_t ppn, int flags) {
  pte->V = (flags & PTE_V) ? 1 : 0;
  pte->R = (flags & PTE_R) ? 1 : 0;
  pte->W = (flags & PTE_W) ? 1 : 0;
  pte->X = (flags & PTE_X) ? 1 : 0;
  pte->PPN = ppn;
}

__attribute__((section(".boot.text")))
static void map_region(uint64_t phys_start, uint64_t phys_end, int flags) {
  uint64_t phys_addr = phys_start & ~(PAGE_SIZE - 1);
  phys_end = (phys_end + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1);
  
  for(;phys_addr < phys_end; phys_addr += PAGE_SIZE) {
    uint64_t offset = phys_addr - KERNEL_PHYS_BASE;
    uint64_t table_idx = offset / (512 * PAGE_SIZE);
    uint64_t entry_idx = (offset / PAGE_SIZE) % 512;
    
    if (table_idx < 8) {
      uint64_t ppn = phys_addr >> PAGE_SHIFT;
      set_pte(&kernel_l0_tables[table_idx][entry_idx], ppn, flags | PTE_V);
    }
    
  }
}

__attribute__((section(".boot.text")))
void setup_boot_paging(void) {
  clear_table(kernel_l2_table);
  clear_table(kernel_l1_boot);
  clear_table(kernel_l1_high);
  for (int i = 0; i < 8; i++) {
    clear_table(kernel_l0_tables[i]);
  }

  uint64_t boot_identity_va = KERNEL_PHYS_BASE;
  uint64_t kernel_high_va = KERNEL_VIRT_BASE + 0x200000;
  
  uint64_t boot_vpn2 = VPN2(boot_identity_va);
  uint64_t boot_vpn1 = VPN1(boot_identity_va);
  
  uint64_t high_vpn2 = VPN2(kernel_high_va);
  uint64_t high_vpn1 = VPN1(kernel_high_va);
  
  uint64_t l1_boot_ppn = ((uint64_t)kernel_l1_boot) >> PAGE_SHIFT;
  uint64_t l1_high_ppn = ((uint64_t)kernel_l1_high) >> PAGE_SHIFT;

  set_pte(&kernel_l2_table[boot_vpn2], l1_boot_ppn, PTE_V);
  set_pte(&kernel_l2_table[high_vpn2], l1_high_ppn, PTE_V);

  for (int i = 0; i < 8; i++) {
    uint64_t l0_ppn = ((uint64_t)kernel_l0_tables[i]) >> PAGE_SHIFT;
    set_pte(&kernel_l1_boot[boot_vpn1 + i], l0_ppn, PTE_V);
    set_pte(&kernel_l1_high[high_vpn1 + i], l0_ppn, PTE_V);
  }

  extern char __boot_start[], __kernel_phys_end[];
  map_region((uint64_t)__boot_start, (uint64_t)__kernel_phys_end, PTE_R | PTE_W | PTE_X);
}

__attribute__((section(".boot.text")))
void enable_paging_and_jump(void) {
  uint64_t ppn = ((uint64_t)kernel_l2_table) >> 12;
  uint64_t satp = (8ULL << 60) | ppn;
  
  asm volatile("csrw satp, %0" :: "r"(satp));
  asm volatile("sfence.vma");

  asm volatile(
    "la t0, kernel_main\n"
    "jalr t0\n"
    :
    :
    : "t0"
  );
}
