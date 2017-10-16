/*
 * File      : backtrace.c
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

#ifdef __GNUC__
/*
-->High Address,Stack Top
PC<-----|
LR          |
IP          |
FP          |
......         |
PC<-|     |
LR     |     |
IP     |     |
FP---|-- |
......     |
PC     |
LR     |
IP      |
FP---
-->Low Address,Stack Bottom
*/
void os_arch_backtrace(uint32_t *fp, uint32_t thread_entry)
{
    uint32_t i, pc, func_entry;

    pc = *fp;
    printk("[0x%x]\n", pc-0xC);

    for (i = 0; i < 10; i++) {
        fp = (uint32_t *)*(fp - 3);
        pc = *fp ;

        func_entry = pc - 0xC;

        if (func_entry <= 0x30000000) break;

        if ((func_entry == thread_entry)) {
            printk("EntryPoint:0x%x\n", func_entry);

            break;
        }

        printk("[0x%x]\n", func_entry);
    }
}
#else
void os_arch_backtrace(uint32_t *fp, uint32_t thread_entry)
{
    /* old compiler implementation */
}
#endif
