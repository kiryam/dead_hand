#ifndef COMMON_H
#define COMMON_H

#ifdef __cplusplus
extern "C" {
#endif
#include "stm32f2xx.h"
#include <stdlib.h>

typedef void (*callback) (void *);
typedef void (*functiontype)();

#define  BTN1  ((uint16_t)0x0001)
#define  BTN2  ((uint16_t)0x0002)
#define  BTN3  ((uint16_t)0x0004)
#define  BTN4  ((uint16_t)0x0008)


enum MENU_LAYOUT {EMPTY, LIST};


#define POOL_SIZE 1024 * 20

#define USE_UMM_MALLOC
//#define USE_TLSF

void memory_init();
void sleepMs(unsigned int);
_PTR malloc_c(size_t size);
void free_c(_PTR);


void SysTick_Init(void);
void TimeTick_Decrement(void);
void delay_1ms(void);
int timeout_ms(u32 timeout, int *stop_if_raised);

int get_memory_allocated_total();

#ifdef __cplusplus
}
#endif

#endif

