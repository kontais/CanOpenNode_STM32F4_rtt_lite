/*
 * File      : os_mqueue.c
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

struct os_mqueue_msg
{
    struct os_mqueue_msg *next;
};
typedef struct os_mqueue_msg os_mqueue_msg_t;

/**
 * This function will initialize a message queue and put it under control of
 * resource management.
 *
 * @param mq the message object
 * @param name the name of message queue
 * @param msgpool the beginning address of buffer to save messages
 * @param msg_size the maximum size of message
 * @param pool_size the size of buffer to save messages
 * @param flag the flag of message queue
 *
 * @return the operation status, OS_OK on successful
 */
os_err_t os_mqueue_init(os_mqueue_t *mq,
                    void       *msgpool,
                    size_t   msg_size,
                    size_t   pool_size,
                    uint8_t  flag)
{
    os_mqueue_msg_t *head;
    uint16_t temp;

    /* parameter check */
    OS_ASSERT(mq != NULL);

    /* set parent flag */
    mq->flag = flag;

    /* init ipc object */
    os_list_init(&(mq->pending_list));

    /* set messasge pool */
    mq->msg_pool = msgpool;

    /* get correct message size */
    mq->msg_size = OS_ALIGN(msg_size, OS_ALIGN_SIZE);
    mq->max_msgs = pool_size / (mq->msg_size + sizeof(os_mqueue_msg_t));

    /* init message list */
    mq->msg_queue_head = NULL;
    mq->msg_queue_tail = NULL;

    /* init message empty list */
    mq->msg_queue_free = NULL;
    for (temp = 0; temp < mq->max_msgs; temp++) {
        head = (os_mqueue_msg_t *)((uint8_t *)mq->msg_pool +
            temp * (mq->msg_size + sizeof(os_mqueue_msg_t)));
        head->next = mq->msg_queue_free;
        mq->msg_queue_free = head;
    }

    /* the initial entry is zero */
    mq->entry = 0;

    return OS_OK;
}

/**
 * This function will detach a message queue object from resource management
 *
 * @param mq the message queue object
 *
 * @return the operation status, OS_OK on successful
 */
os_err_t os_mqueue_delete(os_mqueue_t *mq)
{
    /* parameter check */
    OS_ASSERT(mq != NULL);

    /* resume all suspended task */
    os_list_resume_all(&mq->pending_list);

    return OS_OK;
}

/**
 * This function will put a message to message queue object, if there are
 * tasks suspended on message queue object, it will be waked up.
 *
 * @param mq the message queue object
 * @param buffer the message
 * @param size the size of buffer
 *
 * @return the error code
 */
os_err_t os_mqueue_put(os_mqueue_t *mq, void *buffer, size_t size)
{
    os_sr_t sr;
    os_mqueue_msg_t *msg;

    OS_ASSERT(mq != NULL);
    OS_ASSERT(buffer != NULL);
    OS_ASSERT(size != 0);

    /* greater than one message size */
    if (size > mq->msg_size)
        return OS_ERROR;

    sr = os_enter_critical();

    /* get a free list, there must be an empty item */
    msg = (os_mqueue_msg_t*)mq->msg_queue_free;
    /* message queue is full */
    if (msg == NULL) {
        os_exit_critical(sr);

        return OS_EFULL;
    }
    /* move free list pointer */
    mq->msg_queue_free = msg->next;

    os_exit_critical(sr);

    /* the msg is the new tailer of list, the next shall be NULL */
    msg->next = NULL;
    /* copy buffer */
    memcpy(msg + 1, buffer, size);

    sr = os_enter_critical();
    /* link msg to message queue */
    if (mq->msg_queue_tail != NULL) {
        /* if the tail exists, */
        ((os_mqueue_msg_t *)mq->msg_queue_tail)->next = msg;
    }

    /* set new tail */
    mq->msg_queue_tail = msg;
    /* if the head is empty, set head */
    if (mq->msg_queue_head == NULL)
        mq->msg_queue_head = msg;

    /* increase message entry */
    mq->entry++;

    /* resume suspended task */
    if (!os_list_isempty(&mq->pending_list)) {
        os_list_resume(&(mq->pending_list));

        os_exit_critical(sr);

        os_sched();

        return OS_OK;
    }

    os_exit_critical(sr);

    return OS_OK;
}

/**
 * This function will put an urgent message to message queue object, which
 * means the message will be inserted to the head of message queue. If there
 * are tasks suspended on message queue object, it will be waked up.
 *
 * @param mq the message queue object
 * @param buffer the message
 * @param size the size of buffer
 *
 * @return the error code
 */
os_err_t os_mqueue_put_urgent(os_mqueue_t *mq, void *buffer, size_t size)
{
    os_sr_t sr;
    os_mqueue_msg_t *msg;

    OS_ASSERT(mq != NULL);
    OS_ASSERT(buffer != NULL);
    OS_ASSERT(size != 0);

    /* greater than one message size */
    if (size > mq->msg_size)
        return OS_ERROR;

    sr = os_enter_critical();

    /* get a free list, there must be an empty item */
    msg = (os_mqueue_msg_t *)mq->msg_queue_free;
    /* message queue is full */
    if (msg == NULL) {
        os_exit_critical(sr);

        return OS_EFULL;
    }
    /* move free list pointer */
    mq->msg_queue_free = msg->next;

    os_exit_critical(sr);

    /* copy buffer */
    memcpy(msg + 1, buffer, size);

    sr = os_enter_critical();

    /* link msg to the beginning of message queue */
    msg->next = mq->msg_queue_head;
    mq->msg_queue_head = msg;

    /* if there is no tail */
    if (mq->msg_queue_tail == NULL)
        mq->msg_queue_tail = msg;

    /* increase message entry */
    mq->entry++;

    /* resume suspended task */
    if (!os_list_isempty(&mq->pending_list)) {
        os_list_resume(&(mq->pending_list));

        os_exit_critical(sr);

        os_sched();

        return OS_OK;
    }

    os_exit_critical(sr);

    return OS_OK;
}

/**
 * This function will receive a message from message queue object, if there is
 * no message in message queue object, the task shall wait for a specified
 * time.
 *
 * @param mq the message queue object
 * @param buffer the received message will be saved in
 * @param size the size of buffer
 * @param timeout the waiting time
 *
 * @return the error code
 */
os_err_t os_mqueue_get(os_mqueue_t *mq,
                    void      *buffer,
                    size_t  size,
                    os_tick_t timeout)
{
    os_task_t *task;
    os_sr_t sr;
    os_mqueue_msg_t *msg;
    os_tick_t tick_delta = 0;

    OS_ASSERT(mq != NULL);
    OS_ASSERT(buffer != NULL);
    OS_ASSERT(size != 0);

    /* get current task */
    task = os_task_self();

    sr = os_enter_critical();

    /* for non-blocking call */
    if (mq->entry == 0 && timeout == OS_NO_WAIT) {
        os_exit_critical(sr);

        return OS_TIMEOUT;
    }

    /* message queue is empty */
    while (mq->entry == 0) {
        OS_DEBUG_IN_TASK_CONTEXT;

        /* reset error number in task */
        task->error = OS_OK;

        /* no waiting, return timeout */
        if (timeout == OS_NO_WAIT) {
            os_exit_critical(sr);

            task->error = OS_TIMEOUT;

            return OS_TIMEOUT;
        }

        /* suspend current task */
        os_list_suspend(&(mq->pending_list),
                            task,
                            mq->flag);

        /* no wait forever, start task timer */
        if (timeout != OS_WAIT_FOREVER) {
            /* get the start tick of timer */
            tick_delta = os_tick_get();

            OS_DEBUG_LOG(OS_DEBUG_IPC, ("set task:%s to timer list\n",
                                        task->name));

            /* reset the timeout of task timer and start it */
            os_timer_tick_set(&(task->timer), timeout);
            os_timer_start(&(task->timer));
        }

        os_exit_critical(sr);

        /* re-schedule */
        os_sched();

        /* recv message */
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

    /* get message from queue */
    msg = (os_mqueue_msg_t *)mq->msg_queue_head;

    /* move message queue head */
    mq->msg_queue_head = msg->next;
    /* reach queue tail, set to NULL */
    if (mq->msg_queue_tail == msg)
        mq->msg_queue_tail = NULL;

    /* decrease message entry */
    mq->entry--;

    os_exit_critical(sr);

    /* copy message */
    memcpy(buffer, msg + 1, size > mq->msg_size ? mq->msg_size : size);

    sr = os_enter_critical();
    /* put message to free list */
    msg->next = (os_mqueue_msg_t *)mq->msg_queue_free;
    mq->msg_queue_free = msg;
    os_exit_critical(sr);

    return OS_OK;
}

/**
 * This function can get or set some extra attributions of a message queue
 * object.
 *
 * @param mq the message queue object
 * @param arg the execution argument
 *
 * @return the error code
 */
os_err_t os_mqueue_reset(os_mqueue_t *mq, void *arg)
{
    os_sr_t sr;
    os_mqueue_msg_t *msg;

    OS_ASSERT(mq != NULL);

    sr = os_enter_critical();

    /* resume all waiting task */
    os_list_resume_all(&mq->pending_list);

    /* release all message in the queue */
    while (mq->msg_queue_head != NULL) {
        /* get message from queue */
        msg = (os_mqueue_msg_t *)mq->msg_queue_head;

        /* move message queue head */
        mq->msg_queue_head = msg->next;
        /* reach queue tail, set to NULL */
        if (mq->msg_queue_tail == msg)
            mq->msg_queue_tail = NULL;

        /* put message to free list */
        msg->next = (os_mqueue_msg_t *)mq->msg_queue_free;
        mq->msg_queue_free = msg;
    }

    /* clean entry */
    mq->entry = 0;

    os_exit_critical(sr);

    os_sched();

    return OS_OK;
}

/*@}*/
