/*
 * File      : interrupt.c
 * This file is part of RT-Thread RTOS
 * COPYRIGHT (C) 2010, RT-Thread Development Team
 *
 * The license and distribution terms for this file may be
 * found in the file LICENSE in this distribution or at
 * http://www.rt-thread.org/license/LICENSE
 *
 * Change Logs:
 * Date           Author       Notes
 * 2010-03-30     Kyle         Ported from STM32 to AVR32.
 */

#include <os.h>
#include "compiler.h"

uint32_t interrupt_switch_task_from, interrupt_switch_task_to;
uint32_t interrupt_switch_flag;
