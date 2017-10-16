/*
 * File      : cpuport.c
 * This file is part of RT-Thread RTOS
 * COPYRIGHT (C) 2009 - 2012, RT-Thread Development Team
 *
 * The license and distribution terms for this file may be
 * found in the file LICENSE in this distribution or at
 * http://www.rt-thread.org/license/LICENSE
 *
 * Change Logs:
 * Date           Author       Notes
 * 2011-02-23     Bernard      the first version
 * 2012-09-23     lgnq         set the texit to R31
 */

#include <os.h>

extern volatile uint8_t os_isr_nest;

/* switch flag on interrupt and task pointer to save switch record */
uint32_t interrupt_switch_task_from;
uint32_t interrupt_switch_task_to;
uint32_t interrupt_switch_flag;

/**
 * This function will initialize hardware interrupt
 */
void os_arch_interrupt_init(void)
{
    /* init interrupt nest, and context in task sp */
    os_isr_nest               = 0;
    interrupt_switch_task_from        = 0;
    interrupt_switch_task_to          = 0;
    interrupt_switch_flag = 0;
}

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
uint8_t *os_arch_stack_init(void       *tentry,
                             void       *parameter,
                             uint8_t *stack_addr,
                             void       *texit)
{
    uint32_t *stk;

    stk      = (uint32_t *)stack_addr;    /* Load stack pointer */

    *(--stk) = (uint32_t)0x23232323;      /* r23 */
    *(--stk) = (uint32_t)0x24242424;      /* r24 */
    *(--stk) = (uint32_t)0x25252525;      /* r25 */
    *(--stk) = (uint32_t)0x26262626;      /* r26 */
    *(--stk) = (uint32_t)0x27272727;      /* r27 */
    *(--stk) = (uint32_t)0x28282828;      /* r28 */
    *(--stk) = (uint32_t)0x29292929;      /* r29 */
    *(--stk) = (uint32_t)0x30303030;      /* r30 */
    *(--stk) = (uint32_t)texit;           /* r31 */
    *(--stk) = (uint32_t)0x00000000;      /* Task PSW = Interrupts enabled */
    *(--stk) = (uint32_t)tentry;          /* Task's PC */
    *(--stk) = (uint32_t)0x16161616;      /* r16 */
    *(--stk) = (uint32_t)0x15151515;      /* r15 */
    *(--stk) = (uint32_t)0x14141414;      /* r14 */
    *(--stk) = (uint32_t)0x13131313;      /* r13 */
    *(--stk) = (uint32_t)0x12121212;      /* r12 */
    *(--stk) = (uint32_t)0x11111111;      /* r11 */
    *(--stk) = (uint32_t)0x10101010;      /* r10 */
    *(--stk) = (uint32_t)0x09090909;      /* r9 */
    *(--stk) = (uint32_t)0x08080808;      /* r8 */
    *(--stk) = (uint32_t)0x07070707;      /* r7 */
    *(--stk) = (uint32_t)0x06060606;      /* r6 */
    *(--stk) = (uint32_t)0x05050505;      /* r5 */
    *(--stk) = (uint32_t)0x02020202;      /* r2 */
    *(--stk) = (uint32_t)parameter;       /* r1 */

    return ((uint8_t *)stk);
}

void os_arch_context_switch(uint32_t from, uint32_t to)
{
    interrupt_switch_task_from = from;
    interrupt_switch_task_to = to;
    asm("trap 0x10");
}

void os_arch_context_switch_interrupt(uint32_t from, uint32_t to)
{
    if (interrupt_switch_flag != 1) {
        interrupt_switch_flag = 1;
        interrupt_switch_task_from = from;
    }
    interrupt_switch_task_to = to;
}
