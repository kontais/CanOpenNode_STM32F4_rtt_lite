/*
 * File      : context.S
 * This file is part of RT-Thread RTOS
 * COPYRIGHT (C) 2013, RT-Thread Development Team
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 * Change Logs:
 * Date           Author       Notes
 * 2013-07-05     Bernard      the first version
 */

/*
 * os_sr_t os_enter_critical();
 */
.globl os_enter_critical
os_enter_critical:
    mrs r0, cpsr
    cpsid if
    bx  lr

/*
 * void os_exit_critical(os_sr_t sr);
 */
.globl os_exit_critical
os_exit_critical:
    msr cpsr_c, r0
    bx  lr

/*
 * void os_arch_context_switch(rt_uint32 from, rt_uint32 to);
 * r0 --> from
 * r1 --> to
 */
.globl os_arch_context_switch
os_arch_context_switch:
    stmfd   sp!, {lr}       @ push pc (lr should be pushed in place of PC)
    stmfd   sp!, {r0-r12, lr}   @ push lr & register file

    mrs r4, cpsr
    tst lr, #0x01
    orrne r4, r4, #0x20     @ it's thumb code

    stmfd sp!, {r4}         @ push cpsr

    str sp, [r0]            @ store sp in preempted tasks TCB
    ldr sp, [r1]            @ get new task stack pointer

    ldmfd sp!, {r4}         @ pop new task cpsr to spsr
    msr spsr_cxsf, r4

_do_switch:
    ldmfd sp!, {r0-r12, lr, pc}^  @ pop new task r0-r12, lr & pc, copy spsr to cpsr

/*
 * void os_arch_context_switch_to(rt_uint32 to);
 * r0 --> to
 */
.globl os_arch_context_switch_to
os_arch_context_switch_to:
    ldr sp, [r0]            @ get new task stack pointer

    ldmfd sp!, {r4}         @ pop new task spsr
    msr spsr_cxsf, r4

    bic r4, r4, #0x20       @ must be ARM mode
    msr cpsr_cxsf, r4

    ldmfd sp!, {r0-r12, lr, pc}^   @ pop new task r0-r12, lr & pc

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
    mov r3, #1              @ set interrupt_switch_flag to 1
    str r3, [r2]
    ldr r2, =interrupt_switch_task_from   @ set interrupt_switch_task_from
    str r0, [r2]
_reswitch:
    ldr r2, =interrupt_switch_task_to     @ set interrupt_switch_task_to
    str r1, [r2]
    bx  lr
