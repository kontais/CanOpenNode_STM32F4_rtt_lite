/*
 * File      : cpuport.c
 * This file is part of RT-Thread RTOS
 * COPYRIGHT (C) 2006 - 2014, RT-Thread Development Team
 *
 * The license and distribution terms for this file may be
 * found in the file LICENSE in this distribution or at
 * http://www.rt-thread.org/license/LICENSE
 *
 * Change Logs:
 * Date           Author       Notes
 * 2011-10-21     Bernard      the first version.
 * 2011-10-27     aozima       update for cortex-M4 FPU.
 * 2011-12-31     aozima       fixed stack align issues.
 * 2012-01-01     aozima       support context switch load/store FPU register.
 * 2012-12-11     lgnq         fixed the coding style.
 * 2012-12-23     aozima       stack addr align to 8byte.
 * 2012-12-29     Bernard      Add exception hook.
 * 2013-06-23     aozima       support lazy stack optimized.
 */

#include <os.h>

#define USE_FPU   /* ARMCC */ (  (defined (__CC_ARM) && defined (__TARGET_FPU_VFP)) \
                  /* IAR */   || (defined (__ICCARM__) && defined (__ARMVFP__)) \
                  /* GNU */   || (defined (__GNUC__) && defined (__VFP_FP__) && !defined(__SOFTFP__)) )

/* exception and interrupt handler table */
uint32_t interrupt_switch_task_from;
uint32_t interrupt_switch_task_to;
uint32_t interrupt_switch_flag;
/* exception hook */
static os_err_t (*os_arch_exception_hook)(void *context) = NULL;

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
#if USE_FPU
    uint32_t flag;
#endif /* USE_FPU */

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

struct exception_stack_frame_fpu
{
    uint32_t r0;
    uint32_t r1;
    uint32_t r2;
    uint32_t r3;
    uint32_t r12;
    uint32_t lr;
    uint32_t pc;
    uint32_t psr;

#if USE_FPU
    /* FPU register */
    uint32_t S0;
    uint32_t S1;
    uint32_t S2;
    uint32_t S3;
    uint32_t S4;
    uint32_t S5;
    uint32_t S6;
    uint32_t S7;
    uint32_t S8;
    uint32_t S9;
    uint32_t S10;
    uint32_t S11;
    uint32_t S12;
    uint32_t S13;
    uint32_t S14;
    uint32_t S15;
    uint32_t FPSCR;
    uint32_t NO_NAME;
#endif
};

struct stack_frame_fpu
{
    uint32_t flag;

    /* r4 ~ r11 register */
    uint32_t r4;
    uint32_t r5;
    uint32_t r6;
    uint32_t r7;
    uint32_t r8;
    uint32_t r9;
    uint32_t r10;
    uint32_t r11;

#if USE_FPU
    /* FPU register s16 ~ s31 */
    uint32_t s16;
    uint32_t s17;
    uint32_t s18;
    uint32_t s19;
    uint32_t s20;
    uint32_t s21;
    uint32_t s22;
    uint32_t s23;
    uint32_t s24;
    uint32_t s25;
    uint32_t s26;
    uint32_t s27;
    uint32_t s28;
    uint32_t s29;
    uint32_t s30;
    uint32_t s31;
#endif

    struct exception_stack_frame_fpu exception_stack_frame;
};

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

#if USE_FPU
    stack_frame->flag = 0;
#endif /* USE_FPU */

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

void os_arch_hard_fault_exception(struct exception_stack_frame *exception_stack)
{
    extern long list_task(void);

    if (os_arch_exception_hook != NULL) {
        os_err_t result;

        result = os_arch_exception_hook(exception_stack);
        if (result == OS_OK) return;
    }

    printk("psr: 0x%08x\n", exception_stack->psr);
    printk(" pc: 0x%08x\n", exception_stack->pc);
    printk(" lr: 0x%08x\n", exception_stack->lr);
    printk("r12: 0x%08x\n", exception_stack->r12);
    printk("r03: 0x%08x\n", exception_stack->r3);
    printk("r02: 0x%08x\n", exception_stack->r2);
    printk("r01: 0x%08x\n", exception_stack->r1);
    printk("r00: 0x%08x\n", exception_stack->r0);

    printk("hard fault on task: %s\n", os_task_self()->name);

#ifdef RT_USING_FINSH
    list_task();
#endif

    while (1);
}

/**
 * shutdown CPU
 */
void os_arch_shutdown(void)
{
    printk("shutdown...\n");

    OS_ASSERT(0);
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

#endif
