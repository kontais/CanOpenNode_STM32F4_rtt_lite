/*
 * File      : os_mpool.c
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
 * 2012-03-22     Bernard      fix align issue in os_mpool_init and os_mpool_create.
 */

#include <os.h>

/**
 * @addtogroup MM
 */

/*@{*/

/**
 * This function will initialize a memory pool object, normally which is used
 * for static object.
 *
 * @param mp the memory pool object
 * @param name the name of memory pool
 * @param start the star address of memory pool
 * @param size the total size of memory pool
 * @param block_size the size for each block
 *
 * @return OS_OK
 */
os_err_t os_mpool_init(os_mpool_t *mp,
                    const char        *name,
                    void              *start,
                    size_t          size,
                    size_t          block_size)
{
    uint8_t *block_ptr;
    offset_t offset;

    /* parameter check */
    OS_ASSERT(mp != NULL);

    /* initialize memory pool */
    mp->start_address = start;
    mp->size = OS_ALIGN_DOWN(size, OS_ALIGN_SIZE);

    /* align the block size */
    block_size = OS_ALIGN(block_size, OS_ALIGN_SIZE);
    mp->block_size = block_size;

    /* align to align size byte */
    mp->block_total_count = mp->size / (mp->block_size + sizeof(uint8_t *));
    mp->block_free_count  = mp->block_total_count;

    /* initialize suspended task list */
    os_list_init(&(mp->pending_list));
    mp->suspend_task_count = 0;

    /* initialize free block list */
    block_ptr = (uint8_t *)mp->start_address;
    for (offset = 0; offset < mp->block_total_count; offset++) {
        *(uint8_t **)(block_ptr + offset * (block_size + sizeof(uint8_t *))) =
            (uint8_t *)(block_ptr + (offset + 1) * (block_size + sizeof(uint8_t *)));
    }

    *(uint8_t **)(block_ptr + (offset - 1) * (block_size + sizeof(uint8_t *))) =
        NULL;

    mp->block_list = block_ptr;

    return OS_OK;
}

/**
 * This function will detach a memory pool from system object management.
 *
 * @param mp the memory pool object
 *
 * @return OS_OK
 */
os_err_t os_mpool_delete(os_mpool_t *mp)
{
    os_task_t *task;
    os_sr_t sr;

    /* parameter check */
    OS_ASSERT(mp != NULL);

    /* wake up all suspended tasks */
    while (!os_list_isempty(&(mp->pending_list))) {
        sr = os_enter_critical();

        /* get next suspend task */
        task = OS_LIST_ENTRY(mp->pending_list.next, os_task_t, tlist);
        /* set error code to OS_ERROR */
        task->error = OS_ERROR;

        /*
         * resume task
         * In os_task_resume function, it will remove current task from
         * suspend list
         */
        os_task_resume(task);

        /* decrease suspended task count */
        mp->suspend_task_count--;

        os_exit_critical(sr);
    }

    return OS_OK;
}

/**
 * This function will allocate a block from memory pool
 *
 * @param mp the memory pool object
 * @param time the waiting time
 *
 * @return the allocated memory block or NULL on allocated failed
 */
void *os_mpool_alloc(os_mpool_t *mp, os_tick_t timeout)
{
    uint8_t *block_ptr;
    os_sr_t sr;
    os_task_t *task;
    os_tick_t tick_delta = 0;

    /* get current task */
    task = os_task_self();

    sr = os_enter_critical();

    while (mp->block_free_count == 0) {
        /* memory block is unavailable. */
        if (timeout == OS_NO_WAIT) {
            os_exit_critical(sr);

            os_errno_set(OS_TIMEOUT);

            return NULL;
        }

        OS_DEBUG_NOT_IN_INTERRUPT;

        task->error = OS_OK;

        /* need suspend task */
        os_task_suspend(task);
        os_list_insert_after(&(mp->pending_list), &(task->tlist));
        mp->suspend_task_count++;

        /* no wait forever, start task timer */
        if (timeout != OS_WAIT_FOREVER) {
            /* get the start tick of timer */
            tick_delta = os_tick_get();

            /* init task timer and start it */
            os_timer_tick_set(&(task->timer), timeout);
            os_timer_start(&(task->timer));
        }

        os_exit_critical(sr);

        /* do a schedule */
        os_sched();

        if (task->error != OS_OK)
            return NULL;

        sr = os_enter_critical();

        /* if it's not waiting forever and then re-calculate timeout tick */
        if (timeout != OS_WAIT_FOREVER) {
            tick_delta = os_tick_get() - tick_delta;
            if (tick_delta >= timeout) {
                timeout = OS_NO_WAIT;
            } else {
                timeout -= tick_delta;
            }
        }
    }

    /* memory block is available. decrease the free block counter */
    mp->block_free_count--;

    /* get block from block list */
    block_ptr = mp->block_list;
    OS_ASSERT(block_ptr != NULL);

    /* Setup the next free node. */
    mp->block_list = *(uint8_t **)block_ptr;

    /* point to memory pool */
    *(uint8_t **)block_ptr = (uint8_t *)mp;

    os_exit_critical(sr);

    return (uint8_t *)(block_ptr + sizeof(uint8_t *));
}

/**
 * This function will release a memory block
 *
 * @param block the address of memory block to be released
 */
void os_mpool_free(void *block)
{
    uint8_t **block_ptr;
    os_mpool_t *mp;
    os_task_t *task;
    os_sr_t sr;

    /* get the control block of pool which the block belongs to */
    block_ptr = (uint8_t **)((uint8_t *)block - sizeof(uint8_t *));
    mp        = (os_mpool_t *)*block_ptr;

    sr = os_enter_critical();

    /* increase the free block count */
    mp->block_free_count++;

    /* link the block into the block list */
    *block_ptr = mp->block_list;
    mp->block_list = (uint8_t *)block_ptr;

    if (mp->suspend_task_count > 0) {
        /* get the suspended task */
        task = OS_LIST_ENTRY(mp->pending_list.next,
                               os_task_t,
                               tlist);

        /* set error */
        task->error = OS_OK;

        /* resume task */
        os_task_resume(task);

        /* decrease suspended task count */
        mp->suspend_task_count--;

        os_exit_critical(sr);

        /* do a schedule */
        os_sched();

        return;
    }

    os_exit_critical(sr);
}

/*@}*/
