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

void sleepMs(unsigned int e) {
	e = (SystemCoreClock*e)/1000;
	while(--e > 0){
		__asm("nop");
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
	return umm_poison_malloc(size);
#endif
#ifdef USE_TLSF
	return malloc_ex(size, pool);
#endif
}

void free_c(_PTR p) {
#ifdef USE_UMM_MALLOC
	umm_poison_free( p );
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


