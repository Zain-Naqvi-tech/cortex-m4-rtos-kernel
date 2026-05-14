//A mutex has the same features as a semaphore but has an OWNER field

#ifndef MUTEX_H
#define MUTEX_H

#include <stdio.h>
#include <stdint.h>
#include "os_core.h"

//Mutex Struct
typedef struct {

	TCB* owner; //Points to the task which owns the Mutex and is the only one which can unlock it
	uint8_t counter;
	TCB* waiting_tasks[NUMBER_OF_TASKS];
	uint8_t TaskTracker;

} Mutex;

#endif