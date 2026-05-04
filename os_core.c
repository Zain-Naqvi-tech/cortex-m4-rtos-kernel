#include "os_core.h"
#include <stddef.h>
#include "msp432e401y.h"

#define INITIAL_XPSR 0x01000000 //xPSR - bit 24 must be set in order to avoid a hard fault. 

//Zeros out the psp's of every frame. NULLs out the currentTask pointer
void Init_OS(void) {
	
	for (int i = 0; i < NUMBER_OF_TASKS; i++) {
		task_array[i].psp = NULL;
	}
	CurrentTask = NULL; 
	
}

//fake a stack frame so that when pendSV runs for the very first time and tries to restore a context, there's something valid to restore. 
void Create_Task(uint8_t index, void (*task_function)(void)) {

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
	
	
	
	
	
	
}

//Launches the very first task. the CPU boots using MSP in privileged handler mode. Tasks need to run on the PSP in thread mode. This function allows that switch to happen
//The CONTROL Register can help us in this. Bit 1 is the ASP - This bit reads 0 and ignores writes - Active Stack Pointer in handler mode. Bit 0 is Thread mode Privilege and Bit 2 is for floating-point Context Active (FPCA) (taken from TRM)
void Start_OS(void) {
	//we point the current task pointer (tcb pointer type) to the first task (first element in the list of tasks)
	CurrentTask = &task_array[0];
	
	//We need to load our first task's saved SP into the actual PSP register. we can use the CMSIS command __set_PSP(...)
	//Assign a new value to the PSP register - the top address of a task's stack memory. 
	__set_PSP((uint32_t)CurrentTask->psp);
	
	//Flip the CONTROL Register bit ONE to 1. 
	uint32_t control_bit = __get_CONTROL();
	control_bit = control_bit | 0x02; //0000 0010 (Bit 1 is set now)
	__set_CONTROL(control_bit);
	
	//We need to flush the CPU pipeline - switch takes effect immediately
	__ISB();
	
	//Now we need to jump into the first task
	//We can't just call the first function 
	//We need to trigger the same exception-return mechanism that PendSV will use every other time
	//We want the CPU to pop that fake frame we built off the PSP
	//We use Supervisor Call (SVC) to trigger this
	__asm("SVC #0");
	
	
}

