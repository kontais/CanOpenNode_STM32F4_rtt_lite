/*
 * File      : os_mqueue.h
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
#ifndef _OS_MESSAGE_QUEUE_H_
#define _OS_MESSAGE_QUEUE_H_

/**
 * message queue structure
 */
struct os_mqueue
{
    uint8_t flag;                                    /* flag of kernel object */
    os_list_t        pending_list;                    /* tasks pended on this resource */

    void                *msg_pool;                      /* start address of message queue */

    uint16_t          msg_size;                      /* message size of each message */
    uint16_t          max_msgs;                      /* max number of messages */

    uint16_t          entry;                         /* index of messages in the queue */

    void                *msg_queue_head;                /* list head */
    void                *msg_queue_tail;                /* list tail */
    void                *msg_queue_free;                /* pointer indicated the free node of queue */
};
typedef struct os_mqueue os_mqueue_t;

/*
 * message queue interface
 */
os_err_t os_mqueue_init(os_mqueue_t *mq,
                    void       *msgpool,
                    size_t   msg_size,
                    size_t   pool_size,
                    uint8_t  flag);
os_err_t os_mqueue_delete(os_mqueue_t *mq);

os_err_t os_mqueue_put(os_mqueue_t *mq, void *buffer, size_t size);
os_err_t os_mqueue_put_urgent(os_mqueue_t *mq, void *buffer, size_t size);
os_err_t os_mqueue_get(os_mqueue_t *mq,
                    void      *buffer,
                    size_t  size,
                    os_tick_t timeout);
os_err_t os_mqueue_reset(os_mqueue_t *mq, void *arg);

#endif /* _OS_MESSAGE_QUEUE_H_ */
