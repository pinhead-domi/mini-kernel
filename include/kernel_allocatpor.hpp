#pragma once

#include "types.h"
#include "kprintf.h"
#include "memory.h"

class KernelMemoryManager{
private:
  struct Block {
    bool free;
    size_t size;
    Block* next;
  };
  Block* resource_list_;
  Block* get_free_block(size_t size);
  Block* allocate_new_block(size_t size);
public:
  void* kmalloc(size_t size);
  KernelMemoryManager();
  ~KernelMemoryManager();
};