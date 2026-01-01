#pragma once

#include "types.h"
#include "kprintf.h"
#include "memory.h"

#define MALLOC_MAGIC_WORD 0xDEAD

class KernelMemoryManager{
private:
  struct Block {
    bool free;
    size_t size;
    uint16_t magic;
    Block* next;
  };
  Block* resource_list_;
  Block* get_free_block(size_t size);
  Block* allocate_new_block(size_t size);
  Block* merge_blocks(Block* a, Block* b);
  void check_and_release_last();
public:
  void* kmalloc(size_t size);
  void* realloc(void* ptr, size_t size);
  int free(void* ptr);
  size_t __debug_get_resource_list_length();
  KernelMemoryManager();
  ~KernelMemoryManager();
};
