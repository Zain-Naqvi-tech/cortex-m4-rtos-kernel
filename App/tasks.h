#ifndef TASKS_H
#define TASKS_H

#include "Mutex.h"
#include "Semaphore.h"

extern Mutex Mx;
extern Semaphore smp;
extern volatile uint32_t delta;

void task1(void); //Blinks LED1
void task2(void); //Blinks LED2
void task3(void); //Blinks LED3
void task4(void); //Blinks LED4

void ConsumerTask(void);
void ProducerTask(void);

void MallocTest(void);

void idleTask(void); //Idle task with lowest priority

#endif