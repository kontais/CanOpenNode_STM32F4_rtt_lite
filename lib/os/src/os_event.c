/*
 * File      : os_event.c
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
 * 2013-09-14     Grissiom     add an option check in os_event_get
 */

#include <os.h>

/**
 * This function will initialize an event and put it under control of resource
 * management.
 *
 * @param event the event object
 * @param name the name of event
 * @param flag the flag of event
 *
 * @return the operation status, OS_OK on successful
 */
os_err_t os_event_init(os_event_t *event, const char *name, uint8_t flag)
{
    OS_ASSERT(event != NULL);

    /* set parent flag */
    event->flag = flag;

    /* init ipc object */
    os_list_init(&(event->pending_list));

    /* init event */
    event->set = 0;

    return OS_OK;
}

/**
 * This function will detach an event object from resource management
 *
 * @param event the event object
 *
 * @return the operation status, OS_OK on successful
 */
os_err_t os_event_delete(os_event_t *event)
{
    /* parameter check */
    OS_ASSERT(event != NULL);

    /* resume all suspended task */
    os_list_resume_all(&(event->pending_list));

    return OS_OK;
}

/**
 * This function will put an event to the event object, if there are tasks
 * suspended on event object, it will be waked up.
 *
 * @param event the event object
 * @param set the event set
 *
 * @return the error code
 */
os_err_t os_event_put(os_event_t *event, uint32_t set)
{
    struct os_list_node *n;
    os_task_t *task;
    os_sr_t  sr;
    os_err_t status;
    bool_t need_schedule;

    /* parameter check */
    OS_ASSERT(event != NULL);
    if (set == 0)
        return OS_ERROR;

    need_schedule = FALSE;

    sr = os_enter_critical();

    /* set event */
    event->set |= set;

    if (!os_list_isempty(&event->pending_list)) {
        /* search task list to resume task */
        n = event->pending_list.next;
        while (n != &(event->pending_list)) {
            /* get task */
            task = OS_LIST_ENTRY(n, os_task_t, tlist);

            status = OS_ERROR;
            if (task->event_info & OS_EVENT_AND) {
                if ((task->event_set & event->set) == task->event_set) {
                    /* received an AND event */
                    status = OS_OK;
                }
            } else if (task->event_info & OS_EVENT_OR) {
                if (task->event_set & event->set) {
                    /* save recieved event set */
                    task->event_set = task->event_set & event->set;

                    /* received an OR event */
                    status = OS_OK;
                }
            }

            /* move node to the next */
            n = n->next;

            /* condition is satisfied, resume task */
            if (status == OS_OK) {
                /* clear event */
                if (task->event_info & OS_EVENT_CLEAR)
                    event->set &= ~task->event_set;

                /* resume task, and task list breaks out */
                os_task_resume(task);

                /* need do a scheduling */
                need_schedule = TRUE;
            }
        }
    }

    os_exit_critical(sr);

    /* do a schedule */
    if (need_schedule == TRUE)
        os_sched();

    return OS_OK;
}

/**
 * This function will put an event from event object, if the event is
 * unavailable, the task shall wait for a specified time.
 *
 * @param event the fast event object
 * @param set the interested event set
 * @param option the receive option, either OS_EVENT_AND or
 *        OS_EVENT_OR should be set.
 * @param timeout the waiting time
 * @param recved the received event, if you don't care, NULL can be set.
 *
 * @return the error code
 */
os_err_t os_event_get(os_event_t *event,
                       uint32_t  set,
                       uint8_t   option,
                       os_tick_t timeout,
                       uint32_t *recved)
{
    os_task_t *task;
    os_sr_t  sr;
    os_err_t status;

    OS_DEBUG_IN_TASK_CONTEXT;

    /* parameter check */
    OS_ASSERT(event != NULL);
    if (set == 0)
        return OS_ERROR;

    /* init status */
    status = OS_ERROR;
    /* get current task */
    task = os_task_self();
    /* reset task error */
    task->error = OS_OK;

    sr = os_enter_critical();

    /* check event set */
    if (option & OS_EVENT_AND) {
        if ((event->set & set) == set)
            status = OS_OK;
    } else if (option & OS_EVENT_OR) {
        if (event->set & set)
            status = OS_OK;
    } else {
        /* either OS_EVENT_AND or OS_EVENT_OR should be set */
        OS_ASSERT(0);
    }

    if (status == OS_OK) {
        /* set received event */
        if (recved)
            *recved = (event->set & set);

        /* received event */
        if (option & OS_EVENT_CLEAR)
            event->set &= ~set;
    } else if (timeout == OS_NO_WAIT) {
        /* no waiting */
        task->error = OS_TIMEOUT;
    } else {
        /* fill task event info */
        task->event_set  = set;
        task->event_info = option;

        /* put task to suspended task list */
        os_list_suspend(&(event->pending_list),
                            task,
                            event->flag);

        /* no wait forever, start task timer */
        if (timeout != OS_WAIT_FOREVER) {
            /* reset the timeout of task timer and start it */
            os_timer_tick_set(&(task->timer), timeout);
            os_timer_start(&(task->timer));
        }

        os_exit_critical(sr);

        /* do a schedule */
        os_sched();

        if (task->error != OS_OK) {
            /* return error */
            return task->error;
        }

        /* received an event */
        sr = os_enter_critical();

        /* set received event */
        if (recved)
            *recved = task->event_set;
    }

    os_exit_critical(sr);

    return task->error;
}

/**
 * This function can get or set some extra attributions of an event object.
 *
 * @param event the event object
 * @param set the execution argument
 *
 * @return the error code
 */
os_err_t os_event_reset(os_event_t *event, uint32_t set)
{
    os_sr_t sr;

    OS_ASSERT(event != NULL);

    sr = os_enter_critical();

    /* resume all waiting task */
    os_list_resume_all(&event->pending_list);

    /* init event set */
    event->set = set;

    os_exit_critical(sr);

    os_sched();

    return OS_OK;
}
