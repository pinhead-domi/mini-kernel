#include "kernel_allocatpor.hpp"

KernelMemoryManager::KernelMemoryManager() {
  resource_list_ = nullptr;
  kprintf("Kernel Memory Manager initialized!\n");
  kprintf("Currently BRK is at %p\n", ksbrk(0));
  kbrk((void*) (KERNEL_BRK_BASE + 1));
  kbrk((void*) (KERNEL_BRK_BASE + 1));
  kbrk((void*) (KERNEL_BRK_BASE + 1));
  kbrk((void*) (KERNEL_BRK_BASE + (4096*2) + 1));
  kbrk((void*) (KERNEL_BRK_BASE));
}

KernelMemoryManager::~KernelMemoryManager() {
  kprintf("Good bye cruel world!\n");
}

KernelMemoryManager::Block* KernelMemoryManager::get_free_block(size_t size) {
  if (resource_list_ == nullptr)
    return nullptr;

  Block* tmp = resource_list_;
  while(tmp != nullptr) {
    if (tmp->free && tmp->size >= size) {
      return tmp;
    }
    tmp = tmp->next;
  }

  return nullptr;
}

KernelMemoryManager::Block* KernelMemoryManager::allocate_new_block(size_t size) {
  Block* block = (Block*) ksbrk(sizeof(Block) + size);
  block->free = false;
  block->next = nullptr;
  block->size = size;
  return block;
}

void* KernelMemoryManager::kmalloc(size_t size) {
  if(resource_list_ == nullptr) {
    Block* block = allocate_new_block(size);
    resource_list_ = block;
    return (void*)(block+1);
  }

  // get_free_block returns the first block which has at least the required
  // capcity. This is subject to change as we could potentially split the block
  // or find a better one
  Block* tmp = get_free_block(size);
  if(tmp) {
    tmp->free = false;
    return (void*)(tmp+1);
  }

  Block* tmp = resource_list_;
  while(tmp->next != nullptr) {
    tmp = tmp->next;
  }

  Block* block = allocate_new_block(size);
  tmp->next = block;
  return (void*)(block+1);
}