/*
 * File      : context.asm
 * This file is part of RT-Thread RTOS
 * COPYRIGHT (C) 2009, RT-Thread Development Team
 *
 * The license and distribution terms for this file may be
 * found in the file LICENSE in this distribution or at
 * http://www.rt-thread.org/license/LICENSE
 *
 * Change Logs:
 * Date           Author       Notes
 * 2010-04-09     fify         the first version
 * 2010-04-19     fify         rewrite os_enter_critical/enable fuction
 * 2010-04-20     fify         move peripheral ISR to bsp/interrupts.s34
 *
 * For       : Renesas M16C
 * Toolchain : IAR's EW for M16C v3.401
 */

    RSEG    CSTACK

    RSEG    ISTACK

    RSEG    CODE(1)

    EXTERN  interrupt_switch_task_from
    EXTERN  interrupt_switch_task_to

    PUBLIC  os_enter_critical
    PUBLIC  os_exit_critical
    PUBLIC  os_arch_context_switch_to
    PUBLIC  os_context_switch

os_enter_critical:
    STC     FLG, R0    ;fify 20100419
    FCLR    I
    RTS

os_exit_critical:
    LDC     R0, FLG    ;fify 20100419
    RTS

    .EVEN
os_context_switch:
    PUSHM   R0,R1,R2,R3,A0,A1,SB,FB

    MOV.W   interrupt_switch_task_from, A0
    STC     ISP, [A0]
    MOV.W   interrupt_switch_task_to, A0
    LDC     [A0], ISP

    POPM    R0,R1,R2,R3,A0,A1,SB,FB             ; Restore registers from the new task's stack
    REIT                                        ; Return from interrup

/*
 * void os_arch_context_switch_to(rt_uint32 to);
 * r0 --> to
 * this fucntion is used to perform the first task switch
 */
os_arch_context_switch_to:
    MOV.W   R0, A0
    LDC     [A0], ISP
    POPM    R0,R1,R2,R3,A0,A1,SB,FB
    REIT

    END
