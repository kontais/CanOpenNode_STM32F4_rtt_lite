/*
 * File      : os_version.h
 * This file is part of RT-Thread RTOS
 * COPYRIGHT (C) 2016, RT-Thread Development Team
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
#ifndef _OS_VERSION_H_
#define _OS_VERSION_H_

#define OS_MAJOR_VER                    2UL
#define OS_MINOR_VER                    1UL
#define OS_REVISION_VER                 0UL

#define OS_VERSION              ((OS_MAJOR_VER << 24) + (OS_MINOR_VER << 16) + (OS_REVISION_VER << 8))

#define OS_VER_MAJOR(ver)       ((ver >> 24) & 0xff)
#define OS_VER_MINOR(ver)       ((ver >> 16) & 0xff)
#define OS_VER_REVISION(ver)    ((ver >> 8)  & 0xff)

uint32_t os_version_get(void);

#endif /* _OS_VERSION_H_ */
