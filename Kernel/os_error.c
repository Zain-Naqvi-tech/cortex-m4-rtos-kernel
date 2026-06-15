#include "os_error.h"

void OS_Fault(void) {
	
		__disable_irq(); //Disable all interrupts
		volatile int i; //volatile counter variable
		while (1) {
			GPIO_PORTN_DATA_R ^= 0x02; //Toggle Second Bit for LED1
			for (i = 0; i < 1000000; i++) {} 
		}
	
}