#include "kernel_allocatpor.hpp"
#include "kprintf.h"
#include "memory.h"
#include "mem_util.h"
#include <cstddef>
#include <stddef.h>

KernelMemoryManager::KernelMemoryManager() {
  resource_list_ = nullptr;
  kprintf("Kernel Memory Manager initialized!\n");
  kprintf("Currently BRK is at %p\n", ksbrk(0));
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
  block->magic = MALLOC_MAGIC_WORD;
  return block;
}

void* KernelMemoryManager::kmalloc(size_t size) {
  if(resource_list_ == nullptr) {
    kprintf("kmalloc was called for the first block!\n");
    Block* block = allocate_new_block(size);
    resource_list_ = block;
    return (void*)(block+1);
  }

  kprintf("kmalloc was called, we already have at least one element, trying to find a free one\n");
  // get_free_block returns the first block which has at least the required
  // capcity. This is subject to change as we could potentially split the block
  // or find a better one
  Block* tmp = get_free_block(size);
  if(tmp) {
    kprintf("I found a free one, reusing it!\n");
    tmp->free = false;
    return (void*)(tmp+1);
  }

  kprintf("Unable to find a free one, allocating a new block!\n");
  tmp = resource_list_;
  while(tmp->next != nullptr) {
    tmp = tmp->next;
    kprintf(".");
  }
  kprintf("\n");

  Block* block = allocate_new_block(size);
  tmp->next = block;
  return (void*)(block+1);
}

// Block a preceeds Block b, this function tries to merge block a with block b if 
// both are non-null and free, additionally it also tries to merge with the block 
// after b is non-null and free
// Returns a valid pointer to the latest free block. The block pointer b and it's next 
// pointer may be invalidated through this function, only the returned pointer should
// be used
KernelMemoryManager::Block* KernelMemoryManager::merge_blocks(Block* a, Block* b){
  Block* c = b->next;

  if(a != nullptr && a->free){
    kprintf("Merging with previous block!\n");
    a->size += sizeof(Block) + b->size;
    a->next = b->next;
    b = a;
  }

  if(c != nullptr && c->free){
    kprintf("Mering with next block!\n");
    b->size += sizeof(Block) + c->size;
    b->next = c->next;
  }

  return b;
}

size_t KernelMemoryManager::__debug_get_resource_list_length(){
  if (resource_list_ == nullptr){
    return 0;
  }
  size_t index = 0;
  Block* tmp = resource_list_;

  while(tmp->next){
    tmp = tmp->next;
    index++;
  }

  return index+1;
}

int KernelMemoryManager::free(void* ptr) {
  if(ptr > KERNEL_BRK || ptr < (void*)KERNEL_BRK_BASE){
    kprintf("Error reason: ptr [%p] is out of bounds\n", ptr);
    return -1;
  }

  if(resource_list_ == nullptr) {
    kprintf("Error reason: Nothing was allocated yet!\n");
    return -1;
  }

  // This is a lot slower than using reinterpret_cast on ptr 
  // but in my opinion a lot safer since we do not simply rely 
  // on the magic value to see if the ptr was actually allocated
  // Also this way we get a pointer to the previous Block for 
  // potential merging
  Block* prev = nullptr;
  Block* tmp = resource_list_;
  while(tmp->next != nullptr && (tmp+1) != ptr){
    prev = tmp;
    tmp = tmp->next;
  }

  // We didn't find the given pointer, probably
  // double free
  if((tmp+1) != ptr){
    kprintf("Error reason: Ptr [%p] was not found in the resource_list_!\n", ptr);
    return -1;
  }

  Block* merged = merge_blocks(prev, tmp);
  merged->free = true;
  merged->magic = MALLOC_MAGIC_WORD;
  kmemset((void*)(merged+1), 0, merged->size);

  if(merged->next == nullptr)
    check_and_release_last();

  return 0;
}

// ToDo: This could be done much more efficiently with 
// a double-linked list, but we don't have that yet and 
// right now I don't have the brain capacity to implement it
// cleanly
void KernelMemoryManager::check_and_release_last(){
  if(resource_list_ == nullptr){
    return;
  }

  Block* last = resource_list_;
  Block* prev = nullptr;

  while(last->next != nullptr){
    prev = last;
    last = last->next;
  }

  if(prev == nullptr){
    resource_list_ = nullptr;
    kbrk((void*)KERNEL_BRK_BASE);
    return;
  }

  if(prev->free){
    kprintf("Prev was free???\n");
    return;
  }

  if(!last->free){
    kprintf("Last was not free???\n");
    return;
  }

  if(last->free){
    prev->next = nullptr;
    kbrk((void*)((size_t)(prev+1) + (size_t)prev->size));
  }
}



