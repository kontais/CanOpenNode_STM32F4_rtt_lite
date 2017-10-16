/*
 * File      : os_sem.h
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
#ifndef _OS_SEMAPHORE_H_
#define _OS_SEMAPHORE_H_

/**
 * Semaphore structure
 */
struct os_sem
{
    uint8_t   flag;           /* flag of kernel object */
    os_list_t pending_list;   /* tasks pended on this resource */

    uint32_t  value;          /* value of semaphore. */
};
typedef struct os_sem os_sem_t;

/*
 * semaphore interface
 */
os_err_t os_sem_init(os_sem_t *sem,
                     const char *name,
                     uint32_t value,
                     uint8_t  flag);
os_err_t os_sem_delete(os_sem_t *sem);

os_err_t os_sem_take(os_sem_t *sem, os_tick_t time);
os_err_t os_sem_trytake(os_sem_t *sem);
os_err_t os_sem_give(os_sem_t *sem);
os_err_t os_sem_reset(os_sem_t *sem, uint32_t value);

#endif /* _OS_SEMAPHORE_H_ */
