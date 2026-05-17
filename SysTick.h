//Poke PendSV and get out
#ifndef SYSTICK_H
#define SYSTICK_H

#include "tm4c1294ncpdt.h"
#include <stddef.h>
#include "msp432e401y.h"
#include "os_core.h"
#include <stdint.h>
// Initialize SysTick with busy wait running at bus clock.
void SysTick_Init(void);

#endif