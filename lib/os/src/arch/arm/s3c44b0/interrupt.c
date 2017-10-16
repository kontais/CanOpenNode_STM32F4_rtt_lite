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
 * 2006-09-06     XuXinming    first version
 * 2006-09-15     Bernard      add interrupt bank 0..3 for more effective
 *                             in irq trap
 */

#include <os.h>
#include "s3c44b0.h"

#define MAX_HANDLERS    26

extern uint32_t os_isr_nest;

/* exception and interrupt handler table */
rt_isr_handler_t isr_table[MAX_HANDLERS];
uint32_t interrupt_switch_task_from, interrupt_switch_task_to;
uint32_t interrupt_switch_flag;

unsigned char interrupt_bank0[256];
unsigned char interrupt_bank1[256];
unsigned char interrupt_bank2[256];
unsigned char interrupt_bank3[256];

/**
 * @addtogroup S3C44B0
 */
/*@{*/

void os_arch_interrupt_handle(int vector)
{
    printk("Unhandled interrupt %d occured!!!\n", vector);
}

/**
 * This function will initialize hardware interrupt
 */
void os_arch_interrupt_init()
{
    register int i;

    /* all interrupt disabled include global bit */
    INTMSK = 0x07ffffff;

    /* clear pending register */
       I_ISPC = 0x03ffffff;

    /* non-vector mode IRQ enable */
    INTCON = 0x5;

    /* all IRQ mode */
    INTMOD = 0x0;

    /* init exceptions table */
    for (i = 0; i < MAX_HANDLERS; i++) {
        isr_table[i] = os_arch_interrupt_handle;
    }

    for (i = 0; i < 256; i++) {
        interrupt_bank0[i] = 0;
        interrupt_bank1[i] = 0;
        interrupt_bank2[i] = 0;
        interrupt_bank3[i] = 0;
    }

    /* setup interrupt bank table */
    interrupt_bank0[1]    = 0;
    interrupt_bank0[2]    = 1;
    interrupt_bank0[4]    = 2;
    interrupt_bank0[8]    = 3;
    interrupt_bank0[16]    = 4;
    interrupt_bank0[32]    = 5;
    interrupt_bank0[64]    = 6;
    interrupt_bank0[128]= 7;

    interrupt_bank1[1]    = 8;
    interrupt_bank1[2]    = 9;
    interrupt_bank1[4]    = 10;
    interrupt_bank1[8]    = 11;
    interrupt_bank1[16]    = 12;
    interrupt_bank1[32]    = 13;
    interrupt_bank1[64]    = 14;
    interrupt_bank1[128]= 15;

    interrupt_bank2[1]    = 16;
    interrupt_bank2[2]    = 17;
    interrupt_bank2[4]    = 18;
    interrupt_bank2[8]    = 19;
    interrupt_bank2[16]    = 20;
    interrupt_bank2[32]    = 21;
    interrupt_bank2[64]    = 22;
    interrupt_bank2[128]= 23;

    interrupt_bank3[1]    = 24;
    interrupt_bank3[2]    = 25;

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
    INTMSK &= ~(1 << vector);
}

/**
 * This function will install a interrupt service routine to a interrupt.
 * @param vector the interrupt number
 * @param new_handler the interrupt service routine to be installed
 * @param old_handler the old interrupt service routine
 */
void os_arch_interrupt_install(int vector, rt_isr_handler_t new_handler, rt_isr_handler_t *old_handler)
{
    if (vector < MAX_HANDLERS) {
        if (old_handler != NULL) *old_handler = isr_table[vector];
        if (new_handler != NULL) isr_table[vector] = new_handler;
    }
}

/*@}*/
