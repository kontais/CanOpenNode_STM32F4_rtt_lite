/*
 * File      : os_task.h
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
#ifndef _OS_TASK_H_
#define _OS_TASK_H_

/**
 * @addtogroup Thread
 */

/*@{*/

/*
 * Thread
 */

/*
 * task state definitions
 */
#define OS_TASK_INIT                  0x00                /* Initialized status */
#define OS_TASK_READY                 0x01                /* Ready status */
#define OS_TASK_SUSPEND               0x02                /* Suspend status */
#define OS_TASK_RUNNING               0x03                /* Running status */
#define OS_TASK_BLOCK                 OS_TASK_SUSPEND     /* Blocked status */
#define OS_TASK_CLOSE                 0x04                /* Closed status */

/**
 * task control command definitions
 */
#define OS_TASK_CTRL_STARTUP          0x00                /* Startup task. */
#define OS_TASK_CTRL_CLOSE            0x01                /* Close task. */
#define OS_TASK_CTRL_CHANGE_PRIORITY  0x02                /* Change task priority. */
#define OS_TASK_CTRL_INFO             0x03                /* Get task information. */

/**
 * Thread structure
 */
struct os_task
{
    /* os object */
    char     name[OS_NAME_MAX];                 /* the name of task */
    uint8_t  type;                              /* type of object */
    uint8_t  flags;                             /* task's flags */

    os_list_t   tlist;                          /* the task list */

    /* stack point and entry */
    void       *sp;                             /* stack point */
    void       *entry;                          /* entry */
    void       *parameter;                      /* parameter */
    void       *stack_addr;                     /* stack address */
    uint32_t   stack_size;                      /* stack size */

    /* error code */
    os_err_t error;                             /* error code */

    uint8_t  stat;                              /* task stat */

    /* priority */
    uint8_t  current_priority;                  /* current priority */
#if OS_TASK_PRIORITY_MAX > 32
    uint8_t  priority_group;
#endif
    uint32_t priority_mask;

    /* task event */
    uint32_t event_set;
    uint8_t  event_info;

    os_tick_t  slice_tick;                      /* task's initialized tick */
    os_tick_t  remaining_tick;                  /* remaining tick */

    os_timer_t timer;                           /* built-in task timer */

    void (*cleanup)(struct os_task *task);      /* cleanup function when task exit */

    uint32_t user_data;                         /* private user data beyond this task */
};
typedef struct os_task os_task_t;

/*
 * task interface
 */
os_err_t os_task_init(os_task_t *task,
                        const char       *name,
                        void (*entry)(void *parameter),
                        void             *parameter,
                        void             *stack_start,
                        uint32_t       stack_size,
                        uint8_t        priority,
                        uint32_t       tick);
os_err_t os_task_delete(os_task_t *task);
os_task_t *os_task_self(void);
os_err_t os_task_startup(os_task_t *task);

os_err_t os_task_yield(void);
os_err_t os_task_sleep(os_tick_t tick);
os_err_t os_task_priority_set(os_task_t *task, uint8_t priority);
os_err_t os_task_suspend(os_task_t *task);
os_err_t os_task_resume(os_task_t *task);

#endif /* _OS_TASK_H_ */
