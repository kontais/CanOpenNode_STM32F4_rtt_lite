/*
 * File      : os_mpool.h
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
#ifndef _OS_MPOOL_H_
#define _OS_MPOOL_H_

/**
 * Base structure of Memory pool object
 */
struct os_mpool
{
    void            *start_address;                     /* memory pool start */
    size_t        size;                              /* size of memory pool */

    size_t        block_size;                        /* size of memory blocks */
    uint8_t      *block_list;                        /* memory blocks list */

    size_t        block_total_count;                 /* numbers of memory block */
    size_t        block_free_count;                  /* numbers of free memory block */

    os_list_t        pending_list;                    /* tasks pended on this resource */
    size_t        suspend_task_count;              /* numbers of task pended on this resource */
};
typedef struct os_mpool os_mpool_t;

/*
 * memory pool interface
 */
os_err_t os_mpool_init(os_mpool_t *mp,
                    const char        *name,
                    void              *start,
                    size_t          size,
                    size_t          block_size);
os_err_t os_mpool_delete(os_mpool_t *mp);

void *os_mpool_alloc(os_mpool_t *mp, os_tick_t timeout);
void os_mpool_free(void *block);

#endif /* _OS_MPOOL_H_ */
