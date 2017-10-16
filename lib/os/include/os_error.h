/*
 * File      : os_error.h
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
#ifndef _OS_ERROR_H_
#define _OS_ERROR_H_

typedef enum {
  OS_OK       = 0, 	/* There is no error */
  OS_ERROR    = 1, 	/* A generic error happens */
  OS_TIMEOUT  = 2, 	/* Timed out */
  OS_EFULL    = 3, 	/* The resource is full */
  OS_EEMPTY   = 4, 	/* The resource is empty */
  OS_ENOMEM   = 5, 	/* No memory */
  OS_NOSYS    = 6, 	/* No system */
  OS_EBUSY    = 7, 	/* Busy */
  OS_EIO      = 8, 	/* IO error */
} os_err_t;

os_err_t os_errno_get(void);
void os_errno_set(os_err_t error);

#endif /* _OS_ERROR_H_ */
