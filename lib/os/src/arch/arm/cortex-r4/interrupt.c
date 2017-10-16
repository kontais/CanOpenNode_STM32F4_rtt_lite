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
 * 2008-12-11     XuXinming    first version
 * 2013-03-29     aozima       Modify the interrupt interface implementations.
 */

#include <os.h>
#include <rtcpu.h>

#include <sys_vim.h>
#include <system.h>

#include "armv7.h"

#define MAX_HANDLERS    96

/* exception and interrupt handler table */
struct rt_irq_desc irq_desc[MAX_HANDLERS];

extern volatile uint8_t os_isr_nest;

/* exception and interrupt handler table */
uint32_t interrupt_switch_task_from, interrupt_switch_task_to;
uint32_t interrupt_switch_flag;

/**
 * @addtogroup RM48x50
 */

/*@{*/

static void os_arch_int_not_handle(int vector, void *param)
{
    printk("Unhandled interrupt %d occured!!!\n", vector);
}

#define vimRAM (0xFFF82000U)

void os_arch_interrupt_init(void)
{
    register int i;

    uint32_t *vect_addr;

    /* the initialization is done in sys_startup.c */

    /* init exceptions table */
    os_memset(irq_desc, 0x00, sizeof(irq_desc));
    for (i = 0; i < MAX_HANDLERS; i++) {
        irq_desc[i].handler = os_arch_int_not_handle;

        vect_addr  = (uint32_t *)(vimRAM + i*4);
        *vect_addr = (uint32_t)&irq_desc[i];
    }

    /* init interrupt nest, and context in task sp */
    os_isr_nest = 0;
    interrupt_switch_task_from = 0;
    interrupt_switch_task_to = 0;
    interrupt_switch_flag = 0;
}

void os_arch_interrupt_mask(int vector)
{
    vimDisableInterrupt(vector);
}

void os_arch_interrupt_umask(int vector)
{
    vimEnableInterrupt(vector, SYS_IRQ);
}

/**
 * This function will install a interrupt service routine to a interrupt.
 * @param vector the interrupt number
 * @param handler the interrupt service routine to be installed
 * @param param the parameter for interrupt service routine
 * @name unused.
 *
 * @return the old handler
 */
rt_isr_handler_t os_arch_interrupt_install(int vector, rt_isr_handler_t handler,
                                    void *param, char *name)
{
    rt_isr_handler_t old_handler = NULL;

    if (vector >= 0 && vector < MAX_HANDLERS) {
        old_handler = irq_desc[vector].handler;
        if (handler != NULL) {
            irq_desc[vector].handler = handler;
            irq_desc[vector].param = param;
        }
    }

    return old_handler;
}

/*@}*/
