#include "kprintf.h"
#include "sbi.h"

typedef __builtin_va_list va_list;
#define va_start(ap, last) __builtin_va_start(ap, last)
#define va_arg(ap, type) __builtin_va_arg(ap, type)
#define va_end(ap) __builtin_va_end(ap)

void kputchar(int c) { sbi_putchar(c); }

void kputs(const char *s) {
  while (*s) {
    kputchar(*s++);
  }
}

static uint32_t kputn_base(uint64_t n, int base) {
  char buf[65];
  int i = 0;
  const char *digits = "0123456789abcdef";

  if (n == 0) {
    kputchar('0');
    return 1;
  }

  while (n > 0) {
    buf[i++] = digits[n % base];
    n /= base;
  }

  int len = i;

  while (i > 0) {
    kputchar(buf[--i]);
  }

  return len;
}

static uint32_t kputn_signed(int64_t n) {
  if (n < 0) {
    kputchar('-');
    n = -n;
  }

  return kputn_base((uint64_t)n, 10);
}

static uint32_t kputn_hex(uint64_t n) { return kputn_base(n, 16); }

static uint32_t kputn_decimal(uint64_t n) { return kputn_base(n, 10); }

static uint32_t kput_pointer(void *ptr) {
  kputs("0x");
  return kputn_hex((uint64_t)ptr) + 2;
}

int kprintf(const char *fmt, ...) {
  va_list args;
  int count = 0;

  va_start(args, fmt);

  while (*fmt) {

    // Keep printing 'normal' chars
    if (*fmt != '%') {
      kputchar(*fmt++);
      count++;
      continue;
    }

    fmt++;
    switch (*fmt) {
    case 's': {
      const char *s = va_arg(args, const char *);
      if (s) {
        kputs(s);
        while (*s) {
          count++;
          s++;
        }
      } else {
        kputs("(null)");
        count += 6;
      }
      break;
    }
    case 'd': {
      int64_t arg = va_arg(args, int64_t);
      count += kputn_signed(arg);
      break;
    }
    case 'u': {
      uint64_t arg = va_arg(args, uint64_t);
      count += kputn_decimal(arg);
      break;
    }
    case 'x': {
      uint64_t arg = va_arg(args, uint64_t);
      count += kputn_hex(arg);
      break;
    }
    case 'p': {
      void *arg = va_arg(args, void *);
      count += kput_pointer(arg);
      break;
    }
    case 'c': {
      char arg = va_arg(args, int);
      kputchar(arg);
      count++;
      break;
    }
    case '%': {
      kputchar('%');
      count++;
      break;
    }
    default: {
      kputchar('%');
      count++;
      break;
    }
    }

    fmt++;
  }

  va_end(args);
  return count;
}
