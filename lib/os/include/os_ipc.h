/*
 * File      : os_ipc.h
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
#ifndef _OS_IPC_H_
#define _OS_IPC_H_

#define OS_IPC_FIFO                 0x00            /* FIFOed IPC. @ref IPC. */
#define OS_IPC_PRIO                 0x01            /* PRIOed IPC. @ref IPC. */

/**
 * @addtogroup IPC
 */

/*@{*/

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
                                       uint8_t        flag);

/**
 * This function will resume the first task in the list of a IPC object:
 * - remove the task from suspend queue of IPC object
 * - put the task into system ready queue
 *
 * @param list the task list
 *
 * @return the operation status, OS_OK on successful
 */
os_err_t os_list_resume(os_list_t *list);

/**
 * This function will resume all suspended tasks in a list, including
 * suspend list of IPC object and private list of mailbox etc.
 *
 * @param list of the tasks to resume
 *
 * @return the operation status, OS_OK on successful
 */
os_err_t os_list_resume_all(os_list_t *list);

#endif /* _OS_IPC_H_ */
