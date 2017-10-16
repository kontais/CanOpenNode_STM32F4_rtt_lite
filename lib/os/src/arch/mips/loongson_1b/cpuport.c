/*
 * File      : cpuport.c
 * This file is part of RT-Thread RTOS
 * COPYRIGHT (C) 2006 - 2011, RT-Thread Development Team
 *
 * The license and distribution terms for this file may be
 * found in the file LICENSE in this distribution or at
 * http://www.rt-thread.org/license/LICENSE
 *
 * Change Logs:
 * Date           Author       Notes
 * 2010-07-09     Bernard      first version
 * 2010-09-11     Bernard      add CPU reset implementation
 */

#include <os.h>
#include "ls1b.h"

/**
 * @addtogroup Loongson LS1B
 */

/*@{*/

/**
 * this function will reset CPU
 *
 */
void os_arch_reset(void)
{
    /* open the watch-dog */
    WDT_EN = 0x01;         /* watch dog enable */
    WDT_TIMER = 0x01;    /* watch dog will be timeout after 1 tick */
    WDT_SET = 0x01;        /* watch dog start */

    printk("reboot system...\n");
    while (1);
}

/**
 * this function will shutdown CPU
 *
 */
void os_arch_shutdown(void)
{
    printk("shutdown...\n");

    while (1);
}

extern uint32_t cp0_get_cause(void);
extern uint32_t cp0_get_status(void);
extern uint32_t cp0_get_hi(void);
extern uint32_t cp0_get_lo(void);

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
    static uint32_t g_sr = 0;

    if (g_sr == 0) {
        g_sr = cp0_get_status();
        g_sr &= 0xfffffffe;
        g_sr |= 0x8401;
    }

    /** Start at stack top */
    stk = (uint32_t *)stack_addr;
    *(stk)   = (uint32_t) tentry;        /* pc: Entry Point */
    *(--stk) = (uint32_t) 0xeeee;         /* c0_cause */
    *(--stk) = (uint32_t) 0xffff;        /* c0_badvaddr */
    *(--stk) = (uint32_t) cp0_get_lo();    /* lo */
    *(--stk) = (uint32_t) cp0_get_hi();    /* hi */
    *(--stk) = (uint32_t) g_sr;             /* C0_SR: HW2 = En, IE = En */
    *(--stk) = (uint32_t) texit;            /* ra */
    *(--stk) = (uint32_t) 0x0000001e;    /* s8 */
    *(--stk) = (uint32_t) stack_addr;    /* sp */
    *(--stk) = (uint32_t) 0x0000001c;    /* gp */
    *(--stk) = (uint32_t) 0x0000001b;    /* k1 */
    *(--stk) = (uint32_t) 0x0000001a;    /* k0 */
    *(--stk) = (uint32_t) 0x00000019;    /* t9 */
    *(--stk) = (uint32_t) 0x00000018;    /* t8 */
    *(--stk) = (uint32_t) 0x00000017;    /* s7 */
    *(--stk) = (uint32_t) 0x00000016;    /* s6 */
    *(--stk) = (uint32_t) 0x00000015;    /* s5 */
    *(--stk) = (uint32_t) 0x00000014;    /* s4 */
    *(--stk) = (uint32_t) 0x00000013;    /* s3 */
    *(--stk) = (uint32_t) 0x00000012;    /* s2 */
    *(--stk) = (uint32_t) 0x00000011;    /* s1 */
    *(--stk) = (uint32_t) 0x00000010;    /* s0 */
    *(--stk) = (uint32_t) 0x0000000f;    /* t7 */
    *(--stk) = (uint32_t) 0x0000000e;    /* t6 */
    *(--stk) = (uint32_t) 0x0000000d;    /* t5 */
    *(--stk) = (uint32_t) 0x0000000c;    /* t4 */
    *(--stk) = (uint32_t) 0x0000000b;    /* t3 */
    *(--stk) = (uint32_t) 0x0000000a;     /* t2 */
    *(--stk) = (uint32_t) 0x00000009;    /* t1 */
    *(--stk) = (uint32_t) 0x00000008;    /* t0 */
    *(--stk) = (uint32_t) 0x00000007;    /* a3 */
    *(--stk) = (uint32_t) 0x00000006;    /* a2 */
    *(--stk) = (uint32_t) 0x00000005;    /* a1 */
    *(--stk) = (uint32_t) parameter;        /* a0 */
    *(--stk) = (uint32_t) 0x00000003;    /* v1 */
    *(--stk) = (uint32_t) 0x00000002;    /* v0 */
    *(--stk) = (uint32_t) 0x00000001;    /* at */
    *(--stk) = (uint32_t) 0x00000000;    /* zero */

    /* return task's current stack address */
    return (uint8_t *)stk;
}

/*@}*/

