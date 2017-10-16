/*
 * File      : os_ipc.c
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
 * 2013-09-14     Grissiom     add an option check in os_event_get
 */

#include <os.h>

/**
 * @addtogroup IPC
 */

/*@{*/

/**
 * This function will suspend a task to a specified list. IPC object or some
 * double-queue object (mailbox etc.) contains this kind of list.
 *
 * @param list the IPC suspended task list
 * @param task the task object to be suspended
 * @param flag the IPC object flag,
 *        which shall be OS_IPC_FIFO/OS_IPC_PRIO.
 *
 * @return the operation status, OS_OK on successful
 */
os_err_t os_list_suspend(os_list_t        *list,
                                       os_task_t *task,
                                       uint8_t        flag)
{
    /* suspend task */
    os_task_suspend(task);

    switch (flag) {
    case OS_IPC_FIFO:
        os_list_insert_before(list, &(task->tlist));
        break;

    case OS_IPC_PRIO:
        {
            struct os_list_node *n;
            os_task_t *sleep_task;

            /* find a suitable position */
            for (n = list->next; n != list; n = n->next) {
                sleep_task = OS_LIST_ENTRY(n, os_task_t, tlist);

                /* find out */
                if (task->current_priority < sleep_task->current_priority) {
                    /* insert this task before the sleep_task */
                    os_list_insert_before(&(sleep_task->tlist), &(task->tlist));
                    break;
                }
            }

            /*
             * not found a suitable position,
             * append to the end of pending_list
             */
            if (n == list)
                os_list_insert_before(list, &(task->tlist));
        }
        break;
    }

    return OS_OK;
}

/**
 * This function will resume the first task in the list of a IPC object:
 * - remove the task from suspend queue of IPC object
 * - put the task into system ready queue
 *
 * @param list the task list
 *
 * @return the operation status, OS_OK on successful
 */
os_err_t os_list_resume(os_list_t *list)
{
    os_task_t *task;

    /* get task entry */
    task = OS_LIST_ENTRY(list->next, os_task_t, tlist);

    OS_DEBUG_LOG(OS_DEBUG_IPC, ("resume task:%s\n", task->name));

    /* resume it */
    os_task_resume(task);

    return OS_OK;
}

/**
 * This function will resume all suspended tasks in a list, including
 * suspend list of IPC object and private list of mailbox etc.
 *
 * @param list of the tasks to resume
 *
 * @return the operation status, OS_OK on successful
 */
os_err_t os_list_resume_all(os_list_t *list)
{
    os_task_t *task;
    os_sr_t sr;

    /* wakeup all suspend tasks */
    while (!os_list_isempty(list)) {
        sr = os_enter_critical();

        /* get next suspend task */
        task = OS_LIST_ENTRY(list->next, os_task_t, tlist);
        /* set error code to OS_ERROR */
        task->error = OS_ERROR;

        /*
         * resume task
         * In os_task_resume function, it will remove current task from
         * suspend list
         */
        os_task_resume(task);

        os_exit_critical(sr);
    }

    return OS_OK;
}
