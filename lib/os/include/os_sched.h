/*
 * File      : os_sched.h
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
#ifndef _OS_SCHEDULER_H_
#define _OS_SCHEDULER_H_

/*
 * schedule system service
 */
void os_sched_init(void);
void os_sched_start(void);
void os_sched_insert(os_task_t *task);
void os_sched_remove(os_task_t *task);

/*
 * schedule user service
 */
void os_sched(void);

void os_sched_lock(void);
void os_sched_unlock(void);

#endif /* _OS_SCHEDULER_H_ */
