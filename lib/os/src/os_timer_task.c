/*
 * File      : os_timer_task.c
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
 * 2006-03-12     Bernard      first version
 */

#include <os.h>

#ifdef OS_CFG_SOFT_TIMER

#ifndef OS_TIMER_TASK_STACK_SIZE
#define OS_TIMER_TASK_STACK_SIZE     512
#endif

#ifndef OS_TIMER_TASK_PRIO
#define OS_TIMER_TASK_PRIO           0
#endif

void _os_timer_remove(os_timer_t *timer);

/* soft timer list */
os_list_t os_soft_timer_list = OS_LIST_INIT(os_soft_timer_list);

os_task_t timer_task;
ALIGN(OS_ALIGN_SIZE) static uint8_t timer_task_stack[OS_TIMER_TASK_STACK_SIZE];

/**
 * This function will check timer list, if a timeout event happens, the
 * corresponding timeout function will be invoked.
 */
void os_soft_timer_check(void)
{
    os_tick_t current_tick;
    struct os_list_node *n;
    os_timer_t *timer;

    OS_DEBUG_LOG(OS_DEBUG_TIMER, ("software timer check enter\n"));

    /* lock scheduler */
    os_sched_lock();

    current_tick = os_tick_get();

    for (n = os_soft_timer_list.next; n != &os_soft_timer_list;) {
        timer = OS_LIST_ENTRY(n, os_timer_t, list);

        /* not check anymore */
        if ((current_tick - timer->startup_tick) < timer->interval_tick)
            break;

        /* move node to the next */
        n = n->next;

        /* remove timer from timer list firstly */
        _os_timer_remove(timer);

        /* not lock scheduler when performing timeout function */
        os_sched_unlock();

        /* call timeout function */
        timer->timeout_func(timer->parameter);

        /* re-get tick */
        current_tick = os_tick_get();
        OS_DEBUG_LOG(OS_DEBUG_TIMER, ("current tick: %d\n", current_tick));

        /* lock scheduler */
        os_sched_lock();

        if ((timer->flag & OS_TIMER_PERIODIC) &&
            (timer->flag & OS_TIMER_ACTIVATED)) {
            /* start it */
            timer->flag &= ~OS_TIMER_ACTIVATED;
            os_timer_start(timer);
        } else {
            /* stop timer */
            timer->flag &= ~OS_TIMER_ACTIVATED;
        }
    }

    /* unlock scheduler */
    os_sched_unlock();

    OS_DEBUG_LOG(OS_DEBUG_TIMER, ("software timer check leave\n"));
}

/* system timer task entry */
static void os_task_timer_entry(void *parameter)
{
    os_tick_t  current_tick;
    os_timer_t *timer;

    while (1) {
        if (os_list_isempty(&os_soft_timer_list)) {
            /* no software timer exist, suspend self. */
            os_task_suspend(os_task_self());
            os_sched();
        } else {
            timer = OS_LIST_ENTRY(os_soft_timer_list.next, os_timer_t, list);

            /* get current tick */
            current_tick = os_tick_get();

            if (timer->interval_tick > (current_tick - timer->startup_tick)) {
                /* get the delta timeout tick */
                os_task_sleep(timer->timeout_tick - current_tick);
            }
        }

        /* check software timer */
        os_soft_timer_check();
    }
}

/**
 * @ingroup SystemInit
 *
 * This function will initialize system timer task
 */
void os_init_timer_task(void)
{
    /* start software timer task */
    os_task_init(&timer_task,
                   "timer",
                   os_task_timer_entry,
                   NULL,
                   &timer_task_stack[0],
                   sizeof(timer_task_stack),
                   OS_TIMER_TASK_PRIO,
                   10);

    /* startup */
    os_task_startup(&timer_task);
}

#endif  /* OS_CFG_SOFT_TIMER */
