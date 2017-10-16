/*
 * File      : os_task.c
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
 * 2012-12-29     Bernard      fixed compiling warning.
 */

#include <os.h>

extern os_list_t os_ready_task_priority_list[OS_TASK_PRIORITY_MAX];
extern os_task_t *os_current_task;
extern os_list_t os_defunct_task_list;

void os_task_timeout(void *parameter);

void os_task_exit(void)
{
    os_task_t *task;
    os_sr_t sr;

    /* get current task */
    task = os_current_task;

    sr = os_enter_critical();

    /* remove from schedule */
    os_sched_remove(task);
    /* change stat */
    task->stat = OS_TASK_CLOSE;

    /* remove it from timer list */
    os_timer_delete(&task->timer);

    if (task->cleanup != NULL) {
        /* insert to defunct task list */
        os_list_insert_after(&os_defunct_task_list, &(task->tlist));
    }

    os_exit_critical(sr);

    /* switch to next task */
    os_sched();
}

static os_err_t _os_task_init(os_task_t *task,
                                const char       *name,
                                void (*entry)(void *parameter),
                                void             *parameter,
                                void             *stack_start,
                                uint32_t       stack_size,
                                uint8_t        priority,
                                uint32_t       tick)
{
    /* init task list */
    os_list_init(&(task->tlist));
    
    strncpy(task->name, name, OS_NAME_MAX);

    task->entry = (void *)entry;
    task->parameter = parameter;

    /* stack init */
    task->stack_addr = stack_start;
    task->stack_size = stack_size;

    /* init task stack */
    memset(task->stack_addr, '#', task->stack_size);
    task->sp = (void *)os_arch_task_stack_init(task->entry, task->parameter,
                    (void *)((char *)task->stack_addr + task->stack_size - 4),
                    (void *)os_task_exit);

    /* priority init */
    OS_ASSERT(priority < OS_TASK_PRIORITY_MAX);
    task->current_priority = priority;

    /* tick init */
    task->slice_tick     = tick;
    task->remaining_tick = tick;

    /* error and flags */
    task->error = OS_OK;
    task->stat  = OS_TASK_INIT;

    /* initialize cleanup function and user data */
    task->cleanup   = NULL;
    task->user_data = 0;

    /* init task timer */
    os_timer_init(&(task->timer),
                  os_task_timeout,
                  task,
                  0,
                  0);

    return OS_OK;
}

/**
 * @addtogroup Thread
 */

/*@{*/

/**
 * This function will initialize a task, normally it's used to initialize a
 * static task object.
 *
 * @param task the static task object
 * @param name the name of task, which shall be unique
 * @param entry the entry function of task
 * @param parameter the parameter of task enter function
 * @param stack_start the start address of task stack
 * @param stack_size the size of task stack
 * @param priority the priority of task
 * @param tick the time slice if there are same priority task
 *
 * @return the operation status, OS_OK on OK, OS_ERROR on error
 */
os_err_t os_task_init(os_task_t *task,
                        const char       *name,
                        void (*entry)(void *parameter),
                        void             *parameter,
                        void             *stack_start,
                        uint32_t       stack_size,
                        uint8_t        priority,
                        uint32_t       tick)
{
    /* task check */
    OS_ASSERT(task != NULL);
    OS_ASSERT(stack_start != NULL);

    return _os_task_init(task,
                           name,
                           entry,
                           parameter,
                           stack_start,
                           stack_size,
                           priority,
                           tick);
}

/**
 * This function will return self task object
 *
 * @return the self task object
 */
os_task_t *os_task_self(void)
{
    return os_current_task;
}

/**
 * This function will start a task and put it to system ready queue
 *
 * @param task the task to be started
 *
 * @return the operation status, OS_OK on OK, OS_ERROR on error
 */
os_err_t os_task_startup(os_task_t *task)
{
    /* task check */
    OS_ASSERT(task != NULL);
    OS_ASSERT(task->stat == OS_TASK_INIT);

    /* calculate priority attribute */
#if OS_TASK_PRIORITY_MAX > 32
    task->priority_group = task->current_priority >> 5;             /* 3bit */
#endif
    task->priority_mask  = 1UL << (task->current_priority & 0x1f);  /* 5bit */

    OS_DEBUG_LOG(OS_DEBUG_TASK, ("startup a task:%s with priority:%d\n",
                                   task->name, task->current_priority));
    /* change task stat */
    task->stat = OS_TASK_SUSPEND;
    /* then resume it */
    os_task_resume(task);
    if (os_task_self() != NULL) {
        /* do a scheduling */
        os_sched();
    }

    return OS_OK;
}

/**
 * This function will detach a task. The task object will be removed from
 * task queue and detached/deleted from system object management.
 *
 * @param task the task to be deleted
 *
 * @return the operation status, OS_OK on OK, OS_ERROR on error
 */
os_err_t os_task_delete(os_task_t *task)
{
    os_sr_t sr;

    /* task check */
    OS_ASSERT(task != NULL);

    /* remove from schedule */
    os_sched_remove(task);

    /* release task timer */
    os_timer_delete(&(task->timer));

    /* change stat */
    task->stat = OS_TASK_CLOSE;

    if (task->cleanup != NULL) {
        sr = os_enter_critical();

        /* insert to defunct task list */
        os_list_insert_after(&os_defunct_task_list, &(task->tlist));

        os_exit_critical(sr);
    }

    return OS_OK;
}

/**
 * This function will let current task yield processor, and scheduler will
 * choose a highest task to run. After yield processor, the current task
 * is still in READY state.
 *
 * @return OS_OK
 */
os_err_t os_task_yield(void)
{
    os_sr_t sr;
    os_task_t *task;

    sr = os_enter_critical();

    /* set to current task */
    task = os_current_task;

    /* if the task stat is READY and on ready queue list */
    if (task->stat == OS_TASK_READY &&
        task->tlist.next != task->tlist.prev) {
        /* remove task from task list */
        os_list_remove(&(task->tlist));

        /* put task to end of ready queue */
        os_list_insert_before(&(os_ready_task_priority_list[task->current_priority]),
                              &(task->tlist));

        os_exit_critical(sr);

        os_sched();

        return OS_OK;
    }

    os_exit_critical(sr);

    return OS_OK;
}

/**
 * This function will let current task sleep for some ticks.
 *
 * @param tick the sleep ticks
 *
 * @return OS_OK
 */
os_err_t os_task_sleep(os_tick_t tick)
{
    os_sr_t sr;
    os_task_t *task;

    sr = os_enter_critical();
    /* set to current task */
    task = os_current_task;
    OS_ASSERT(task != NULL);

    /* suspend task */
    os_task_suspend(task);

    /* reset the timeout of task timer and start it */
    os_timer_tick_set(&(task->timer), tick);
    os_timer_start(&(task->timer));

    os_exit_critical(sr);

    os_sched();

    /* clear error number of this task to OS_OK */
    if (task->error == OS_TIMEOUT)
        task->error = OS_OK;

    return OS_OK;
}

os_err_t os_task_priority_set(os_task_t *task, uint8_t priority)
{
    os_sr_t sr;

    /* task check */
    OS_ASSERT(task != NULL);

    sr = os_enter_critical();

    /* for ready task, change queue */
    if (task->stat == OS_TASK_READY) {
        /* remove task from schedule queue first */
        os_sched_remove(task);

        /* change task priority */
        task->current_priority = priority;

        /* recalculate priority attribute */
    #if OS_TASK_PRIORITY_MAX > 32
        task->priority_group = task->current_priority >> 5;             /* 3bit */
    #endif
        task->priority_mask  = 1UL << (task->current_priority & 0x1f);  /* 5bit */

        /* insert task to schedule queue again */
        os_sched_insert(task);
    } else {
        task->current_priority = priority;

        /* recalculate priority attribute */
    #if OS_TASK_PRIORITY_MAX > 32
        task->priority_group = task->current_priority >> 5;             /* 3bit */
    #endif
        task->priority_mask  = 1UL << (task->current_priority & 0x1f);  /* 5bit */
    }

    os_exit_critical(sr);

    return OS_OK;
}

/**
 * This function will suspend the specified task.
 *
 * @param task the task to be suspended
 *
 * @return the operation status, OS_OK on OK, OS_ERROR on error
 *
 * @note if suspend self task, after this function call, the
 * os_sched() must be invoked.
 */
os_err_t os_task_suspend(os_task_t *task)
{
    os_sr_t sr;

    /* task check */
    OS_ASSERT(task != NULL);

    OS_DEBUG_LOG(OS_DEBUG_TASK, ("task suspend:  %s\n", task->name));

    if (task->stat != OS_TASK_READY) {
        OS_DEBUG_LOG(OS_DEBUG_TASK, ("task suspend: task disorder, %d\n",
                                       task->stat));

        return OS_ERROR;
    }

    sr = os_enter_critical();

    /* change task stat */
    task->stat = OS_TASK_SUSPEND;
    os_sched_remove(task);

    /* stop task timer anyway */
    os_timer_stop(&(task->timer));

    os_exit_critical(sr);

    return OS_OK;
}

/**
 * This function will resume a task and put it to system ready queue.
 *
 * @param task the task to be resumed
 *
 * @return the operation status, OS_OK on OK, OS_ERROR on error
 */
os_err_t os_task_resume(os_task_t *task)
{
    os_sr_t sr;

    /* task check */
    OS_ASSERT(task != NULL);

    OS_DEBUG_LOG(OS_DEBUG_TASK, ("task resume:  %s\n", task->name));

    if (task->stat != OS_TASK_SUSPEND) {
        OS_DEBUG_LOG(OS_DEBUG_TASK, ("task resume: task disorder, %d\n",
                                       task->stat));

        return OS_ERROR;
    }

    sr = os_enter_critical();

    /* remove from suspend list */
    os_list_remove(&(task->tlist));

    os_timer_stop(&task->timer);

    os_exit_critical(sr);

    /* insert to schedule ready list */
    os_sched_insert(task);

    return OS_OK;
}

/**
 * This function is the timeout function for task, normally which is invoked
 * when task is timeout to wait some resource.
 *
 * @param parameter the parameter of task timeout function
 */
void os_task_timeout(void *parameter)
{
    os_task_t *task;

    task = (os_task_t *)parameter;

    /* task check */
    OS_ASSERT(task != NULL);
    OS_ASSERT(task->stat == OS_TASK_SUSPEND);

    /* set error number */
    task->error = OS_TIMEOUT;

    /* remove from suspend list */
    os_list_remove(&(task->tlist));

    /* insert to schedule ready list */
    os_sched_insert(task);

    /* do schedule */
    os_sched();
}

/*@}*/
