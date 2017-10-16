/*
 * File      : cpuport.c
 * This file is part of RT-Thread RTOS
 * COPYRIGHT (C) 2012, RT-Thread Development Team
 *
 * The license and distribution terms for this file may be
 * found in the file LICENSE in this distribution or at
 * http://www.rt-thread.org/license/LICENSE
 *
 * Change Logs:
 * Date            Author        Notes
 * 2012-02-13     mojingxian     first version
 */

#include <os.h>

/* flag in interrupt handling */
uint32_t interrupt_switch_task_from;
uint32_t interrupt_switch_task_to;
uint32_t interrupt_switch_flag;

/**
 * initializes stack of task
 */
uint8_t *os_arch_stack_init(void *tentry, void *parameter,
    uint8_t *stack_addr, void *texit)
{
    unsigned char i;
    uint32_t *stk;

    stk    = (uint32_t *)stack_addr;     /* Load stack pointer                                    */

                                              /* Simulate a function call to the task with an argument */
    stk   -= 3;                               /* 3 words assigned for incoming args (R0, R1, R2)       */

                                              /* Now simulating vectoring to an ISR                    */
    *--stk = (uint32_t)parameter;        /* R0 value - caller's incoming argument #1              */
    *--stk = (uint32_t)0;                /* P1 value - value irrelevant                           */

    *--stk = (uint32_t)texit;            /* RETS value - NO task should return with RTS.          */
                                              /* however OS_CPU_Invalid_Task_Return is a safety        */
                                              /* catch-allfor tasks that return with an RTS            */

    *--stk = (uint32_t)parameter;        /* R1 value - caller's incoming argument #2              */
                                              /* (not relevant in current test example)                */
    *--stk = (uint32_t)parameter;        /* R2 value - caller's incoming argument #3              */
                                              /* (not relevant in current test example)                */
    *--stk = (uint32_t)0;                /* P0 value - value irrelevant                           */
    *--stk = (uint32_t)0;                /* P2 value - value irrelevant                           */
    *--stk = (uint32_t)0;                /* ASTAT value - caller's ASTAT value - value            */
                                              /* irrelevant                                            */

    *--stk = (uint32_t)tentry;           /* RETI value- pushing the start address of the task     */

    for (i = 0; i < 35; i++)                  /* remaining reg values - R7:3, P5:3,                    */
    {                                         /* 4 words of A1:0(.W,.X), LT0, LT1,                     */
        *--stk = (uint32_t)0;            /* LC0, LC1, LB0, LB1,I3:0, M3:0, L3:0, B3:0,            */
    }                                         /* All values irrelevant                                 */

    return (uint8_t *)stk;                 /* Return top-of-stack                                   */
}

void os_arch_context_switch(uint32_t from, uint32_t to)
{
    if (interrupt_switch_flag != 1) {
        interrupt_switch_flag = 1;
        interrupt_switch_task_from = from;
    }

    interrupt_switch_task_to = to;
    asm("raise 14;");                     // Raise Interrupt 14 (trap)
}

void os_arch_context_switch_interrupt(uint32_t from, uint32_t to)
{
    if (interrupt_switch_flag != 1) {
        interrupt_switch_flag = 1;
        interrupt_switch_task_from = from;
    }

    interrupt_switch_task_to = to;
    asm("raise 14;");                     // Raise Interrupt 14 (trap)
}

void os_arch_context_switch_to(uint32_t to)
{
    interrupt_switch_flag = 1;
    interrupt_switch_task_from = 0;
    interrupt_switch_task_to = to;
    asm("raise 14;");                     // Raise Interrupt 14 (trap)
}
