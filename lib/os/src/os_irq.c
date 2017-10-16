/*
 * File      : os_irq.c
 * This file is part of RT-Thread RTOS
 * COPYRIGHT (C) 2006 - 2012, RT-Thread Development Team
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
 * 2006-05-03     Bernard      add IRQ_DEBUG
 */

#include <os.h>

/* #define IRQ_DEBUG */

/**
 * @addtogroup Kernel
 */

/*@{*/

volatile uint8_t os_isr_nest = 0;

/**
 * This function will be invoked by BSP, when enter interrupt service routine
 *
 * @note please don't invoke this routine in application
 *
 * @see os_isr_leave
 */
void os_isr_enter(void)
{
    os_sr_t sr;

    OS_DEBUG_LOG(OS_DEBUG_IRQ, ("irq coming..., irq nest:%d\n",
                                os_isr_nest));

    sr = os_enter_critical();
    os_isr_nest++;
    os_exit_critical(sr);
}

/**
 * This function will be invoked by BSP, when leave interrupt service routine
 *
 * @note please don't invoke this routine in application
 *
 * @see os_isr_enter
 */
void os_isr_leave(void)
{
    os_sr_t sr;

    OS_DEBUG_LOG(OS_DEBUG_IRQ, ("irq leave, irq nest:%d\n",
                                os_isr_nest));

    sr = os_enter_critical();
    os_isr_nest--;
    os_exit_critical(sr);
}

/**
 * This function will return the nest of interrupt.
 *
 * User application can invoke this function to get whether current
 * context is interrupt context.
 *
 * @return the number of nested interrupts.
 */
uint8_t os_isr_nest_get(void)
{
    return os_isr_nest;
}

