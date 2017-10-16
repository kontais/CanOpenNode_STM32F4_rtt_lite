;/*
; * File      : context_rvds.S
; * This file is part of RT-Thread RTOS
; * COPYRIGHT (C) 2009, RT-Thread Development Team
; *
; * The license and distribution terms for this file may be
; * found in the file LICENSE in this distribution or at
; * http://www.rt-thread.org/license/LICENSE
; *
; * Change Logs:
; * Date           Author       Notes
; * 2010-01-25     Bernard      first version
; * 2012-06-01     aozima       set pendsv priority to 0xFF.
; * 2012-08-17     aozima       fixed bug: store r8 - r11.
; * 2013-06-18     aozima       add restore MSP feature.
; */

;/**
; * @addtogroup CORTEX-M0
; */
;/*@{*/

SCB_VTOR        EQU     0xE000ED08               ; Vector Table Offset Register
NVIC_INT_CTRL   EQU     0xE000ED04               ; interrupt control state register
NVIC_SHPR3      EQU     0xE000ED20               ; system priority register (2)
NVIC_PENDSV_PRI EQU     0x00FF0000               ; PendSV priority value (lowest)
NVIC_PENDSVSET  EQU     0x10000000               ; value to trigger PendSV exception

    AREA |.text|, CODE, READONLY, ALIGN=2
    THUMB
    REQUIRE8
    PRESERVE8

    IMPORT interrupt_switch_flag
    IMPORT interrupt_switch_task_from
    IMPORT interrupt_switch_task_to

;/*
; * os_sr_t os_enter_critical();
; */
os_enter_critical    PROC
    EXPORT  os_enter_critical
    MRS     r0, PRIMASK
    CPSID   I
    BX      LR
    ENDP

;/*
; * void os_exit_critical(os_sr_t sr);
; */
os_exit_critical    PROC
    EXPORT  os_exit_critical
    MSR        PRIMASK, r0
    BX        LR
    ENDP

;/*
; * void os_arch_context_switch(rt_uint32 from, rt_uint32 to);
; * r0 --> from
; * r1 --> to
; */
os_arch_context_switch_interrupt
    EXPORT os_arch_context_switch_interrupt
os_arch_context_switch    PROC
    EXPORT os_arch_context_switch

    ; set interrupt_switch_flag to 1
    LDR     r2, =interrupt_switch_flag
    LDR     r3, [r2]
    CMP     r3, #1
    BEQ     _reswitch
    MOVS    r3, #0x01
    STR     r3, [r2]

    LDR     r2, =interrupt_switch_task_from   ; set interrupt_switch_task_from
    STR     r0, [r2]

_reswitch
    LDR     r2, =interrupt_switch_task_to     ; set interrupt_switch_task_to
    STR     r1, [r2]

    LDR     r0, =NVIC_INT_CTRL              ; trigger the PendSV exception (causes context switch)
    LDR     r1, =NVIC_PENDSVSET
    STR     r1, [r0]
    BX      LR
    ENDP

; r0 --> switch from task stack
; r1 --> switch to task stack
; psr, pc, lr, r12, r3, r2, r1, r0 are pushed into [from] stack
PendSV_Handler    PROC
    EXPORT PendSV_Handler

    ; disable interrupt to protect context switch
    MRS     r2, PRIMASK
    CPSID   I

    ; get interrupt_switch_flag
    LDR     r0, =interrupt_switch_flag
    LDR     r1, [r0]
    CMP     r1, #0x00
    BEQ     pendsv_exit                ; pendsv already handled

    ; clear interrupt_switch_flag to 0
    MOVS    r1, #0x00
    STR     r1, [r0]

    LDR     r0, =interrupt_switch_task_from
    LDR     r1, [r0]
    CMP     r1, #0x00
    BEQ     switch_to_task        ; skip register save at the first time

    MRS     r1, psp                 ; get from task stack pointer

    SUBS    r1, r1, #0x20           ; space for {r4 - r7} and {r8 - r11}
    LDR     r0, [r0]
    STR     r1, [r0]                ; update from task stack pointer

    STMIA   r1!, {r4 - r7}          ; push task {r4 - r7} register to task stack

    MOV     r4, r8                  ; mov task {r8 - r11} to {r4 - r7}
    MOV     r5, r9
    MOV     r6, r10
    MOV     r7, r11
    STMIA   r1!, {r4 - r7}          ; push task {r8 - r11} high register to task stack

switch_to_task
    LDR     r1, =interrupt_switch_task_to
    LDR     r1, [r1]
    LDR     r1, [r1]                ; load task stack pointer

    LDMIA   r1!, {r4 - r7}          ; pop task {r4 - r7} register from task stack
    PUSH    {r4 - r7}               ; push {r4 - r7} to MSP for copy {r8 - r11}

    LDMIA   r1!, {r4 - r7}          ; pop task {r8 - r11} high register from task stack to {r4 - r7}
    MOV     r8,  r4                 ; mov {r4 - r7} to {r8 - r11}
    MOV     r9,  r5
    MOV     r10, r6
    MOV     r11, r7

    POP     {r4 - r7}               ; pop {r4 - r7} from MSP

    MSR     psp, r1                 ; update stack pointer

pendsv_exit
    ; restore interrupt
    MSR     PRIMASK, r2

    MOVS    r0, #0x04
    RSBS    r0, r0, #0x00
    BX      r0
    ENDP

;/*
; * void os_arch_context_switch_to(rt_uint32 to);
; * r0 --> to
; * this fucntion is used to perform the first task switch
; */
os_arch_context_switch_to    PROC
    EXPORT os_arch_context_switch_to
    ; set to task
    LDR     r1, =interrupt_switch_task_to
    STR     r0, [r1]

    ; set from task to 0
    LDR     r1, =interrupt_switch_task_from
    MOVS    r0, #0x0
    STR     r0, [r1]

    ; set interrupt flag to 1
    LDR     r1, =interrupt_switch_flag
    MOVS    r0, #1
    STR     r0, [r1]

    ; set the PendSV exception priority
    LDR     r0, =NVIC_SHPR3
    LDR     r1, =NVIC_PENDSV_PRI
    LDR     r2, [r0,#0x00]       ; read
    ORRS    r1,r1,r2             ; modify
    STR     r1, [r0]             ; write-back

    ; trigger the PendSV exception (causes context switch)
    LDR     r0, =NVIC_INT_CTRL
    LDR     r1, =NVIC_PENDSVSET
    STR     r1, [r0]

    ; restore MSP
    LDR     r0, =SCB_VTOR
    LDR     r0, [r0]
    LDR     r0, [r0]
    MSR     msp, r0

    ; enable interrupts at processor level
    CPSIE   I

    ; never reach here!
    ENDP

; compatible with old version
os_arch_interrupt_task_switch PROC
    EXPORT os_arch_interrupt_task_switch
    BX      lr
    ENDP

    IMPORT os_arch_hard_fault_exception

HardFault_Handler    PROC
    EXPORT HardFault_Handler

    ; get current context
    MRS     r0, psp                 ; get fault task stack pointer
    PUSH    {lr}
    BL      os_arch_hard_fault_exception
    POP     {pc}
    ENDP

    ALIGN   4

    END
