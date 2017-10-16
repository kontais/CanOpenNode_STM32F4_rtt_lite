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
 */

#include <os.h>

#include <i386.h>

/**
 * @addtogroup I386
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

    stk = (uint32_t *)stack_addr;
    *(--stk) = (uint32_t)parameter;
    *(--stk) = (uint32_t)texit;
    *(--stk) = 0x200;                        /*flags*/
    *(--stk) = 0x08;                        /*cs*/
    *(--stk) = (uint32_t)tentry;        /*eip*/
    *(--stk) = 0;                            /*irqno*/
    *(--stk) = 0x10;                        /*ds*/
    *(--stk) = 0x10;                        /*es*/
    *(--stk) = 0;                            /*eax*/
    *(--stk) = 0;                            /*ecx*/
    *(--stk) = 0;                            /*edx*/
    *(--stk) = 0;                            /*ebx*/
    *(--stk) = 0;                            /*esp*/
    *(--stk) = 0;                            /*ebp*/
    *(--stk) = 0;                            /*esi*/
    *(--stk) = 0;                            /*edi*/

    /* return task's current stack address */
    return (uint8_t *)stk;
}
/*@}*/
