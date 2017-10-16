/*
 * File      : cpuport.c
 * This file is part of RT-Thread RTOS
 * COPYRIGHT (C) 2006 - 2012, RT-Thread Development Team
 *
 * The license and distribution terms for this file may be
 * found in the file LICENSE in this distribution or at
 * http://www.rt-thread.org/license/LICENSE
 *
 * Change Logs:
 * Date           Author       Notes
 * 2010-01-25     Bernard      first version
 * 2012-05-31     aozima       Merge all of the C source code into cpuport.c
 * 2012-08-17     aozima       fixed bug: store r8 - r11.
 * 2012-12-23     aozima       stack addr align to 8byte.
 */

#include <os.h>

struct exception_stack_frame
{
    uint32_t r0;
    uint32_t r1;
    uint32_t r2;
    uint32_t r3;
    uint32_t r12;
    uint32_t lr;
    uint32_t pc;
    uint32_t psr;
};

struct stack_frame
{
    /* r4 ~ r7 low register */
    uint32_t r4;
    uint32_t r5;
    uint32_t r6;
    uint32_t r7;

    /* r8 ~ r11 high register */
    uint32_t r8;
    uint32_t r9;
    uint32_t r10;
    uint32_t r11;

    struct exception_stack_frame exception_stack_frame;
};

/* flag in interrupt handling */
uint32_t interrupt_switch_task_from, interrupt_switch_task_to;
uint32_t interrupt_switch_flag;

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
    struct stack_frame *stack_frame;
    uint8_t         *stk;
    uint32_t       i;

    stk  = stack_addr + sizeof(uint32_t);
    stk  = (uint8_t *)OS_ALIGN_DOWN((uint32_t)stk, 8);
    stk -= sizeof(struct stack_frame);

    stack_frame = (struct stack_frame *)stk;

    /* init all register */
    for (i = 0; i < sizeof(struct stack_frame) / sizeof(uint32_t); i++) {
        ((uint32_t *)stack_frame)[i] = 0xdeadbeef;
    }

    stack_frame->exception_stack_frame.r0  = (uint32_t)parameter; /* r0 : argument */
    stack_frame->exception_stack_frame.r1  = 0;                        /* r1 */
    stack_frame->exception_stack_frame.r2  = 0;                        /* r2 */
    stack_frame->exception_stack_frame.r3  = 0;                        /* r3 */
    stack_frame->exception_stack_frame.r12 = 0;                        /* r12 */
    stack_frame->exception_stack_frame.lr  = (uint32_t)texit;     /* lr */
    stack_frame->exception_stack_frame.pc  = (uint32_t)tentry;    /* entry point, pc */
    stack_frame->exception_stack_frame.psr = 0x01000000L;              /* PSR */

    /* return task's current stack address */
    return stk;
}

extern long list_task(void);
extern os_task_t os_current_task;
/**
 * fault exception handling
 */
void os_arch_hard_fault_exception(struct exception_stack_frame *contex)
{
    printk("psr: 0x%08x\n", contex->psr);
    printk(" pc: 0x%08x\n", contex->pc);
    printk(" lr: 0x%08x\n", contex->lr);
    printk("r12: 0x%08x\n", contex->r12);
    printk("r03: 0x%08x\n", contex->r3);
    printk("r02: 0x%08x\n", contex->r2);
    printk("r01: 0x%08x\n", contex->r1);
    printk("r00: 0x%08x\n", contex->r0);

    printk("hard fault on task: %s\n", os_current_task->name);

#ifdef RT_USING_FINSH
    list_task();
#endif

    while (1);
}
