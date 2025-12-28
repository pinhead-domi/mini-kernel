#include "mem_util.h"

void kmemset(void* dst, uint8_t value, uint64_t len) {
  for(uint64_t i=0; i<len; i++)
    ((uint8_t*)dst)[i] = value;
}
