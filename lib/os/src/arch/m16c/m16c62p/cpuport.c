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
 * 2012-09-25     lgnq         save texit address in to task stack
 */

#include <os.h>

extern volatile uint8_t os_isr_nest;

/* switch flag on interrupt and task pointer to save switch record */
uint32_t interrupt_switch_task_from;
uint32_t interrupt_switch_task_to;
uint8_t interrupt_switch_flag;

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
    uint16_t *pstk16;
    uint16_t  flag;

    flag      = 0x0040;
    pstk16    = (uint16_t *)stack_addr;

    *pstk16-- = (uint32_t)texit >> 16L;
    *pstk16-- = (uint32_t)texit & 0x0000FFFFL;

    /* Simulate ISR entry */
    *pstk16-- = (flag&0x00FF) |                            /* The lowest byte of the FLAG register    */
                (((uint32_t)tentry>>8)&0x00000F00) |    /* The highest nibble of the PC register   */
                ((flag<<4)&0xF000);                        /* The highest nibble of the FLAG register */
    *pstk16-- = (((uint32_t)tentry)&0x0000FFFF);        /* The lowest bytes of the PC register     */

    /* Save registers onto stack frame */
    *pstk16-- = (uint16_t)0xFBFB;                       /* FB register                             */
    *pstk16-- = (uint16_t)0x3B3B;                       /* SB register                             */
    *pstk16-- = (uint16_t)0xA1A1;                       /* A1 register                             */
    *pstk16-- = (uint16_t)0xA0A0;                       /* A0 register                             */
    *pstk16-- = (uint16_t)0x3333;                       /* R3 register                             */
    *pstk16-- = (uint32_t)parameter >> 16L;             /* Pass argument in R2 register            */
    *pstk16-- = (uint32_t)parameter & 0x0000FFFFL;      /* Pass argument in R1 register            */
    *pstk16   = (uint16_t)0x0000;                       /* R0 register                             */

    /* return task's current stack address */
    return (uint8_t *)pstk16;
}

void os_arch_context_switch(uint32_t from, uint32_t to)
{
    interrupt_switch_task_from = from;
    interrupt_switch_task_to   = to;
    asm("INT #0");
}

void os_arch_context_switch_interrupt(uint32_t from, uint32_t to)
{
    if (interrupt_switch_flag != 1) {
        interrupt_switch_flag = 1;
        interrupt_switch_task_from        = from;
    }
    interrupt_switch_task_to = to;
}

#if defined(__GNUC__)
os_sr_t os_enter_critical(void)
{
    uint16_t temp;

    asm("STC  FLG, %0":"=r" (temp));
    asm("FCLR I");

    return (int32_t)temp;
}

void os_exit_critical(os_sr_t sr)
{
    uint16_t temp;

    temp = level & 0xffff;
    asm("LDC %0, FLG": :"r" (temp));
}
#endif