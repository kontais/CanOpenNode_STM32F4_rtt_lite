/*
 * File      : main.c
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
#include <board.h>
#include <application.h>

int main(void)
{
    os_enter_critical();

    board_init();

    os_init();

    application_init();

    os_start();

    /* never reach here */
    while (1);
}
