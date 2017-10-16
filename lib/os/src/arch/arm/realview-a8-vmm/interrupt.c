/*
 * File      : interrupt.c
 * This file is part of RT-Thread RTOS
 * COPYRIGHT (C) 2013-2014, RT-Thread Development Team
 *
 * The license and distribution terms for this file may be
 * found in the file LICENSE in this distribution or at
 * http://www.rt-thread.org/license/LICENSE
 *
 * Change Logs:
 * Date           Author       Notes
 * 2013-07-06     Bernard      first version
 * 2014-04-03     Grissiom     port to VMM
 */

#include <rtcpu.h>
#include <os.h>
#include "realview.h"
#include "gic.h"

#ifdef RT_USING_VMM
#include <vmm.h>
#endif

#define MAX_HANDLERS                NR_IRQS_PBA8

extern volatile uint8_t os_isr_nest;

/* exception and interrupt handler table */
struct rt_irq_desc isr_table[MAX_HANDLERS];

/* Those varibles will be accessed in ISR, so we need to share them. */
uint32_t interrupt_switch_task_from SECTION(".bss.share.int");
uint32_t interrupt_switch_task_to SECTION(".bss.share.int");
uint32_t interrupt_switch_flag SECTION(".bss.share.int");

const unsigned int VECTOR_BASE = 0x00;
extern void rt_os_arch_vector_set_base(unsigned int addr);
extern int system_vectors;

static void os_arch_vector_init(void)
{
#ifndef RT_USING_VMM
    unsigned int *dest = (unsigned int *)VECTOR_BASE;
    unsigned int *src =  (unsigned int *)&system_vectors;

    os_memcpy(dest, src, 16 * 4);
    rt_os_arch_vector_set_base(VECTOR_BASE);
#endif
}

/**
 * This function will initialize hardware interrupt
 */
void os_arch_interrupt_init(void)
{
    uint32_t gic_os_arch_base;
    uint32_t gic_dist_base;

    /* initialize vector table */
    os_arch_vector_init();

    /* initialize exceptions table */
    os_memset(isr_table, 0x00, sizeof(isr_table));

    /* initialize ARM GIC */
#ifdef RT_USING_VMM
    gic_dist_base = vmm_find_iomap("GIC_DIST");
    gic_os_arch_base = vmm_find_iomap("GIC_CPU");
#else
    gic_dist_base = REALVIEW_GIC_DIST_BASE;
    gic_os_arch_base = REALVIEW_GIC_CPU_BASE;
#endif
    arm_gic_dist_init(0, gic_dist_base, 0);
    arm_gic_os_arch_init(0, gic_os_arch_base);
    /*arm_gic_dump_type(0);*/

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
    arm_gic_mask(0, vector);
}

/**
 * This function will un-mask a interrupt.
 * @param vector the interrupt number
 */
void os_arch_interrupt_umask(int vector)
{
    arm_gic_umask(0, vector);
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

/**
 * Trigger a software IRQ
 *
 * Since we are running in single core, the target CPU are always CPU0.
 */
void os_arch_interrupt_trigger(int vector)
{
    arm_gic_trigger(0, 1, vector);
}

void os_arch_interrupt_clear(int vector)
{
    arm_gic_clear_sgi(0, 1, vector);
}
