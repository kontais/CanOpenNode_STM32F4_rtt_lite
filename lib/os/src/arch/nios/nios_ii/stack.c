/*
 * File      : stack.c
 * This file is part of RT-Thread RTOS
 * COPYRIGHT (C) 2006-2011, RT-Thread Development Team
 *
 * The license and distribution terms for this file may be
 * found in the file LICENSE in this distribution or at
 * http://www.rt-thread.org/license/LICENSE
 *
 * Change Logs:
 * Date           Author       Notes
 * 2011-02-14     aozima       first implementation for Nios II.
 */

#include <os.h>

/**
 * @addtogroup NIOS_II
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

    stk      = (uint32_t *)stack_addr;
    *(stk)   = 0x01;                        /* status  */
    *(--stk) = (uint32_t)texit;        /* ra  */
    *(--stk) = 0xdeadbeef;                  /* fp  */
    *(--stk) = 0xdeadbeef;                  /* r23 */
    *(--stk) = 0xdeadbeef;                  /* r22 */
    *(--stk) = 0xdeadbeef;                  /* r21 */
    *(--stk) = 0xdeadbeef;                  /* r20 */
    *(--stk) = 0xdeadbeef;                  /* r19 */
    *(--stk) = 0xdeadbeef;                  /* r18 */
    *(--stk) = 0xdeadbeef;                  /* r17 */
    *(--stk) = 0xdeadbeef;                  /* r16 */
//    *(--stk) = 0xdeadbeef;                  /* r15 */
//    *(--stk) = 0xdeadbeef;                  /* r14 */
//    *(--stk) = 0xdeadbeef;                  /* r13 */
//    *(--stk) = 0xdeadbeef;                  /* r12 */
//    *(--stk) = 0xdeadbeef;                  /* r11 */
//    *(--stk) = 0xdeadbeef;                  /* r10 */
//    *(--stk) = 0xdeadbeef;                  /* r9  */
//    *(--stk) = 0xdeadbeef;                  /* r8  */
    *(--stk) = 0xdeadbeef;                  /* r7  */
    *(--stk) = 0xdeadbeef;                  /* r6  */
    *(--stk) = 0xdeadbeef;                  /* r5  */
    *(--stk) = (uint32_t)parameter;    /* r4 argument */
    *(--stk) = 0xdeadbeef;                  /* r3  */
    *(--stk) = 0xdeadbeef;                  /* r2  */
    *(--stk) = (uint32_t)tentry;       /* pc  */

//    *(stk)   = (uint32_t)tentry;        /* task entry (ra) */
//    *(--stk) = (uint32_t)parameter;    /* task argument, r4 */

    /* return task's current stack address */
    return (uint8_t *)stk;
}

/*@}*/
