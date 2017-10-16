/*
 * File      : showmem.c
 * This file is part of RT-Thread RTOS
 * COPYRIGHT (C) 2006, 2008 RT-Thread Development Team
 *
 * The license and distribution terms for this file may be
 * found in the file LICENSE in this distribution or at
 * http://openlab.rt-thread.com/license/LICENSE
 *
 * Change Logs:
 * Date           Author       Notes
 * 2008-07-29     Bernard      first version from QiuYi implementation
 */

#include <os.h>

void os_arch_show_memory(uint32_t addr, uint32_t size)
{
    int i = 0, j =0;

    OS_ASSERT(addr);

    addr = addr & ~0xF;
    size = 4*((size + 3)/4);

    while (i < size) {
        printk("0x%08x: ", addr);

        for (j = 0; j < 4; j++) {
            printk("0x%08x  ", *(uint32_t *)addr);

            addr += 4;
            i++;
        }

        printk("\n");
    }

    return;
}
