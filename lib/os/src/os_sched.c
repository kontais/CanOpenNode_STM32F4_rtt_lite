/*
 * File      : os_sched.c
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
 * 2013-12-21     Grissiom     add os_critical_level
 */

#include <os.h>

static int16_t os_sched_lock_nest;

os_list_t os_ready_task_priority_list[OS_TASK_PRIORITY_MAX];
os_task_t *os_current_task;

#if OS_TASK_PRIORITY_MAX > 32
/* Maximum priority level, 256 */
uint8_t  os_ready_task_group;
uint32_t os_ready_task_bitmap[8];
#else
/* Maximum priority level, 32 */
uint32_t os_ready_task_bitmap;
#endif

os_list_t os_defunct_task_list = OS_LIST_INIT(os_defunct_task_list);

#ifdef OS_CFG_OVERFLOW_CHECK
static void _os_sched_stack_check(os_task_t *task)
{
    OS_ASSERT(task != NULL);

    if (*((uint8_t *)task->stack_addr) != '#' ||
	(uint32_t)task->sp <= (uint32_t)task->stack_addr ||
        (uint32_t)task->sp >
        (uint32_t)task->stack_addr + (uint32_t)task->stack_size) {
        os_sr_t sr;

        printf("task:%s stack overflow\n", task->name);

        sr = os_enter_critical();
        while (sr);
    } else if ((uint32_t)task->sp <= ((uint32_t)task->stack_addr + 32)) {
        printf("warning: %s stack is close to end of stack address.\n",
                   task->name);
    }
}
#endif

/**
 * @ingroup SystemInit
 * This function will initialize the system scheduler
 */
void os_sched_init(void)
{
    offset_t offset;

    os_sched_lock_nest = 0;

    OS_DEBUG_LOG(OS_DEBUG_SCHEDULER, ("start scheduler: max priority 0x%02x\n",
                                      OS_TASK_PRIORITY_MAX));

    for (offset = 0; offset < OS_TASK_PRIORITY_MAX; offset++) {
        os_list_init(&os_ready_task_priority_list[offset]);
    }

    os_current_task     = NULL;

#if OS_TASK_PRIORITY_MAX > 32
    /* initialize ready priority group */
    os_ready_task_group = 0;
    /* initialize ready table */
    memset(os_ready_task_bitmap, 0, sizeof(os_ready_task_bitmap));
#else
    os_ready_task_bitmap = 0;
#endif
}

/**
 * @ingroup SystemInit
 * This function will startup scheduler. It will select one task
 * with the highest priority level, then switch to it.
 */
void os_sched_start(void)
{
    register os_task_t *to_task;
    uint32_t highest_ready_priority;

#if OS_TASK_PRIORITY_MAX > 32
    uint8_t group;

    group = __ffs(os_ready_task_group) - 1;
    highest_ready_priority = (group << 5) + __ffs(os_ready_task_bitmap[group]) - 1;
#else
    highest_ready_priority = __ffs(os_ready_task_bitmap) - 1;
#endif

    /* get switch to task */
    to_task = OS_LIST_ENTRY(os_ready_task_priority_list[highest_ready_priority].next,
                              os_task_t,
                              tlist);

    os_current_task = to_task;

    /* switch to new task */
    os_arch_context_switch_to((uint32_t)&to_task->sp);

    /* never come back */
}

/**
 * @addtogroup Thread
 */

/*@{*/

/**
 * This function will perform one schedule. It will select one task
 * with the highest priority level, then switch to it.
 */
void os_sched(void)
{
    os_sr_t sr;
    os_task_t *to_task;
    os_task_t *from_task;

    sr = os_enter_critical();

    /* check the scheduler is enabled or not */
    if (os_sched_lock_nest == 0) {
        uint32_t highest_ready_priority;

#if OS_TASK_PRIORITY_MAX > 32
        uint8_t group;

        group = __ffs(os_ready_task_group) - 1;
        highest_ready_priority = (group << 5) + __ffs(os_ready_task_bitmap[group]) - 1;
#else
        highest_ready_priority = __ffs(os_ready_task_bitmap) - 1;
#endif

        /* get switch to task */
        to_task = OS_LIST_ENTRY(os_ready_task_priority_list[highest_ready_priority].next,
                                  os_task_t,
                                  tlist);

        /* if the destination task is not the same as current task */
        if (to_task != os_current_task) {
            from_task           = os_current_task;
            os_current_task     = to_task;

            /* switch to new task */
            OS_DEBUG_LOG(OS_DEBUG_SCHEDULER,
                         ("[%d]switch to priority#%d "
                          "task:%.*s(sp:0x%p), "
                          "from task:%.*s(sp: 0x%p)\n",
                          os_isr_nest, highest_ready_priority,
                          OS_NAME_MAX, to_task->name, to_task->sp,
                          OS_NAME_MAX, from_task->name, from_task->sp));

#ifdef OS_CFG_OVERFLOW_CHECK
            _os_sched_stack_check(to_task);
#endif

            if (os_isr_nest == 0) {
                os_arch_context_switch((uint32_t)&from_task->sp,
                                     (uint32_t)&to_task->sp);
            } else {
                OS_DEBUG_LOG(OS_DEBUG_SCHEDULER, ("switch in interrupt\n"));

                os_arch_context_switch_interrupt((uint32_t)&from_task->sp,
                                               (uint32_t)&to_task->sp);
            }
        }
    }

    os_exit_critical(sr);
}

/*
 * This function will insert a task to system ready queue. The state of
 * task will be set as READY and remove from suspend queue.
 *
 * @param task the task to be inserted
 * @note Please do not invoke this function in user application.
 */
void os_sched_insert(os_task_t *task)
{
    os_sr_t sr;

    OS_ASSERT(task != NULL);

    sr = os_enter_critical();

    /* change stat */
    task->stat = OS_TASK_READY;

    /* insert task to ready list */
    os_list_insert_before(&(os_ready_task_priority_list[task->current_priority]),
                          &(task->tlist));

    /* set priority mask */
#if OS_TASK_PRIORITY_MAX > 32
    OS_DEBUG_LOG(OS_DEBUG_SCHEDULER,
                 ("insert task[%.*s], the priority: %d %d\n",
                  OS_NAME_MAX,
                  task->name,
                  task->priority_group,
                  task->priority_mask));
#else
                  OS_DEBUG_LOG(OS_DEBUG_SCHEDULER, ("insert task[%.*s], the priority: %d\n",
                                      OS_NAME_MAX, task->name, task->current_priority));
#endif

#if OS_TASK_PRIORITY_MAX > 32
    os_ready_task_group |= 1UL << task->priority_group;
    os_ready_task_bitmap[task->priority_group] |= task->priority_mask;
#else
    os_ready_task_bitmap |= task->priority_mask;
#endif

    os_exit_critical(sr);
}

/*
 * This function will remove a task from system ready queue.
 *
 * @param task the task to be removed
 *
 * @note Please do not invoke this function in user application.
 */
void os_sched_remove(os_task_t *task)
{
    os_sr_t sr;

    OS_ASSERT(task != NULL);

    sr = os_enter_critical();

#if OS_TASK_PRIORITY_MAX > 32
    OS_DEBUG_LOG(OS_DEBUG_SCHEDULER,
                 ("remove task[%.*s], the priority: %d %d\n",
                  OS_NAME_MAX,
                  task->name,
                  task->priority_group,
                  task->priority_mask));
#else
    OS_DEBUG_LOG(OS_DEBUG_SCHEDULER, ("remove task[%.*s], the priority: %d\n",
                                      OS_NAME_MAX, task->name,
                                      task->current_priority));
#endif

    /* remove task from ready list */
    os_list_remove(&(task->tlist));
    if (os_list_isempty(&(os_ready_task_priority_list[task->current_priority]))) {
#if OS_TASK_PRIORITY_MAX > 32
        os_ready_task_bitmap[task->priority_group] &= ~task->priority_mask;
        if (os_ready_task_bitmap[task->priority_group] == 0) {
            os_ready_task_group &= ~(1UL << task->priority_group);
        }
#else
        os_ready_task_bitmap &= ~task->priority_mask;
#endif
    }

    os_exit_critical(sr);
}

/**
 * This function will lock the task scheduler.
 */
void os_sched_lock(void)
{
    os_sr_t sr;

    sr = os_enter_critical();

    os_sched_lock_nest++;

    os_exit_critical(sr);
}

/**
 * This function will unlock the task scheduler.
 */
void os_sched_unlock(void)
{
    os_sr_t sr;

    sr = os_enter_critical();

    os_sched_lock_nest--;

    if (os_sched_lock_nest <= 0) {
        os_sched_lock_nest = 0;
        os_exit_critical(sr);

        os_sched();
    } else {
        os_exit_critical(sr);
    }
}
