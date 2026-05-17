
#include "os_malloc.h"

static uint8_t heap_array[HEAP_SIZE];
Header* Hdr;

//Treats the start of the array as a header pointer and sets it all up as one big free block
void OS_HeapInit(void) {
	
	Hdr = (Header*)heap_array; //cast heap to Header*
	Hdr->size = HEAP_SIZE - sizeof(Header); //Total usable bytes after accounting for Header
	Hdr->is_free = 1; //Free
	
}

void* os_malloc(uint32_t size) {
	
	for (int i = 0; i < HEAP_SIZE; i++) {
		if (Hdr->size >= size && Hdr->is_free == 1) { //carve it out
			OS_Fault();
		}
	}
	
}