#ifndef MEMORY_H
#define MEMORY_H

#include "types.h"

typedef union {
  uint64_t value;

  struct {
    uint64_t V : 1;         // Valid
    uint64_t R : 1;         // Read
    uint64_t W : 1;         // Write
    uint64_t X : 1;         // Execute
    uint64_t U : 1;         // User
    uint64_t G : 1;         // Global
    uint64_t A : 1;         // Accessed
    uint64_t D : 1;         // Dirty
    uint64_t RSW : 2;       // Reserved for software
    uint64_t PPN : 44;      // Physical Page Number
    uint64_t reserved : 10; // Must be zero
  };
} pte_t;

#define VPN0(va) (((va) >> 12) & 0x1FF)
#define VPN1(va) (((va) >> 21) & 0x1FF)
#define VPN2(va) (((va) >> 30) & 0x1FF)

#define PAGE_SIZE 4096
#define PAGE_SHIFT 12

#define PTE_V 0x01
#define PTE_R 0x02
#define PTE_W 0x04
#define PTE_X 0x08
#define PTE_U 0x10
#define PTE_G 0x20
#define PTE_A 0x40
#define PTE_D 0x80

#define PHYS_MEM_BASE    0x80000000UL
#define DIRECT_MAP_BASE  0xFFFFFFC000000000UL

#define PHYS_TO_VIRT(paddr) ((void*)((uint64_t)(paddr) + DIRECT_MAP_BASE - PHYS_MEM_BASE))
#define VIRT_TO_PHYS(vaddr) ((uint64_t)(vaddr) - DIRECT_MAP_BASE + PHYS_MEM_BASE)

void map_page(pte_t *root_table, uint64_t va, uint64_t pa);
void remove_boot_mapping(pte_t *root_table);

#endif
