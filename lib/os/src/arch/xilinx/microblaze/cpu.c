/*
 * File      : cpu.c
 * This file is part of RT-Thread RTOS
 * COPYRIGHT (C) 2006, RT-Thread Develop Team
 *
 * The license and distribution terms for this file may be
 * found in the file LICENSE in this distribution or at
 * http://www.rt-thread.org/license/LICENSE
 *
 * Change Logs:
 * Date           Author       Notes
 * 2006-08-23     Bernard      first version
 */

#include <os.h>

/**
 * this function will reset CPU
 *
 */
void os_arch_reset()
{
}

/**
 * this function will shutdown CPU
 *
 */
void os_arch_shutdown()
{
    printk("shutdown...\n");

    while (1);
}

