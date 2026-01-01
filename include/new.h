#pragma once
#include "types.h"
#include "kernel_allocatpor.hpp"

extern KernelMemoryManager mem;

inline void *operator new(size_t size)
{
  return mem.kmalloc(size);
}

inline void *operator new[](size_t size)
{
  return mem.kmalloc(size);
}

inline void operator delete(void *p)
{
  mem.free(p);
}

inline void operator delete[](void *p)
{
  mem.free(p);
}

inline void operator delete(void* ptr, size_t size) noexcept {
  (void)size;
  mem.free(ptr);
}

inline void operator delete[](void* ptr, size_t size) noexcept {
  (void)size;
  mem.free(ptr);
}