//This is the main semaphore file which makes use of the two specialties of semaphores, wait and signal

#include "Semaphore.h"
#include "os_core.h"
#include "tm4c1294ncpdt.h"
#include <stddef.h>
#include "msp432e401y.h"
#include "itm.h"

//Initialises the Semaphore elements
void OS_SemaphoreInit(Semaphore* smp) {

	smp->counter = 0;
	smp->TaskTracker = 0;
}

void OS_Wait(Semaphore* smp) { //Blocking

	if (smp->counter) { //if the counter is 1
		smp->counter--; //We decrement the counter back to 0
	}
	else { //counter is 0
		CurrentTask->state = BLOCKED;
		ITM_Write(ITM_PACKET(ITM_EVT_SEM_WAIT, CurrentTask - task_array, OS_Ticks)); //ITM Packet
		smp->waiting_tasks[smp->TaskTracker] = CurrentTask; //Add the currenttask to the waiting list array. We add at the TaskTracker variable as it moves with OS_Signal and OS_Wait
		smp->TaskTracker++; //Increment the number of tasks currently waiting on it
		SCB->ICSR |= SCB_ICSR_PENDSVSET_Msk; //bit 28 of INTCTRL Register. Mask is 0x10000000 - This allows for a context switch immediately after the block
	}

}

void OS_Signal(Semaphore* smp) { //Waking

	smp->counter = 1; //Set the counter to 1
	if (smp->TaskTracker > 0) { //If any tasks are waiting on it
		int8_t MaxPriority = -1;
		int8_t MaxPIndex = -1;
		
		//Now, we need to pick a task from the array. This depends on their priorities
		for (int i = 0; i < smp->TaskTracker; i++) {
			if (smp->waiting_tasks[i]->priority > MaxPriority) {
				MaxPriority = smp->waiting_tasks[i]->priority; 
				MaxPIndex = i; //Finds the index with the highest priority
			}
		}
		smp->waiting_tasks[MaxPIndex]->state = READY; //sets the task with the highest priority to READY
		ITM_Write(ITM_PACKET(ITM_EVT_SEM_SIGNAL, CurrentTask - task_array, OS_Ticks));
		
		//Now, we shift every element down by an index in order to fill the hole created by removing the task which called OS_Signal()
		for (int j = MaxPIndex; j < (smp->TaskTracker - 1); j++) {
			smp->waiting_tasks[j] = smp->waiting_tasks[j + 1]; //Moves the elements down by one level
		}
		smp->waiting_tasks[smp->TaskTracker - 1] = NULL; //Set the last element to NULL (this fixes the hole issue)
		smp->TaskTracker--; //Decrement the number of tasks currently waiting on it
		
		smp->counter = 0; //Set the counter back to 0. 
		
	}

}