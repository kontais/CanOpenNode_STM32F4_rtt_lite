/*
 * File      : trap.c
 * This file is part of RT-Thread RTOS
 * COPYRIGHT (C) 2006 - 2013, RT-Thread Development Team
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
 * 2013-03-16     Peng Fan     Modifiled from sep4020
 */

#include <os.h>
#include <rtcpu.h>

#include <sep6200.h>

/**
 * @addtogroup sep6200
 */
/*@{*/

extern os_task_t *os_current_task;

/**
 * this function will show registers of CPU
 *
 * @param regs the registers point
 */

void os_arch_show_register (struct os_arch_register *regs)
{
    printk("Execption:\n");
    printk("r00:0x%08x r01:0x%08x r02:0x%08x r03:0x%08x\n", regs->r0, regs->r1, regs->r2, regs->r3);
    printk("r04:0x%08x r05:0x%08x r06:0x%08x r07:0x%08x\n", regs->r4, regs->r5, regs->r6, regs->r7);
    printk("r08:0x%08x r09:0x%08x r10:0x%08x r11:0x%08x\n", regs->r8, regs->r9, regs->r10,regs->r11);
    printk("r12:0x%08x r13:0x%08x r14:0x%08x r15:0x%08x\n", regs->r12,regs->r13,regs->r14,regs->r15);
    printk("r16:0x%08x r17:0x%08x r18:0x%08x r19:0x%08x\n", regs->r16,regs->r17,regs->r18,regs->r19);
    printk("r20:0x%08x r21:0x%08x r22:0x%08x r23:0x%08x\n", regs->r20,regs->r21,regs->r22,regs->r23);
    printk("r24:0x%08x sb:0x%08x  sl:0x%08xfp :0x%08x ip :0x%08x\n",regs->r24,regs->sb,regs->sl,regs->fp,regs->ip);
    printk("sp :0x%08x lr :0x%08x pc :0x%08x\n", regs->sp, regs->lr, regs->pc);
    printk("asr:0x%08x bsr:0x%08x\n", regs->asr,regs->bsr);
}

/**
 * When unicore comes across an instruction which it cannot handle,
 * it takes the extn instruction trap.
 *
 * @param regs system registers
 *
 * @note never invoke this function in application
 */
void os_arch_trap_extn(struct os_arch_register *regs)
{
    os_arch_show_register(regs);

    printk("extn instruction\n");
    printk("task - %s stack:\n", os_current_task->name);
    os_arch_backtrace((uint32_t *)regs->fp, (uint32_t)os_current_task->entry);

    os_arch_shutdown();
}

/**
 * The software interrupt instruction (SWI) is used for entering
 * Supervisor mode, usually to request a particular supervisor
 * function.
 *
 * @param regs system registers
 *
 * @note never invoke this function in application
 */
void os_arch_trap_swi(struct os_arch_register *regs)
{
    os_arch_show_register(regs);

    printk("software interrupt\n");
    os_arch_shutdown();
}

/**
 * An abort indicates that the current memory access cannot be completed,
 * which occurs during an instruction prefetch.
 *
 * @param regs system registers
 *
 * @note never invoke this function in application
 */
void os_arch_trap_pabt(struct os_arch_register *regs)
{
    os_arch_show_register(regs);

    printk("prefetch abort\n");
    printk("task - %s stack:\n", os_current_task->name);
    os_arch_backtrace((uint32_t *)regs->fp, (uint32_t)os_current_task->entry);

    os_arch_shutdown();
}

/**
 * An abort indicates that the current memory access cannot be completed,
 * which occurs during a data access.
 *
 * @param regs system registers
 *
 * @note never invoke this function in application
 */
void os_arch_trap_dabt(struct os_arch_register *regs)
{
    os_arch_show_register(regs);

    printk("data abort\n");
    printk("task - %s stack:\n", os_current_task->name);
    os_arch_backtrace((uint32_t *)regs->fp, (uint32_t)os_current_task->entry);

    os_arch_shutdown();
}

/**
 * Normally, system will never reach here
 *
 * @param regs system registers
 *
 * @note never invoke this function in application
 */
void os_arch_trap_resv(struct os_arch_register *regs)
{
    printk("not used\n");
    os_arch_show_register(regs);
    os_arch_shutdown();
}

extern struct rt_irq_desc isr_table[];

void os_arch_trap_irq(void)
{
  uint32_t intstat;
  uint32_t irq = 0;
  rt_isr_handler_t isr_func;
  void *param;

  /* get the interrupt number */
    irq = *(RP)(SEP6200_VIC_IRQ_VECTOR_NUM);

  /* get interrupt service routine */
  isr_func = isr_table[irq].handler;
  param = isr_table[irq].param;

  /* turn to interrupt service routine */
  isr_func(irq, param);

#ifdef RT_USING_INTERRUPT_INFO
    isr_table[irq].counter++;
#endif /* RT_USING_INTERRUPT_INFO */
}

void os_arch_trap_fiq()
{
    printk("fast interrupt request\n");
}

/*@}*/
