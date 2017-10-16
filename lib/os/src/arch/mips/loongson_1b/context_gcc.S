/*
 * File      : context_gcc.S
 * This file is part of RT-Thread RTOS
 * COPYRIGHT (C) 2006 - 2011, RT-Thread Development Team
 *
 * The license and distribution terms for this file may be
 * found in the file LICENSE in this distribution or at
 * http://www.rt-thread.org/license/LICENSE
 *
 * Change Logs:
 * Date           Author       Notes
 * 2010-05-17     swkyer       first version
 * 2010-09-11     bernard      port to Loongson SoC3210
 * 2011-08-08     lgnq         port to Loongson LS1B
 */

#include "../common/mips.inc"
#include "../common/stackframe.h"

    .section ".text", "ax"
    .set noreorder

/*
 * os_sr_t os_enter_critical()
 */
    .globl os_enter_critical
os_enter_critical:
    mfc0    v0, CP0_STATUS
    and     v1, v0, 0xfffffffe
    mtc0    v1, CP0_STATUS
    jr      ra
    nop

/*
 * void os_exit_critical(os_sr_t sr)
 */
    .globl os_exit_critical
os_exit_critical:
    mtc0    a0, CP0_STATUS
    jr      ra
    nop

/*
 * void os_arch_context_switch(rt_uint32 from, rt_uint32 to)
 * a0 --> from
 * a1 --> to
 */
    .globl os_arch_context_switch
os_arch_context_switch:
    mtc0    ra, CP0_EPC
    SAVE_ALL

    sw      sp, 0(a0)       /* store sp in preempted tasks TCB */
    lw      sp, 0(a1)       /* get new task stack pointer */

    RESTORE_ALL_AND_RET

/*
 * void os_arch_context_switch_to(rt_uint32 to)/*
 * a0 --> to
 */
    .globl os_arch_context_switch_to
os_arch_context_switch_to:
    lw      sp, 0(a0)       /* get new task stack pointer */

    RESTORE_ALL_AND_RET

/*
 * void os_arch_context_switch_interrupt(rt_uint32 from, rt_uint32 to)/*
 */
    .globl interrupt_switch_flag
    .globl interrupt_switch_task_from
    .globl interrupt_switch_task_to
    .globl os_arch_context_switch_interrupt
os_arch_context_switch_interrupt:
    la      t0, interrupt_switch_flag
    lw      t1, 0(t0)
    nop
    bnez    t1, _reswitch
    nop
    li      t1, 0x01                       /* set interrupt_switch_flag to 1 */
    sw      t1, 0(t0)
    la      t0, interrupt_switch_task_from   /* set interrupt_switch_task_from */
    sw      a0, 0(t0)
_reswitch:
    la      t0, interrupt_switch_task_to     /* set interrupt_switch_task_to */
    sw      a1, 0(t0)
    jr      ra
    nop

/*
 * void os_arch_context_switch_interrupt_do(int32_t flag)
 */
    .globl os_isr_enter
    .globl os_isr_leave
    .globl mips_irq_handle
mips_irq_handle:
    SAVE_ALL

    mfc0    t0, CP0_CAUSE
    and     t1, t0, 0xff
    bnez    t1, spurious_interrupt        /* check exception */
    nop

    /* let k0 keep the current context sp */
    move    k0, sp
    /* switch to kernel stack */
    li      sp, SYSTEM_STACK

    jal     os_isr_enter
    nop
    jal     interrupt_switch_dispatch
    nop
    jal     os_isr_leave
    nop

    /* switch sp back to task's context */
    move    sp, k0

    /*
     * if interrupt_switch_flag set, jump to
     * os_arch_context_switch_interrupt_do and don't return
     */
    la      k0, interrupt_switch_flag
    lw      k1, 0(k0)
    beqz    k1, spurious_interrupt
    nop
    sw      zero, 0(k0)                     /* clear flag */
    nop

    /*
     * switch to the new task
     */
    la      k0, interrupt_switch_task_from
    lw      k1, 0(k0)
    nop
    sw      sp, 0(k1)                       /* store sp in preempted tasks's TCB */

    la      k0, interrupt_switch_task_to
    lw      k1, 0(k0)
    nop
    lw      sp, 0(k1)                       /* get new task's stack pointer */
    j       spurious_interrupt
    nop

spurious_interrupt:
    RESTORE_ALL_AND_RET

    .set reorder
