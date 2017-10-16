/*
 * File      : cache.c
 * This file is part of RT-Thread RTOS
 * COPYRIGHT (C) 2010, RT-Thread Development Team
 *
 * The license and distribution terms for this file may be
 * found in the file LICENSE in this distribution or at
 * http://www.rt-thread.org/license/LICENSE
 *
 * Change Logs:
 * Date           Author       Notes
 * 2010-05-17     swkyer       first version
 */
#include <os.h>
#include "mipscfg.h"
#include "cache.h"

extern void cache_init(uint32_t cache_size, uint32_t cache_line_size);
void r4k_cache_init(void)
{
    cache_init(dcache_size, os_arch_dcache_line_size());
}

void r4k_cache_flush_all(void)
{
    blast_dcache16();
    blast_icache16();
}

void r4k_icache_flush_all(void)
{
    blast_icache16();
}

void r4k_icache_flush_range(uint32_t addr, uint32_t size)
{
    uint32_t end, a;

    if (size > icache_size) {
        blast_icache16();
    } else {
        uint32_t ic_lsize = os_arch_icache_line_size();

        a = addr & ~(ic_lsize - 1);
        end = ((addr + size) - 1) & ~(ic_lsize - 1);
        while (1) {
            flush_icache_line(a);
            if (a == end)
                break;
            a += ic_lsize;
        }
    }
}

void r4k_icache_lock_range(uint32_t addr, uint32_t size)
{
    uint32_t end, a;
    uint32_t ic_lsize = os_arch_icache_line_size();

    a = addr & ~(ic_lsize - 1);
    end = ((addr + size) - 1) & ~(ic_lsize - 1);
    while (1) {
        lock_icache_line(a);
        if (a == end)
            break;
        a += ic_lsize;
    }
}

void r4k_dcache_inv(uint32_t addr, uint32_t size)
{
    uint32_t end, a;
    uint32_t dc_lsize = os_arch_dcache_line_size();

    a = addr & ~(dc_lsize - 1);
    end = ((addr + size) - 1) & ~(dc_lsize - 1);
    while (1) {
        invalidate_dcache_line(a);
        if (a == end)
            break;
        a += dc_lsize;
    }
}

void r4k_dcache_wback_inv(uint32_t addr, uint32_t size)
{
    uint32_t end, a;

    if (size >= dcache_size) {
        blast_dcache16();
    } else {
        uint32_t dc_lsize = os_arch_dcache_line_size();

        a = addr & ~(dc_lsize - 1);
        end = ((addr + size) - 1) & ~(dc_lsize - 1);
        while (1) {
            flush_dcache_line(a);
            if (a == end)
                break;
            a += dc_lsize;
        }
    }
}
