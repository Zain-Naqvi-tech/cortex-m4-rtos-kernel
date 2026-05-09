//SysTick.c
#include "SysTick.h"
#include "tm4c1294ncpdt.h"
#include <stddef.h>
#include "msp432e401y.h"


void SysTick_Init(void) {
	
	//Configure STRELOAD
	NVIC_ST_RELOAD_R = 119999; //For 1ms delay, we need 120,000 cycles. This can be found using 1ms/8.33ns at 120MHz clock speed. 
	
	//Configure STCURRENT
	NVIC_ST_CURRENT_R = 5; //writing any value to it to clear and reset the counter to zero before starting.  
	
	NVIC_SetPriority(PendSV_IRQn, 0xFF); //Set PendSV to the lowest possible priority. 
	
	//Configure STCTRL - Enable systick counter, enable the SysTick Interrupt, and select the clock source. 
	NVIC_ST_CTRL_R |= 0x01; //Bit 0 set - Enables SysTick to operate in a multi-shot way.
	NVIC_ST_CTRL_R |= 0x02; //Bit 1 set - Enables the interrupt
	NVIC_ST_CTRL_R |= 0x04; //Bit 2 set - Selects the System Clock
	
}

void SysTick_Handler(void) {
	
	//PendSV Set Pending
	SCB->ICSR |= SCB_ICSR_PENDSVSET_Msk; //bit 28 of INTCTRL Register. Mask is 0x10000000
	
}