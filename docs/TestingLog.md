# Testing Log

**Project Overview:** This arm-cortex based RTOS (Real-time operating system) features a preemptive scheduler, context-switching, sleep delays. malloc/free, mutexes, and semaphores as a whole package. It is used to differentiate between tasks of different priorities and run tasks in a round-robin fashion. 

This document serves as a testing log to record test results, lessons learned, and any subsequent changes made to the RTOS.

OS_Fault() is referred to commonly in this document. This function disables all interrupts and flashes LED1 very fast. It also blocks all other tasks consequently. This is an effective way to see physically that the system has thrown an error.  

---

## Verification: Round-Robin with 4 LED Tasks

4 tasks were created, each toggling a different onboard LED on the cortex board. This was done after the context-switching mechanism was created. 

**Purpose:** Test the basic RTOS Functionality. 

**Test:** Toggling LEDs at different rates using for loops (eats up CPU cycles)

**Initial Observation:** All LEDs turned ON and remained ON. None of them blinked. The program was traced using the CMSIS-DAP debugger. The system was throwing a HardFault as soon as OS_Start() was called. This meant that the context-switch caused a HardFault. 

**Root Cause:** I initially attempted to trigger the context switch using a supervisor call (SVC) after manually flipping the CONTROL register to use the PSP. Because the hardware exceptions immediately push 8 registers to the active stack, the CPU violently pushed the garbage state of the OS_Start() directly onto the new PSP stack frame, corrupting the initial PC. 

**The Fix:** Removed the CONTROL register manipulation. By only executing the SVC #0 call, while still on the MSP, the CPU safely saved the garbage state to the MSP. Inside my SVC_Handler, loading the needed value into LR (EXC_RETURN) and executing BX LR forced the CPU to switch to the PSP and pop the uncorrupted stack frame. 

**Status:** Success

---

## Verification: OS_Sleep & Priority Scheduling

**Purpose:** Test the RTOS functionality after the OS_Sleep() function implementation. This function replaced the time-delaying for loops in task functions. It ensures that the task is blocked and stays away until it is done sleeping. While it sleeps, other tasks can run, based on priority. 

**Initial Observation:** LED output was inconsistent. Some were stuck at ON and some would blink erratically. 

**Root Cause:** When all active applications called OS_Sleep(), the system’s READY list became entirely empty. This is why the index tracker did not update properly. Therefore, the OS forced the last sleeping task back into the RUNNING state, ignoring the sleep timer. 

**Fix:** Implemented an Idle Task, our 5th task. It was hard coded directly into the Init_OS() boot sequence and placed at the final index of task_array. It is just an infinite loop and has the lowest priority of 0. 

**Status:** Success

---

## Verification: Preemption

**Purpose:** Prove the stability of the priority scheduler, a mixed-priority test. 

**Test:** 4 distinct tasks (blinking LEDs). Three have the same priority. One has the highest priority. One task with the same priority as the other two, has no sleep function. Gets stuck in an infinite loop after single LED toggle. 

**Proof:** Preemption was confirmed. LED4 blinked perfectly at the chosen delay. LED 1 and 2 blinked synchronously at their delay rate. The schedler correctly identified the priority tie. LED3 turned ON once and remained solidly ON. This shows that the OS gave it CPU time, but the task simply spun in an empty void, proving that tasks without sleep commands will consume available cycles without altering states. 

**Extra Observation:** An intentional starvation test was conducted. Task 3 was set to the highest priority with no OS_Sleep. Task 1,2,4 set to lower priorities. LEDs 1 and 3 remained permanently ON, while LEDs 2 and 4 never activated. Task 1 booted first, toggles ON. It was then immediately preempted by the first SysTick hardware interrupt. The scheduler identified task 3 as the highest priority, and handed it the CPU. Because Task 3 has an infinite loop, it took the CPU at 100%. Task 1 was left permanently frozen in the exact state it was preempted, demonstrating system starvation. 

**Status:** Success

---

## Verification: Priority Inversion

This was the most time-consuming and debugging-heavy test. Let’s dig deeper into what was going on.

**Purpose:** Check the RTOS mutex implementation by creating a situation where priority inheritance would come into play. 

**Test:** Task 1 was given the lowest priority. It locked the mutex as soon as the program ran, turned on LED1, and ran a for loop which hogged up CPU time. It then unlocks the mutex and OS_Sleep runs for 100ms. 

Task 2 was designed to be a higher priority task which hogs the CPU without ever giving it up. Keeps running the loop. It started after a bit, so task 1 was able to run and get its Mutex locked. 

Task 3 was the highest priority task out of the three. This was the main component in testing priority inheritance. It sleeps for 1s before turning ON to ensure task 2 can safely preempt task 1. Now, task 3 tried to access the mutex, but could not because it was still taken by Task 1. Here, once Mutex_Lock is called, it elevates Task 1’s priority to that of Task 3. This ensures that at the context switch, Task 1 runs which preempts task 2. Then, it released the mutex which meant Task 3 can now use it on the next iteration, lock it, run the task, and unlock it.

This would be reflected in real-time by seeing that Task 1 stays ON forever, as it never has a high priority after turning ON, other than those few milliseconds to release the mutex. Tasks 2 and 3 should be running as needed, and task 4 which was just an extra LED, runs at its own pace as well (Highest of all priorities, not linked to the priority inversion test - used as a system heartbeat). 

**Initial Observation:** All 4 LEDs were used for the testing. The first iteration of testing constantly resulted in the system going into the custom OS_Fault() function. The OS_Fault function disables all interrupts and starts flashing LED1 at a high speed. Upon debugger tracing, it was determined that Mutex_Unlock() was the problem. 

**Root Cause:** When unlocking a mutex, the new task must update the Mutex’s original priority as well. This was figured out using the debugger which tracked priority variables throughout. Another issue was calling a bare-metal context switch before making all changes to the Mutex and its components. 

**Fix:** Mx->originalPriority = WinnerTask->priority; line was added in Mutex_Unlock(&Mx) function after the highest priority task from the waiting_tasks array was identified. Then, the context switch was moved to the end of the if block in order to let the Mutex update properly before calling the switch. 

**Status:** Success. 

---

## Verification: Semaphore Producer-Consumer Test

**Purpose:** Test the semaphore functions of this RTOS implementation. In order to test this effectively, we need to come up with some imaginary context. So, let’s say we are working in a vehicle system. 

**Test:** When the system boots, the high-priority Consumer runs first and calls OS_Wait() to block itself (counter starts off at 0). The Producer runs and sleeps. After the sleeping period ends (500ms in this case), it wakes up and calls OS_Signal(). It increments the counter, sees the consumer waiting, and marks it as READY. Because the consumer has a higher priority, the kernel context-switches to the consumer, runs its LED task, loops around, and hits OS_Wait() to sleep. The CPU is then handed to the Producer which toggles its LED. Based on this code, we expect LED 1 and LED 2 to blink together, at the same speeds. 

**Initial Observation:** No LED turns ON. 

**Root Cause:** Despite the code progressing and moving forward in the debugger, there was an initialization issue. Only two tasks were created and running out of the 4 available tasks on the tcb_array. This meant that there were two spots with garbage data. This did not allow the two existing tasks to run. Another issue was no context switch used in semaphores. That was more of a code miss than a proper fault. 

**Fix:** For testing purposes, two additional tasks were added with basic OS_Sleep() functions in a while(1) loop. As a result, the two LEDs for the consumer and producer started flashing as expected. As a proper fix, the initialisation of the tasks in the array needs to include a way to make unused slots to be blocked with a priority of 0. 

**Status:** Success

---

## Verification: Heap Allocation (os_malloc(size))

**Purpose:** Test the RTOS Heap allocator. Make sure it is effectively allocating memory, jumping busy blocks, making use of free space, and not corrupting memory/addresses. 

**Test 1:** Basic malloc, filling up the allocated space, freeing, and then reallocating the same space, repeating the cycle to check for any fragmentation or overwrite. The success indicators were checking if the data in the allocated space matches the hardcoded value. The onboard LEDs were used for debugging and checking for any errors. The case where malloc throws NULL was also tested. OS_Fault() was never hit, which meant the allocations were successful the first time. 
**Status:** Success

**Test 2:** Allocate multiple blocks sequentially: allocating 20 bytes, filling them up with known values, allocating 4 bytes, filling them with known values, then allocating 8 bytes, filling them with known values as well. Any unsuccessful allocations were programmed to trigger the OS_Fault() function which was never reached.  

The main check for this Test was to ensure there is no overlapping of addresses and no overwriting of values. The debugger watch windows were used to see all three allocated arrays. Their addresses were different, and their specific values were never overwritten. 
**Status:** Success

**Test 3:** Continuously allocate memory on the heap until it runs out of space and consequently return NULL. We need to check if the space is being properly utilised and if the math works. Therefore, 50 bytes of blocks were allocated in a while loop until the allocation returned NULL. A counter kept track of every iteration. This counter variable was then used to run a loop and toggle an LED. Every block was 58 bytes of data. Therefore, the expected blinks were 17. The desired results were achieved. To further clarify, the code was run in the debugger and the final value of the counter variable was read which came out to be 17. 
**Status:** Success