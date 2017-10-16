/*
 * File      : os_heap.h
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
#ifndef _OS_MEM_H_
#define _OS_MEM_H_

/*
 * heap memory system service
 */
void os_heap_init(void *begin_addr, void *end_addr);

/*
 * heap memory user service
 */
void *os_malloc(size_t nbytes);
void os_free(void *ptr);
void *os_realloc(void *ptr, size_t nbytes);
void *os_calloc(size_t count, size_t size);

void *os_malloc_align(size_t size, size_t align);
void os_free_align(void *ptr);

void os_memory_info(uint32_t *total,
                    uint32_t *used,
                    uint32_t *max_used);

#endif /* _OS_MEM_H_ */
