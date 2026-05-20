#include "os_core.h"
#include <stddef.h>
#include "msp432e401y.h"

#define INITIAL_XPSR 0x01000000 //xPSR - bit 24 must be set in order to avoid a hard fault. 

TCB* CurrentTask = NULL; //Added after linker error

TCB task_array[NUMBER_OF_TASKS]; //Added after linker error

//Zeros out the psp's of every frame. NULLs out the currentTask pointer
void Init_OS(void) {
	
	for (int i = 0; i < NUMBER_OF_TASKS; i++) {
		task_array[i].psp = NULL;
		task_array[i].state = READY;
		task_array[i].ticks = 0;
	}
	CurrentTask = NULL;
	
}

//fake a stack frame so that when pendSV runs for the very first time and tries to restore a context, there's something valid to restore. 
void Create_Task(uint8_t index, void (*task_function)(void), uint8_t priority) {

	//find the top of stack - This is the last element of tcb_array
	uint32_t* stackTop = &task_array[index].tcb_array[TASK_STACK_SIZE - 1];
	
	//This is the core of the program. When PendSV fires and does an exception return, the CPU hardware automatically pops 8 registers off the PSP stack in the order of 
	//PSR, PC, LR, R12, R3, R2, R1
	//We need to manually pre-fill these so the first 'return' has something valid. 
	
	task_array[index].psp = stackTop - 16; //we are 16 behind the absolute ceiling. When the CPU wakes up this task and pops 16 registers, it perfectly walks back up to the ceiling without overflowing. 
	
	//xPSR
	task_array[index].psp[15] = INITIAL_XPSR; //first to get popped off
	
	//PC - points to the instruction to be done. 
	task_array[index].psp[14] = (uint32_t)task_function; 
	
	//LR - tells the CPU to return to Thread mode on PSP. 
	task_array[index].psp[13] = 0xFFFFFFFD;
	
	//R12
	task_array[index].psp[12] = 0;
	
	//R3
	task_array[index].psp[11] = 0;
	
	//R2
	task_array[index].psp[10] = 0;
	
	//R1
	task_array[index].psp[9] = 0;
	
	//R0
	task_array[index].psp[8] = 0;
	
	//Now we need to fake the software frame (R4-R11)
	for (int i = 7; i >= 0; i--) {
		task_array[index].psp[i] = 0;
	}		
	
	task_array[index].state = READY;
	
	task_array[index].priority = priority; 
	
	task_array[index].tcb_array[0] = 0xDEADBEEF; //Canary Value to check stack overflow
	
}

//Launches the very first task. the CPU boots using MSP in privileged handler mode. Tasks need to run on the PSP in thread mode. This function allows that switch to happen
/* We trick the CPU into jumping from the OS in Handler mode to the very first task in Thread mode. We remain
on the MSP. If we trigger SVC, the CPU panics and pushes 8 registers onto whatever stack it is currently on. 
Initially, we were switching to the PSP before SVC. Therefore, the CPU shoved garbage state of the Start_OS function on top
of the task1 stack, destroying it. By remaining on the MSP, the CPU safely dumps that garbage data onto the main OS stack.*/
void Start_OS(void) {
	//we point the current task pointer (tcb pointer type) to the first task (first element in the list of tasks)
	CurrentTask = &task_array[0];
	
	//We need to load our first task's saved SP into the actual PSP register. we can use the CMSIS command __set_PSP(...)
	//Assign a new value to the PSP register - the top address of a task's stack memory. 
	__set_PSP((uint32_t)(CurrentTask->psp + 8));
	
	//Now we need to jump into the first task
	//We can't just call the first function 
	//We need to trigger the same exception-return mechanism that PendSV will use every other time
	//We want the CPU to pop that fake frame we built off the PSP
	//We use Supervisor Call (SVC) to trigger this
	__asm("SVC #0"); //Triggers the SVC exception; the assembly handler will then force the switch to the PSP.
	
	
}

//Advance CurrentTask to the next task in task_array, wrapping back to 0
void OS_Schedule (void) {
	
	uint32_t nextIndex;
	uint32_t currentIndex;
	uint32_t indexTracker;
	
	//Before picking the next task, the outgoing task should be READY
	if (CurrentTask->state == RUNNING) {
		CurrentTask->state = READY;
	}
	
	currentIndex = CurrentTask - task_array; //Pointer Subtraction to find the exact index of currentTask in task_array
	
	indexTracker = currentIndex; 
	
	int8_t highest_priority_yet = -1;
	
	for (int i = 1; i <= NUMBER_OF_TASKS; i++) {
		nextIndex = (currentIndex + i) % NUMBER_OF_TASKS; //Finds the next index using Modulo Operator for wrap-around logic.
		if (task_array[nextIndex].state == READY && task_array[nextIndex].priority > highest_priority_yet) {
			highest_priority_yet = task_array[nextIndex].priority;
			indexTracker = nextIndex;
		}
	}
	
	CurrentTask = &task_array[indexTracker]; //CurentTask now points to the next task in line
	CurrentTask->state = RUNNING;
	
}

void OS_Sleep(uint32_t ticks) {

	CurrentTask->ticks = ticks;
	CurrentTask->state = BLOCKED; //Blocks the task
	SCB->ICSR |= SCB_ICSR_PENDSVSET_Msk; //bit 28 of INTCTRL Register. Mask is 0x10000000. Scheduler picks someone else

}

