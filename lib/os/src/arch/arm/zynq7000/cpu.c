/*
 * File      : cpu.c
 * This file is part of RT-Thread RTOS
 * COPYRIGHT (C) 2013, RT-Thread Develop Team
 *
 * The license and distribution terms for this file may be
 * found in the file LICENSE in this distribution or at
 * http://www.rt-thread.org/license/LICENSE
 *
 * Change Logs:
 * Date           Author       Notes
 * 2013-07-20     Bernard      first version
 */

#include <rtcpu.h>
#include <os.h>
#include "zynq7000.h"

/**
 * reset cpu by dog's time-out
 *
 */
void os_arch_reset()
{
    while (1);  /* loop forever and wait for reset to happen */

    /* NEVER REACHED */
}

/**
 *  shutdown CPU
 *
 */
void os_arch_shutdown()
{
    uint32_t level;
    printk("shutdown...\n");

    sr = os_enter_critical();
    while (level) {
        OS_ASSERT(0);
    }
}

