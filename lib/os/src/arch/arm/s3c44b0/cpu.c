/*
 * File      : cpu.c
 * This file is part of RT-Thread RTOS
 * COPYRIGHT (C) 2006, RT-Thread Develop Team
 *
 * The license and distribution terms for this file may be
 * found in the file LICENSE in this distribution or at
 * http://openlab.rt-thread.com/license/LICENSE
 *
 * Change Logs:
 * Date           Author       Notes
 * 2006-09-06     XuXinming    first version
 */

#include <os.h>
#include "s3c44b0.h"

/**
 * @addtogroup S3C44B0
 */
/*@{*/

/**
 * This function will enable I-Cache of CPU
 *
 */
void os_arch_icache_enable()
{
    int32_t reg;

    volatile int i;
    /* flush cycle */
    for (i = 0x10002000; i < 0x10004800; i+=16) {
        *((int *)i)=0x0;
    }

    /*
     *    Init cache
     *    Non-cacheable area (everything outside RAM)
     *    0x0000:0000 - 0x0C00:0000
     */
    NCACHBE0 = 0xC0000000;
    NCACHBE1 = 0x00000000;

    /*
        Enable chache
    */
    reg = SYSCFG;
    reg |= 0x00000006; /* 8kB */
    SYSCFG = reg;
}

/**
 * This function will disable I-Cache of CPU
 *
 */
void os_arch_icache_disable()
{
    int32_t reg;

    reg = SYSCFG;
    reg &= ~0x00000006; /* 8kB */
    SYSCFG = reg;
}

/**
 * this function will get the status of I-Cache
 *
 */
int32_t os_arch_icache_status()
{
    return 0;
}

/**
 * this function will enable D-Cache of CPU
 *
 */
void os_arch_dcache_enable()
{
    os_arch_icache_enable();
}

/**
 * this function will disable D-Cache of CPU
 *
 */
void os_arch_dcache_disable()
{
    os_arch_icache_disable();
}

/**
 * this function will get the status of D-Cache
 *
 */
int32_t os_arch_dcache_status()
{
    return os_arch_icache_status();
}

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

/*@}*/
