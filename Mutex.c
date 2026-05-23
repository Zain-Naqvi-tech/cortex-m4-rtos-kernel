//Mutex Lock and Unlock features are included here

#include "Mutex.h"
#include "uart.h"

void Mutex_Init(Mutex* Mx) {
	Mx->counter = 1;
	Mx->TaskTracker = 0;
}

void Mutex_Lock(Mutex* Mx) { //Taking the Mutex
	
	if (Mx->counter) { //The task takes the mutex
		Mx->owner = CurrentTask; //Set the owner to the current task. 
		Mx->OriginalPriority = CurrentTask->priority; //We save the priority once it takes the mutex
		Mx->counter--;
		UART_Trace(UART_PACKET(EVT_MUTEX_LOCK, CurrentTask - task_array, OS_Ticks));
	}
	else {
		if (CurrentTask->priority > Mx->owner->priority) {Mx->owner->priority = CurrentTask->priority;} //Elevate the owner if it has a lower priority
		CurrentTask->state = BLOCKED;
		UART_Trace(UART_PACKET(EVT_MUTEX_LOCK, CurrentTask - task_array, OS_Ticks));
		Mx->waiting_tasks[Mx->TaskTracker] = CurrentTask;
		Mx->TaskTracker++;
		SCB->ICSR |= SCB_ICSR_PENDSVSET_Msk; //bit 28 of INTCTRL Register. Mask is 0x10000000 - This allows for a context switch immediately after the block
	}

} 

void Mutex_Unlock(Mutex* Mx) { //Releasing the Mutex
	
	if (CurrentTask == Mx->owner) {
		
		Mx->owner->priority = Mx->OriginalPriority; //Restore original priority
		
		if (Mx->TaskTracker > 0) {
		
			int8_t MaxPriority = -1;
			int8_t MaxPIndex = -1;
		
			//Find the Highest priority task from the waiting_task array and wake it up
			for (int i = 0; i < Mx->TaskTracker; i++) {
			
				if (Mx->waiting_tasks[i]->priority > MaxPriority) {
					MaxPriority = Mx->waiting_tasks[i]->priority;
					MaxPIndex = i;
				}
				
			}
		
			TCB* WinnerTask = Mx->waiting_tasks[MaxPIndex];
			WinnerTask->state = READY; //Wake the task at the index which had the highest priority task
			UART_Trace(UART_PACKET(EVT_MUTEX_UNLOCK, CurrentTask - task_array, OS_Ticks));
			SCB->ICSR |= SCB_ICSR_PENDSVSET_Msk; //bit 28 of INTCTRL Register. Mask is 0x10000000 - This allows for a context switch
		
			//Fill the hole
			for (int j = MaxPIndex; j < Mx->TaskTracker - 1; j++) {
				Mx->waiting_tasks[j] = Mx->waiting_tasks[j + 1];
			}
		
			Mx->waiting_tasks[Mx->TaskTracker - 1] = NULL;
			Mx->owner = WinnerTask;
			Mx->TaskTracker--;
			Mx->counter = 0;
	}
		
	else {
		Mx->counter = 1;
		Mx->owner = NULL;
		UART_Trace(UART_PACKET(EVT_MUTEX_UNLOCK, CurrentTask - task_array, OS_Ticks));
	}
		
	}
	else {
		OS_Fault();
	}
	
}

