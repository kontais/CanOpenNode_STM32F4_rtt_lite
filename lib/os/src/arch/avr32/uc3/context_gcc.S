/*
 * File      : context.S
 * This file is part of RT-Thread RTOS
 * COPYRIGHT (C) 2010, RT-Thread Development Team
 *
 * The license and distribution terms for this file may be
 * found in the file LICENSE in this distribution or at
 * http://www.rt-thread.org/license/LICENSE
 *
 * Change Logs:
 * Date           Author       Notes
 * 2010-03-27     Kyle         First version
 */

#define AVR32_SR            0
#define AVR32_SR_GM_OFFSET    16

.text

/*
 * os_sr_t os_enter_critical()
 */
.globl os_enter_critical
.type os_enter_critical, %function
os_enter_critical:
    ssrf    AVR32_SR_GM_OFFSET
    mov        pc, lr

/*
 * void os_exit_critical(os_sr_t sr)
 */
.globl os_exit_critical
.type os_exit_critical, %function
os_exit_critical:
    csrf    AVR32_SR_GM_OFFSET
    mov        pc, lr

/*
 * void os_arch_context_switch(rt_uint32 from, rt_uint32 to)/*
 * r8 --> from
 * r9 --> to
 */
.globl os_arch_context_switch
.type os_arch_context_switch, %function
os_arch_context_switch:
    ssrf    AVR32_SR_GM_OFFSET    /* Disable global interrupt */
    stm        --sp, r8-r12, lr    /* Push R8-R12, LR */
    st.w    --sp, lr            /* Push LR (instead of PC) */
    mfsr    r8, AVR32_SR        /* Read Status Register */
    cbr        r8, AVR32_SR_GM_OFFSET    /* Clear GM bit */
    st.w    --sp, r8            /* Push SR */
    stm        --sp, r0-r7            /* Push R0-R7 */
                                /* Stack layout: R8-R12, LR, PC, SR, R0-R7 */

    st.w    r12[0], sp            /* Store SP in preempted tasks TCB */
    ld.w    sp, r11[0]            /* Get new task stack pointer */

    ldm        sp++, r0-r7            /* pop R0-R7 */
    ld.w    r8, sp++            /* pop SR */
    mtsr    AVR32_SR, r8        /* Restore SR */
    ldm        sp++, r8-r12, lr, pc/* Pop R8-R12, LR, PC and resume to task */

/*
 * void os_arch_context_switch_to(rt_uint32 to)/*
 * r0 --> to
 */
.globl os_arch_context_switch_to
.type os_arch_context_switch_to, %function
os_arch_context_switch_to:
    ld.w    sp, r12[0]            /* Get new task stack pointer */

    ldm        sp++, r0-r7            /* pop R0-R7 */
    ld.w    r8, sp++            /* pop SR */
    mtsr    AVR32_SR, r8        /* Restore SR */
    ldm        sp++, r8-r12, lr, pc/* Pop R8-R12, LR, PC and resume execution */

/*
 * void os_arch_context_switch_interrupt(rt_uint32 from, rt_uint32 to)/*
 */
.globl interrupt_switch_flag
.globl interrupt_switch_task_from
.globl interrupt_switch_task_to
.globl os_arch_context_switch_interrupt
.type os_arch_context_switch_interrupt, %function
os_arch_context_switch_interrupt:
    lda.w    r8, interrupt_switch_flag
    ld.w    r9, r8[0]
    cp.w    r9, 1
    breq    _reswitch
    mov        r9, 1
    st.w    r8[0], r9
    lda.w    r8, interrupt_switch_task_from
    st.w    r8[0], r12
_reswitch:
    lda.w    r8, interrupt_switch_task_to
    st.w    r8[0], r11
    mov        pc, lr
