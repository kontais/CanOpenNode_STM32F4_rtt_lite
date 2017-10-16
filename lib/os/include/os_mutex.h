/*
 * File      : os_mutex.h
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
#ifndef _OS_MUTEX_H_
#define _OS_MUTEX_H_

/**
 * Mutual exclusion (mutex) structure
 */
struct os_mutex
{
    uint8_t flag;                                    /* flag of kernel object */
    os_list_t        pending_list;                    /* tasks pended on this resource */

    uint16_t          value;                         /* value of mutex */

    uint8_t           original_priority;             /* priority of last task hold the mutex */
    uint8_t           hold;                          /* numbers of task hold the mutex */

    os_task_t    *owner;                         /* current owner of mutex */
};
typedef struct os_mutex os_mutex_t;

/*
 * mutex interface
 */
os_err_t os_mutex_init(os_mutex_t *mutex, uint8_t flag);
os_err_t os_mutex_delete(os_mutex_t *mutex);

os_err_t os_mutex_take(os_mutex_t *mutex, os_tick_t timeout);
os_err_t os_mutex_release(os_mutex_t *mutex);

#endif /* _OS_MUTEX_H_ */
