/*
 * File      : context_gcc.S
 * This file is part of RT-Thread RTOS
 * COPYRIGHT (C) 2009, RT-Thread Development Team
 *
 * The license and distribution terms for this file may be
 * found in the file LICENSE in this distribution or at
 * http://www.rt-thread.org/license/LICENSE
 *
 * Change Logs:
 * Date           Author       Notes
 * 2009-10-11     Bernard      first version
 * 2012-01-01     aozima       support context switch load/store FPU register.
 * 2013-06-18     aozima       add restore MSP feature.
 * 2013-06-23     aozima       support lazy stack optimized.
 */

/**
 * @addtogroup cortex-m4
 */
/*@{*/

.cpu cortex-m4
.syntax unified
.thumb
.text

.equ    SCB_VTOR,           0xE000ED08              /* Vector Table Offset Register */
.equ    NVIC_INT_CTRL,      0xE000ED04              /* interrupt control state register */
.equ    NVIC_SYSPRI2,       0xE000ED20              /* system priority register (2) */
.equ    NVIC_PENDSV_PRI,    0x00FF0000              /* PendSV priority value (lowest) */
.equ    NVIC_PENDSVSET,     0x10000000              /* value to trigger PendSV exception */

/*
 * os_sr_t os_enter_critical();
 */
.global os_enter_critical
.type os_enter_critical, %function
os_enter_critical:
    MRS     r0, PRIMASK
    CPSID   I
    BX      LR

/*
 * void os_exit_critical(os_sr_t sr);
 */
.global os_exit_critical
.type os_exit_critical, %function
os_exit_critical:
    MSR     PRIMASK, r0
    BX      LR

/*
 * void os_arch_context_switch(rt_uint32 from, rt_uint32 to);
 * r0 --> from
 * r1 --> to
 */
.global os_arch_context_switch_interrupt
.type os_arch_context_switch_interrupt, %function
.global os_arch_context_switch
.type os_arch_context_switch, %function

os_arch_context_switch_interrupt:
os_arch_context_switch:
    /* set interrupt_switch_flag to 1 */
    LDR     r2, =interrupt_switch_flag
    LDR     r3, [r2]
    CMP     r3, #1
    BEQ     _reswitch
    MOV     r3, #1
    STR     r3, [r2]

    LDR     r2, =interrupt_switch_task_from   /* set interrupt_switch_task_from */
    STR     r0, [r2]

_reswitch:
    LDR     r2, =interrupt_switch_task_to     /* set interrupt_switch_task_to */
    STR     r1, [r2]

    LDR r0, =NVIC_INT_CTRL              /* trigger the PendSV exception (causes context switch) */
    LDR r1, =NVIC_PENDSVSET
    STR r1, [r0]
    BX  LR

/* r0 --> switch from task stack
 * r1 --> switch to task stack
 * psr, pc, lr, r12, r3, r2, r1, r0 are pushed into [from] stack
 */
.global PendSV_Handler
.type PendSV_Handler, %function
PendSV_Handler:
    /* disable interrupt to protect context switch */
    MRS r2, PRIMASK
    CPSID   I

    /* get interrupt_switch_flag */
    LDR r0, =interrupt_switch_flag
    LDR r1, [r0]
    CBZ r1, pendsv_exit         /* pendsv already handled */

    /* clear interrupt_switch_flag to 0 */
    MOV r1, #0x00
    STR r1, [r0]

    LDR r0, =interrupt_switch_task_from
    LDR r1, [r0]
    CBZ r1, switch_to_task    /* skip register save at the first time */

    MRS r1, psp                 /* get from task stack pointer */

#if defined (__VFP_FP__) && !defined(__SOFTFP__)
    TST     lr, #0x10           /* if (!EXC_RETURN[4]) */
    VSTMDBEQ r1!, {d8 - d15}    /* push FPU register s16~s31 */
#endif

    STMFD   r1!, {r4 - r11}     /* push r4 - r11 register */

#if defined (__VFP_FP__) && !defined(__SOFTFP__)
    MOV     r4, #0x00           /* flag = 0 */

    TST     lr, #0x10           /* if (!EXC_RETURN[4]) */
    MOVEQ   r4, #0x01           /* flag = 1 */

    STMFD   r1!, {r4}           /* push flag */
#endif

    LDR r0, [r0]
    STR r1, [r0]                /* update from task stack pointer */

switch_to_task:
    LDR r1, =interrupt_switch_task_to
    LDR r1, [r1]
    LDR r1, [r1]                /* load task stack pointer */

#if defined (__VFP_FP__) && !defined(__SOFTFP__)
    LDMFD   r1!, {r3}           /* pop flag */
#endif

    LDMFD   r1!, {r4 - r11}     /* pop r4 - r11 register */

#if defined (__VFP_FP__) && !defined(__SOFTFP__)
    CMP     r3,  #0             /* if (flag_r3 != 0) */
    VLDMIANE  r1!, {d8 - d15}   /* pop FPU register s16~s31 */
#endif

    MSR psp, r1                 /* update stack pointer */

pendsv_exit:
    /* restore interrupt */
    MSR PRIMASK, r2

#if defined (__VFP_FP__) && !defined(__SOFTFP__)
    ORR     lr, lr, #0x10       /* lr |=  (1 << 4), clean FPCA. */
    CMP     r3,  #0             /* if (flag_r3 != 0) */
    BICNE   lr, lr, #0x10       /* lr &= ~(1 << 4), set FPCA. */
#endif

    ORR lr, lr, #0x04
    BX  lr

/*
 * void os_arch_context_switch_to(rt_uint32 to);
 * r0 --> to
 */
.global os_arch_context_switch_to
.type os_arch_context_switch_to, %function
os_arch_context_switch_to:
    LDR r1, =interrupt_switch_task_to
    STR r0, [r1]

#if defined (__VFP_FP__) && !defined(__SOFTFP__)
    /* CLEAR CONTROL.FPCA */
    MRS     r2, CONTROL         /* read */
    BIC     r2, #0x04           /* modify */
    MSR     CONTROL, r2         /* write-back */
#endif

    /* set from task to 0 */
    LDR r1, =interrupt_switch_task_from
    MOV r0, #0x0
    STR r0, [r1]

    /* set interrupt flag to 1 */
    LDR     r1, =interrupt_switch_flag
    MOV     r0, #1
    STR     r0, [r1]

    /* set the PendSV exception priority */
    LDR r0, =NVIC_SYSPRI2
    LDR r1, =NVIC_PENDSV_PRI
    LDR.W   r2, [r0,#0x00]       /* read       */
    ORR     r1,r1,r2             /* modify     */
    STR     r1, [r0]             /* write-back */

    LDR r0, =NVIC_INT_CTRL      /* trigger the PendSV exception (causes context switch) */
    LDR r1, =NVIC_PENDSVSET
    STR r1, [r0]

    /* restore MSP */
    LDR     r0, =SCB_VTOR
    LDR     r0, [r0]
    LDR     r0, [r0]
    NOP
    MSR     msp, r0

    CPSIE   I                       /* enable interrupts at processor level */

    /* never reach here! */

/* compatible with old version */
.global os_arch_interrupt_task_switch
.type os_arch_interrupt_task_switch, %function
os_arch_interrupt_task_switch:
    BX  lr
    NOP

.global HardFault_Handler
.type HardFault_Handler, %function
HardFault_Handler:
    /* get current context */
    MRS     r0, psp                 /* get fault task stack pointer */
    PUSH    {lr}
    BL      os_arch_hard_fault_exception
    POP     {lr}

    ORR     lr, lr, #0x04
    BX      lr
