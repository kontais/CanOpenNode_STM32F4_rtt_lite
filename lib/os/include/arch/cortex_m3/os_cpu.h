/*
 * File      : os_cpu.h
 * This file is part of RT-Thread RTOS
 * COPYRIGHT (C) 2006 - 2012, RT-Thread Development Team
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 * Change Logs:
 * Date           Author       Notes
 * 2006-03-18     Bernard      the first version
 * 2006-04-25     Bernard      add os_arch_context_switch_interrupt declaration
 * 2006-09-24     Bernard      add os_arch_context_switch_to declaration
 * 2012-12-29     Bernard      add os_arch_exception_install declaration
 */

#ifndef __OS_CPU_H__
#define __OS_CPU_H__

#ifdef __cplusplus
extern "C" {
#endif


typedef uint32_t    os_sr_t;    /* CPU status register */

/*
 * Context switch interfaces
 */
os_sr_t os_enter_critical(void);
void os_exit_critical(os_sr_t sr);

uint8_t *os_arch_task_stack_init(void       *entry,
                             void       *parameter,
                             uint8_t *stack_addr,
                             void       *exit);

/*
 * Context switch interfaces
 */
void os_arch_context_switch(uint32_t from, uint32_t to);
void os_arch_context_switch_to(uint32_t to);
void os_arch_context_switch_interrupt(uint32_t from, uint32_t to);

/*
 * Exception interfaces (Optional)
 */
void os_arch_exception_install(os_err_t (*exception_handle)(void *context));

#ifdef __cplusplus
}
#endif

#endif  /* __OS_CPU_H__ */
