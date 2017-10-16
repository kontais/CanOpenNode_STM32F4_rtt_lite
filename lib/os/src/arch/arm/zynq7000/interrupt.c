/*
 * COPYRIGHT (C) 2013-2014, Shanghai Real-Thread Technology Co., Ltd
 *
 *  All rights reserved.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#include <rtcpu.h>
#include <os.h>
#include "zynq7000.h"
#include "cp15.h"
#include "gic.h"

#define MAX_HANDLERS                IRQ_Zynq7000_MAXNR

extern volatile uint8_t os_isr_nest;

/* exception and interrupt handler table */
struct rt_irq_desc isr_table[MAX_HANDLERS];
uint32_t interrupt_switch_task_from, interrupt_switch_task_to;
uint32_t interrupt_switch_flag;

static void os_arch_interrupt_handle(int vector, void *param)
{
    printk("Unhandled interrupt %d occured!!!\n", vector);
}

const unsigned int VECTOR_BASE = 0x00;
extern int system_vectors;

static void os_arch_vector_init(void)
{
    int sctrl;
    unsigned int *src = (unsigned int *)&system_vectors;

    /* C12-C0 is only active when SCTLR.V = 0 */
    asm volatile ("mrc p15, #0, %0, c1, c0, #0"
                  :"=r" (sctrl));
    sctrl &= ~(1 << 13);
    asm volatile ("mcr p15, #0, %0, c1, c0, #0"
                  :
                  :"r" (sctrl));

    asm volatile ("mcr p15, #0, %0, c12, c0, #0"
                  :
                  :"r" (src));
}

/**
 * This function will initialize hardware interrupt
 */
void os_arch_interrupt_init(void)
{
    uint32_t idx;

    /* set vector table */
    os_arch_vector_init();

    /* init exceptions table */
    os_memset(isr_table, 0x00, sizeof(isr_table));
    for (idx = 0; idx < MAX_HANDLERS; idx++) {
        isr_table[idx].handler = os_arch_interrupt_handle;
    }

    /* initialize ARM GIC */
    arm_gic_dist_init(0, Zynq7000_GIC_DIST_BASE, 0);
    arm_gic_os_arch_init(0, Zynq7000_GIC_CPU_BASE);

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
        /* set the interrupt to this cpu */
        arm_gic_set_cpu(0, vector, 1 << rt_os_arch_get_smp_id());
    }

    return old_handler;
}

void os_arch_interrupt_clear(int vector)
{
    /* SGI will be cleared automatically. */
    if (vector < 16)
        return;
}
