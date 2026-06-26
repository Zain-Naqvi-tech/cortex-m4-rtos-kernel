# cortex-m4-rtos-kernel

A bare-metal RTOS built from scratch on the ARM Cortex-M4 MSP432E401Y MCU. No HAL. No vendor RTOS libraries. Every line of kernel code was written and debugged by hand against the ARM architecture technical reference manual and the chip datasheet.

---

## What this is

Most embedded projects reach for FreeRTOS or a vendor SDK. This one does not. The goal was to understand WHAT an RTOS actually does at the hardware level and HOW a CPU saves and restores task state, HOW a scheduler decides who runs next, WHAT priority inheritance actually means in silicon, and HOW a heap allocator carves memory from a flat array.

The result is a working preemptive kernel running on real hardware, with four tasks (can be more), a priority scheduler, binary semaphores, mutexes with priority inheritance, a first-fit heap allocator, stack overflow detection, and a UART trace system for real-time kernel debugging.

---

## Milestones

| Version | Description | Status |
|---|---|---|
| v1.0 | Preemptive round-robin context switching, 4 tasks | Complete |
| v2.0 | Priority scheduling, semaphores, mutexes, heap, sleep | Complete |
| v3.0 | UART trace instrumentation, Testing | Complete |

---

## Kernel features

**Context switching**: SysTick fires every 1ms and pends PendSV. The PendSV handler, written in ARM assembly, saves R4–R11 to the current task's Process Stack Pointer, calls the C scheduler, then restores R4–R11 from the next task's stack. The hardware automatically handles R0–R3, R12, LR, PC, and xPSR on exception entry and exit. The kernel runs on the Main Stack Pointer; every task runs on its own isolated Process Stack Pointer.

**Priority scheduling**: Each TCB carries a priority field. The scheduler scans all READY tasks and selects the highest priority one. Equal-priority tasks share the CPU in round-robin order by scanning forward from the current task index.

**Binary semaphores**: Counter-based, with a priority-ordered wait queue. `OS_Wait()` blocks and context-switches immediately if the semaphore is unavailable. `OS_Signal()` wakes the highest-priority waiting task and triggers a reschedule.

**Mutexes with priority inheritance**: Ownership-tracked. When a high-priority task blocks on a mutex held by a lower-priority task, the owner's priority is temporarily elevated to the blocker's level. This prevents a medium-priority task from indefinitely starving both. Priority is restored on unlock, and ownership transfers directly to the next task waiting.

**Heap allocator**: First-fit, with block splitting. The heap is a 1024 Byte static array. Each allocation carves a block and leaves a correctly-sized free block for the remainder. `os_free()` marks the block header as available. `sizeof(Header)` is 8 bytes on this target due to struct alignment —> verified against actual allocation addresses in the debugger.

**Stack overflow detection**: `Create_Task()` writes `0xDEADBEEF` to `tcb_array[0]`, the lowest address in each task's stack. `SysTick_Handler` checks all task canaries every millisecond. A corrupted canary calls `OS_Fault()` immediately.

**OS_Fault**: Disables all interrupts, takes direct control of an LED, and blinks it in a tight infinite loop. The system halts. 

**OS_Sleep**: `OS_Sleep(N)` stores N in the TCB's tick counter, marks the task BLOCKED, and triggers an immediate context switch. `SysTick_Handler` decrements all BLOCKED tasks with non-zero tick counts each millisecond, marking them READY when the counter reaches zero. Zero busy-waiting.

**Idle task**: automatically created at the lowest priority (0) in `Init_OS()`. Guarantees the scheduler always has a valid READY task to select when every user task is sleeping or blocked. It is a simple while (1) loop.

**UART trace**: kernel hooks emit 32-bit packets over UART on every context switch, semaphore wait/signal, mutex lock/unlock, and malloc/free. Packet format: `[31:24] event type | [23:16] task index | [15:0] OS tick counter`.

---

## Architecture

### Boot sequence
```
PLL_Init()          — lock system clock at 120MHz
UART_Init()         — enable UART0 for trace output
PortN_Init()        — configure LED pins
PortF_Init()        — configure LED pins
OS_HeapInit()       — initialises the heap for malloc
Init_OS()           — zero TCBs, write canaries, create idle task
Mutex/Semaphore     — initialise any sync primitives
Create_Task(...)    — forge fake 16-word stack frames for each task
SysTick_Init()      — arm timer, set PendSV to lowest priority
Start_OS()          — load first task PSP, trigger SVC #0
```

`Start_OS` loads the first task's PSP, switches to PSP mode via the CONTROL register, and calls `SVC #0`. The SVC handler loads `0xFFFFFFFD` into LR and executes `BX LR`. This EXC_RETURN value tells the CPU to return to thread mode using PSP, which pops the fake hardware frame and jumps to the task function. The kernel never returns to `main`. 

PendSV runs at the lowest exception priority, ensuring it only fires after all other interrupts have completed. 

### Task stack layout (100 words per task)
```
tcb_array[0]         0xDEADBEEF canary
tcb_array[1..83]     usable stack space
tcb_array[84..91]    R4–R11 software save frame  (PendSV)
tcb_array[92..99]    xPSR, PC, LR, R12, R3–R0   (hardware, on exception entry)
```

On first launch, `Create_Task` pre-fills this frame with the task function address at PC and `0x01000000` at xPSR (Thumb bit set). The EXC_RETURN in `Start_OS` pops this frame and the CPU jumps directly into the task function.

---

## Stress tests and verified behaviour

**Context Switching & Scheduler Boot:** Executed `SVC #0` from the MSP to prevent hardware exception frame corruption on the PSP.
**Priority Scheduling & Starvation:** Verified a high-priority CPU-hog successfully freezes lower-priority tasks in their exact preempted states.
**Mutexes & Priority Inheritance:** Proven dynamic priority elevation when a low-priority task holds a mutex required by a high-priority task.
**Semaphore Signaling:** Confirmed zero-latency preemption using a Producer-Consumer test with LED toggling.
**Heap Allocator:** Tests confirmed `os_malloc` safely returns `NULL` without faulting after exactly 17 allocations of 50-byte blocks.

[Read the full raw testing log here](docs/TestingLog.md)

[Read the tasks.c code snippets used for each test here](docs/code_snippets.md)

---

## File structure
```
kernel/
  os_core.c / os_core.h      TCB, scheduler, OS_Sleep, OS_Init, Create_Task
  os_asm.s                   PendSV and SVC handlers in ARM Thumb assembly
  semaphore.c / semaphore.h  binary semaphores with priority wait queues
  mutex.c / mutex.h          mutexes with ownership and priority inheritance
  os_malloc.c / os_malloc.h  first-fit heap allocator with block splitting
  os_error.c / os_error.h    OS_Fault kernel panic handler
  systick.c / systick.h      SysTick init, tick handler, sleep countdown, canary checks
  pll.c / pll.h              120MHz PLL clock initialisation
  uart.c / uart.h            UART0 driver and UART_Trace packet sender

app/
  main.c                     hardware init, task creation, OS start
  tasks.c / tasks.h          demo and test task functions

While the logical architecture is separated into kernel/ and app/ domains, the physical repository is flattened. This ensures seamless compilation for anyone cloning the repo into Keil. 

```

---

## Building and flashing

Toolchain: Keil MDK v5, ARMCLANG v6.23  
Target: MSP432E401Y MCU
Debug probe: CMSIS-DAP over USB  
Flash: Keil debugger via SWD

Clone the repo, open the Keil project file, build, and flash. UART trace output on UART0 (PA0/PA1) at 115200 baud, PIOSC clock source.

---

## Engineering notes

**The CONTROL register and SVC**: The initial approach switched to PSP mode before triggering SVC. This caused the CPU to push the exception frame onto the newly-forged task stack, corrupting the fake PC value and jumping to garbage. Fix: remain on MSP through SVC. The CPU dumps the exception frame onto the main stack safely, and EXC_RETURN switches to PSP cleanly.

**PendSV ordering**: During mutex stress testing, `OS_Fault` fired when a task called `Mutex_Unlock` on a mutex it didn't own. Root cause: `SCB->ICSR |= PENDSVSET` was written before `Mx->owner = WinnerTask`. The context switch fired between those two lines, waking the winner before ownership transferred. Rule: trigger context switches only after all data structures are fully consistent.

**ITM/SWO trace**: The kernel instrumentation was designed around ARM ITM hardware trace with SWO output. During implementation it became clear that CMSIS-DAP does not support SWO, it only handles JTAG and SWD. The trace transport was pivoted to UART with the same 32-bit packet format. Lesson: verify hardware capabilities before building software around them.

**struct padding**: `sizeof(Header)` was assumed to be 5 bytes (4 + 1). The actual value on this target is 8 bytes due to alignment. The heap exhaustion test initially predicted 113 allocations of 4 bytes; the actual figure was different. Always measure `sizeof()` rather than summing field sizes manually.

---

## Measuring System Performance

The next step after confirming functional correctness is to check system performance. It will answer questions like 'how fast?' or 'how consistent?'

**Context Switch Latency**:

**SysTick Jitter**: Answers 'how consistent is the 1ms tick?'

**OS_Sleep(ticks) Accuracy**:
- Added a DWT CYCCNT Counter to check the time elapsed between sleep cycles for a task
- A reading was taken before `OS_SLEEP()` is called and a reading is taken right after
- The delta is calculated after which it is printed, using a new added function `UART_numeric-print()`. 
- It comes out to be 59998103 CPU cycles which is (59998103 cycles) / (120M cycles/second) = 499.984ms. This is really close to the desired value of 500ms. 

**Heap allocator timing**: 
- Added a DWT CYCCNT counter before and after the os_malloc() call. 
- The result comes out to be 119 cycles which is 0.9917us based on our Clock Speed of 120MHz. 
---
- Then, the same canbe done on the os_free() function. 
- The result comes out to be 

**Scheduler Overhead**: How long does `OS_Schedule` take to run
- Added a DWT CYCCNT counter to check the time is takes for `OS_Schedule()` to run. 
- A reading was taken at the start, and the final reading was taken at the end, right after the task state is set to READY.
- A global delta variable was used to find/track the time difference and printing it using the `UART_numeric_print()` function in any one of the tasks in tasks.c
- The result was 394 cycles, which comes out to be 3.2833us. 
- Based on research, this should take anything between 100 and 300 cpu clock cycles per scheduling event. 
