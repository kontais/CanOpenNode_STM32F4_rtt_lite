/*
 * COPYRIGHT (C) 2013-2014, Shanghai Real-Thread Technology Co., Ltd
 *
 *  All rights reserved.
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

#ifndef __GIC_H__
#define __GIC_H__

int arm_gic_dist_init(uint32_t index, uint32_t dist_base, int irq_start);
int arm_gic_os_arch_init(uint32_t index, uint32_t os_arch_base);

void arm_gic_mask(uint32_t index, int irq);
void arm_gic_umask(uint32_t index, int irq);
void arm_gic_set_cpu(uint32_t index, int irq, unsigned int cpumask);
void arm_gic_set_group(uint32_t index, int vector, int group);

int arm_gic_get_active_irq(uint32_t index);
void arm_gic_ack(uint32_t index, int irq);

void arm_gic_trigger(uint32_t index, int target_cpu, int irq);
void arm_gic_clear_sgi(uint32_t index, int target_cpu, int irq);

void arm_gic_dump_type(uint32_t index);

#endif

