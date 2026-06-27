;SVC Handler
;We need to perform an EXC_RETURN
;PendSV Context Switcher

	AREA os_asm, CODE, READONLY
	THUMB
		
	IMPORT CurrentTask
	IMPORT OS_Schedule
	IMPORT pendSV0
	IMPORT pendSV1

;Load LR with 0xFFFFFFFD (EXC_RETURN value)
	EXPORT SVC_Handler ;used by the linker so it can be used by other files
SVC_Handler
	LDR LR, =0xFFFFFFFD
	BX LR ;LSB is 1, the processor switches to Thumb State

	EXPORT PendSV_Handler
PendSV_Handler

	CPSID i ; Disable Interrupts
	
	LDR R2, =0xE0001004 ; reads the address of the DWT CYCCNT Counter to find the current cycle count
	LDR R3, [R2] ; dereferences R2's value and puts it into a temp register
	LDR R2, =pendSV0 ; overwrites R2's address with the global C variable pendSV0
	STR R3, [R2] ; stores the R3 value (current count) into R2, our pendSV0 variable
	
	MRS R0, PSP ; moves the PSP into a general-purpose register
	
	STMDB R0!, {R4-R11} ; Stores registers R4 through R11 into memory starting at the address in R0. R0 points to the new top of the current task's stack.
	
	LDR R1, =CurrentTask ; R1 is basically &CurrentTask right now
	LDR R1, [R1] ; R1 now holds the actual memory address of the active TCB Struct. Dereferences the address to CurrentTask
	STR R0, [R1] ; Store R0 into offest 0 of the TCB (psp is the first field in the TCB struct)
	
	;BL will overwrite LR. We need to save LR somewhere before BL and restore it after. We can use the stack. The other option is to just use any of the approved AAPCS registers, which in this case would be R4-R11
	PUSH {LR}
	BL OS_Schedule
	POP {LR}
	;In this case, we are using the MSP because Handler mode always uses MSP. 
	
	LDR R2, =CurrentTask ; Load Current Task
	LDR R2, [R2] ; Dereference it 
	LDR R0, [R2] ; Load the new task's PSP into a register
	
	LDMIA R0!, {R4-R11} ; Pops R4-R11 off the next task's stack
	MSR PSP, R0 ; write the updated stack pointer back into the PSP register
	
	LDR R2, =0xE0001004 ; reads the address of the DWT CYCCNT Counter to find the current cycle count
	LDR R3, [R2] ; dereferences R2's value and puts it into a temp register
	LDR R2, =pendSV1 ; overwrites R2's address with the global C variable pendSV1
	STR R3, [R2] ; stores the R3 value (current count) into R2, our pendSV1 variable
	
	CPSIE i ; Enable Interrupts
	BX LR ; Triggers EXC_RETURN, jumps to the new task's PC. 
	
	END
	