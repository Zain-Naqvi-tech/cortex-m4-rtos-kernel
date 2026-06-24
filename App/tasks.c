#include "tasks.h"
#include "tm4c1294ncpdt.h"
#include "SysTick.h"
#include "dwt.h"
#include <stdint.h>
#include "uart.h"

void task1(void) { //Blinks LED1
	while (1) {
		GPIO_PORTN_DATA_R ^= 0x02; //Toggle Second Bit for LED1
		uint32_t T0 = DWT->CYCCNT; 
		OS_Sleep(500);
		uint32_t T1 = DWT->CYCCNT;
		uint32_t timeDiff = T1 - T0;
		UART_numeric_print(timeDiff);
	}
}

void task2(void) { //Blinks LED2
	while (1) {
		GPIO_PORTN_DATA_R ^= 0x01; //Toggle First Bit for LED2
		OS_Sleep(500);
	}
}

void task3(void) { //Blinks LED3
	while (1) {
		GPIO_PORTF_DATA_R ^= 0x10; //Toggle Fourth Bit for LED3
		OS_Sleep(500);
	}
}

void task4(void) { //Blinks LED4
	while (1) {
		GPIO_PORTF_DATA_R ^= 0x01; //Toggle First Bit for LED4
		OS_Sleep(500);
	}
}

void idleTask(void) {
	
	while (1) { //Runs infinitely
		
	}
	
}