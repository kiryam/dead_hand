#include "common.h"
#include <stdlib.h>
#include <string.h>

void sleepMs(unsigned int e) {
	e = e*1000;
	while(--e > 0){
		__asm("nop");
	}
}

static int size_allocated;

_PTR malloc_c(size_t size)
{
	_PTR allocated = malloc(size+sizeof(size_t));
	if( allocated == NULL ){
		return NULL;
	}
	memcpy(allocated, &size, sizeof(size_t));
	size_allocated+=size;

	// TODO REMOVE DEBUG
	size_t test_size;
	memcpy(&test_size, allocated, sizeof(size_t));
	if( size > 10000 ){
		Log_Message("err");
	}

	return allocated+sizeof(size_t);
}

void free_c(_PTR p){
	_PTR real_block = p-(sizeof(size_t));
	size_t size;
	memcpy(&size, real_block, sizeof(size_t));
	if( size > 10000 ){
		Log_Message("err");
	}
	size_allocated -= size;
	free(real_block);
}

// sum off all memory allocated
int get_memory_allocated_total(){
	return size_allocated;
}
