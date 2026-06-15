//Working with Semaphores
//A semaphore is a counter which is either 0 or 1. 
//Wait - If the counter is 1, decrement it to 0 and let it run. If the counter is 0, mark the task as BLOCKED and add it to the list of waiting tasks
//Signal - Increment the counter to 1 and wake up any task that was BLOCKED waiting on it (in the list mentioned above)
//It has no owner which is the key difference it has from a mutex. 
//Example: Task A does some work, signals the semaphore, task B was waiting and now wakes up. 

#ifndef SEMAPHORE_H
#define SEMAPHORE_H

#include <stdio.h>
#include <stdint.h>
#include "os_core.h"

//Semaphore Struct
typedef struct {

	uint8_t counter; //Sempahore counter - Indicates locked or unlocked state
	TCB* waiting_tasks[NUMBER_OF_TASKS]; //An array of TCB pointers. Tells us WHICH tasks are waiting on it
	uint8_t TaskTracker; //Tracks the number of waiting tasks
	
} Semaphore; 

void OS_SemaphoreInit(Semaphore* smp);
void OS_Wait(Semaphore* smp);
void OS_Signal(Semaphore* smp);

#endif