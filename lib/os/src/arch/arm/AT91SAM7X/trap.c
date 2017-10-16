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
 * 2006-08-25     Bernard      first version
 */

#include <os.h>
#include <rtcpu.h>

#include "AT91SAM7X256.h"

/**
 * @addtogroup AT91SAM7
 */
/*@{*/

void os_arch_trap_irq(void)
{
    int irqno;
    extern struct rt_irq_desc irq_desc[];

    /* get interrupt number */
    irqno = AT91C_BASE_AIC->AIC_ISR;

    /* invoke isr with parameters */
    irq_desc[irqno].handler(irqno, irq_desc[irqno].param);

    /* end of interrupt */
    AT91C_BASE_AIC->AIC_EOICR = 0;
}

void os_arch_trap_fiq(void)
{
    printk("fast interrupt request\n");
}

extern os_task_t* os_current_task;
void os_arch_trap_abort(void)
{
    printk("Abort occured!!! Thread [%s] suspended.\n",os_current_task->name);
    os_task_suspend(os_current_task);
    os_sched();

}
/*@}*/
