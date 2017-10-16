/*
 * File      : trap.c
 * This file is part of RT-Thread RTOS
 * COPYRIGHT (C) 2006, RT-Thread Develop Team
 *
 * The license and distribution terms for this file may be
 * found in the file LICENSE in this distribution or at
 * http://www.rt-thread.org/license/LICENSE
 *
 * Change Logs:
 * Date           Author       Notes
 * 2011-09-23     Bernard      first version
 */

#include <os.h>
#include <rtcpu.h>

#include "am33xx.h"
#include "interrupt.h"

#ifdef RT_USING_GDB
#include "gdb_stub.h"
#endif

/**
 * @addtogroup AM33XX
 */
/*@{*/

extern os_task_t *os_current_task;
#ifdef RT_USING_FINSH
extern long list_task(void);
#endif

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

#ifdef RT_USING_GDB
    regs->pc -= 4; //lr in undef is pc + 4
    if (gdb_undef_hook(regs))
        return;
#endif

    os_arch_show_register(regs);

    printk("undefined instruction\n");
    printk("task %.*s stack:\n", OS_NAME_MAX, os_current_task->name);

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
    printk("task %.*s stack:\n", OS_NAME_MAX, os_current_task->name);

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

#ifdef RT_USING_GDB
    if (gdb_mem_fault_handler) {
        regs->pc = (uint32_t)gdb_mem_fault_handler;
        return;
    }
#endif
    os_arch_show_register(regs);

    printk("data abort\n");
    printk("task %.*s stack:\n", OS_NAME_MAX, os_current_task->name);

#ifdef RT_USING_FINSH
    list_task();
#endif
    os_arch_shutdown();
}

void os_arch_trap_irq()
{
    void *param;
    uint32_t ir;
    rt_isr_handler_t isr_func;
    extern struct rt_irq_desc isr_table[];

    ir = os_arch_interrupt_get_active(INT_IRQ);
    if (ir == 127) {
        /* new IRQ generation */
        os_arch_interrupt_ack(INT_IRQ);
        ir = os_arch_interrupt_get_active(INT_IRQ);
        if (ir == 127) {
            /* still spurious interrupt, get out */
            /*printk("still spurious interrupt\n");*/
            return;
        }
        /*printk("new IRQ: %d\n", ir);*/
    }

    /* get interrupt service routine */
    isr_func = isr_table[ir].handler;
    param = isr_table[ir].param;

    /* turn to interrupt service routine */
    if (isr_func != NULL)
        isr_func(ir, param);

    /* new IRQ generation */
    os_arch_interrupt_ack(INT_IRQ);
}

void os_arch_trap_fiq()
{
    void *param;
    uint32_t ir;
    rt_isr_handler_t isr_func;
    extern struct rt_irq_desc isr_table[];

    ir = os_arch_interrupt_get_active(INT_FIQ);

    /* get interrupt service routine */
    isr_func = isr_table[ir].handler;
    param = isr_table[ir].param;

    /* turn to interrupt service routine */
    isr_func(ir, param);

    /* new FIQ generation */
    os_arch_interrupt_ack(INT_FIQ);
}

/*@}*/
