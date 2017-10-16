/*
 * File      : os_mutex.c
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
 * This function will initialize a mutex and put it under control of resource
 * management.
 *
 * @param mutex the mutex object
 * @param name the name of mutex
 * @param flag the flag of mutex
 *
 * @return the operation status, OS_OK on successful
 */
os_err_t os_mutex_init(os_mutex_t *mutex, uint8_t flag)
{
    OS_ASSERT(mutex != NULL);

    /* init ipc object */
    os_list_init(&(mutex->pending_list));

    mutex->value = 1;
    mutex->owner = NULL;
    mutex->original_priority = 0xFF;
    mutex->hold  = 0;

    /* set flag */
    mutex->flag = flag;

    return OS_OK;
}

/**
 * This function will detach a mutex from resource management
 *
 * @param mutex the mutex object
 *
 * @return the operation status, OS_OK on successful
 *
 */
os_err_t os_mutex_delete(os_mutex_t *mutex)
{
    OS_ASSERT(mutex != NULL);

    /* wakeup all suspend tasks */
    os_list_resume_all(&(mutex->pending_list));

    return OS_OK;
}

/**
 * This function will take a mutex, if the mutex is unavailable, the
 * task shall wait for a specified time.
 *
 * @param mutex the mutex object
 * @param timeout the waiting time
 *
 * @return the error code
 */
os_err_t os_mutex_take(os_mutex_t *mutex, os_tick_t timeout)
{
    os_sr_t sr;
    os_task_t *task;

    /* this function must not be used in interrupt even if time = 0 */
    OS_DEBUG_IN_TASK_CONTEXT;

    OS_ASSERT(mutex != NULL);

    /* get current task */
    task = os_task_self();

    sr = os_enter_critical();

    OS_DEBUG_LOG(OS_DEBUG_IPC,
                 ("mutex_take: current task %s, mutex value: %d, hold: %d\n",
                  task->name, mutex->value, mutex->hold));

    /* reset task error */
    task->error = OS_OK;

    /* it's the same task */
    if (mutex->owner == task) {
        mutex->hold++;

        os_exit_critical(sr);

        return OS_OK;
    }

    /* The value of mutex is 1 in initial status. Therefore, if the
     * value is great than 0, it indicates the mutex is avaible.
     */
    if (mutex->value > 0) {
        /* mutex is available */
        mutex->value--;

        /* set mutex owner and original priority */
        mutex->owner             = task;
        mutex->original_priority = task->current_priority;
        mutex->hold++;

        os_exit_critical(sr);

        /* get the mutex successfully */
        return OS_OK;
    }

    /* no waiting, return with timeout */
    if (timeout == OS_NO_WAIT) {
        /* set error as timeout */
        task->error = OS_TIMEOUT;

        os_exit_critical(sr);

        return OS_TIMEOUT;
    }

    /* mutex is unavailable, push to suspend list */
    OS_DEBUG_LOG(OS_DEBUG_IPC, ("mutex_take: suspend task: %s\n",
                                task->name));

    /* change the owner task priority of mutex */
    if (task->current_priority < mutex->owner->current_priority) {
        /* change the owner task priority */
        os_task_priority_set(mutex->owner, task->current_priority);
    }

    /* suspend current task */
    os_list_suspend(&(mutex->pending_list),
                        task,
                        mutex->flag);

    /* no wait forever, start task timer */
    if (timeout != OS_WAIT_FOREVER) {
        OS_DEBUG_LOG(OS_DEBUG_IPC,
                     ("mutex_take: start the timer of task:%s\n",
                      task->name));

        /* reset the timeout of task timer and start it */
        os_timer_tick_set(&(task->timer), timeout);
        os_timer_start(&(task->timer));
    }

    os_exit_critical(sr);

    /* do schedule */
    os_sched();

    return task->error;
}

/**
 * This function will release a mutex, if there are tasks suspended on mutex,
 * it will be waked up.
 *
 * @param mutex the mutex object
 *
 * @return the error code
 */
os_err_t os_mutex_release(os_mutex_t *mutex)
{
    os_sr_t sr;
    os_task_t *task;
    bool_t need_schedule;

    need_schedule = FALSE;

    /* only task could release mutex because we need test the ownership */
    OS_DEBUG_IN_TASK_CONTEXT;

    /* get current task */
    task = os_task_self();

    OS_DEBUG_LOG(OS_DEBUG_IPC,
                 ("mutex_release:current task %s, mutex value: %d, hold: %d\n",
                  task->name, mutex->value, mutex->hold));

    sr = os_enter_critical();

    /* mutex only can be released by owner */
    if (mutex->owner != task) {
        task->error = OS_ERROR;

        os_exit_critical(sr);

        return OS_ERROR;
    }

    /* decrease hold */
    mutex->hold--;
    /* if no hold */
    if (mutex->hold == 0) {
        /* change the owner task to original priority */
        if (mutex->original_priority != mutex->owner->current_priority) {
            os_task_priority_set(mutex->owner, mutex->original_priority);
        }

        /* wakeup suspended task */
        if (!os_list_isempty(&mutex->pending_list)) {
            /* get suspended task */
            task = OS_LIST_ENTRY(mutex->pending_list.next,
                                   os_task_t,
                                   tlist);

            OS_DEBUG_LOG(OS_DEBUG_IPC, ("mutex_release: resume task: %s\n",
                                        task->name));

            /* set new owner and priority */
            mutex->owner             = task;
            mutex->original_priority = task->current_priority;
            mutex->hold++;

            /* resume task */
            os_list_resume(&(mutex->pending_list));

            need_schedule = TRUE;
        } else {
            /* increase value */
            mutex->value++;

            /* clear owner */
            mutex->owner             = NULL;
            mutex->original_priority = 0xff;
        }
    }

    os_exit_critical(sr);

    /* perform a schedule */
    if (need_schedule == TRUE)
        os_sched();

    return OS_OK;
}
