#ifndef SBI_H
#define SBI_H

#include "types.h"

/* SBI legacy extension IDs */
#define SBI_SET_TIMER              0
#define SBI_CONSOLE_PUTCHAR        1
#define SBI_CONSOLE_GETCHAR        2
#define SBI_CLEAR_IPI              3
#define SBI_SEND_IPI               4
#define SBI_REMOTE_FENCE_I         5
#define SBI_REMOTE_SFENCE_VMA      6
#define SBI_REMOTE_SFENCE_VMA_ASID 7
#define SBI_SHUTDOWN               8

#define SBI_EXT_ID_TIME      0x54494D45


#ifdef __cplusplus
extern "C" {
#endif

void sbi_putchar(int ch);
int sbi_getchar(void);
void sbi_shutdown(void);
long sbi_set_timer(uint64_t stime_value);

#ifdef __cplusplus
}
#endif

#endif