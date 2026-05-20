
#include "os_malloc.h"

static uint8_t heap_array[HEAP_SIZE];

//Treats the start of the array as a header pointer and sets it all up as one big free block
void OS_HeapInit(void) {

	Header* Hdr;
	Hdr = (Header*)heap_array; //cast heap to Header*
	Hdr->size = HEAP_SIZE - sizeof(Header); //Total usable bytes after accounting for Header
	Hdr->is_free = 1; //Free
	
}

void* os_malloc(uint32_t size) {
	
	Header* Current = (Header*)heap_array; //Current points to the start of the heap_array and both sides are a Header* type

	uint32_t remaining_size;
	
	//We need to keep looping in order to find the desirable block in memory
	while (1) {

		if ((uint8_t*)Current >= heap_array + HEAP_SIZE) { return NULL; } //If the pointer moves past the end of the heap, we stop and return NULL, as there is no more space left

		//if the payload block is big enough and free
		if (Current->size >= size && Current->is_free == 1) {
			
			Header* leftOver;
			remaining_size = Current->size - size; //Size of leftover memory area

			//if the remaining size is bigger than a basic header size, we need to make use of that memory as well
			if (remaining_size > sizeof(Header)) {
				leftOver = (Header*)((uint8_t*)Current + sizeof(Header) + size); //The leftover header pointer now points to the start of the new payload we are making after splitting
				leftOver->is_free = 1; //Mark this new payload as free
				leftOver->size = Current->size - size - sizeof(Header); //Find the size of this new payload as well
				Current->size = size;
			}

			Current->is_free = 0; //No longer free in memory

			return (void*)(Current + 1); //We return a pointer to the start of the payload
		}

		else {
			Current = (Header*)((uint8_t*)Current + sizeof(Header) + Current->size); //This makes the Current pointer jump the header and its payload, reaching the next header in the heap
		}
	}
	
}

void os_free(void* data) {

	if (data == NULL) return;

	Header* Current = data; //Current header pointer points to the allocated payload
	Current = Current - 1; //We move back by a header
	Current->is_free = 1; //We set the payload as free to use

}