/*
 * File      : context_gcc.S
 * This file is part of RT-Thread RTOS
 * COPYRIGHT (C) 2006, RT-Thread Development Team
 *
 * The license and distribution terms for this file may be
 * found in the file LICENSE in this distribution or at
 * http://www.rt-thread.org/license/LICENSE
 *
 * Change Logs:
 * Date           Author       Notes
 * 2006-09-15     QiuYi        The first version
 * 2006-10-09     Bernard      add os_arch_context_switch_to implementation
 */

 /**
 * @addtogroup ia32
 */
/*@{*/

/*
 * void os_arch_context_switch(rt_uint32 from, rt_uint32 to);
 */
.globl os_arch_context_switch
os_arch_context_switch:
    pushfl                    /*pushed eflags*/
/*
 *    add by ssslady@gmail.com 2009-10-14
 *      When we return again the esp should no be change.
 *     The old code change the esp to esp-4 :-(.
 *    A protection fault maybe occure for img created by some compiler,eg.gcc in the fedor-11
 *      -------------------------------------------------------------------------
 *    entry            old code            new code
 *    EIP    ->return esp    EIP                FLAGS ->return esp
 *    ...            FLAGS    ->retern esp        CS
 *                CS                EIP
 *                EIP
 */
    popl %eax    /*get flags*/
    popl %ebx    /*get eip*/
    pushl %eax    /*push flags*/
    push %cs    /*push cs*/
    pushl %ebx    /*push eip*/

/*-------------------------------------------------------------------
 */

    /*push %cs*/                /*push cs register*/
    /*pushl 0x8(%esp)*/            /*pushed eip register*/

    pushl $0                /*fill irqno*/
    push %ds                /*push ds register*/
    push %es                /*push es register*/
    pushal                    /*push eax,ecx,edx,ebx,esp,ebp,esp,edi registers*/

    /*movl 0x40(%esp), %eax*/    /*to task TCB*/
    /*movl 0x3c(%esp), %ebx*/    /*from task TCB*/
    movl 0x3c(%esp), %eax    /*to task TCB*/
    movl 0x38(%esp), %ebx    /*from task TCB*/

    movl %esp, (%ebx)        /*store esp in preempted tasks TCB*/
    movl (%eax), %esp        /*get new task stack pointer*/

    popal                    /*restore new task TCB*/
    pop %es
    pop %ds
    add $4,%esp                /*skip irqno*/
    iret

/*
 * void os_arch_context_switch_to(rt_uint32 to);
 */
.globl os_arch_context_switch_to
os_arch_context_switch_to:
    push %ebp
    movl %esp, %ebp

    movl 0x8(%ebp), %eax    /* to task TCB */
    movl (%eax), %esp        /* get new task stack pointer */

    popal                    /* restore new task TCB*/
    pop %es
    pop %ds
    add $4, %esp            /* skip irqno */
    iret

/*
 * void os_arch_context_switch_interrupt(rt_uint32 from, rt_uint32 to);
 */
.globl interrupt_switch_flag
.globl interrupt_switch_task_from
.globl interrupt_switch_task_to
.globl os_arch_context_switch_interrupt
os_arch_context_switch_interrupt:
    pushl %ebp
    movl %esp, %ebp
    movl 0xc(%ebp), %eax
    movl 0x8(%ebp), %ebx

    movl $interrupt_switch_flag, %ecx
    movl (%ecx), %edx
    cmp $0x1, %edx
    jz _reswitch

    movl $0x1, %edx                /*set interrupt_switch_flag to 1*/
    movl %edx, (%ecx)
    movl $interrupt_switch_task_from, %edx    /*set interrupt_switch_task_from*/
    movl %ebx, (%edx)
_reswitch:
    movl $interrupt_switch_task_to, %edx        /*set interrupt_switch_task_to*/
    movl %eax, (%edx)
    leave
    ret
