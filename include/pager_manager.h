#ifndef PM_H
#define PM_H

#include "types.h"

#define PHYSICAL_MEMORY_SIZE (128lu * 1024lu * 1024lu)
#define PAGE_SIZE (4lu * 1024lu)
#define NUM_PAGES ((PHYSICAL_MEMORY_SIZE) / PAGE_SIZE)
#define BITMAP_SIZE ((NUM_PAGES + 7lu) / 8lu)

#define PHYSICAL_MEMORY_START 0x80000000

extern char __kernel_start[];
extern char __kernel_end[];
extern char __stack_top[];

void *alloc_page();
void free_page(void *page);
void init_page_manager();

#endif
