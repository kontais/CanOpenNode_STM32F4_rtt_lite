/*
 * File      : context_gcc.S
 * This file is part of RT-Thread RTOS
 * COPYRIGHT (C) 2009 - 2013, RT-Thread Development Team
 *
 * The license and distribution terms for this file may be
 * found in the file LICENSE in this distribution or at
 * http://www.rt-thread.org/license/LICENSE
 *
 * Change Logs:
 * Date         Author    Notes
 * 2010-01-25   Bernard      first version
 * 2012-06-01   aozima       set pendsv priority to 0xFF.
 * 2012-08-17   aozima       fixed bug: store r8 - r11.
 * 2013-02-20   aozima       port to gcc.
 * 2013-06-18   aozima       add restore MSP feature.
 * 2013-11-04   bright       fixed hardfault bug for gcc.
 */

    .cpu    cortex-m0
    .fpu    softvfp
    .syntax unified
    .thumb
    .text

    .equ     SCB_VTOR, 0xE000ED08            /* Vector Table Offset Register */
    .equ     NVIC_INT_CTRL, 0xE000ED04       /* interrupt control state register */
    .equ     NVIC_SHPR3, 0xE000ED20          /* system priority register (3) */
    .equ     NVIC_PENDSV_PRI, 0x00FF0000     /* PendSV priority value (lowest) */
    .equ     NVIC_PENDSVSET, 0x10000000      /* value to trigger PendSV exception */

/*
 * os_sr_t os_enter_critical();
 */
    .global os_enter_critical
    .type os_enter_critical, %function
os_enter_critical:
    MRS     R0, PRIMASK
    CPSID   I
    BX      LR

/*
 * void os_exit_critical(os_sr_t sr);
 */
    .global os_exit_critical
    .type os_exit_critical, %function
os_exit_critical:
    MSR     PRIMASK, R0
    BX      LR

/*
 * void os_arch_context_switch(rt_uint32 from, rt_uint32 to);
 * R0 --> from
 * R1 --> to
 */
    .global os_arch_context_switch_interrupt
    .type os_arch_context_switch_interrupt, %function
    .global os_arch_context_switch
    .type os_arch_context_switch, %function
os_arch_context_switch_interrupt:
os_arch_context_switch:
    /* set interrupt_switch_flag to 1 */
    LDR     R2, =interrupt_switch_flag
    LDR     R3, [R2]
    CMP     R3, #1
    BEQ     _reswitch
    MOVS    R3, #1
    STR     R3, [R2]

    LDR     R2, =interrupt_switch_task_from   /* set interrupt_switch_task_from */
    STR     R0, [R2]

_reswitch:
    LDR     R2, =interrupt_switch_task_to     /* set interrupt_switch_task_to */
    STR     R1, [R2]

    LDR     R0, =NVIC_INT_CTRL           /* trigger the PendSV exception (causes context switch) */
    LDR     R1, =NVIC_PENDSVSET
    STR     R1, [R0]
    BX      LR

/* R0 --> switch from task stack
 * R1 --> switch to task stack
 * psr, pc, LR, R12, R3, R2, R1, R0 are pushed into [from] stack
 */
    .global PendSV_Handler
    .type PendSV_Handler, %function
PendSV_Handler:
    /* disable interrupt to protect context switch */
    MRS     R2, PRIMASK
    CPSID   I

    /* get interrupt_switch_flag */
    LDR     R0, =interrupt_switch_flag
    LDR     R1, [R0]
    CMP     R1, #0x00
    BEQ     pendsv_exit        /* pendsv aLReady handled */

    /* clear interrupt_switch_flag to 0 */
    MOVS    R1, #0
    STR     R1, [R0]

    LDR     R0, =interrupt_switch_task_from
    LDR     R1, [R0]
    CMP     R1, #0x00
    BEQ     switch_to_task    /* skip register save at the first time */

    MRS     R1, PSP                 /* get from task stack pointer */

    SUBS    R1, R1, #0x20           /* space for {R4 - R7} and {R8 - R11} */
    LDR     R0, [R0]
    STR     R1, [R0]                /* update from task stack pointer */

    STMIA   R1!, {R4 - R7}          /* push task {R4 - R7} register to task stack */

    MOV     R4, R8                  /* mov task {R8 - R11} to {R4 - R7} */
    MOV     R5, R9
    MOV     R6, R10
    MOV     R7, R11
    STMIA   R1!, {R4 - R7}          /* push task {R8 - R11} high register to task stack */
switch_to_task:
    LDR     R1, =interrupt_switch_task_to
    LDR     R1, [R1]
    LDR     R1, [R1]                /* load task stack pointer */

    LDMIA   R1!, {R4 - R7}          /* pop task {R4 - R7} register from task stack */
    PUSH    {R4 - R7}               /* push {R4 - R7} to MSP for copy {R8 - R11} */

    LDMIA   R1!, {R4 - R7}          /* pop task {R8 - R11} high register from task stack to {R4 - R7} */
    MOV     R8,  R4                 /* mov {R4 - R7} to {R8 - R11} */
    MOV     R9,  R5
    MOV     R10, R6
    MOV     R11, R7

    POP     {R4 - R7}               /* pop {R4 - R7} from MSP */

    MSR     PSP, R1                 /* update stack pointer */

pendsv_exit:
    /* restore interrupt */
    MSR     PRIMASK, R2

    MOVS    R0, #0x04
    RSBS    R0, R0, #0x00
    BX      R0
/*
 * void os_arch_context_switch_to(rt_uint32 to);
 * R0 --> to
 */
    .global os_arch_context_switch_to
    .type os_arch_context_switch_to, %function
os_arch_context_switch_to:
    LDR     R1, =interrupt_switch_task_to
    STR     R0, [R1]

    /* set from task to 0 */
    LDR     R1, =interrupt_switch_task_from
    MOVS    R0, #0
    STR     R0, [R1]

    /* set interrupt flag to 1 */
    LDR     R1, =interrupt_switch_flag
    MOVS    R0, #1
    STR     R0, [R1]

    /* set the PendSV exception priority */
    LDR     R0, =NVIC_SHPR3
    LDR     R1, =NVIC_PENDSV_PRI
    LDR     R2, [R0,#0x00]       /* read */
    ORRS    R1, R1, R2             /* modify */
    STR     R1, [R0]             /* write-back */

    LDR     R0, =NVIC_INT_CTRL               /* trigger the PendSV exception (causes context switch) */
    LDR     R1, =NVIC_PENDSVSET
    STR     R1, [R0]
    NOP
    /* restore MSP */
    LDR     R0, =SCB_VTOR
    LDR     R0, [R0]
    LDR     R0, [R0]
    NOP
    MSR     MSP, R0

    CPSIE   I                       /* enable interrupts at processor level */

    /* never reach here! */

/* compatible with old version */
    .global os_arch_interrupt_task_switch
    .type os_arch_interrupt_task_switch, %function
os_arch_interrupt_task_switch:
    BX      LR
    NOP

    .global HardFault_Handler
    .type HardFault_Handler, %function
HardFault_Handler:
    /* get current context */
    MRS     R0, PSP                 /* get fault task stack pointer */
    PUSH    {LR}
    BL      os_arch_hard_fault_exception
    POP     {PC}

/*
 * uint32_t os_arch_interrupt_check(void);
 * R0 --> state
 */
    .global os_arch_interrupt_check
    .type os_arch_interrupt_check, %function
os_arch_interrupt_check:
    MRS     R0, IPSR
    BX      LR
