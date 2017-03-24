#include "common.h"
#include <stdlib.h>

void sleepMs(unsigned int e) {
	e = e*1000;
	while(--e > 0){
		__asm("nop");
	}
}

static int size_allocated;
_PTR malloc_c(size_t size)
{
	size_allocated+=size;
    return malloc(size);
}

void free_c(_PTR p){
	free(p);
}
