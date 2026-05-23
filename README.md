# cortex-m4-rtos-kernel

A custom, bare-metal Real-Time Operating System (RTOS) built from scratch for the ARM Cortex-M4 architecture (TM4C1294NCPDT / MSP432E401Y Launchpad). No HAL. No vendor RTOS libraries.

---

## Features

### v1.0 — Context Switching Core
- Preemptive round-robin scheduling across 4 tasks
- SysTick heartbeat triggering a context switch every 1ms
- Isolated PSP stack per task with a forged 16-word initial frame
- Safe kernel/task memory split using MSP (kernel) and PSP (tasks)
- PendSV and SVC exception-driven context switching in ARM assembly

### v2.0 — Full Kernel
- Priority-based preemptive scheduling with round-robin tiebreaking for equal-priority tasks
- Binary semaphores with priority-ordered wait queues
- Mutexes with ownership tracking and priority inheritance
- First-fit heap allocator with block splitting (os_malloc / os_free)
- Stack overflow detection using 0xDEADBEEF canary values
- OS_Sleep for tick-based task blocking
- OS_Fault kernel panic handler with interrupt lockout and visual distress signal

---

## Architecture

### Boot Sequence
1. PLL locks the system clock at 120MHz
2. GPIO configured for LED output
3. OS initialises TCB array, forges fake stack frames, writes canary values
4. SysTick armed at 1ms tick rate with PendSV set to lowest priority
5. Start_OS loads the first task PSP and triggers SVC #0
6. SVC handler performs EXC_RETURN —> CPU enters thread mode on PSP

### Context Switch (every 1ms)
1. SysTick fires and sets PENDSVSET in SCB->ICSR
2. PendSV fires after all higher-priority interrupts clear
3. Assembly handler saves R4-R11 to the current task PSP and stores SP in TCB
4. C scheduler selects the highest-priority READY task
5. Assembly handler restores R4-R11 from the new task stack and updates PSP
6. EXC_RETURN —> hardware restores R0-R3, LR, PC, xPSR automatically

### Priority Inheritance
When a high-priority task blocks on a mutex held by a lower-priority task, the owner's priority is temporarily elevated to prevent priority inversion. The original priority is restored on Mutex_Unlock.

### Memory Layout Per Task
```
tcb_array[0]        — 0xDEADBEEF canary (stack overflow detection)
tcb_array[1..83]    — usable stack space
tcb_array[84..91]   — R4-R11 software save frame (saved by PendSV)
tcb_array[92..99]   — R0-R3, R12, LR, PC, xPSR hardware frame (saved by CPU)
```

---

## File Structure
```
kernel/
  os_core.c / os_core.h     — TCB struct, scheduler, OS_Sleep, task creation
  os_asm.s                  — PendSV and SVC handlers in ARM assembly
  semaphore.c / semaphore.h — binary semaphores with priority wait queues
  mutex.c / mutex.h         — mutexes with priority inheritance
  os_malloc.c / os_malloc.h — first-fit heap allocator
  os_error.c / os_error.h   — OS_Fault kernel panic handler
  systick.c / systick.h     — SysTick init, tick handler, canary checks
  pll.c / pll.h             — 120MHz clock initialisation

app/
  main.c                    — system init and task creation
  tasks.c / tasks.h         — demo task functions
```

---

## Building and Flashing
Built with Keil MDK v5, ARMCLANG v6.23.  
Target: TM4C1294NCPDT (MSP432E401Y compatible pinout).  
Flash via Keil debugger over JTAG/SWD.

---

## Project Status

| Milestone | Status |
|---|---|
| v1.0 — Preemptive round-robin with 3+ tasks | Complete |
| v2.0 — Full kernel: sync primitives, heap, sleep | Complete |
| v3.0 — ITM trace + Gantt chart visualiser | In progress |

Currently implementing ITM instrumentation

For ITM trace, I Planned SWO trace output without first verifying if my debug probe has the functionality. CMSIS-DAP does not support SWO. Shifted to UART trace. It has the same packet format. Lesson: verify hardware capabilities before designing software around them. While it is tempting to jump straight into new architectural features, software design must always be grounded in the physical realities of the hardware toolchain