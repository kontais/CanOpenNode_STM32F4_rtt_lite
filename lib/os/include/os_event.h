/*
 * File      : os_event.h
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
#ifndef _OS_EVENT_H_
#define _OS_EVENT_H_

/**
 * flag defintions in event
 */
#define OS_EVENT_AND               0x01            /* logic and */
#define OS_EVENT_OR                0x02            /* logic or */
#define OS_EVENT_CLEAR             0x04            /* clear flag */

/*
 * event structure
 */
struct os_event
{
    uint8_t flag;                                    /* flag of kernel object */
    os_list_t        pending_list;                    /* tasks pended on this resource */

    uint32_t          set;                           /* event set */
};
typedef struct os_event os_event_t;

/*
 * event interface
 */
os_err_t os_event_init(os_event_t *event, const char *name, uint8_t flag);
os_err_t os_event_delete(os_event_t *event);

os_err_t os_event_put(os_event_t *event, uint32_t set);
os_err_t os_event_get(os_event_t *event,
                       uint32_t  set,
                       uint8_t   opt,
                       os_tick_t timeout,
                       uint32_t *recved);
os_err_t os_event_reset(os_event_t *event, uint32_t set);

#endif /* _OS_EVENT_H_ */
