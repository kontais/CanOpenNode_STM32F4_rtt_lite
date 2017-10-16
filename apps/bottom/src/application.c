/*
 * File      : application.c
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
#include <board.h>
#include <os.h>

#include <CO_os.h>

#define MAIN_TASK_STACK_SIZE    2048
static os_task_t main;
ALIGN(OS_ALIGN_SIZE) static uint8_t main_task_stack[MAIN_TASK_STACK_SIZE];

void os_task_main_cleanup(os_task_t *task)
{
    printf("main task exit\n");
}

void main_loop(void);

void os_task_main_entry(void* parameter)
{
    os_task_t *task;

    task = os_task_self();
    task->cleanup = os_task_main_cleanup;

    main_loop();
}

int application_init(void)
{
    uint32_t os_ver;

    os_ver = os_version_get();

    printf("os verion %d.%d.%d\n", 
                OS_VER_MAJOR(os_ver),
                OS_VER_MINOR(os_ver),
                OS_VER_REVISION(os_ver));

    os_task_init(&main,
                    "main",
                    os_task_main_entry,
                    NULL,
                    &main_task_stack[0],
                    MAIN_TASK_STACK_SIZE,
                    OS_MAIN_TASK_PRIO,
                    20);
    os_task_startup(&main);

    return 0;
}
