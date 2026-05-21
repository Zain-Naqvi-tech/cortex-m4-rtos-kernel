//This file will be the core of the Operating System
//We need to define a normal OS's features before going into real-time OS. 
#ifndef OS_CORE_H
#define OS_CORE_H

#include <stdint.h>
#define TASK_STACK_SIZE 100
#define NUMBER_OF_TASKS 5

//STATES Enumeration
typedef enum {
		
	READY, //0
	RUNNING, //1
	BLOCKED //2
		
} STATE; 

//TASK CONTROL BLOCK STRUCT
typedef struct {
	
	//Stack pointer (Process Stack Pointer)
	uint32_t* psp; 
	
	//stack array - physical chunk of RAM that this specific task will use to store its local variables and frozen registers. 
	uint32_t tcb_array[TASK_STACK_SIZE];
	
	//State ENUM
	STATE state; 
	
	//Integer value for task priority
	uint8_t priority;

	//Sleep ticks used to track the cycles for which the task sleeps
	uint32_t ticks;
	
} TCB; //A task control block is a central place for the OS to remember exactly who this task is and what it was doing. 

//GLOBAL TRACKING VARIABLES

//Pointer to the current task
extern TCB* CurrentTask; //use extern to ensure it can be used by other files

//Array of TCB structs. This holds the number of tasks
extern TCB task_array[NUMBER_OF_TASKS];

//OS FUNCTIONS
void Init_OS(void);
void Create_Task(uint8_t index, void (*task_function)(void), uint8_t priority); //need to add WHAT task and WHICH slot to use 
void Start_OS(void);
void OS_Schedule(void); 
void OS_Sleep(uint32_t ticks);

#endif