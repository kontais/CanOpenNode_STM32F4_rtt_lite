/*-----------------------------------------------------------------------
* tick.c  -
*
*
*
* Copyright (C) 2016 kontais@aliyun.com
*
*-----------------------------------------------------------------------*/
#include <types.h>
#include <os.h>

bool_t time_out(uint32_t *start, uint32_t interval)
{
    os_tick_t current;

    current = os_tick_get();

    if (current - *start < interval) {
        return FALSE;
    }
    *start = current;
    return TRUE;
}

uint32_t time_elapsed(uint32_t *before)
{
    os_tick_t current, elapsed;

    current = os_tick_get();

    elapsed = current - *before;
    *before = current;

    return elapsed;
}
