;/*
; * File      : context_rvds.S
; * This file is part of RT-Thread RTOS
; * COPYRIGHT (C) 2006, RT-Thread Development Team
; *
; * The license and distribution terms for this file may be
; * found in the file LICENSE in this distribution or at
; * http://www.rt-thread.org/license/LICENSE
; *
; * Change Logs:
; * Date           Author       Notes
; * 2009-01-20     Bernard      first version
; */

NOINT    EQU        0xc0    ; disable interrupt in psr

    AREA |.text|, CODE, READONLY, ALIGN=2
    ARM
    REQUIRE8
    PRESERVE8

;/*
; * os_sr_t os_enter_critical();
; */
os_enter_critical    PROC
    EXPORT os_enter_critical
    MRS r0, cpsr
    ORR r1, r0, #NOINT
    MSR cpsr_c, r1
    BX    lr
    ENDP

;/*
; * void os_exit_critical(os_sr_t sr);
; */
os_exit_critical    PROC
    EXPORT os_exit_critical
    MSR cpsr_c, r0
    BX    lr
    ENDP

;/*
; * void os_arch_context_switch(rt_uint32 from, rt_uint32 to);
; * r0 --> from
; * r1 --> to
; */
os_arch_context_switch    PROC
    EXPORT os_arch_context_switch
    STMFD    sp!, {lr}            ; push pc (lr should be pushed in place of PC)
    STMFD    sp!, {r0-r12, lr}    ; push lr & register file

    MRS        r4, cpsr
    STMFD    sp!, {r4}            ; push cpsr
    MRS        r4, spsr
    STMFD    sp!, {r4}            ; push spsr

    STR    sp, [r0]                ; store sp in preempted tasks TCB
    LDR    sp, [r1]                ; get new task stack pointer

    LDMFD    sp!, {r4}            ; pop new task spsr
    MSR    spsr_cxsf, r4
    LDMFD    sp!, {r4}            ; pop new task cpsr
    MSR    cpsr_cxsf, r4

    LDMFD    sp!, {r0-r12, lr, pc}    ; pop new task r0-r12, lr & pc
    ENDP

;/*
; * void os_arch_context_switch_to(rt_uint32 to);
; * r0 --> to
; */
os_arch_context_switch_to    PROC
    EXPORT os_arch_context_switch_to
    LDR    sp, [r0]                ; get new task stack pointer

    LDMFD    sp!, {r4}            ; pop new task spsr
    MSR    spsr_cxsf, r4
    LDMFD    sp!, {r4}            ; pop new task cpsr
    MSR    cpsr_cxsf, r4

    LDMFD    sp!, {r0-r12, lr, pc}    ; pop new task r0-r12, lr & pc
    ENDP

;/*
; * void os_arch_context_switch_interrupt(rt_uint32 from, rt_uint32 to);
; */
    IMPORT interrupt_switch_flag
    IMPORT interrupt_switch_task_from
    IMPORT interrupt_switch_task_to

os_arch_context_switch_interrupt    PROC
    EXPORT os_arch_context_switch_interrupt
    LDR r2, =interrupt_switch_flag
    LDR r3, [r2]
    CMP r3, #1
    BEQ _reswitch
    MOV r3, #1                            ; set interrupt_switch_flag to 1
    STR r3, [r2]
    LDR r2, =interrupt_switch_task_from    ; set interrupt_switch_task_from
    STR r0, [r2]
_reswitch
    LDR r2, =interrupt_switch_task_to        ; set interrupt_switch_task_to
    STR r1, [r2]
    BX    lr
    ENDP

    END