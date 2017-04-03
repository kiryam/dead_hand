#include "common.h"
#include <stdlib.h>
#include <string.h>
#include "tlsf.h"

#define POOL_SIZE 1024 * 100
static char pool[POOL_SIZE];

void sleepMs(unsigned int e) {
	e = e*1000;
	while(--e > 0){
		__asm("nop");
	}
}

void memory_init(){
	init_memory_pool(POOL_SIZE, pool);
}

_PTR malloc_c(size_t size) {
	return malloc_ex(size, pool);
}

void free_c(_PTR p) {
	free_ex(p, pool);
}

// sum off all memory allocated
int get_memory_allocated_total(){
	return get_used_size(pool);
}

int get_memory_max(){
	return get_max_size(pool);
}
