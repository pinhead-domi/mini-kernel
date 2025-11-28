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

void map_page(pte_t *root_table, uint64_t va, uint64_t pa);

#endif
