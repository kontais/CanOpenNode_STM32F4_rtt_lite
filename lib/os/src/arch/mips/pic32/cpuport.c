/*
 * File      : cpuport.c
 * This file is part of RT-Thread RTOS
 * COPYRIGHT (C) 2009 - 2011, RT-Thread Development Team
 *
 * The license and distribution terms for this file may be
 * found in the file LICENSE in this distribution or at
 * http://www.rt-thread.org/license/LICENSE
 *
 * Change Logs:
 * Date           Author       Notes
 * 20011-05-23    aozima       the first version for PIC32.
 * 20011-09-05    aozima       merge all of C source code into cpuport.c.
 */
#include <os.h>

/**
 * @addtogroup PIC32
 */
/*@{*/

/* exception and interrupt handler table */
uint32_t interrupt_switch_task_from, interrupt_switch_task_to;
uint32_t interrupt_switch_flag;

uint32_t __attribute__((nomips16)) _get_gp(void)
{
    uint32_t result;

    // get the gp reg
    asm volatile("move   %0, $28" : "=r"(result));

    return result;
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
uint8_t *os_arch_stack_init(void *tentry, void *parameter, uint8_t *stack_addr, void *texit)
{
    uint32_t *stk;

    /** Start at stack top */
    stk = (uint32_t *)stack_addr;
    *(stk)   = (uint32_t) tentry;        /* pc: Entry Point */
    *(--stk) = (uint32_t) 0x00800000;    /* c0_cause: IV=1, */
    *(--stk) = (uint32_t) 0;                /* c0_badvaddr */
    *(--stk) = (uint32_t) 0;             /* lo */
    *(--stk) = (uint32_t) 0;                /* hi */
    *(--stk) = (uint32_t) 1;             /* C0_SR: IE = En, */
    *(--stk) = (uint32_t) texit;            /* 31 ra */
    *(--stk) = (uint32_t) 0x0000001e;    /* 30 s8 */
    *(--stk) = (uint32_t) stack_addr;    /* 29 sp */
    *(--stk) = (uint32_t) _get_gp();     /* 28 gp */
    *(--stk) = (uint32_t) 0x0000001b;    /* 27 k1 */
    *(--stk) = (uint32_t) 0x0000001a;    /* 26 k0 */
    *(--stk) = (uint32_t) 0x00000019;    /* 25 t9 */
    *(--stk) = (uint32_t) 0x00000018;    /* 24 t8 */
    *(--stk) = (uint32_t) 0x00000017;    /* 23 s7 */
    *(--stk) = (uint32_t) 0x00000016;    /* 22 s6 */
    *(--stk) = (uint32_t) 0x00000015;    /* 21 s5 */
    *(--stk) = (uint32_t) 0x00000014;    /* 20 s4 */
    *(--stk) = (uint32_t) 0x00000013;    /* 19 s3 */
    *(--stk) = (uint32_t) 0x00000012;    /* 18 s2 */
    *(--stk) = (uint32_t) 0x00000011;    /* 17 s1 */
    *(--stk) = (uint32_t) 0x00000010;    /* 16 s0 */
    *(--stk) = (uint32_t) 0x0000000f;    /* 15 t7 */
    *(--stk) = (uint32_t) 0x0000000e;    /* 14 t6 */
    *(--stk) = (uint32_t) 0x0000000d;    /* 13 t5 */
    *(--stk) = (uint32_t) 0x0000000c;    /* 12 t4 */
    *(--stk) = (uint32_t) 0x0000000b;    /* 11 t3 */
    *(--stk) = (uint32_t) 0x0000000a;     /* 10 t2 */
    *(--stk) = (uint32_t) 0x00000009;    /* 9 t1 */
    *(--stk) = (uint32_t) 0x00000008;    /* 8 t0 */
    *(--stk) = (uint32_t) 0x00000007;    /* 7 a3 */
    *(--stk) = (uint32_t) 0x00000006;    /* 6 a2 */
    *(--stk) = (uint32_t) 0x00000005;    /* 5 a1 */
    *(--stk) = (uint32_t) parameter;        /* 4 a0 */
    *(--stk) = (uint32_t) 0x00000003;    /* 3 v1 */
    *(--stk) = (uint32_t) 0x00000002;    /* 2 v0 */
    *(--stk) = (uint32_t) 0x00000001;    /* 1 at */
    *(--stk) = (uint32_t) 0x00000000;    /* 0 zero */

    /* return task's current stack address */
    return (uint8_t *)stk;
}

/*@}*/
