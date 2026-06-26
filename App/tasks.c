#include "tasks.h"
#include "tm4c1294ncpdt.h"
#include "SysTick.h"
#include "os_malloc.h"
#include "uart.h"
void MallocTest(void) {
   
		uint32_t T0 = DWT->CYCCNT;
    uint32_t* arr = (uint32_t*)os_malloc(5 * sizeof(uint32_t)); //Allocating memory for 5 uint32_t integers
		uint32_t T1 = DWT->CYCCNT;
		uint32_t delta = T1 - T0;
		UART_numeric_print(delta);
    if (arr == NULL) { OS_Fault(); }
    arr[0] = 4;
    if (arr[0] == 4) { GPIO_PORTN_DATA_R ^= 0x01; } //led 2
   
    os_free(arr);
   
    uint32_t* array = (uint32_t*)os_malloc(5 * sizeof(uint32_t));
   
    array[0] = 67;
    if (array[0] == 67) { GPIO_PORTF_DATA_R ^= 0x01; } //LED 4
   
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