#ifndef KPRINTF_H
#define KPRINTF_H

#include "types.h"

#ifdef __cplusplus
extern "C" {
#endif
int kprintf(const char *fmt, ...);
void kputs(const char *s);
void kputchar(int c);
#ifdef __cplusplus
}
#endif

#endif