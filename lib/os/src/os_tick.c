/*
 * File      : os_tick.c
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
 * 2011-06-26     Bernard      add os_tick_set function.
 */

#include <os.h>

static os_tick_t os_tick = 0;

extern void os_timer_check(void);

/**
 * @addtogroup Clock
 */

/*@{*/

/**
 * This function will return current tick from operating system startup
 *
 * @return current tick
 */
os_tick_t os_tick_get(void)
{
    /* return the global tick */
    return os_tick;
}

/**
 * This function will notify kernel there is one tick passed. Normally,
 * this function is invoked by clock ISR.
 */
void os_tick_increase(void)
{
    os_task_t *task;

    /* increase the global tick */
    os_tick++;

    /* check time slice */
    task = os_task_self();

    task->remaining_tick--;
    if (task->remaining_tick == 0) {
        /* change to initialized tick */
        task->remaining_tick = task->slice_tick;

        /* yield */
        os_task_yield();
    }

    /* check timer */
    os_timer_check();
}

/**
 * This function will calculate the tick from millisecond.
 *
 * @param ms the specified millisecond
 *
 * @return the calculated tick
 */
os_tick_t os_tick_from_millisecond(uint32_t ms)
{
    /* return the calculated tick */
    return (OS_TICKS_PER_SEC * ms + 999) / 1000;
}

/*@}*/
