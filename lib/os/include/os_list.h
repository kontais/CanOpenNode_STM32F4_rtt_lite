/*
 * File      : os_list.h
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
 * 2006-03-16     Bernard      the first version
 */

#ifndef __OS_LIST_H__
#define __OS_LIST_H__

#ifdef __cplusplus
extern "C" {
#endif

struct os_list_node
{
    struct os_list_node *next;                          /* point to next node. */
    struct os_list_node *prev;                          /* point to prev node. */
};
typedef struct os_list_node os_list_t;                  /* Type for lists. */

#define OS_LIST_INIT(head) {&(head), &(head)}

/**
 * @brief get the struct for this entry
 * @param node the entry point
 * @param type the type of structure
 * @param member the name of list in structure
 */
#define OS_LIST_ENTRY(node, type, member) \
    ((type *)((char *)(node) - (unsigned int)(&((type *)0)->member)))

void os_list_init(os_list_t *l);
void os_list_insert_after(os_list_t *l, os_list_t *n);
void os_list_insert_before(os_list_t *l, os_list_t *n);
void os_list_remove(os_list_t *n);
int os_list_isempty(const os_list_t *l);

#ifdef __cplusplus
}
#endif

#endif /* __OS_LIST_H__ */
