/*
 * File      : cpuport.c
 * This file is part of RT-Thread RTOS
 * COPYRIGHT (C) 2009 - 2011, RT-Thread Development Team
 *
 * The license and distribution terms for this file may be
 * found in the file LICENSE in this distribution or at
 * http://www.rt-thread.org/license/LICENSE
 *
 * Change Logs:
 * Date           Author       Notes
 * 2011-06-15     aozima       the first version for lpc214x
 * 2013-03-29     aozima       Modify the interrupt interface implementations.
 */

#include <os.h>
#include <rtcpu.h>
#include "lpc214x.h"

#define MAX_HANDLERS    32
#define SVCMODE            0x13

extern uint32_t os_isr_nest;

/* exception and interrupt handler table */
struct rt_irq_desc irq_desc[MAX_HANDLERS];

/**
 * @addtogroup LPC214x
 */
/*@{*/

/**
 * This function will initialize task stack
 *
 * @param tentry the entry of task
 * @param parameter the parameter of entry
 * @param stack_addr the beginning stack address
 * @param texit the function will be called when task exit
 *
 * @return stack address
 */
uint8_t *os_arch_stack_init(void *tentry, void *parameter,
    uint8_t *stack_addr, void *texit)
{
    uint32_t *stk;

    stk      = (uint32_t *)stack_addr;
    *(stk)      = (uint32_t)tentry;        /* entry point */
    *(--stk) = (uint32_t)texit;        /* lr */
    *(--stk) = 0;                            /* r12 */
    *(--stk) = 0;                            /* r11 */
    *(--stk) = 0;                            /* r10 */
    *(--stk) = 0;                            /* r9 */
    *(--stk) = 0;                            /* r8 */
    *(--stk) = 0;                            /* r7 */
    *(--stk) = 0;                            /* r6 */
    *(--stk) = 0;                            /* r5 */
    *(--stk) = 0;                            /* r4 */
    *(--stk) = 0;                            /* r3 */
    *(--stk) = 0;                            /* r2 */
    *(--stk) = 0;                            /* r1 */
    *(--stk) = (uint32_t)parameter;    /* r0 : argument */

    /* cpsr */
    if ((uint32_t)tentry & 0x01)
        *(--stk) = SVCMODE | 0x20;            /* thumb mode */
    else
        *(--stk) = SVCMODE;                    /* arm mode   */

    /* return task's current stack address */
    return (uint8_t *)stk;
}

/* exception and interrupt handler table */
uint32_t interrupt_switch_task_from, interrupt_switch_task_to;
uint32_t interrupt_switch_flag;

void os_arch_interrupt_handler(int vector, void *param)
{
    printk("Unhandled interrupt %d occured!!!\n", vector);
}

/**
 * This function will initialize hardware interrupt
 */
void os_arch_interrupt_init(void)
{
    int32_t index;
    uint32_t *vect_addr, *vect_ctl;

    /* initialize VIC*/
    VICIntEnClr = 0xffffffff;
    VICVectAddr = 0;
    /* set all to IRQ */
    VICIntSelect = 0;

    os_memset(irq_desc, 0x00, sizeof(irq_desc));
    for (index = 0; index < MAX_HANDLERS; index++) {
        irq_desc[index].handler = os_arch_interrupt_handler;

        vect_addr     = (uint32_t *)(VIC_BASE_ADDR + 0x100 + (index << 2));
        vect_ctl     = (uint32_t *)(VIC_BASE_ADDR + 0x200 + (index << 2));

        *vect_addr     = (uint32_t)&irq_desc[index];
        *vect_ctl     = 0xF;
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
    VICIntEnClr = (1 << vector);
}

/**
 * This function will un-mask a interrupt.
 * @param vector the interrupt number
 */
void os_arch_interrupt_umask(int vector)
{
    VICIntEnable = (1 << vector);
}

/**
 * This function will install a interrupt service routine to a interrupt.
 * @param vector the interrupt number
 * @param handler the interrupt service routine to be installed
 * @param param the interrupt service function parameter
 * @param name the interrupt name
 * @return old handler
 */
rt_isr_handler_t os_arch_interrupt_install(int vector, rt_isr_handler_t handler,
                                         void *param, char *name)
{
    rt_isr_handler_t old_handler = NULL;

    if (vector >= 0 && vector < MAX_HANDLERS) {
        uint32_t* vect_ctl     = (uint32_t *)(VIC_BASE_ADDR + 0x200 + (vector << 2));

        /* assign IRQ slot and enable this slot */
        *vect_ctl = 0x20 | (vector & 0x1F);

        old_handler = irq_desc[vector].handler;
        if (handler != NULL) {
            irq_desc[vector].handler = handler;
            irq_desc[vector].param = param;
        }
    }

    return old_handler;
}

/**
 * this function will reset CPU
 *
 */
void os_arch_reset(void)
{
}

/**
 * this function will shutdown CPU
 *
 */
void os_arch_shutdown()
{
    printk("shutdown...\n");

    while (1);
}

void os_arch_trap_irq(void)
{
    int irqno;
    struct rt_irq_desc* irq;
    extern struct rt_irq_desc irq_desc[];

    irq = (struct rt_irq_desc*) VICVectAddr;
    irqno = ((uint32_t) irq - (uint32_t) &irq_desc[0])/sizeof(struct rt_irq_desc);

    /* invoke isr */
    irq->handler(irqno, irq->param);

    /* acknowledge Interrupt */
    // VICVectAddr = 0;
}

void os_arch_trap_fiq(void)
{
    printk("fast interrupt request\n");
}

/*@}*/
