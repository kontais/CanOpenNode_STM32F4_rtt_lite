/*
 * File      : os_mbox.c
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
 * This function will initialize a mailbox and put it under control of resource
 * management.
 *
 * @param mb the mailbox object
 * @param name the name of mailbox
 * @param msgpool the begin address of buffer to save received mail
 * @param size the size of mailbox
 * @param flag the flag of mailbox
 *
 * @return the operation status, OS_OK on successful
 */
os_err_t os_mbox_init(os_mbox_t *mb,
                    const char  *name,
                    void        *msgpool,
                    size_t    size,
                    uint8_t   flag)
{
    OS_ASSERT(mb != NULL);

    /* set parent flag */
    mb->flag = flag;

    /* init ipc object */
    os_list_init(&(mb->pending_list));

    /* init mailbox */
    mb->msg_pool   = msgpool;
    mb->size       = size;
    mb->entry      = 0;
    mb->in_offset  = 0;
    mb->out_offset = 0;

    /* init an additional list of sender suspend task */
    os_list_init(&(mb->sender_pending_list));

    return OS_OK;
}

/**
 * This function will detach a mailbox from resource management
 *
 * @param mb the mailbox object
 *
 * @return the operation status, OS_OK on successful
 */
os_err_t os_mbox_delete(os_mbox_t *mb)
{
    /* parameter check */
    OS_ASSERT(mb != NULL);

    /* resume all suspended task */
    os_list_resume_all(&(mb->pending_list));
    /* also resume all mailbox private suspended task */
    os_list_resume_all(&(mb->sender_pending_list));

    return OS_OK;
}

/**
 * This function will put a mail to mailbox object. If the mailbox is full,
 * current task will be suspended until timeout.
 *
 * @param mb the mailbox object
 * @param value the mail
 * @param timeout the waiting time
 *
 * @return the error code
 */
os_err_t os_mbox_put_wait(os_mbox_t *mb,
                         uint32_t  value,
                         os_tick_t   timeout)
{
    os_task_t *task;
    os_sr_t sr;
    os_tick_t tick_delta = 0;

    /* parameter check */
    OS_ASSERT(mb != NULL);

    /* get current task */
    task = os_task_self();

    sr = os_enter_critical();

    /* for non-blocking call */
    if (mb->entry == mb->size && timeout == OS_NO_WAIT) {
        os_exit_critical(sr);

        return OS_EFULL;
    }

    /* mailbox is full */
    while (mb->entry == mb->size) {
        /* reset error number in task */
        task->error = OS_OK;

        /* no waiting, return timeout */
        if (timeout == OS_NO_WAIT) {
            os_exit_critical(sr);

            return OS_EFULL;
        }

        OS_DEBUG_IN_TASK_CONTEXT;
        /* suspend current task */
        os_list_suspend(&(mb->sender_pending_list),
                            task,
                            mb->flag);

        /* no wait forever, start task timer */
        if (timeout != OS_WAIT_FOREVER) {
            /* get the start tick of timer */
            tick_delta = os_tick_get();

            OS_DEBUG_LOG(OS_DEBUG_IPC, ("mb_put_wait: start timer of task:%s\n",
                                        task->name));

            /* reset the timeout of task timer and start it */
            os_timer_tick_set(&(task->timer), timeout);
            os_timer_start(&(task->timer));
        }

        os_exit_critical(sr);

        /* re-schedule */
        os_sched();

        /* resume from suspend state */
        if (task->error != OS_OK) {
            /* return error */
            return task->error;
        }

        sr = os_enter_critical();

        /* if it's not waiting forever and then re-calculate timeout tick */
        if (timeout != OS_WAIT_FOREVER) {
            tick_delta = os_tick_get() - tick_delta;
            if (tick_delta >= timeout) {
                timeout = OS_NO_WAIT;
            } else {
                timeout -= tick_delta;
            }
        }
    }

    /* set ptr */
    mb->msg_pool[mb->in_offset] = value;
    /* increase input offset */
    mb->in_offset++;
    if (mb->in_offset >= mb->size)
        mb->in_offset = 0;
    /* increase message entry */
    mb->entry++;

    /* resume suspended task */
    if (!os_list_isempty(&mb->pending_list)) {
        os_list_resume(&(mb->pending_list));

        os_exit_critical(sr);

        os_sched();

        return OS_OK;
    }

    os_exit_critical(sr);

    return OS_OK;
}

/**
 * This function will put a mail to mailbox object, if there are tasks
 * suspended on mailbox object, it will be waked up. This function will return
 * immediately, if you want blocking put, use os_mbox_put_wait instead.
 *
 * @param mb the mailbox object
 * @param value the mail
 *
 * @return the error code
 */
os_err_t os_mbox_put(os_mbox_t *mb, uint32_t value)
{
    return os_mbox_put_wait(mb, value, OS_NO_WAIT);
}

/**
 * This function will receive a mail from mailbox object, if there is no mail
 * in mailbox object, the task shall wait for a specified time.
 *
 * @param mb the mailbox object
 * @param value the received mail will be saved in
 * @param timeout the waiting time
 *
 * @return the error code
 */
os_err_t os_mbox_get(os_mbox_t *mb, uint32_t *value, os_tick_t timeout)
{
    os_task_t *task;
    os_sr_t sr;
    os_tick_t tick_delta = 0;

    /* parameter check */
    OS_ASSERT(mb != NULL);

    /* get current task */
    task = os_task_self();

    sr = os_enter_critical();

    /* for non-blocking call */
    if (mb->entry == 0 && timeout == OS_NO_WAIT) {
        os_exit_critical(sr);

        return OS_TIMEOUT;
    }

    /* mailbox is empty */
    while (mb->entry == 0) {
        /* reset error number in task */
        task->error = OS_OK;

        /* no waiting, return timeout */
        if (timeout == OS_NO_WAIT) {
            os_exit_critical(sr);

            task->error = OS_TIMEOUT;

            return OS_TIMEOUT;
        }

        OS_DEBUG_IN_TASK_CONTEXT;
        /* suspend current task */
        os_list_suspend(&(mb->pending_list),
                            task,
                            mb->flag);

        /* no wait forever, start task timer */
        if (timeout != OS_WAIT_FOREVER) {
            /* get the start tick of timer */
            tick_delta = os_tick_get();

            OS_DEBUG_LOG(OS_DEBUG_IPC, ("mb_get: start timer of task:%s\n",
                                        task->name));

            /* reset the timeout of task timer and start it */
            os_timer_tick_set(&(task->timer), timeout);
            os_timer_start(&(task->timer));
        }

        os_exit_critical(sr);

        /* re-schedule */
        os_sched();

        /* resume from suspend state */
        if (task->error != OS_OK) {
            /* return error */
            return task->error;
        }

        sr = os_enter_critical();

        /* if it's not waiting forever and then re-calculate timeout tick */
        if (timeout != OS_WAIT_FOREVER) {
            tick_delta = os_tick_get() - tick_delta;
            if (tick_delta >= timeout) {
                timeout = OS_NO_WAIT;
            } else {
                timeout -= tick_delta;
            }
        }
    }

    /* fill ptr */
    *value = mb->msg_pool[mb->out_offset];

    /* increase output offset */
   	mb->out_offset++;
    if (mb->out_offset >= mb->size) {
        mb->out_offset = 0;
    }
    /* decrease message entry */
    mb->entry--;

    /* resume suspended task */
    if (!os_list_isempty(&(mb->sender_pending_list))) {
        os_list_resume(&(mb->sender_pending_list));

        os_exit_critical(sr);

        os_sched();

        return OS_OK;
    }

    os_exit_critical(sr);

    return OS_OK;
}

/**
 * This function can get or set some extra attributions of a mailbox object.
 *
 * @param mb the mailbox object
 * @param arg the execution argument
 *
 * @return the error code
 */
os_err_t os_mbox_reset(os_mbox_t *mb, void *arg)
{
    os_sr_t sr;
    OS_ASSERT(mb != NULL);

    sr = os_enter_critical();

    /* resume all waiting task */
    os_list_resume_all(&(mb->pending_list));
    /* also resume all mailbox private suspended task */
    os_list_resume_all(&(mb->sender_pending_list));

    /* re-init mailbox */
    mb->entry      = 0;
    mb->in_offset  = 0;
    mb->out_offset = 0;

    os_exit_critical(sr);

    os_sched();

    return OS_OK;
}
