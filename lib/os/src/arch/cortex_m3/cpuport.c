/*
 * File      : cpuport.c
 * This file is part of RT-Thread RTOS
 * COPYRIGHT (C) 2006 - 2013, RT-Thread Development Team
 *
 * The license and distribution terms for this file may be
 * found in the file LICENSE in this distribution or at
 * http://www.rt-thread.org/license/LICENSE
 *
 * Change Logs:
 * Date         Author      Notes
 * 2009-01-05   Bernard     first version
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
    /* r4 ~ r11 register */
    uint32_t r4;
    uint32_t r5;
    uint32_t r6;
    uint32_t r7;
    uint32_t r8;
    uint32_t r9;
    uint32_t r10;
    uint32_t r11;

    struct exception_stack_frame exception_stack_frame;
};

/* flag in interrupt handling */
uint32_t interrupt_switch_task_from;
uint32_t interrupt_switch_task_to;
uint32_t interrupt_switch_flag;

/* exception hook */
static os_err_t (*os_arch_exception_hook)(void *context) = NULL;

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
uint8_t *os_arch_task_stack_init(void       *tentry,
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

    stack_frame->exception_stack_frame.r0  = (uint32_t)parameter;   /* r0 : argument */
    stack_frame->exception_stack_frame.r1  = 0;                     /* r1 */
    stack_frame->exception_stack_frame.r2  = 0;                     /* r2 */
    stack_frame->exception_stack_frame.r3  = 0;                     /* r3 */
    stack_frame->exception_stack_frame.r12 = 0;                     /* r12 */
    stack_frame->exception_stack_frame.lr  = (uint32_t)texit;       /* lr */
    stack_frame->exception_stack_frame.pc  = (uint32_t)tentry;      /* entry point, pc */
    stack_frame->exception_stack_frame.psr = 0x01000000L;           /* PSR */

    /* return task's current stack address */
    return stk;
}

/**
 * This function set the hook, which is invoked on fault exception handling.
 *
 * @param exception_handle the exception handling hook function.
 */
void os_arch_exception_install(os_err_t (*exception_handle)(void* context))
{
    os_arch_exception_hook = exception_handle;
}

#define SCB_CFSR        (*(volatile const unsigned *)0xE000ED28) /* Configurable Fault Status Register */
#define SCB_HFSR        (*(volatile const unsigned *)0xE000ED2C) /* HardFault Status Register */
#define SCB_MMAR        (*(volatile const unsigned *)0xE000ED34) /* MemManage Fault Address register */
#define SCB_BFAR        (*(volatile const unsigned *)0xE000ED38) /* Bus Fault Address Register */

#define SCB_CFSR_MFSR   (*(volatile const unsigned char*)0xE000ED28)  /* Memory-management Fault Status Register */
#define SCB_CFSR_BFSR   (*(volatile const unsigned char*)0xE000ED29)  /* Bus Fault Status Register */
#define SCB_CFSR_UFSR   (*(volatile const unsigned short*)0xE000ED2A) /* Usage Fault Status Register */

struct exception_info
{
    uint32_t exc_return;
    struct stack_frame stack_frame;
};

/*
 * fault exception handler
 */
void os_arch_hard_fault_exception(struct exception_info * exception_info)
{
    struct stack_frame* context = &exception_info->stack_frame;

    if (os_arch_exception_hook != NULL) {
        os_err_t result;

        result = os_arch_exception_hook(exception_info);
        if (result == OS_OK)
            return;
    }

    printf("psr: 0x%08x\n", context->exception_stack_frame.psr);

    printf("r00: 0x%08x\n", context->exception_stack_frame.r0);
    printf("r01: 0x%08x\n", context->exception_stack_frame.r1);
    printf("r02: 0x%08x\n", context->exception_stack_frame.r2);
    printf("r03: 0x%08x\n", context->exception_stack_frame.r3);
    printf("r04: 0x%08x\n", context->r4);
    printf("r05: 0x%08x\n", context->r5);
    printf("r06: 0x%08x\n", context->r6);
    printf("r07: 0x%08x\n", context->r7);
    printf("r08: 0x%08x\n", context->r8);
    printf("r09: 0x%08x\n", context->r9);
    printf("r10: 0x%08x\n", context->r10);
    printf("r11: 0x%08x\n", context->r11);
    printf("r12: 0x%08x\n", context->exception_stack_frame.r12);
    printf(" lr: 0x%08x\n", context->exception_stack_frame.lr);
    printf(" pc: 0x%08x\n", context->exception_stack_frame.pc);

    if (exception_info->exc_return & (1 << 2) ) {
        printf("hard fault on task: %s\r\n\r\n", os_task_self()->name);

    } else {
        printf("hard fault on handler\r\n\r\n");
    }

    while (1);
}

#ifdef OS_CFG_CPU_FFS
/**
 * This function finds the first bit set (beginning with the least significant bit)
 * in value and return the index of that bit.
 *
 * Bits are numbered starting at 1 (the least significant bit).  A return value of
 * zero from any of these functions means that the argument was zero.
 *
 * @return return the index of the first bit set. If value is 0, then this function
 * shall return 0.
 */
#if defined(__CC_ARM)
__asm int __ffs(int value)
{
    CMP     r0, #0x00
    BEQ     exit
    RBIT    r0, r0
    CLZ     r0, r0
    ADDS    r0, r0, #0x01

exit
    BX      lr
}
#elif defined(__IAR_SYSTEMS_ICC__)
int __ffs(int value)
{
    if (value == 0) return value;

    __ASM("RBIT r0, r0");
    __ASM("CLZ  r0, r0");
    __ASM("ADDS r0, r0, #0x01");
}
#elif defined(__GNUC__)
int __ffs(int value)
{
    return __builtin_ffs(value);
}
#endif

#endif  /* OS_CFG_CPU_FFS */
