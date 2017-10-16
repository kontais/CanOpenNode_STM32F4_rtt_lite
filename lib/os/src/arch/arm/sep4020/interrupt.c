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
#include <sep4020.h>

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

    /*Make sure all intc registers in proper state*/

    /*mask all the irq*/
    *(RP)(INTC_IMR) = 0xFFFFFFFF;

    /*enable all the irq*/
    *(RP)(INTC_IER)    = 0XFFFFFFFF;

    /*Dont use any forced irq*/
    *(RP)(INTC_IFR) = 0x0;

    /*Disable all the fiq*/
    *(RP)(INTC_FIER) = 0x0;

    /*Mask all the fiq*/
    *(RP)(INTC_FIMR) = 0x0F;

    /*Dont use forced fiq*/
    *(RP)(INTC_FIFR) = 0x0;

    /*Intrrupt priority register*/
    *(RP)(INTC_IPLR) = 0x0;

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
    *(RP)(INTC_IMR) |= 1 << vector;
}

/**
 * This function will un-mask a interrupt.
 * @param vector the interrupt number
 */
void os_arch_interrupt_umask(int vector)
{
    if (vector == 16) {
        printk("Interrupt vec %d is not used!\n", vector);
    }
    else
        *(RP)(INTC_IMR) &= ~(1 << vector);
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
