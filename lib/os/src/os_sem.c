/*
 * File      : os_sem.c
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
 * This function will initialize a semaphore and put it under control of
 * resource management.
 *
 * @param sem the semaphore object
 * @param name the name of semaphore
 * @param value the init value of semaphore
 * @param flag the flag of semaphore
 *
 * @return the operation status, OS_OK on successful
 */
os_err_t os_sem_init(os_sem_t *sem,
                     const char *name,
                     uint32_t value,
                     uint8_t  flag)
{
    OS_ASSERT(sem != NULL);

    /* init ipc object */
    os_list_init(&(sem->pending_list));

    /* set init value */
    sem->value = value;

    /* set parent */
    sem->flag = flag;

    return OS_OK;
}

/**
 * This function will detach a semaphore from resource management
 *
 * @param sem the semaphore object
 *
 * @return the operation status, OS_OK on successful
 *
 */
os_err_t os_sem_delete(os_sem_t *sem)
{
    OS_ASSERT(sem != NULL);

    /* wakeup all suspend tasks */
    os_list_resume_all(&(sem->pending_list));

    return OS_OK;
}

/**
 * This function will take a semaphore, if the semaphore is unavailable, the
 * task shall wait for a specified time.
 *
 * @param sem the semaphore object
 * @param time the waiting time
 *
 * @return the error code
 */
os_err_t os_sem_take(os_sem_t *sem, os_tick_t timeout)
{
    os_sr_t sr;
    os_task_t *task;

    OS_ASSERT(sem != NULL);

    /* get current task */
    task = os_task_self();

    sr = os_enter_critical();

    OS_DEBUG_LOG(OS_DEBUG_IPC, ("task %s take sem which value is: %d\n",
                                task->name,
                                sem->value));

    if (sem->value > 0) {
        /* semaphore is available */
        sem->value--;

        os_exit_critical(sr);

        return OS_OK;
    }

    /* no waiting, return with timeout */
    if (timeout == OS_NO_WAIT) {
        os_exit_critical(sr);

        return OS_TIMEOUT;
    }

    /* current context checking */
    OS_DEBUG_IN_TASK_CONTEXT;

    /* reset task error */
    task->error = OS_OK;

    /* semaphore is unavailable, push to suspend list */
    os_list_suspend(&(sem->pending_list),
                        task,
                        sem->flag);

    /* no wait forever, start task timer */
    if (timeout != OS_WAIT_FOREVER) {
        OS_DEBUG_LOG(OS_DEBUG_IPC, ("set task:%s to timer list\n",
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
 * This function will try to take a semaphore and immediately return
 *
 * @param sem the semaphore object
 *
 * @return the error code
 */
os_err_t os_sem_trytake(os_sem_t *sem)
{
    return os_sem_take(sem, 0);
}

/**
 * This function will release a semaphore, if there are tasks suspended on
 * semaphore, it will be waked up.
 *
 * @param sem the semaphore object
 *
 * @return the error code
 */
os_err_t os_sem_give(os_sem_t *sem)
{
    os_sr_t sr;
    bool_t need_schedule;

    need_schedule = FALSE;

    sr = os_enter_critical();

    OS_DEBUG_LOG(OS_DEBUG_IPC, ("task %s releases sem which value is: %d\n",
                                os_task_self()->name,
                                sem->value));

    if (!os_list_isempty(&sem->pending_list)) {
        /* resume the suspended task */
        os_list_resume(&(sem->pending_list));
        need_schedule = TRUE;
    } else {
        sem->value++; /* increase value */
    }

    os_exit_critical(sr);

    /* resume a task, re-schedule */
    if (need_schedule == TRUE) {
        os_sched();
    }

    return OS_OK;
}

/**
 * This function can get or set some extra attributions of a semaphore object.
 *
 * @param sem the semaphore object
 * @param value the execution argument
 *
 * @return the error code
 */
os_err_t os_sem_reset(os_sem_t *sem, uint32_t value)
{
    os_sr_t sr;
    OS_ASSERT(sem != NULL);

    sr = os_enter_critical();

    /* resume all waiting task */
    os_list_resume_all(&sem->pending_list);

    /* set new value */
    sem->value = value;

    os_exit_critical(sr);

    os_sched();

    return OS_OK;
}

