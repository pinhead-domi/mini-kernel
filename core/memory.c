#include "memory.h"
#include "kprintf.h"
#include "mem_util.h"
#include "pager_manager.h"

void map_page(pte_t *root_table, uint64_t va, uint64_t pa) {
  if(va % PAGE_SIZE != 0) {
    kprintf("WARNING: Mapping VMA that is not page-aligned, right now this works but I don't know for how long!\n");
  }

  uint64_t vpn2 = (va >> 30) & 0x1FF;
  uint64_t vpn1 = (va >> 21) & 0x1FF;
  uint64_t vpn0 = (va >> 12) & 0x1FF;

  pte_t *l2_pte = &root_table[vpn2];
  pte_t *l1_table = (pte_t *)(l2_pte->PPN << 12);

  if (!l1_table) {
    l1_table = alloc_page();
    memset(l1_table, 0, PAGE_SIZE);

    kprintf("L1 was mapped to %p\n", l1_table);

    l2_pte->PPN = (uint64_t)l1_table >> 12;
    l2_pte->V = 1;
  }

  pte_t *l1_pte = &l1_table[vpn1];
  pte_t *l0_table = (pte_t *)(l1_pte->PPN << 12);

  if (!l0_table) {
    l0_table = alloc_page();
    memset(l0_table, 0, PAGE_SIZE);

    kprintf("L0 was mapped to %p\n", l0_table);

    l1_pte->PPN = (uint64_t)l0_table >> 12;
    l1_pte->V = 1;
  }

  pte_t *l0_pte = &l0_table[vpn0];
  if (l0_pte->PPN) {
    // Page is already mapped???
    kprintf("Tried to map a page that is already mapped!\n");
    return;
  }

  l0_pte->PPN = pa >> 12;
  l0_pte->V = 1;
  l0_pte->R = 1;
  l0_pte->W = 1;
  l0_pte->X = 1;

  kprintf("VA %p successfully mapped to PA %p\n", va, pa);
}
