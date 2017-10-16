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
 * 2011-02-23     Bernard      the first version
 * 2012-03-03     xuzhenglim   modify for rx62N
 */
#include <rtcpu.h>
#include <os.h>

#include "cpuconfig.h"

#include "machine.h"
#include "iorx62n.h"

#define ENTER_INTERRUPT()  ICU.SWINTR.BIT.SWINT = 1;

extern volatile uint8_t os_isr_nest;

/* switch flag on interrupt and task pointer to save switch record */
uint32_t interrupt_switch_task_from;
uint32_t interrupt_switch_task_to;
uint32_t interrupt_switch_flag;

/* stack frame*/
struct stack_frame
{
    uint32_t ACCLO;
    uint32_t ACCHI;
    uint32_t FPSW;
    uint32_t R1;
    uint32_t R2;
    uint32_t R3;
    uint32_t R4;
    uint32_t R5;
    uint32_t R6;
    uint32_t R7;
    uint32_t R8;
    uint32_t R9;
    uint32_t R10;
    uint32_t R11;
    uint32_t R12;
    uint32_t R13;
    uint32_t R14;
    uint32_t R15;
    //there is not R0 register,it is special for stack pointer
    uint32_t PC;
    uint32_t PSW;
};

/**
 * Initilial the threah stack.
 *
 * @author LXZ (2014/11/8)
 *
 * @param void* tentry
 * @param void* parameter
 * @param uint8_t* stack_addr
 * @param void* texit
 *
 * @return uint8_t*
 */
uint8_t *os_arch_stack_init(void *tentry, void *parameter,
                             uint8_t *stack_addr, void *texit)
{
    uint32_t *stk;
    struct stack_frame *stack_frame;
    uint32_t       i;

    stk      = (uint32_t *)stack_addr;
    *(stk)   = (uint32_t)texit;
    stack_frame = (struct stack_frame *)(stack_addr - sizeof(struct stack_frame)) ;

    //Initilial all register
    for (i = 0; i < sizeof(struct stack_frame) / sizeof(uint32_t); i++) {
        ((uint32_t *)stack_frame)[i] = 0xdeadbeef;
    }

    stack_frame->PSW = (uint32_t)0x00030000 ;   /* psw */
    stack_frame->PC = (uint32_t)tentry;        /* task entery*/
    stack_frame->R1 = (uint32_t)parameter;   /* r1 : parameter */
    stack_frame->FPSW = 0x00000100;                  /* fpsw */

    return(uint8_t *)stack_frame;
}

#ifdef RT_USING_FINSH
extern void list_task(void);
#endif
extern os_task_t os_current_task;
/**
 * deal exception
 *
 * @author LXZ (2014/11/8)
 *
 * @param struct stack_frame* exception_contex
 */
void os_arch_hard_fault_exception(struct stack_frame* exception_contex)
{
    if (exception_contex != NULL) {
        printk("psw: 0x%08x\n", exception_contex->PSW);
        printk("pc: 0x%08x\n", exception_contex->PC);
        printk("r0: 0x%08x\n", exception_contex->R1);
        printk("r0: 0x%08x\n", exception_contex->R2);
        printk("r0: 0x%08x\n", exception_contex->R3);
        printk("r0: 0x%08x\n", exception_contex->R4);
        printk("r0: 0x%08x\n", exception_contex->R5);
        printk("r0: 0x%08x\n", exception_contex->R6);
        printk("r0: 0x%08x\n", exception_contex->R7);
        printk("r0: 0x%08x\n", exception_contex->R8);
        printk("r0: 0x%08x\n", exception_contex->R9);
        printk("r0: 0x%08x\n", exception_contex->R10);
        printk("r0: 0x%08x\n", exception_contex->R11);
        printk("r0: 0x%08x\n", exception_contex->R12);
        printk("r0: 0x%08x\n", exception_contex->R13);
        printk("r0: 0x%08x\n", exception_contex->R14);
        printk("r0: 0x%08x\n", exception_contex->R15);
        printk("fpsw: 0x%08x\n", exception_contex->FPSW);
        printk("acchi: 0x%08x\n", exception_contex->ACCHI);
        printk("acclo: 0x%08x\n", exception_contex->ACCLO);
    }
        printk("hard fault on task: %s\n", os_current_task->name);
    #ifdef RT_USING_FINSH
        list_task();
    #endif
        while (1);

}

/**
 * switch task in interrupt
 *
 * @author LXZ (2014/11/8)
 *
 * @param uint32_t from
 * @param uint32_t to
 */
void os_arch_context_switch(uint32_t from, uint32_t to)
{
    if (interrupt_switch_flag == 0) {
        interrupt_switch_flag = 1;
        interrupt_switch_task_from = from;
    }

    interrupt_switch_task_to = to;
    ENTER_INTERRUPT();
}
/**
 * swithc task out the interrupt
 *
 * @author LXZ (2014/11/8)
 *
 * @param uint32_t from
 * @param uint32_t to
 */
void os_arch_context_switch_interrupt(uint32_t from, uint32_t to)
{
    if (interrupt_switch_flag == 0) {
        interrupt_switch_flag = 1;
        interrupt_switch_task_from = from;
    }

    interrupt_switch_task_to = to;
    ENTER_INTERRUPT();
}

/**
 * shut down the chip
 *
 * @author LXZ (2014/11/8)
 */
void os_arch_shutdown(void)
{
    printk("shutdown...\n");

    OS_ASSERT(0);
}
/**
 * switch to the first task,it just call one time
 *
 * @author LXZ (2014/11/8)
 *
 * @param uint32_t to
 */
void os_arch_context_switch_to(uint32_t to)
{

    interrupt_switch_task_from = 0;
    interrupt_switch_task_to = to;
    interrupt_switch_flag = 1;
    /* enable interrupt */
    _IEN(_ICU_SWINT) = 1;

    /*clear the interrupt flag*/
    _IR(_ICU_SWINT) = 0;
    _IPR(_ICU_SWINT) = MAX_SYSCALL_INTERRUPT_PRIORITY + 1;

    /*touch the software interrupt*/
    ENTER_INTERRUPT();
    /*wait for first task start up*/
    while (1);
}

