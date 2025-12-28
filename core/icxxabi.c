#include "icxxabi.h"

#ifdef __cplusplus
extern "C" {
#endif

void *__dso_handle = 0;

int __cxa_atexit(void (*f)(void *), void *objptr, void *dso)
{
	return 0;
};

void __cxa_finalize(void *f)
{}

#ifdef __cplusplus
};
#endif
