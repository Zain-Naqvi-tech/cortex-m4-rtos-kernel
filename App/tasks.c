#include "tasks.h"
#include "tm4c1294ncpdt.h"
#include "SysTick.h"
#include "dwt.h"
#include <stdint.h>
#include "uart.h"

volatile uint32_t pendSV0;
volatile uint32_t pendSV1;
volatile uint32_t delta;

void task1(void) { //Blinks LED1
	while (1) {
		GPIO_PORTN_DATA_R ^= 0x02; //Toggle Second Bit for LED1
		OS_Sleep(500);
	}
}

void task2(void) { //Blinks LED2
	while (1) {
		UART_numeric_print(pendSV1 - pendSV0);
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