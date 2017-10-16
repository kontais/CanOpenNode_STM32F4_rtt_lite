/*
 * File      : interrupt.c
 * This file is part of RT-Thread RTOS
 * COPYRIGHT (C) 2009, RT-Thread Development Team
 *
 * The license and distribution terms for this file may be
 * found in the file LICENSE in this distribution or at
 * http://www.rt-thread.org/license/LICENSE
 *
 * Change Logs:
 * Date           Author       Notes
 * 2009-01-05     Bernard      first version
 */

#include <rtcpu.h>
#include <asm/ppc4xx.h>
#include <asm/processor.h>

/* interrupt nest */
extern volatile uint8_t os_isr_nest;

/* exception and interrupt handler table */
#define MAX_HANDLERS 32
struct rt_irq_desc isr_table[MAX_HANDLERS];

uint32_t interrupt_switch_task_from, interrupt_switch_task_to;
uint32_t os_task_switch_interrput_flag;

rt_isr_handler_t os_arch_interrupt_handler(uint32_t vector, void* param)
{
    printk("Unhandled interrupt %d occured!!!\n", vector);
    return NULL;
}

void uic_irq_ack(unsigned int vec)
{
    mtdcr(uic0sr, UIC_MASK(vec));
}

void  uic_int_handler (unsigned int vec)
{
    os_isr_enter();

    /* Allow external interrupts to the CPU. */
    if (isr_table [vec].handler != 0) {
       (*isr_table[vec].handler)(vec, isr_table[vec].param);
    }
    uic_irq_ack(vec);

    os_isr_leave();
}

/* handler for UIC interrupt */
void uic_interrupt(uint32_t uic_base, int vec_base)
{
    int vec;
    uint32_t uic_msr;
    uint32_t msr_shift;

    /*
     * Read masked interrupt status register to determine interrupt source
     */
    uic_msr = get_dcr(uic_base + UIC_MSR);
    msr_shift = uic_msr;
    vec = vec_base;

    while (msr_shift != 0) {
        if (msr_shift & 0x80000000)
            uic_int_handler(vec);

        /*
         * Shift msr to next position and increment vector
         */
        msr_shift <<= 1;
        vec++;
    }
}

rt_isr_handler_t os_arch_interrupt_install(int vector, rt_isr_handler_t new_handler,
    void* param, char* name)
{
    int    intVal;
    rt_isr_handler_t old_handler;

    if (((int)vector < 0)  || ((int) vector >= MAX_HANDLERS)) {
        return NULL;   /*  out of range  */
    }

    /* install the handler in the system interrupt table  */
    intVal = os_enter_critical (); /* lock interrupts to prevent races */

    old_handler = isr_table[vector].handler;
    isr_table[vector].handler = new_handler;
    isr_table[vector].param = param;

    os_exit_critical (intVal);
}

void os_arch_interrupt_mask(int vector)
{
    mtdcr(uic0er, mfdcr(uic0er) & ~UIC_MASK(vector));
}

void os_arch_interrupt_unmask(int vector)
{
    mtdcr(uic0er, mfdcr(uic0er) | UIC_MASK(vector));
}

void os_arch_interrupt_init()
{
    int vector;
    uint32_t pit_value;

    pit_value = OS_TICKS_PER_SEC * (100000000 / RT_CPU_FREQ);

    /* enable pit */
    mtspr(SPRN_PIT, pit_value);
    mtspr(SPRN_TCR, 0x4400000);

    /* set default interrupt handler */
    for (vector = 0; vector < MAX_HANDLERS; vector++) {
        isr_table [vector].handler = (rt_isr_handler_t)os_arch_interrupt_handler;
        isr_table [vector].param = NULL;
    }

    /* initialize interrupt nest, and context in task sp */
    os_isr_nest = 0;
    interrupt_switch_task_from = 0;
    interrupt_switch_task_to = 0;
    os_task_switch_interrput_flag = 0;
}

/*@}*/
