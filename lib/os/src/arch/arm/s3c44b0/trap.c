/*
 * File      : trap.c
 * This file is part of RT-Thread RTOS
 * COPYRIGHT (C) 2006, RT-Thread Development Team
 *
 * The license and distribution terms for this file may be
 * found in the file LICENSE in this distribution or at
 * http://openlab.rt-thread.com/license/LICENSE
 *
 * Change Logs:
 * Date           Author       Notes
 * 2006-09-06     XuXinming    first version
 * 2006-09-15     Bernard      modify os_arch_trap_irq for more effective
 */

#include <os.h>
#include <rtcpu.h>

#include "s3c44b0.h"

extern unsigned char interrupt_bank0[256];
extern unsigned char interrupt_bank1[256];
extern unsigned char interrupt_bank2[256];
extern unsigned char interrupt_bank3[256];

extern os_task_t *os_current_task;

/**
 * @addtogroup S3C44B0
 */
/*@{*/

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
    printk("r08:0x%08x r09:0x%08x r10:0x%08x\n", regs->r8, regs->r9, regs->r10);
    printk("fp :0x%08x ip :0x%08x\n", regs->fp, regs->ip);
    printk("sp :0x%08x lr :0x%08x pc :0x%08x\n", regs->sp, regs->lr, regs->pc);
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
    printk("software interrupt\n");
    os_arch_show_register(regs);
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

extern rt_isr_handler_t isr_table[];
void os_arch_trap_irq()
{
    uint32_t ispr, intstat;
    rt_isr_handler_t isr_func;

#ifdef BSP_INT_DEBUG
    printk("irq coming, ");
#endif
    intstat = I_ISPR & 0x7ffffff;
#ifdef BSP_INT_DEBUG
    printk("I_ISPR: %d\n", intstat);
#endif

    ispr = intstat;

    /* to find interrupt */
    if (intstat & 0xff) /* lowest 8bits */
    {
        intstat = interrupt_bank0[intstat & 0xff];
        isr_func = (rt_isr_handler_t)isr_table[ intstat ];
    } else if (intstat & 0xff00) /* low 8bits */
    {
        intstat = interrupt_bank1[(intstat & 0xff00) >> 8];
        isr_func = (rt_isr_handler_t)isr_table[ intstat ];
    } else if (intstat & 0xff0000) /* high 8bits */
    {
        intstat = interrupt_bank2[(intstat & 0xff0000) >> 16];
        isr_func = (rt_isr_handler_t)isr_table[ intstat ];
    } else if (intstat & 0xff000000) /* highest 8bits */
    {
        intstat = interrupt_bank3[(intstat & 0xff000000) >> 24];
        isr_func = (rt_isr_handler_t)isr_table[ intstat ];
    }
    else return;

#ifdef BSP_INT_DEBUG
    printk("irq: %d happen\n", intstat);
#endif

    /* turn to interrupt service routine */
    isr_func(intstat);

    I_ISPC = ispr;        /* clear interrupt */
}

void os_arch_trap_fiq()
{
    printk("fast interrupt request\n");
}

/*@}*/
