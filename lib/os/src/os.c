/*
 * File      : os.c
 * This file is part of RT-Thread RTOS
 * COPYRIGHT (C) 2006 - 2013, RT-Thread Develop Team
 *
 * The license and distribution terms for this file may be
 * found in the file LICENSE in this distribution or at
 * http://openlab.rt-thread.com/license/LICENSE
 *
 * Change Logs:
 * Date           Author       Notes
 * 2016-12-06     kontais      kontais@aliyun.com
 */
#include <os.h>

void os_init(void)
{
#ifdef OS_CFG_HEAP
    os_heap_init((void*)HEAP_START, (void*)OS_HEAP_END);
#endif

    /* init scheduler system */
    os_sched_init();
}

void os_start(void)
{
    /* init timer task */
#ifdef OS_CFG_SOFT_TIMER
    os_init_timer_task();
#endif
    /* init idle task */
    os_init_idle_task();

    /* start scheduler */
    os_sched_start();
}
