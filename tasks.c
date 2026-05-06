#include "tasks.h"
#include "tm4c1294ncpdt.h"
#include "SysTick.h"

void task1(void) { //Blinks LED1
	volatile int i; //volatile counter variable
	while (1) {
		GPIO_PORTN_DATA_R ^= 0x02; //Toggle Second Bit for LED1
		for (i = 0; i < 1000000; i++) {} //gets replaced by OS_Sleep()
	}
}

void task2(void) { //Blinks LED2
	volatile int i; 
	while (1) {
		GPIO_PORTN_DATA_R ^= 0x01; //Toggle First Bit for LED2
		for (i = 0; i < 5000000; i++) {} //gets replaced by OS_Sleep()
	}
}

void task3(void) { //Blinks LED3
	volatile int i;
	while (1) {
		GPIO_PORTF_DATA_R ^= 0x10; //Toggle Fourth Bit for LED3
		for (i = 0; i < 1000000; i++) {} //gets replaced by OS_Sleep()
	}
}

void task4(void) { //Blinks LED4
	volatile int i;
	while (1) {
		GPIO_PORTF_DATA_R ^= 0x01; //Toggle First Bit for LED4
		for (i = 0; i < 5000000; i++) {} //gets replaced by OS_Sleep()
	}
}