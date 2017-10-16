/*
 * File      : os_debug.h
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
 */

#ifndef __OS_DEBUG_H__
#define __OS_DEBUG_H__

/* Using this macro to control all kernel debug features. */
#ifdef OS_CFG_DEBUG

/* Turn on some of these (set to non-zero) to debug kernel */
#ifndef OS_DEBUG_HEAP
#define OS_DEBUG_HEAP                   0
#endif

#ifndef OS_DEBUG_SCHEDULER
#define OS_DEBUG_SCHEDULER             0
#endif

#ifndef OS_DEBUG_TASK
#define OS_DEBUG_TASK                0
#endif

#ifndef OS_DEBUG_TIMER
#define OS_DEBUG_TIMER                 0
#endif

#ifndef OS_DEBUG_IRQ
#define OS_DEBUG_IRQ                   0
#endif

#ifndef OS_DEBUG_IPC
#define OS_DEBUG_IPC                   0
#endif

#ifndef OS_DEBUG_CONTEXT_CHECK
#define OS_DEBUG_CONTEXT_CHECK         1
#endif

#define OS_ASSERT(EX)                                                         \
if (!(EX))                                                                    \
{                                                                             \
    os_assert(#EX, __FUNCTION__, __LINE__);                                   \
}

#define OS_DEBUG_LOG(type, message)                                           \
do                                                                            \
{                                                                             \
    if (type)                                                                 \
        printf message;                                                   \
}                                                                             \
while (0)

/* Macro to check current context */
#if OS_DEBUG_CONTEXT_CHECK
#define OS_DEBUG_NOT_IN_INTERRUPT                                             \
do                                                                            \
{                                                                             \
    os_sr_t sr;                                                               \
    sr = os_enter_critical();                                                 \
    if (os_isr_nest_get() != 0)                                         \
    {                                                                         \
        printf("Function[%s] shall not used in ISR\n", __FUNCTION__);     \
        OS_ASSERT(0)                                                          \
    }                                                                         \
    os_exit_critical(sr);                                                     \
}                                                                             \
while (0)

/* "In task context" means:
 *     1) the scheduler has been started
 *     2) not in interrupt context.
 */
#define OS_DEBUG_IN_TASK_CONTEXT                                            \
do                                                                            \
{                                                                             \
    os_sr_t sr;                                                               \
    sr = os_enter_critical();                                                 \
    if (os_task_self() == NULL)                                          \
    {                                                                         \
        printf("Function[%s] shall not be used before scheduler start\n", \
                   __FUNCTION__);                                             \
        OS_ASSERT(0)                                                          \
    }                                                                         \
    OS_DEBUG_NOT_IN_INTERRUPT;                                                \
    os_exit_critical(sr);                                                     \
}                                                                             \
while (0)
#else
#define OS_DEBUG_NOT_IN_INTERRUPT
#define OS_DEBUG_IN_TASK_CONTEXT
#endif

#else /* OS_CFG_DEBUG */

#define OS_ASSERT(EX)
#define OS_DEBUG_LOG(type, message)
#define OS_DEBUG_NOT_IN_INTERRUPT
#define OS_DEBUG_IN_TASK_CONTEXT

#endif /* OS_CFG_DEBUG */

#endif /* __OS_DEBUG_H__ */
