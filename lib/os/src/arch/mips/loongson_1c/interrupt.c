/*
 * File      : interrupt.c
 * This file is part of RT-Thread RTOS
 * COPYRIGHT (C) 2006 - 2011, RT-Thread Development Team
 *
 * The license and distribution terms for this file may be
 * found in the file LICENSE in this distribution or at
 * http://www.rt-thread.org/license/LICENSE
 *
 * Change Logs:
 * Date                  Author       Notes
 * 2010-10-15     Bernard      first version
 * 2010-10-15     lgnq           modified for LS1B
 * 2013-03-29     aozima       Modify the interrupt interface implementations.
 * 2015-07-06     chinesebear modified for loongson 1c
 */

#include <os.h>
#include <rtcpu.h>
#include "ls1c.h"

#define MAX_INTR 32

extern uint32_t os_isr_nest;
uint32_t interrupt_switch_task_from;
uint32_t interrupt_switch_task_to;
uint32_t interrupt_switch_flag;

static struct rt_irq_desc irq_handle_table[MAX_INTR];
void interrupt_switch_dispatch(void *ptreg);
void os_arch_timer_handler();

static struct ls1c_intc_regs volatile *ls1c_hw0_icregs
= (struct ls1c_intc_regs volatile *)(LS1C_INTREG_BASE);

/**
 * @addtogroup Loongson LS1B
 */

/*@{*/

static void os_arch_interrupt_handler(int vector, void *param)
{
    printk("Unhandled interrupt %d occured!!!\n", vector);
}

/**
 * This function will initialize hardware interrupt
 */
void os_arch_interrupt_init(void)
{
    int32_t idx;

    /* pci active low */
    ls1c_hw0_icregs->int_pol = -1;       //must be done here 20110802 lgnq
    /* make all interrupts level triggered */
    (ls1c_hw0_icregs+0)->int_edge = 0x0000e000;
    /* mask all interrupts */
    (ls1c_hw0_icregs+0)->int_clr = 0xffffffff;

    os_memset(irq_handle_table, 0x00, sizeof(irq_handle_table));
    for (idx = 0; idx < MAX_INTR; idx++) {
        irq_handle_table[idx].handler = os_arch_interrupt_handler;
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
    /* mask interrupt */
    (ls1c_hw0_icregs+(vector>>5))->int_en &= ~(1 << (vector&0x1f));
}

/**
 * This function will un-mask a interrupt.
 * @param vector the interrupt number
 */
void os_arch_interrupt_umask(int vector)
{
    (ls1c_hw0_icregs+(vector>>5))->int_en |= (1 << (vector&0x1f));
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

    if (vector >= 0 && vector < MAX_INTR) {
        old_handler = irq_handle_table[vector].handler;

#ifdef RT_USING_INTERRUPT_INFO
        os_strncpy(irq_handle_table[vector].name, name, OS_NAME_MAX);
#endif /* RT_USING_INTERRUPT_INFO */
        irq_handle_table[vector].handler = handler;
        irq_handle_table[vector].param = param;
    }

    return old_handler;
}

void interrupt_switch_dispatch(void *ptreg)
{
    int irq;
    void *param;
    rt_isr_handler_t irq_func;
    static uint32_t status = 0;
    uint32_t c0_status;
    uint32_t c0_cause;
    volatile uint32_t cause_im;
    volatile uint32_t status_im;
    uint32_t pending_im;

    /* check os timer */
    c0_status = read_c0_status();
    c0_cause = read_c0_cause();

    cause_im = c0_cause & ST0_IM;
    status_im = c0_status & ST0_IM;
    pending_im = cause_im & status_im;

    if (pending_im & CAUSEF_IP7) {
        os_arch_timer_handler();
    }

    if (pending_im & CAUSEF_IP2) {
        /* the hardware interrupt */
        status = ls1c_hw0_icregs->int_isr;
        if (!status)
            return;

        for (irq = MAX_INTR; irq > 0; --irq) {
            if ((status & (1 << irq))) {
                status &= ~(1 << irq);

                irq_func = irq_handle_table[irq].handler;
                param = irq_handle_table[irq].param;

                /* do interrupt */
                irq_func(irq, param);

#ifdef RT_USING_INTERRUPT_INFO
                irq_handle_table[irq].counter++;
#endif /* RT_USING_INTERRUPT_INFO */

                /* ack interrupt */
                ls1c_hw0_icregs->int_clr |= (1 << irq);
            }
        }
    } else if (pending_im & CAUSEF_IP3) {
        printk("%s %d\r\n", __FUNCTION__, __LINE__);
    } else if (pending_im & CAUSEF_IP4) {
        printk("%s %d\r\n", __FUNCTION__, __LINE__);
    } else if (pending_im & CAUSEF_IP5) {
        printk("%s %d\r\n", __FUNCTION__, __LINE__);
    } else if (pending_im & CAUSEF_IP6) {
        printk("%s %d\r\n", __FUNCTION__, __LINE__);
    }
}

/*@}*/

