/*
 * File      : os_mbox.h
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
#ifndef _OS_MAIL_BOX_H_
#define _OS_MAIL_BOX_H_

/**
 * mailbox structure
 */
struct os_mbox
{
    uint8_t flag;                                    /* flag of kernel object */
    os_list_t        pending_list;                    /* tasks pended on this resource */

    uint32_t         *msg_pool;                      /* start address of message buffer */

    uint16_t          size;                          /* size of message pool */

    uint16_t          entry;                         /* index of messages in msg_pool */
    uint16_t          in_offset;                     /* input offset of the message buffer */
    uint16_t          out_offset;                    /* output offset of the message buffer */

    os_list_t         sender_pending_list;         /* sender task suspended on this mailbox */
};
typedef struct os_mbox os_mbox_t;

/*
 * mailbox interface
 */
os_err_t os_mbox_init(os_mbox_t *mb,
                    const char  *name,
                    void        *msgpool,
                    size_t    size,
                    uint8_t   flag);

os_err_t os_mbox_put(os_mbox_t *mb, uint32_t value);
os_err_t os_mbox_put_wait(os_mbox_t *mb,
                         uint32_t  value,
                         os_tick_t   timeout);
os_err_t os_mbox_get(os_mbox_t *mb, uint32_t *value, os_tick_t timeout);
os_err_t os_mbox_reset(os_mbox_t *mb, void *arg);

#endif /* _OS_MAIL_BOX_H_ */
