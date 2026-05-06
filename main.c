#include <stdint.h>
#include "PLL.h"
#include "SysTick.h"
#include "tm4c1294ncpdt.h"
#include "os_core.h"
#include "tasks.h"

void PortN_Init(void) {
	
	SYSCTL_RCGCGPIO_R |= SYSCTL_RCGCGPIO_R12; //enabling clock for Port N
	while((SYSCTL_PRGPIO_R&SYSCTL_PRGPIO_R12) == 0){}; //allow time for clock
	
	GPIO_PORTN_DIR_R |= 0x03; //Enables the first and second bits to be outputs
	GPIO_PORTN_DEN_R |= 0x03; //Enables Digital I/O on Port N
		
	return; 

}

void PortF_Init(void) {
	
	SYSCTL_RCGCGPIO_R |= SYSCTL_RCGCGPIO_R5; //enabling clock for Port F
	while((SYSCTL_PRGPIO_R&SYSCTL_PRGPIO_R5) == 0){}; //allow time for clock
	
	GPIO_PORTF_DIR_R |= 0x11; //Enables the first and fourth bits to be outputs
	GPIO_PORTF_DEN_R |= 0xFF; //Enables Digital I/O on Port F
	
}

int main(void) {

	PLL_Init(); //Sets the clock speed at 120MHz
	PortN_Init(); //Initializes Port N
	PortF_Init(); //Initializes Port F
	Init_OS(); //Zeros out the TCBs, and nulls the CurrentTask
	Create_Task(0,task1); //Populate the fake stack for task 1
	Create_Task(1,task2); //Populate the fake stack for task 2
	Create_Task(2,task3); //Populate the fake stack for task 3
	Create_Task(3,task4); //Populate the fake stack for task 4
	SysTick_Init(); 
	Start_OS();
	
}