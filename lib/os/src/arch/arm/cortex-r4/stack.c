/*
 * File      : stack.c
 * This file is part of RT-Thread RTOS
 * COPYRIGHT (C) 2006, RT-Thread Development Team
 *
 * The license and distribution terms for this file may be
 * found in the file LICENSE in this distribution or at
 * http://openlab.rt-thread.com/license/LICENSE
 *
 * Change Logs:
 * Date           Author       Notes
 * 2008-12-11     XuXinming    first version
 * 2013-05-24     Grissiom     port to RM48x50
 */
#include <os.h>

#include "armv7.h"
/**
 * @addtogroup RM48x50
 */
/*@{*/

/**
 * This function will initialize task stack
 *
 * @param tentry the entry of task
 * @param parameter the parameter of entry
 * @param stack_addr the beginning stack address
 * @param texit the function will be called when task exit
 *
 * @return stack address
 */
uint8_t *os_arch_stack_init(void *tentry, void *parameter,
    uint8_t *stack_addr, void *texit)
{
    uint32_t *stk;

        stk  = (uint32_t*)stack_addr;
    *(  stk) = (uint32_t) tentry;        /* entry point */
    *(--stk) = (uint32_t) texit;        /* lr */
    *(--stk) = 0;                            /* r12 */
    *(--stk) = 0;                            /* r11 */
    *(--stk) = 0;                            /* r10 */
    *(--stk) = 0;                            /* r9 */
    *(--stk) = 0;                            /* r8 */
    *(--stk) = 0;                            /* r7 */
    *(--stk) = 0;                            /* r6 */
    *(--stk) = 0;                            /* r5 */
    *(--stk) = 0;                            /* r4 */
    *(--stk) = 0;                            /* r3 */
    *(--stk) = 0;                            /* r2 */
    *(--stk) = 0;                            /* r1 */
    *(--stk) = (uint32_t)parameter;    /* r0 : argument */

    /* cpsr */
    if ((uint32_t)tentry & 0x01)
        *(--stk) = SVCMODE | 0x20;            /* thumb mode */
    else
        *(--stk) = SVCMODE;                    /* arm mode   */

#if defined(__TI_VFP_SUPPORT__) || (defined (__VFP_FP__) && !defined(__SOFTFP__))
#ifndef RT_VFP_LAZY_STACKING
    {
        int i;

        for (i = 0; i < VFP_DATA_NR; i++) {
            *(--stk) = 0;
        }
        /* FPSCR TODO: do we need to set the values other than 0? */
        *(--stk) = 0;
        /* FPEXC. Enable the FVP if no lazy stacking. */
        *(--stk) = 0x40000000;
    }
#else
        /* FPEXC. Disable the FVP by default. */
        *(--stk) = 0x00000000;
#endif
#endif

    /* return task's current stack address */
    return (uint8_t *)stk;
}

/*@}*/
