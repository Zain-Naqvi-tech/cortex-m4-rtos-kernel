#include "itm.h"

void ITM_Init(void) {
	CoreDebug->DEMCR |= DCB_DEMCR_TRCENA_Msk; //Sets bit 24 of the Debug Exception and Monitor Control Register to turn on the trace subsystem
	ITM->LAR = 0xC5ACCE55; //Unlocks the LAR (Lock Access Register) in the ITM. Signals that LSR (Lock Status Register) must permit writing to the other registers in the component
	ITM->TCR |= ITM_TCR_ITMENA_Msk | (1 << 16); //Trace control set
	ITM->TER |= 1; //Trace enable port 0
}



void ITM_Write(uint32_t data) {
	if (ITM->PORT[0].u32 != 0) { //If it is zero, the port is busy or ITM is disabled, so we do not write
		ITM->PORT[0].u32 = data;
	}
	else {
		return;
	}
}