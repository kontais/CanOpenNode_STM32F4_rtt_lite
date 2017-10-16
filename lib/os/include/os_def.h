/*
 * File      : os_def.h
 * This file is part of RT-Thread RTOS
 * COPYRIGHT (C) 2006 - 2015, RT-Thread Development Team
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
 * 2007-01-10     Bernard      the first version
 * 2008-07-12     Bernard      remove all rt_int8, uint32_t etc typedef
 * 2010-10-26     yi.qiu       add module support
 * 2010-11-10     Bernard      add cleanup callback function in task exit.
 * 2011-05-09     Bernard      use builtin va_arg in GCC 4.x
 * 2012-11-16     Bernard      change NULL from ((void*)0) to 0.
 * 2012-12-29     Bernard      change the OS_CFG_MEMPOOL location and add
 *                             OS_CFG_HEAP condition.
 * 2012-12-30     Bernard      add more control command for graphic.
 * 2013-01-09     Bernard      change version number.
 * 2015-02-01     Bernard      change version number to v2.1.0
 */

#ifndef __OS_DEF_H__
#define __OS_DEF_H__

#include <types.h>

//typedef uint32_t                    time_t;         /* Type for time stamp */
typedef uint32_t                    os_tick_t;      /* Type for tick count */
typedef uint32_t                    size_t;         /* Type for size number */
typedef int32_t                     offset_t;       /* Type for offset */

/**
 * @ingroup BasicDef
 *
 * @def OS_ALIGN(size, align)
 * Return the most contiguous size aligned at specified width. OS_ALIGN(13, 4)
 * would return 16.
 */
#define OS_ALIGN(size, align)           (((size) + (align) - 1) & ~((align) - 1))

/**
 * @ingroup BasicDef
 *
 * @def OS_ALIGN_DOWN(size, align)
 * Return the down number of aligned at specified width. OS_ALIGN_DOWN(13, 4)
 * would return 12.
 */
#define OS_ALIGN_DOWN(size, align)      ((size) & ~((align) - 1))

#define OS_WAIT_FOREVER             0xffffffff      /* Block forever until get resource. */
#define OS_NO_WAIT                  0x00000000      /* Non-block. */

#define OS_MAIN_TASK_PRIO             (OS_TASK_PRIORITY_MAX - 2)
#define OS_IDLE_TASK_PRIO             (OS_TASK_PRIORITY_MAX - 1)

#endif
