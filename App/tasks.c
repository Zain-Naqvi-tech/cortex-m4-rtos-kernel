#include "tasks.h"
#include "tm4c1294ncpdt.h"
#include "SysTick.h"
#include "os_malloc.h"
#include "uart.h"
void MallocTest(void) {
   
		uint32_t deltas[20];
    uint32_t count = 0;
   
    while (count < 20) {
       
				uint32_t T0 = DWT->CYCCNT;
        uint32_t* arr = (uint32_t*)os_malloc(50);
				uint32_t T1 = DWT->CYCCNT;
        if (arr == NULL) { break; }
				deltas[count] = T1 - T0;
        count++;
       
    }
   
    for (volatile uint32_t i = 0; i < count; i++) { UART_numeric_print(deltas[i]); GPIO_PORTF_DATA_R ^= 0x01; OS_Sleep(500); }
   
    while (1) { OS_Sleep(1000); }
   
}
void task2(void) { //Blinks LED2
    while (1) {
        //GPIO_PORTN_DATA_R ^= 0x01; //Toggle First Bit for LED2
        OS_Sleep(500);
    }
}
void task3(void) { //Blinks LED3
    while (1) {
        //GPIO_PORTF_DATA_R ^= 0x10; //Toggle Fourth Bit for LED3
        OS_Sleep(500);
    }
}
void task4(void) { //Blinks LED4 (Used as a system heartbeat in this malloc test)
    while (1) {
        //GPIO_PORTF_DATA_R ^= 0x01; //Toggle First Bit for LED4
        OS_Sleep(500);
    }
}
void idleTask(void) {
   
    while (1) { //Runs infinitely
       
    }
   
}