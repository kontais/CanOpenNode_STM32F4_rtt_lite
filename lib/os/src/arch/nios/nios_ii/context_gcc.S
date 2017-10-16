/*
 * File      : context_gcc.S
 * This file is part of RT-Thread RTOS
 * COPYRIGHT (C) 2006-2011, RT-Thread Development Team
 *
 * The license and distribution terms for this file may be
 * found in the file LICENSE in this distribution or at
 * http://www.rt-thread.org/license/LICENSE
 *
 * Change Logs:
 * Date           Author       Notes
 * 2011-02-14     aozima       first implementation for Nios II.
 * 2011-02-20     aozima       fix context&switch bug.
 */

/**
 * @addtogroup NIOS_II
 */
/*@{*/

.text

.set noat

/*
 * os_sr_t os_enter_critical();
 */
.global os_enter_critical
.type os_enter_critical, %function
os_enter_critical:
    rdctl r2, status       /* return status */
    wrctl status, zero     /* disable interrupt */
    ret

/*
 * void os_exit_critical(os_sr_t sr);
 */
.global os_exit_critical
.type os_exit_critical, %function
os_exit_critical:
    wrctl status, r4       /* enable interrupt by argument */
    ret

/* void os_arch_context_switch_interrupt_do(void) */
.global os_arch_context_switch_interrupt_do
.type os_arch_context_switch_interrupt_do, %function
os_arch_context_switch_interrupt_do:
    /* save from task */
    addi sp,sp,-72

    /* frist save r2,so that save status */
    stw r2,   4(sp)

    /* save status */
    /* when the interrupt happen,the interrupt is enable */
    movi r2, 1
    stw  r2, 68(sp)  /* status */

    stw r3,   8(sp)
    stw r4,  12(sp)

    /* get & save from task pc */
    ldw r4,%gprel(os_current_task_entry)(gp)
    stw r4,   0(sp)  /* task pc */

    stw r5,  16(sp)
    stw r6,  20(sp)
    stw r7,  24(sp)

    stw r16, 28(sp)
    stw r17, 32(sp)
    stw r18, 36(sp)
    stw r19, 40(sp)
    stw r20, 44(sp)
    stw r21, 48(sp)
    stw r22, 52(sp)
    stw r23, 56(sp)

    stw fp,  60(sp)
    stw ra,  64(sp)

    /* save from task sp */
    /* interrupt_switch_task_from = &from_task->sp */
    ldw r4, %gprel(interrupt_switch_task_from)(gp)
    /* *r4(from_task->sp) = sp */
    stw sp, (r4)

    /* clear interrupt_switch_flag */
    /* interrupt_switch_flag = 0 */
    stw zero,%gprel(interrupt_switch_flag)(gp)

    /* load to task sp */
    /* r4 = interrupt_switch_task_to(&to_task->sp) */
    ldw r4, %gprel(interrupt_switch_task_to)(gp)
    /* sp = to_task->sp */
    ldw sp, (r4)

    ldw r2,  68(sp)  /* status */
    wrctl estatus, r2

    ldw ea,   0(sp)  /* task pc */
    ldw r2,   4(sp)
    ldw r3,   8(sp)
    ldw r4,  12(sp)
    ldw r5,  16(sp)
    ldw r6,  20(sp)
    ldw r7,  24(sp)

    ldw r16, 28(sp)
    ldw r17, 32(sp)
    ldw r18, 36(sp)
    ldw r19, 40(sp)
    ldw r20, 44(sp)
    ldw r21, 48(sp)
    ldw r22, 52(sp)
    ldw r23, 56(sp)

    ldw fp,  60(sp)
    ldw ra,  64(sp)

    addi sp, sp, 72

    /* estatus --> status,ea --> pc */
    eret

/*
 * void os_arch_context_switch(rt_uint32 from, rt_uint32 to);
 * r4: from
 * r5: to
 */
.global os_arch_context_switch
.type os_arch_context_switch, %function
os_arch_context_switch:
    /* save from task */
    addi sp,sp,-72

    /* frist save r2,so that save status */
    stw r2,   4(sp)

    /* save status */
    rdctl r2, status
    stw r2,  68(sp)  /* status */

    stw ra,   0(sp)  /* return from os_arch_context_switch */
    stw r3,   8(sp)
    stw r4,  12(sp)
    stw r5,  16(sp)
    stw r6,  20(sp)
    stw r7,  24(sp)

    stw r16, 28(sp)
    stw r17, 32(sp)
    stw r18, 36(sp)
    stw r19, 40(sp)
    stw r20, 44(sp)
    stw r21, 48(sp)
    stw r22, 52(sp)
    stw r23, 56(sp)

    stw fp,  60(sp)
    stw ra,  64(sp)

    /* save form task sp */
    /* from_task->sp(r4) = sp */
    stw sp, (r4)

    /* update interrupt_switch_task_from */
    /* interrupt_switch_task_from = r4(from_task->sp) */
    stw r4,%gprel(interrupt_switch_task_from)(gp)

    /* update interrupt_switch_task_to */
    /* interrupt_switch_task_to = r5 */
    stw r5,%gprel(interrupt_switch_task_to)(gp)

    /* get to task sp */
    /* sp = interrupt_switch_task_to(r5:to_task->sp) */
    ldw sp, (r5)

    ldw r2,  68(sp)  /* status */
    wrctl estatus, r2

    ldw ea, 0(sp)    /* task pc */

    ldw r2,   4(sp)
    ldw r3,   8(sp)
    ldw r4,  12(sp)
    ldw r5,  16(sp)
    ldw r6,  20(sp)
    ldw r7,  24(sp)

    ldw r16, 28(sp)
    ldw r17, 32(sp)
    ldw r18, 36(sp)
    ldw r19, 40(sp)
    ldw r20, 44(sp)
    ldw r21, 48(sp)
    ldw r22, 52(sp)
    ldw r23, 56(sp)

    ldw fp,  60(sp)
    ldw ra,  64(sp)

    addi sp, sp, 72

    /* estatus --> status,ea --> pc */
    eret

/*
 * void os_arch_context_switch_interrupt(rt_uint32 from, rt_uint32 to);
 * r4: from
 * r5: to
 */
.global os_arch_context_switch_interrupt
.type os_arch_context_switch_interrupt, %function
os_arch_context_switch_interrupt:
    /* if (interrupt_switch_flag != 0) _from_task_not_change */
    ldw r2,%gprel(interrupt_switch_flag)(gp)
    bne r2,zero,_from_task_not_change

_from_task_change:
    /* save ea -> os_current_task_entry */
    addi ea,ea,-4
    stw ea,%gprel(os_current_task_entry)(gp)

    /* set interrupt_switch_flag to 1 */
    movi r2, 1
    stw r2,%gprel(interrupt_switch_flag)(gp)

    /* update interrupt_switch_task_from */
    stw r4,%gprel(interrupt_switch_task_from)(gp)

_from_task_not_change:
    /* update interrupt_switch_task_to */
    stw r5,%gprel(interrupt_switch_task_to)(gp)

    ret

/*
 * void os_arch_context_switch_to(rt_uint32 to);
 * r4: to
 */
.global os_arch_context_switch_to
.type os_arch_context_switch_to, %function
os_arch_context_switch_to:
    /* save to task */
    stw r4,%gprel(interrupt_switch_task_to)(gp)

    /* get sp */
    ldw sp, (r4) // sp = *r4

    ldw r2,  68(sp)  /* status */
    wrctl estatus, r2

    ldw ea, 0(sp)    /* task entry */

    ldw r2,   4(sp)
    ldw r3,   8(sp)
    ldw r4,  12(sp)
    ldw r5,  16(sp)
    ldw r6,  20(sp)
    ldw r7,  24(sp)

    ldw r16, 28(sp)
    ldw r17, 32(sp)
    ldw r18, 36(sp)
    ldw r19, 40(sp)
    ldw r20, 44(sp)
    ldw r21, 48(sp)
    ldw r22, 52(sp)
    ldw r23, 56(sp)

    ldw fp,  60(sp)
    ldw ra,  64(sp)

    addi sp, sp, 72

    /* estatus --> status,ea --> pc */
    eret

/*@}*/
