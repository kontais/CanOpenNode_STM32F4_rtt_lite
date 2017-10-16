/*
 * File      : interrupt.c
 * This file is part of RT-Thread RTOS
 * COPYRIGHT (C) 2006 - 2015, RT-Thread Development Team
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
 *
 * Change Logs:
 * Date           Author       Notes
 * 2015/9/15      Bernard      Update to new interrupt framework.
 */

#include <os.h>
#include <rtcpu.h>

#include <bsp.h>

extern uint32_t os_isr_nest;
extern void os_arch_idt_init(void);

uint32_t interrupt_switch_task_from, interrupt_switch_task_to;
uint32_t interrupt_switch_flag;

/* exception and interrupt handler table */
struct rt_irq_desc irq_desc[MAX_HANDLERS];

uint16_t irq_mask_8259A = 0xFFFF;

void os_arch_interrupt_handle(int vector, void* param);

/**
 * @addtogroup I386
 */
/*@{*/

/**
 * This function initializes 8259 interrupt controller
 */
void os_arch_pic_init()
{
    outb(IO_PIC1, 0x11);
    outb(IO_PIC1+1, IRQ_OFFSET);
    outb(IO_PIC1+1, 1<<IRQ_SLAVE);
    outb(IO_PIC1+1, 0x3);
    outb(IO_PIC1+1, 0xff);
    outb(IO_PIC1, 0x68);
    outb(IO_PIC1, 0x0a);
    outb(IO_PIC2, 0x11);
    outb(IO_PIC2+1, IRQ_OFFSET + 8);
    outb(IO_PIC2+1, IRQ_SLAVE);
    outb(IO_PIC2+1, 0x3);
    outb(IO_PIC2+1, 0xff);
    outb(IO_PIC2, 0x68);
    outb(IO_PIC2, 0x0a);

    if (irq_mask_8259A != 0xFFFF) {
        outb(IO_PIC1+1, (char)irq_mask_8259A);
        outb(IO_PIC2+1, (char)(irq_mask_8259A >> 8));
    }

    /* init interrupt nest, and context */
    os_isr_nest = 0;
    interrupt_switch_task_from = 0;
    interrupt_switch_task_to = 0;
    interrupt_switch_flag = 0;
}

void os_arch_interrupt_handle(int vector, void* param)
{
    printk("Unhandled interrupt %d occured!!!\n", vector);
}

void os_arch_isr(int vector)
{
    if (vector < MAX_HANDLERS) {
        irq_desc[vector].handler(vector, irq_desc[vector].param);
    }
}

/**
 * This function initializes interrupt descript table and 8259 interrupt controller
 *
 */
void os_arch_interrupt_init(void)
{
    int idx;

    os_arch_idt_init();
    os_arch_pic_init();

    /* init exceptions table */
    for (idx = 0; idx < MAX_HANDLERS; idx++) {
        irq_desc[idx].handler = (rt_isr_handler_t)os_arch_interrupt_handle;
        irq_desc[idx].param = NULL;
#ifdef RT_USING_INTERRUPT_INFO
        os_snprintf(irq_desc[idx].name, OS_NAME_MAX - 1, "default");
        irq_desc[idx].counter = 0;
#endif
    }
}

void os_arch_interrupt_umask(int vector)
{
    irq_mask_8259A = irq_mask_8259A&~(1<<vector);
    outb(IO_PIC1+1, (char)irq_mask_8259A);
    outb(IO_PIC2+1, (char)(irq_mask_8259A >> 8));
}

void os_arch_interrupt_mask(int vector)
{
    irq_mask_8259A = irq_mask_8259A | (1<<vector);
    outb(IO_PIC1+1, (char)irq_mask_8259A);
    outb(IO_PIC2+1, (char)(irq_mask_8259A >> 8));
}

rt_isr_handler_t os_arch_interrupt_install(int              vector,
                                         rt_isr_handler_t handler,
                                         void            *param,
                                         char            *name)
{
    rt_isr_handler_t old_handler = NULL;

    if (vector < MAX_HANDLERS) {
        old_handler = irq_desc[vector].handler;
        if (handler != NULL) {
            irq_desc[vector].handler = (rt_isr_handler_t)handler;
            irq_desc[vector].param = param;
#ifdef RT_USING_INTERRUPT_INFO
            os_snprintf(irq_desc[vector].name, OS_NAME_MAX - 1, "%s", name);
            irq_desc[vector].counter = 0;
#endif
        }
    }

    return old_handler;
}

os_sr_t os_enter_critical(void)
{
    os_sr_t sr;

    __asm__ __volatile__("pushfl ; popl %0 ; cli":"=g" (level): :"memory");
    return level;
}

void os_exit_critical(os_sr_t sr)
{
    __asm__ __volatile__("pushl %0 ; popfl": :"g" (level):"memory", "cc");
}

/*@}*/
