# cortex-m4-rtos-kernel

A custom, bare-metal Real-Time Operating System (RTOS) built from scratch for the ARM Cortex-M4 architecture (specifically the TM4C1294NCPDT / MSP432E401Y Launchpad). 

This project demonstrates core operating system concepts, including hardware exception handling, stack memory management, and preemptive task scheduling.

## Current Features (v1.0 - Milestone 1)
* **Preemptive Round-Robin Scheduling:** Automatically distributes CPU time equally among 4 active tasks.
* **Hardware Timer Heartbeat:** Utilizes the onboard SysTick timer to trigger a context switch every 1ms.
* **Isolated Task Stacks:** Manages individual memory spaces for each task using the Cortex-M4 Process Stack Pointer (PSP).
* **Safe Context Switching:** Leverages `PendSV` and `SVC` hardware exceptions to safely pause and restore task states without corrupting the Main Stack Pointer (MSP).

## System Architecture
### The Boot Sequence
1. The hardware initializes the 120MHz system clock and necessary GPIO ports.
2. The OS allocates memory for the Task Control Blocks (TCBs) and manually forges a 16-word hardware/software stack frame for each task in RAM.
3. A Supervisor Call (`SVC #0`) is triggered to force the CPU to pop the initial task's stack frame, transition to Thread Mode, and begin execution on the PSP.

### The Context Switch
1. The **SysTick Timer** fires every 1 millisecond. 
2. The SysTick handler sets the `PENDSVSET` bit in the Interrupt Control and State Register (ICSR).
3. The **PendSV Exception** fires. The assembly handler pushes registers `R4-R11` to the active task's software stack and saves the updated PSP to the active TCB.
4. The C-based **Round-Robin Scheduler** advances the `CurrentTask` pointer to the next active TCB.
5. The PendSV handler loads the new task's PSP, pops `R4-R11` off its stack, and triggers an Exception Return (`BX LR`).
6. The CPU hardware automatically restores the remaining registers (`R0-R3`, `PC`, `LR`, `xPSR`) and resumes the new task exactly where it left off.

## 🚧 Current Status & Next Steps
* **Status:** 4 tasks are currently running concurrently, toggling the 4 independent onboard LEDs at varying frequencies to demonstrate isolated CPU time-slicing.
* **Next Milestone:** Replace the current busy-wait `volatile` delay loops with a proper `OS_Sleep()` API to introduce blocking states and improve CPU efficiency.