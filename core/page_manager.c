#include "kprintf.h"
#include "mem_util.h"
#include "pager_manager.h"

uint8_t page_bitmap[BITMAP_SIZE];

void init_page_manager() {
  kprintf("Page Manager Init started\n");
  memset(page_bitmap, 0, sizeof(page_bitmap));
  kprintf("Memset finished\n");

  uint64_t kernel_size =
      (uint64_t)__kernel_phys_end - (uint64_t)PHYSICAL_MEMORY_START;
  uint64_t openSBI_pages = kernel_size / PAGE_SIZE;

  kprintf("%d pages will be reserved for kernel\n", openSBI_pages);
  for (uint64_t i = 0; i < openSBI_pages; i++) {
    alloc_page(); // Mark pages used by openSBI and kernel as used
  }
}

void *alloc_page() {
  for (uint64_t i = 0; i < NUM_PAGES; i++) {
    uint64_t byte = i / 8;
    uint8_t idx = i % 8;

    if (!(page_bitmap[byte] & (1 << idx))) {
      page_bitmap[byte] |= (1 << idx);
      return (void *)(PHYSICAL_MEMORY_START + i * PAGE_SIZE);
    }
  }
  return (void *)0;
}

void free_page(void *page) {
  kprintf("Trying to unmap page %p\n", page);

  uint64_t offset = (uint64_t) page - (uint64_t)PHYSICAL_MEMORY_START;
  uint64_t ppn = offset / PAGE_SIZE;
  uint64_t byte = ppn / 8;
  uint8_t idx = ppn % 8;

  if(!(page_bitmap[byte] & (1 << idx))) {
    kprintf("ERROR: FREE FOR PPN THAT WAS NOT ALLOCATED!\n");
  }

  page_bitmap[byte] &= ~(1 << idx);
}
