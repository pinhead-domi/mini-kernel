#ifndef MEM_UTIL_H
#define MEM_UTIL_H

#include "types.h"

#ifdef __cplusplus
extern "C"{
#endif

void kmemset(void* dst, uint8_t value, uint64_t len);

#ifdef __cplusplus
}
#endif

#endif
