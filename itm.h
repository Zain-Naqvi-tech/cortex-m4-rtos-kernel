#ifndef ITM_H
#define ITM_H

#include "tm4c1294ncpdt.h"
#include "msp432e401y.h"
#include "core_cm4.h"
#include "system_msp432e401y.h"

// Packet Format: [31:24] event type (top 8 bits) | [23:16] task index (Next 8 bits) | [15:0] tick counter (Bottom 16 bits)
#define ITM_PACKET(event, index, ticks) (((uint32_t)(event) << 24) | ((uint32_t)(index) << 16) | ((uint16_t)(ticks)))

//The following definitions account for any task that our kernel might try to complete. 
#define ITM_EVT_TASK_SWITCH   0x01 //Context Switch
#define ITM_EVT_SEM_WAIT      0x02 //Semaphore Wait()
#define ITM_EVT_SEM_SIGNAL    0x03 //Semaphore Signal()
#define ITM_EVT_MUTEX_LOCK    0x04 //Mutex Lock()
#define ITM_EVT_MUTEX_UNLOCK  0x05 //Mutex Unlock()
#define ITM_EVT_MALLOC        0x06 //malloc (memory allocation on the heap)
#define ITM_EVT_FREE          0x07 //free (freeing the allocated memory on the heap)

void ITM_Init(void);
void ITM_Write(uint32_t data);

#endif