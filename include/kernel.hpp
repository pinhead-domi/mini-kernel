#ifndef KERNEL_H
#define KERNEL_H

#include "kprintf.h"
#include "mem_util.h"
#include "memory.h"
#include "pager_manager.h"
#include "sbi.h"
#include "types.h"

extern "C"
void kernel_main(uint64_t hartid, uint64_t dtb_addr);
void allocator_test();

#endif
