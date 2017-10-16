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
 * 2006-03-13     Bernard      first version
 * 2013-03-29     aozima       Modify the interrupt interface implementations.
 */

#include <os.h>
#include <rtcpu.h>
#include "s3c24x0.h"

#define MAX_HANDLERS    32

extern uint32_t os_isr_nest;

/* exception and interrupt handler table */
struct rt_irq_desc isr_table[MAX_HANDLERS];
uint32_t interrupt_switch_task_from, interrupt_switch_task_to;
uint32_t interrupt_switch_flag;

/**
 * @addtogroup S3C24X0
 */
/*@{*/

static void os_arch_interrupt_handle(int vector, void *param)
{
    printk("Unhandled interrupt %d occured!!!\n", vector);
}

/**
 * This function will initialize hardware interrupt
 */
void os_arch_interrupt_init(void)
{
    uint32_t idx;

    /* all clear source pending */
    SRCPND = 0x0;

    /* all clear sub source pending */
    SUBSRCPND = 0x0;

    /* all=IRQ mode */
    INTMOD = 0x0;

    /* all interrupt disabled include global bit */
    INTMSK = BIT_ALLMSK;

    /* all sub interrupt disable */
    INTSUBMSK = BIT_SUB_ALLMSK;

    /* all clear interrupt pending */
    INTPND = BIT_ALLMSK;

    /* init exceptions table */
    os_memset(isr_table, 0x00, sizeof(isr_table));
    for (idx = 0; idx < MAX_HANDLERS; idx++) {
        isr_table[idx].handler = os_arch_interrupt_handle;
    }

    /* init interrupt nest, and context in task sp */
    os_isr_nest = 0;
    interrupt_switch_task_from = 0;
    interrupt_switch_task_to = 0;
    interrupt_switch_flag = 0;
}

/**
 * This function will mask a interrupt.
 * @param vector the interrupt number
 */
void os_arch_interrupt_mask(int vector)
{
    INTMSK |= 1 << vector;
}

/**
 * This function will un-mask a interrupt.
 * @param vector the interrupt number
 */
void os_arch_interrupt_umask(int vector)
{
    if (vector == INTNOTUSED6) {
        printk("Interrupt vec %d is not used!\n", vector);
        // while (1);
    } else if (vector == INTGLOBAL)
        INTMSK = 0x0;
    else
        INTMSK &= ~(1 << vector);
}

/**
 * This function will install a interrupt service routine to a interrupt.
 * @param vector the interrupt number
 * @param new_handler the interrupt service routine to be installed
 * @param old_handler the old interrupt service routine
 */
rt_isr_handler_t os_arch_interrupt_install(int vector, rt_isr_handler_t handler,
                                        void *param, char *name)
{
    rt_isr_handler_t old_handler = NULL;

    if (vector < MAX_HANDLERS) {
        old_handler = isr_table[vector].handler;

        if (handler != NULL) {
#ifdef RT_USING_INTERRUPT_INFO
            os_strncpy(isr_table[vector].name, name, OS_NAME_MAX);
#endif /* RT_USING_INTERRUPT_INFO */
            isr_table[vector].handler = handler;
            isr_table[vector].param = param;
        }
    }

    return old_handler;
}

/*@}*/
