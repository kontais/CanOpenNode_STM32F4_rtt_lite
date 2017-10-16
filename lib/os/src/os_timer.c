/*
 * File      : os_timer.c
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
 * 2014-07-12     Bernard      does not lock scheduler when invoking soft-timer
 *                             timeout function.
 */

#include <os.h>

/* hard timer list */
static os_list_t os_timer_list = OS_LIST_INIT(os_timer_list);

#ifdef OS_CFG_SOFT_TIMER
extern os_task_t timer_task;
#endif

void _os_timer_remove(os_timer_t *timer)
{
    os_list_remove(&timer->list);
}

/**
 * @addtogroup Clock
 */

static void _os_timer_init(os_timer_t *timer,
                           void (*timeout)(void *parameter),
                           void      *parameter,
                           os_tick_t  time,
                           uint8_t flag)
{
    /* set flag */
    timer->flag  = flag;

    /* set deactivated */
    timer->flag &= ~OS_TIMER_ACTIVATED;

    timer->timeout_func = timeout;
    timer->parameter    = parameter;

    timer->startup_tick = 0;
    timer->timeout_tick = 0;
    timer->interval_tick    = time;

    /* initialize timer list */
    os_list_init(&(timer->list));
}

/*@{*/

/**
 * This function will initialize a timer, normally this function is used to
 * initialize a static timer object.
 *
 * @param timer the static timer object
 * @param timeout the timeout function
 * @param parameter the parameter of timeout function
 * @param time the tick of timer
 * @param flag the flag of timer
 */
void os_timer_init(os_timer_t *timer,
                   void (*timeout)(void *parameter),
                   void       *parameter,
                   os_tick_t   time,
                   uint8_t  flag)
{
    /* timer check */
    OS_ASSERT(timer != NULL);

    _os_timer_init(timer, timeout, parameter, time, flag);
}

/**
 * This function will start the timer
 *
 * @param timer the timer to be started
 *
 * @return the operation status, OS_OK on OK, OS_ERROR on error
 */
os_err_t os_timer_start(os_timer_t *timer)
{
    os_list_t *timer_list;
    os_sr_t sr;
    struct os_list_node *n;
    os_tick_t current_tick;

    /* timer check */
    OS_ASSERT(timer != NULL);

    /* stop timer firstly */
    sr = os_enter_critical();
    /* remove timer from list */
    _os_timer_remove(timer);
    /* change status of timer */
    timer->flag &= ~OS_TIMER_ACTIVATED;
    /* set tick */
    timer->startup_tick = os_tick_get();
    timer->timeout_tick = timer->startup_tick + timer->interval_tick;

    current_tick = os_tick_get();

#ifdef OS_CFG_SOFT_TIMER
    if (timer->flag & OS_TIMER_SOFT_TIMER) {
        /* insert timer to soft timer list */
        timer_list = &os_soft_timer_list;
    }
    else
#endif
    {
        /* insert timer to system timer list */
        timer_list = &os_timer_list;
    }

    for (n = timer_list; n != timer_list->prev; n  = n->next) {
        os_timer_t *timer_entry;

        /* fix up the entry pointer */
        timer_entry = OS_LIST_ENTRY(n->next, os_timer_t, list);

        /* If we have two timers that timeout at the same time, it's
         * preferred that the timer inserted early get called early.
         * So insert the new timer to the end the the some-timeout timer
         * list.
         */
        if ((timer_entry->timeout_tick - timer->timeout_tick) == 0) {
            continue;
        } else if ((timer_entry->timeout_tick - current_tick) > (timer->timeout_tick - current_tick)) {
            break;
        }
    }

    os_list_insert_after(n, &(timer->list));

    timer->flag |= OS_TIMER_ACTIVATED;

    os_exit_critical(sr);

#ifdef OS_CFG_SOFT_TIMER
    if (timer->flag & OS_TIMER_SOFT_TIMER) {
        /* check whether timer task is ready */
        if (timer_task.stat != OS_TASK_READY) {
            /* resume timer task to check soft timer */
            os_task_resume(&timer_task);
            os_sched();
        }
    }
#endif

    return OS_OK;
}

/**
 * This function will stop the timer
 *
 * @param timer the timer to be stopped
 *
 * @return the operation status, OS_OK on OK, OS_ERROR on error
 */
os_err_t os_timer_stop(os_timer_t *timer)
{
    os_sr_t sr;

    /* timer check */
    OS_ASSERT(timer != NULL);
    if (!(timer->flag & OS_TIMER_ACTIVATED))
        return OS_ERROR;

    sr = os_enter_critical();

    _os_timer_remove(timer);

    os_exit_critical(sr);

    /* change stat */
    timer->flag &= ~OS_TIMER_ACTIVATED;

    return OS_OK;
}

os_err_t os_timer_tick_set(os_timer_t *timer, os_tick_t tick)
{
    /* timer check */
    OS_ASSERT(timer != NULL);

    timer->interval_tick = tick;

    return OS_OK;
}

os_err_t os_timer_tick_get(os_timer_t *timer, os_tick_t *tick)
{
    /* timer check */
    OS_ASSERT(timer != NULL);

    *tick = timer->interval_tick;

    return OS_OK;
}

/**
 * This function will detach a timer from timer management.
 *
 * @param timer the static timer object
 *
 * @return the operation status, OS_OK on OK; OS_ERROR on error
 */
os_err_t os_timer_delete(os_timer_t *timer)
{
    os_sr_t sr;

    /* timer check */
    OS_ASSERT(timer != NULL);

    sr = os_enter_critical();

    _os_timer_remove(timer);

    os_exit_critical(sr);

    return OS_OK;
}

/**
 * This function will check timer list, if a timeout event happens, the
 * corresponding timeout function will be invoked.
 *
 * @note this function shall be invoked in operating system timer interrupt.
 */
void os_timer_check(void)
{
    os_tick_t current_tick;
    struct os_list_node *n;
    os_timer_t *timer;
    os_sr_t sr;

    OS_DEBUG_LOG(OS_DEBUG_TIMER, ("timer check enter\n"));

    sr = os_enter_critical();

    current_tick = os_tick_get();

    for (n = os_timer_list.next; n != &os_timer_list;) {
        timer = OS_LIST_ENTRY(n, os_timer_t, list);

        /* the timer not timeout */
        if ((current_tick -timer->startup_tick) < timer->interval_tick)
            break;

        /* move node to the next */
        n = n->next;

        /* remove timer from timer list firstly */
        _os_timer_remove(timer);

        /* call timeout function */
        timer->timeout_func(timer->parameter);

        /* re-get tick */
        current_tick = os_tick_get();
        OS_DEBUG_LOG(OS_DEBUG_TIMER, ("current tick: %d\n", current_tick));

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

    os_exit_critical(sr);

    OS_DEBUG_LOG(OS_DEBUG_TIMER, ("timer check leave\n"));
}
