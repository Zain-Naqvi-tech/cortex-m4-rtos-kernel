
#ifndef OS_MALLOC_H
#define OS_MALLOC_H

#include <stdio.h>
#include <stdint.h>
#include "os_error.h"

#define HEAP_SIZE 1024 //1KB space for the Heap allocation array

//Block Header Struct Definition
typedef struct {

	uint32_t size; //Tells us the number of bytes per block
	uint8_t is_free; //Boolean value which indicates whether the block of RAM is free for use (1) or not (0)

} Header;

void OS_HeapInit(void);
void* os_malloc(uint32_t size);
void os_free(void* data);

#endif