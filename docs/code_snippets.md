# Code Snippets — RTOS Testing Phase

---

## Priority Inversion Test

How I set up the tasks.

```c
#include "tasks.h"
#include "tm4c1294ncpdt.h"
#include "SysTick.h"
void task1(void) { //Blinks LED1
    while (1) {
        Mutex_Lock(&Mx);
        GPIO_PORTN_DATA_R ^= 0x02; //Toggle Second Bit for LED1
                for (volatile uint32_t i = 0; i < 3000000; i++) {}
        Mutex_Unlock(&Mx);
                OS_Sleep(100);
    }
}
void task2(void) { //Blinks LED2
    OS_Sleep(500);
    while (1) {
        GPIO_PORTN_DATA_R ^= 0x01; //Toggle First Bit for LED2
                for (volatile uint32_t i = 0; i < 1000000; i++) {}
    }
}
void task3(void) { //Blinks LED3
    OS_Sleep(1000);
    while (1) {
        Mutex_Lock(&Mx); //shouldnt be able to access
        GPIO_PORTF_DATA_R ^= 0x10; //Toggle Fourth Bit for LED3
        for (volatile uint32_t i = 0; i < 500000; i++) {}
        Mutex_Unlock(&Mx);
                    OS_Sleep(500);
    }
}
void task4(void) { //Blinks LED4
    while (1) {
        GPIO_PORTF_DATA_R ^= 0x01; //Toggle First Bit for LED4
        OS_Sleep(250);
    }
}
void idleTask(void) {
    while (1) { //Runs infinitely
    }
}
```

---

## Semaphore Producer Consumer Test

```c
#include "tasks.h"
#include "tm4c1294ncpdt.h"
#include "SysTick.h"
void ConsumerTask (void) {
    while (1) {
        OS_Wait(&smp); //Sets the counter to 0 if it is at 1. If it is at 0, block the current task and put it on the waiting_tasks array
        GPIO_PORTN_DATA_R ^= 0x02;
        OS_Sleep(100);
    }
   
}
   
void ProducerTask(void) {
    while (1) {
        OS_Sleep(500); //Sleep for some time to simulate waiting for a message
        OS_Signal(&smp); //Calls the OS_Signal function on the semaphore which sets the counter to 1 and works on waiting tasks
        GPIO_PORTN_DATA_R ^= 0x01; //Turn ON status LED
        OS_Sleep(100);
    }
}
void task3(void) { //Blinks LED3
    while (1) {
        //GPIO_PORTF_DATA_R ^= 0x10; //Toggle Fourth Bit for LED3
        OS_Sleep(500);
    }
}
void task4(void) { //Blinks LED4
    while (1) {
        //GPIO_PORTF_DATA_R ^= 0x01; //Toggle First Bit for LED4
        OS_Sleep(500);
    }
}
void idleTask(void) {
   
    while (1) { //Runs infinitely
       
    }
   
}
```

---

## Heap Allocation — Test 1

Testing malloc and free.

```c
#include "tasks.h"
#include "tm4c1294ncpdt.h"
#include "SysTick.h"
#include "os_malloc.h"
void MallocTest(void) {
   
    uint32_t* arr = (uint32_t*)os_malloc(5 * sizeof(uint32_t)); //Allocating memory for 5 uint32_t integers
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
```

As a result, the OS was able to allocate memory and be written to and be read from. The LEDs lit up as expected and did not throw an OS_Fault(). Therefore, the basic malloc and free both work.

---

## Heap Allocation — Test 2

We are allocating multiple places, filling them up, and checking for overlap.

```c
#include "tasks.h"
#include "tm4c1294ncpdt.h"
#include "SysTick.h"
#include "os_malloc.h"
void MallocTest(void) {
   
    //allocating 20 bytes of memory
    uint32_t* arr1 = (uint32_t*)os_malloc(5 * sizeof(uint32_t)); //Allocating memory for 5 uint32_t integers
    if (arr1 == NULL) {
        OS_Fault(); //If the allocation fails, we fault the system.
    }
    for (uint32_t i = 0; i < 5; i++) {
        arr1[i] = i + 1; //Filling the allocated memory with values from 1 to 5
    }
    //allocating 4 bytes of memory
    uint32_t* arr2 = (uint32_t*)os_malloc(sizeof(uint32_t)); //Allocating memory for 1 uint32_t integer
    if (arr2 == NULL) {
        OS_Fault(); //If the allocation fails, we fault the system.
    }
    arr2[0] = 10;
    //allocating 8 bytes of memory
    uint32_t* arr3 = (uint32_t*)os_malloc(2 * sizeof(uint32_t)); //Allocating memory for 2 uint32_t integers
    if (arr3 == NULL) {
        OS_Fault(); //If the allocation fails, we fault the system.
    }
    arr3[0] = 15;
    arr3[1] = 20;
   
    //now we need to test for overlap
    if (arr2 < (arr1 + 5)) { OS_Fault(); } //We know that arr2 must start AFTER arr1 + 5 integer spaces - if it is less than the end of arr2's block, they overlap and it's faulty
   
    if (arr3 < (arr2 + 1)) { OS_Fault(); } //We know that arr3 must start AFTER arr2's one integer space - if it is less than the end of arr2's block, they overlap and it's faulty
   
    //Check if the values are still the same for arr1
    for (uint32_t i = 0; i < 5; i++) {
        if (arr1[i] != (i + 1)) {
            OS_Fault();
        }
    }
   
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
```

---

## Heap Allocation — Test 3

```c
#include "tasks.h"
#include "tm4c1294ncpdt.h"
#include "SysTick.h"
#include "os_malloc.h"
void MallocTest(void) {
   
    uint32_t count = 0;
   
    while (1) {
       
        uint32_t* arr = (uint32_t*)os_malloc(50);
        if (arr == NULL) { break; }
        count++;
       
    }
   
    for (volatile uint32_t i = 0; i < count; i++) { GPIO_PORTF_DATA_R ^= 0x01; OS_Sleep(500); }
   
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
```
