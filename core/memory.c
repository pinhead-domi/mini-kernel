#include "memory.h"
#include "kprintf.h"
#include "mem_util.h"
#include "pager_manager.h"

void* KERNEL_BRK = (void*) (KERNEL_BRK_BASE);
extern pte_t kernel_l2_table[];

void map_page(pte_t *root_table, uint64_t va, uint64_t pa) {
  if(va % PAGE_SIZE != 0) {
    kprintf("WARNING: Mapping VMA that is not page-aligned, right now this works but I don't know for how long!\n");
  }

  uint64_t vpn2 = (va >> 30) & 0x1FF;
  uint64_t vpn1 = (va >> 21) & 0x1FF;
  uint64_t vpn0 = (va >> 12) & 0x1FF;

  pte_t *l2_pte = &((pte_t* )PHYS_TO_VIRT(root_table))[vpn2];
  pte_t *l1_table;
  void *l1_table_phys;

  kprintf("l2_pte is at VIRT %p\n", l2_pte);

  if (!l2_pte->V) {
    l1_table_phys = alloc_page();
    l1_table = PHYS_TO_VIRT(l1_table_phys);
    kmemset(l1_table, 0, PAGE_SIZE);

    kprintf("L1 was mapped to %p\n", l1_table);

    l2_pte->PPN = (uint64_t)l1_table_phys >> 12;
    l2_pte->V = 1;
  }
  else {
    l1_table = (pte_t *)PHYS_TO_VIRT(l2_pte->PPN << 12);
  }

  pte_t *l1_pte = &l1_table[vpn1]; 
  pte_t *l0_table;
  void *l0_table_phys;

  if (!l1_pte->V) {
    l0_table_phys = alloc_page();
    l0_table = PHYS_TO_VIRT(l0_table_phys);
    kmemset(l0_table, 0, PAGE_SIZE);

    kprintf("L0 was mapped to %p\n", l0_table);

    l1_pte->PPN = (uint64_t)l0_table_phys >> 12;
    l1_pte->V = 1;
  }
  else {
    l0_table = (pte_t *)PHYS_TO_VIRT(l1_pte->PPN << 12);
  }

  pte_t *l0_pte = &l0_table[vpn0];
  if (l0_pte->V) {
    // Page is already mapped???
    kprintf("Tried to map a page that is already mapped!\n");
    return;
  }

  l0_pte->PPN = pa >> 12;
  l0_pte->V = 1;
  l0_pte->R = 1;
  l0_pte->W = 1;
  l0_pte->X = 1;

  asm volatile("sfence.vma");
  kprintf("VA %p successfully mapped to PA %p\n", va, pa);
}

void unmap_page(pte_t* root_table, uint64_t va) {
    if(va % PAGE_SIZE != 0) {
    kprintf("WARNING: Mapping VMA that is not page-aligned, right now this works but I don't know for how long!\n");
  }

  uint64_t vpn2 = (va >> 30) & 0x1FF;
  uint64_t vpn1 = (va >> 21) & 0x1FF;
  uint64_t vpn0 = (va >> 12) & 0x1FF;

  pte_t *l2_pte = &((pte_t* )PHYS_TO_VIRT(root_table))[vpn2];
  pte_t *l1_table;
  void *l1_table_phys;

  kprintf("l2_pte is at VIRT %p\n", l2_pte);

  if (!l2_pte->V) {
    kprintf("l1 table was already unmapped!\n");
    return;
  }
  else {
    l1_table = (pte_t *)PHYS_TO_VIRT(l2_pte->PPN << 12);
  }

  pte_t *l1_pte = &l1_table[vpn1]; 
  pte_t *l0_table;
  void *l0_table_phys;

  if (!l1_pte->V) {
    kprintf("l0 table was already unmapped!\n");
  }
  else {
    l0_table = (pte_t *)PHYS_TO_VIRT(l1_pte->PPN << 12);
  }

  pte_t *l0_pte = &l0_table[vpn0];
  if (!l0_pte->V) {
    // Page is already unmapped???
    kprintf("WARNING: Tried to unmap a page that is not mapped: %p!\n", va);
    return;
  }

  uint64_t pa = l0_pte->PPN << 12;

  l0_pte->PPN = 0;
  l0_pte->V = 0;
  l0_pte->R = 0;
  l0_pte->W = 0;
  l0_pte->X = 0;

  free_page((void*)pa);

  asm volatile("sfence.vma");
  kprintf("VA %p successfully unmapped from PA %p\n", va, pa);
}

int kbrk(void* brk) {
  if(brk == NULL || brk < KERNEL_BRK_BASE)
    return -1;
    
  uint64_t old_end_page = ((uint64_t)KERNEL_BRK + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1);
  uint64_t new_end_page = ((uint64_t)brk + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1);

  if(new_end_page > old_end_page) {
    size_t pages_mapped = 0;
    for (uint64_t va = old_end_page; va < new_end_page; va += PAGE_SIZE) {
      void* pa = alloc_page();
      map_page(kernel_l2_table, va, (uint64_t)pa);
      pages_mapped++;
    }
    kprintf("Mapped %d pages to kernel break!\n", (int)pages_mapped);
  }
  else if(new_end_page < old_end_page) {
    size_t pages_unmapped = 0;
    for (uint64_t va = new_end_page; va < old_end_page; va += PAGE_SIZE) {
      unmap_page(kernel_l2_table, va);
      pages_unmapped++;
    }
    kprintf("Unmapped %d pages from kernel break!\n", (int)pages_unmapped);
  }

  KERNEL_BRK = brk;
  return 0;
}

void* ksbrk(size_t diff) {
  void* old_brk = KERNEL_BRK;
  if(kbrk((void*) ((size_t)old_brk + diff)) == 0) {
    return old_brk;
  }
  return (void*)-1;
}

void remove_boot_mapping(pte_t *root_table_phys) {
  kprintf("ATTENTION: BOOT MAPPING WILL BE REMOVED!\n");

  pte_t *root = (pte_t*)PHYS_TO_VIRT(root_table_phys);
  root[2].value = 0;
  asm volatile("sfence.vma");
  
  kprintf("Successfully removed boot mapping!\n");
}
