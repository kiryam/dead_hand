#include "common.h"
#include <stdlib.h>
#include <string.h>


#ifdef USE_UMM_MALLOC
#include "umm_malloc.h"
#include "umm_malloc_cfg.h"
char umm_heap_pool[POOL_SIZE];
#endif



#ifdef USE_TLSF
#include "tlsf.h"
static char pool[POOL_SIZE];
#endif

static __IO uint32_t sysTickCounter;

void SysTick_Init(void) {
	/****************************************
	 *SystemFrequency/1000      1ms         *
	 *SystemFrequency/100000    10us        *
	 *SystemFrequency/1000000   1us         *
	 *****************************************/
	while (SysTick_Config(SystemCoreClock / 1000) != 0) {
	} // One SysTick interrupt now equals 1us

}

/**
 * This method needs to be called in the SysTick_Handler
 */
void TimeTick_Decrement(void) {
	if (sysTickCounter != 0x00) {
		sysTickCounter--;
	}
}

void delay_nus(u32 n) {
	sysTickCounter = n;
	while (sysTickCounter != 0) {
	}
}

void delay_1ms(void) {
	sysTickCounter = 1;
	while (sysTickCounter != 0) {
	}
}

int timeout_ms(u32 timeout, int *stop_if_raised) {
	sysTickCounter = timeout;
	while (sysTickCounter != 0) {
		if ( *stop_if_raised ){
			return 0;
		}
	}

	return 1;
}

void delay_nms(u32 n) {
	while (n--) {
		delay_1ms();
	}
}

void SysTick_Handler(void) {
	TimeTick_Decrement();
}

void sleepMs(unsigned int e) {
	while (e--) {
		delay_1ms();
	}
}


#ifdef USE_UMM_MALLOC
void Heap_Corruption_Handler(){
	Log_Message("Heap corrupted");
	return;
}
#endif
void memory_init(){
#ifdef USE_UMM_MALLOC
	 umm_init();
#endif
#ifdef USE_TLSF
	init_memory_pool(POOL_SIZE, pool);
#endif
}

_PTR malloc_c(size_t size) {
#ifdef USE_UMM_MALLOC
	return umm_malloc(size);
#endif
#ifdef USE_TLSF
	return malloc_ex(size, pool);
#endif
}

void free_c(_PTR p) {
#ifdef USE_UMM_MALLOC
	umm_free( p );
#endif
#ifdef USE_TLSF
	free_ex(p, pool);
#endif
}

int get_memory_max(){
#ifdef USE_UMM_MALLOC
	return POOL_SIZE;
#endif
#ifdef USE_TLSF
	return get_max_size(pool);
#endif
}

// sum off all memory allocated
int get_memory_allocated_total(){
#ifdef USE_UMM_MALLOC
	return get_memory_max()-umm_free_heap_size();
#endif
#ifdef USE_TLSF
	return get_used_size(pool);
#endif
}


