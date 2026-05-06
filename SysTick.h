//Poke PendSV and get out
#ifndef SYSTICK_H
#define SYSTICK_H

#include <stdint.h>
// Initialize SysTick with busy wait running at bus clock.
void SysTick_Init(void);

#endif