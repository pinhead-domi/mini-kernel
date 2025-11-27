#ifndef KPRINTF_H
#define KPRINTF_H

#include "types.h"

int kprintf(const char *fmt, ...);
void kputs(const char *s);
void kputchar(int c);

#endif