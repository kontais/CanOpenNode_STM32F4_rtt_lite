/*
 * File      : os_idle.c
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
 * 2013-12-21     Grissiom     let os_task_idle_excute loop until there is no
 *                             dead task.
 */

#include <os.h>

#ifndef IDLE_TASK_STACK_SIZE
#define IDLE_TASK_STACK_SIZE  128
#endif

static os_task_t idle;
ALIGN(OS_ALIGN_SIZE)
static uint8_t os_task_stack[IDLE_TASK_STACK_SIZE];

extern os_list_t os_defunct_task_list;

/* Return whether there is defunctional task to be deleted. */
STATIC_INLINE int _has_defunct_task(void)
{
    /* The os_list_isempty has prototype of "int os_list_isempty(const os_list_t *l)".
     * So the compiler has a good reason that the os_defunct_task_list list does
     * not change within os_task_idle_excute thus optimize the "while" loop
     * into a "if".
     *
     * So add the volatile qualifier here. */
    const volatile os_list_t *l = (const volatile os_list_t*)&os_defunct_task_list;

    return l->next != l;
}

/**
 * @ingroup Thread
 *
 * This function will perform system background job when system idle.
 */
void os_task_idle_excute(void)
{
    /* Loop until there is no dead task. So one call to os_task_idle_excute
     * will do all the cleanups. */
    while (_has_defunct_task()) {
        os_sr_t sr;
        os_task_t *task;

        OS_DEBUG_NOT_IN_INTERRUPT;

        sr = os_enter_critical();

        /* re-check whether list is empty */
        if (_has_defunct_task()) {
            /* get defunct task */
            task = OS_LIST_ENTRY(os_defunct_task_list.next,
                                   os_task_t,
                                   tlist);
            /* remove defunct task */
            os_list_remove(&(task->tlist));
            /* invoke task cleanup */
            if (task->cleanup != NULL) {
                task->cleanup(task);
            }
        }

        os_exit_critical(sr);
    }
}

static void os_task_idle_entry(void *parameter)
{
    while (1) {
        os_task_idle_excute();
    }
}

/**
 * @ingroup SystemInit
 *
 * This function will initialize idle task, then start it.
 *
 * @note this function must be invoked when system init.
 */
void os_init_idle_task(void)
{
    /* initialize task */
    os_task_init(&idle,
                   "idle",
                   os_task_idle_entry,
                   NULL,
                   &os_task_stack[0],
                   sizeof(os_task_stack),
                   OS_IDLE_TASK_PRIO,
                   32);

    /* startup */
    os_task_startup(&idle);
}
