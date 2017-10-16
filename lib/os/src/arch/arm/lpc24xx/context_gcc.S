/*
 * File      : context.S
 * This file is part of RT-Thread RTOS
 * COPYRIGHT (C) 2006, RT-Thread Development Team
 *
 * The license and distribution terms for this file may be
 * found in the file LICENSE in this distribution or at
 * http://openlab.rt-thread.com/license/LICENSE
 *
 * Change Logs:
 * Date           Author       Notes
 * 2008-12-11     XuXinming    first version
 */

/*!
 * \addtogroup LPC2478
 */
/*@{*/

#define NOINT            0xc0

/*
 * os_sr_t os_enter_critical();
 */
.globl os_enter_critical
os_enter_critical:
    mrs r0, cpsr
    orr r1, r0, #NOINT
    msr cpsr_c, r1
    mov pc, lr

/*
 * void os_exit_critical(os_sr_t sr);
 */
.globl os_exit_critical
os_exit_critical:
    msr cpsr, r0
    mov pc, lr

/*
 * void os_arch_context_switch(rt_uint32 from, rt_uint32 to);
 * r0 --> from
 * r1 --> to
 */
.globl os_arch_context_switch
os_arch_context_switch:
    stmfd    sp!, {lr}        @ push pc (lr should be pushed in place of PC)
    stmfd    sp!, {r0-r12, lr}    @ push lr & register file

    mrs    r4, cpsr
    stmfd    sp!, {r4}        @ push cpsr
    mrs    r4, spsr
    stmfd    sp!, {r4}        @ push spsr

    str    sp, [r0]            @ store sp in preempted tasks TCB
    ldr    sp, [r1]            @ get new task stack pointer

    ldmfd    sp!, {r4}        @ pop new task spsr
    msr    spsr_cxsf, r4
    ldmfd    sp!, {r4}        @ pop new task cpsr
    msr    cpsr_cxsf, r4

    ldmfd    sp!, {r0-r12, lr, pc}    @ pop new task r0-r12, lr & pc

/*
 * void os_arch_context_switch_to(rt_uint32 to);
 * r0 --> to
 */
.globl os_arch_context_switch_to
os_arch_context_switch_to:
    ldr    sp, [r0]        @ get new task stack pointer

    ldmfd    sp!, {r4}        @ pop new task spsr
    msr    spsr_cxsf, r4
    ldmfd    sp!, {r4}        @ pop new task cpsr
    msr    cpsr_cxsf, r4

    ldmfd    sp!, {r0-r12, lr, pc}    @ pop new task r0-r12, lr & pc

/*
 * void os_arch_context_switch_interrupt(rt_uint32 from, rt_uint32 to);
 */
.globl interrupt_switch_flag
.globl interrupt_switch_task_from
.globl interrupt_switch_task_to
.globl os_arch_context_switch_interrupt
os_arch_context_switch_interrupt:
    ldr r2, =interrupt_switch_flag
    ldr r3, [r2]
    cmp r3, #1
    beq _reswitch
    mov r3, #1                @ set interrupt_switch_flag to 1
    str r3, [r2]
    ldr r2, =interrupt_switch_task_from    @ set interrupt_switch_task_from
    str r0, [r2]
_reswitch:
    ldr r2, =interrupt_switch_task_to        @ set interrupt_switch_task_to
    str r1, [r2]
    mov pc, lr
