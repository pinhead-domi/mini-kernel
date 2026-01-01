#include "kernel.hpp"
#include "kernel_allocatpor.hpp"
#include "mem_util.h"
#include "pager_manager.h"
#include <new>
#include <ustl/uvector.h>
#include <ustl/umap.h>
#include <ustl/ustring.h>

#define BIT_POS_SIE 1
#define BIT_POS_STIE 5

#define SCAUSE_MTI 5
#define SCAUSE_PF_I 12
#define SCAUSE_PF_L 13
#define SCAUSE_PF_W 15

#define SBI_FUNC_SET_TIMER 0
#define TIME_DELTA_1_SEC 10000000UL

KernelMemoryManager mem;

static inline void enable_interrupts() {
  asm volatile("csrs sie, %0" ::"r"(1 << BIT_POS_STIE));
  asm volatile("csrs sstatus, %0" ::"r"(1 << BIT_POS_SIE));
}

static inline void disable_interrupts() {
  asm volatile("csrc sstatus, %0" ::"r"(1 << BIT_POS_SIE));
}

static inline uint64_t read_time(void) {
  uint64_t time;
  asm volatile("rdtime %0" : "=r"(time));
  return time;
}

volatile uint64_t system_timer = 0;

void handle_pagefault() {
  uint64_t scause, sepc, stval;

  asm volatile ("csrr %0, scause" : "=r"(scause));
  asm volatile ("csrr %0, sepc" : "=r"(sepc));
  asm volatile ("csrr %0, stval" : "=r"(stval));

  scause &= ~(1lu << 63);
  const char* type = scause == 12 ? "INSTRUCTION FETCH" : scause == 13 ? "LOAD" : "WRITE";

  kprintf("%s PAGEFAULT\n", type);
  kprintf("Faulting address: %p\n", stval);
  kprintf("PC that caused it: %p\n", sepc);

  while(1);
}

extern "C"
void trap_handler() {
  uint64_t scause;
  asm volatile("csrr %0, scause" : "=r"(scause));
  scause &= ~(1lu << 63);

  switch (scause) {
  case SCAUSE_MTI: {
    kprintf("Timer Interrupt Trap\n");
    uint64_t next_timer = read_time() + 10000000;
    system_timer += 10000000;
    sbi_set_timer(next_timer);
    break;
  }
  case SCAUSE_PF_I:
  case SCAUSE_PF_L:
  case SCAUSE_PF_W:
    handle_pagefault();
    break;
  default:
    kprintf("Unknown Trap; 0x%x\n", scause);
    while (1)
      ;
    break;
  }
}

void allocator_test(){
  struct A{
    char name[2*PAGE_SIZE + 1];
  };
  struct B{
    char name[100];
  };

  void* a = mem.kmalloc(sizeof(A));
  void* b = mem.kmalloc(sizeof(B));
  void* c = mem.kmalloc(sizeof(A));
  void* d = mem.kmalloc(sizeof(B));
  void* e = mem.kmalloc(sizeof(A));
  void* f = mem.kmalloc(sizeof(B));

  kmemset(a, 69, sizeof(A));
  kmemset(b, 69, sizeof(B));
  kmemset(c, 69, sizeof(A));
  kmemset(d, 69, sizeof(B));
  kmemset(e, 69, sizeof(A));
  kmemset(f, 69, sizeof(B));

  kprintf("After six allocations the resource list has %d elements\n", mem.__debug_get_resource_list_length());

  int result = mem.free(a);
  kprintf("Free returned %d\n", result);

  a = mem.kmalloc(sizeof(A));

  mem.free(a);
  mem.free(b);
  mem.free(c);
  mem.free(d);
  mem.free(e);

  kprintf("After five frees the resource list has %d elements\n", mem.__debug_get_resource_list_length());

  mem.free(f);
  kprintf("After six frees the resource list has %d elements\n", mem.__debug_get_resource_list_length());
}

extern "C" void trap_entry(void);
extern pte_t kernel_l2_table[];
extern char __stack_top[];

extern "C"
void kernel_main(uint64_t hartid, uint64_t dtb_addr) {
  kprintf("========================================\n");
  kprintf("RISC-V Bare-Metal Kernel\n");
  kprintf("========================================\n");

  kprintf("Hart ID: %u\n", hartid);
  kprintf("DTB Address: %p\n", (void *)dtb_addr);
  kprintf("Kernel loaded at: 0x%X\n", 0x80200000);

  kprintf("\nTesting kprintf formats:\n");
  kprintf("  Decimal: %d\n", 42);
  kprintf("  Negative: %d\n", -42);
  kprintf("  Unsigned: %u\n", 12345);
  kprintf("  Hex (lowercase): 0x%x\n", 0xDEADBEEF);
  kprintf("  Hex (uppercase): 0x%X\n", 0xCAFEBABE);
  kprintf("  String: %s\n", "Hello, World!");
  kprintf("  String: %s\n", 0);
  kprintf("  Character: %c%c%c\n", 'A', 'B', 'C');
  kprintf("  Pointer: %p\n", (void *)0x80200000);
  kprintf("  Percent: 100%%\n");

  kprintf("\nHello from RISC-V kernel!\n");
  kprintf("========================================\n");

  uint64_t trap_addr = (uint64_t)trap_entry;
  asm volatile("csrw stvec, %0" :: "r"(trap_addr));
  kprintf("Trap handler set at %p\n", trap_addr);

  remove_boot_mapping(kernel_l2_table);

  kprintf("NUM_PAGES: %d\n", (int)NUM_PAGES);
  init_page_manager();
  kprintf("Page Manager Initialized\n");

  new (&mem) KernelMemoryManager();
  enable_interrupts();

  //allocator_test();
  {
    ustl::vector<int> important_numbers;
    for(size_t i=0; i<4096;i++){
      important_numbers.push_back((int)i);
    }
  }

  ustl::map<int,int>mapped_values;
  mapped_values[1]=69;

  ustl::string now_we_are_racing = "Now we are racing";
  now_we_are_racing.append(", wow the ustl lib is so cool!");
  kprintf("Now lets see if the ustl string works: %s\n", now_we_are_racing.c_str());

  uint64_t next_timer = read_time() + 10000000;
  sbi_set_timer(next_timer);

  uint64_t* data = (uint64_t*)(__stack_top + 0x1000000);

  map_page(kernel_l2_table, (uint64_t)data, (uint64_t)alloc_page());
  kprintf("I read %d from the data pointer!\n", *data);

  kprintf("Timer set! Waiting for interrupt...\n");

  while (1) {
    asm volatile("wfi"); /* Wait for interrupt */
  }
}
