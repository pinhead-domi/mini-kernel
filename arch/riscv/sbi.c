#include "sbi.h"

/*
    Legacy OpenSBI interface
*/
static inline long sbi_call(long ext, long arg0, long arg1, long arg2) {
  register long a0 asm("a0") = arg0;
  register long a1 asm("a1") = arg1;
  register long a2 asm("a2") = arg2;
  register long a7 asm("a7") = ext;

  asm volatile("ecall" : "+r"(a0) : "r"(a1), "r"(a2), "r"(a7) : "memory");

  return a0;
}

/*
    New OpenSBI Interface
*/
static inline long sbi_ecall(long eid, long fid, long arg0, long arg1,
                             long arg2) {
  register long a0 asm("a0") = arg0;
  register long a1 asm("a1") = arg1;
  register long a2 asm("a2") = arg2;
  register long a6 asm("a6") = fid; // Function ID
  register long a7 asm("a7") = eid; // Extension ID

  asm volatile("ecall"
               : "+r"(a0)
               : "r"(a1), "r"(a2), "r"(a6), "r"(a7)
               : "memory");

  return a0;
}

long sbi_set_timer(uint64_t stime_value) {
  return sbi_ecall(SBI_EXT_ID_TIME, 0, stime_value, 0, 0);
}

void sbi_putchar(int ch) { sbi_call(SBI_CONSOLE_PUTCHAR, ch, 0, 0); }

int sbi_getchar(void) { return sbi_call(SBI_CONSOLE_GETCHAR, 0, 0, 0); }

void sbi_shutdown(void) { sbi_call(SBI_SHUTDOWN, 0, 0, 0); }
