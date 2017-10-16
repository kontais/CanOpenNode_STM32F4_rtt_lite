/*
 * File      : os_error.c
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
 * 2006-03-16     Bernard      the first version
 */

#include <os.h>

/**
 * @addtogroup KernelService
 */

/*@{*/

/* global errno in RT-Thread */
static volatile os_err_t _errno;

/*
 * This function will get errno
 *
 * @return errno
 */
os_err_t os_errno_get(void)
{
    os_task_t *task;

    if (os_isr_nest_get() != 0) {
        /* it's in interrupt context */
        return _errno;
    }

    task = os_task_self();
    if (task == NULL)
        return _errno;

    return task->error;
}

/*
 * This function will set errno
 *
 * @param error the errno shall be set
 */
void os_errno_set(os_err_t error)
{
    os_task_t *task;

    if (os_isr_nest_get() != 0) {
        /* it's in interrupt context */
        _errno = error;

        return;
    }

    task = os_task_self();
    if (task == NULL) {
        _errno = error;

        return;
    }

    task->error = error;
}
