#include "kernel.h"

#define BIT_POS_SIE 1
#define BIT_POS_STIE 5

#define SCAUSE_MTI 5
#define SCAUSE_PF_I 12
#define SCAUSE_PF_L 13
#define SCAUSE_PF_W 15

#define SBI_FUNC_SET_TIMER 0
#define TIME_DELTA_1_SEC 10000000UL

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
  char* type = scause == 12 ? "INSTRUCTION FETCH" : scause == 13 ? "LOAD" : "WRITE";

  kprintf("%s PAGEFAULT\n", type);
  kprintf("Faulting address: %p\n", stval);
  kprintf("PC that caused it: %p\n", sepc);

  while(1);
}

void trap_handler() {
  uint64_t scause;
  asm volatile("csrr %0, scause" : "=r"(scause));
  scause &= ~(1lu << 63);

  switch (scause) {
  case SCAUSE_MTI:
    kprintf("Timer Interrupt Trap\n");
    uint64_t next_timer = read_time() + 10000000;
    system_timer += 10000000;
    sbi_set_timer(next_timer);
    break;
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

extern void trap_entry(void);

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

  kprintf("NUM_PAGES: %d\n", (int)NUM_PAGES);
  init_page_manager();
  kprintf("Page Manager Initialized\n");
  enable_interrupts();

  uint64_t next_timer = read_time() + 10000000;
  sbi_set_timer(next_timer);

  kprintf("Timer set! Waiting for interrupt...\n");

  while (1) {
    asm volatile("wfi"); /* Wait for interrupt */
  }
}
