/*
 * File      : os_timer.h
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
 * 2016-02-16     kontais      the first version
 */
#ifndef _OS_TIMER_H_
#define _OS_TIMER_H_

/**
 * @addtogroup Clock
 */

/*@{*/

/**
 * timer macros
 */
#define OS_TIMER_ACTIVATED         0x1             /* timer is active */
#define OS_TIMER_PERIODIC          0x2             /* periodic timer */
#define OS_TIMER_SOFT_TIMER        0x4             /* soft timer,the timer's callback function will be called in timer task. */

#define OS_TIMER_SET_TIME          0x0             /* set timer control command */
#define OS_TIMER_GET_TIME          0x1             /* get timer control command */
#define OS_TIMER_SET_ONESHOT       0x2             /* change timer to one shot */
#define OS_TIMER_SET_PERIODIC      0x3             /* change timer to periodic */

/**
 * timer structure
 */
struct os_timer
{
    uint8_t          flag;                              /* flag of kernel object */
    os_list_t        list;

    void (*timeout_func)(void *parameter);              /* timeout function */
    void             *parameter;                        /* timeout function's parameter */

    os_tick_t        interval_tick;                     /* timer tick count */
    os_tick_t        startup_tick;                      /* start tick */
    os_tick_t        timeout_tick;                      /* end tick */
};
typedef struct os_timer os_timer_t;

/*
 * timer user service
 */
void os_timer_init(os_timer_t *timer,
                   void (*timeout)(void *parameter),
                   void       *parameter,
                   os_tick_t   time,
                   uint8_t  flag);
os_err_t os_timer_delete(os_timer_t *timer);
os_err_t os_timer_start(os_timer_t *timer);
os_err_t os_timer_stop(os_timer_t *timer);
os_err_t os_timer_tick_set(os_timer_t *timer, os_tick_t tick);

void os_timer_check(void);

#endif /* _OS_TIMER_H_ */
