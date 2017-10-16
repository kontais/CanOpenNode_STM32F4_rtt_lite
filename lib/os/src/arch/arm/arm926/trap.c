/*
 * File      : trap.c
 * This file is part of RT-Thread RTOS
 * COPYRIGHT (C) 2006 - 2015, RT-Thread Development Team
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
 * 2011-01-13     weety      modified from mini2440
 * 2015-04-15     ArdaFu     Split from AT91SAM9260 BSP
 */

#include <os.h>
#include <rtcpu.h>

#define INT_IRQ     0x00
#define INT_FIQ     0x01

extern os_task_t *os_current_task;
#ifdef RT_USING_FINSH
extern long list_task(void);
#endif

struct os_arch_register
{
    uint32_t r0;
    uint32_t r1;
    uint32_t r2;
    uint32_t r3;
    uint32_t r4;
    uint32_t r5;
    uint32_t r6;
    uint32_t r7;
    uint32_t r8;
    uint32_t r9;
    uint32_t r10;
    uint32_t fp;
    uint32_t ip;
    uint32_t sp;
    uint32_t lr;
    uint32_t pc;
    uint32_t cpsr;
    uint32_t ORIG_r0;
};

/**
 * this function will show registers of CPU
 *
 * @param regs the registers point
 */

void os_arch_show_register (struct os_arch_register *regs)
{
    printk("Execption:\n");
    printk("r00:0x%08x r01:0x%08x r02:0x%08x r03:0x%08x\n",
               regs->r0, regs->r1, regs->r2, regs->r3);
    printk("r04:0x%08x r05:0x%08x r06:0x%08x r07:0x%08x\n",
               regs->r4, regs->r5, regs->r6, regs->r7);
    printk("r08:0x%08x r09:0x%08x r10:0x%08x\n",
               regs->r8, regs->r9, regs->r10);
    printk("fp :0x%08x ip :0x%08x\n",
               regs->fp, regs->ip);
    printk("sp :0x%08x lr :0x%08x pc :0x%08x\n",
               regs->sp, regs->lr, regs->pc);
    printk("cpsr:0x%08x\n", regs->cpsr);
}

/**
 * When ARM7TDMI comes across an instruction which it cannot handle,
 * it takes the undefined instruction trap.
 *
 * @param regs system registers
 *
 * @note never invoke this function in application
 */
void os_arch_trap_udef(struct os_arch_register *regs)
{
    os_arch_show_register(regs);

    printk("undefined instruction\n");
    printk("task - %s stack:\n", os_current_task->name);

#ifdef RT_USING_FINSH
    list_task();
#endif
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
    printk("task - %s stack:\n", OS_NAME_MAX, os_current_task->name);

#ifdef RT_USING_FINSH
    list_task();
#endif
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
    printk("task - %s stack:\n", OS_NAME_MAX, os_current_task->name);

#ifdef RT_USING_FINSH
    list_task();
#endif
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

extern struct rt_irq_desc irq_desc[];
extern uint32_t os_arch_interrupt_get_active(uint32_t fiq_irq);
extern void os_arch_interrupt_ack(uint32_t fiq_irq, uint32_t id);

void os_arch_trap_irq()
{
    rt_isr_handler_t isr_func;
    uint32_t irq;
    void *param;

    /* get irq number */
    irq = os_arch_interrupt_get_active(INT_IRQ);

    /* get interrupt service routine */
    isr_func = irq_desc[irq].handler;
    param = irq_desc[irq].param;

    /* turn to interrupt service routine */
    isr_func(irq, param);

    os_arch_interrupt_ack(INT_IRQ, irq);
#ifdef RT_USING_INTERRUPT_INFO
    irq_desc[irq].counter++;
#endif
}

void os_arch_trap_fiq()
{
    rt_isr_handler_t isr_func;
    uint32_t irq;
    void *param;

    /* get irq number */
    irq = os_arch_interrupt_get_active(INT_FIQ);

    /* get interrupt service routine */
    isr_func = irq_desc[irq].handler;
    param = irq_desc[irq].param;

    /* turn to interrupt service routine */
    isr_func(irq, param);

    os_arch_interrupt_ack(INT_FIQ, irq);
#ifdef RT_USING_INTERRUPT_INFO
    irq_desc[irq].counter++;
#endif
}
