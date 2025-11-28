#include "pager_manager.h"
#include "mem_util.h"

void init_page_manager() {
  memset(page_bitmap, 0, sizeof(page_bitmap));

  uint64_t openSBI_size = (uint64_t) __kernel_start - (uint64_t) PHYSICAL_MEMORY_SIZE;
  uint64_t openSBI_pages = openSBI_size / PAGE_SIZE;

  for(uint64_t i=0; i<openSBI_pages; i++) {
    alloc_page(); // Mark pages used by openSBI as used
  }
}

void* alloc_page() {
  for(uint64_t i=0; i<NUM_PAGES; i++) {
    uint64_t byte = i / 8;
    uint8_t idx = i % 8;

    if(!(page_bitmap[byte] & (1 << idx))) {
      page_bitmap[byte] |= (1 << idx);
      return (void*) (PHYSICAL_MEMORY_SIZE + i * PAGE_SIZE);
    }
  }
  return (void*)0;
}

void free_page(void* page) {
  uint64_t offset = (uint64_t) PHYSICAL_MEMORY_START - (uint64_t) page;
  uint64_t ppn = offset / PAGE_SIZE;
  uint64_t byte = ppn / 8;
  uint8_t idx = ppn % 8;

  page_bitmap[byte] &= ~(1 << idx);
}