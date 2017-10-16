/*-----------------------------------------------------------------------
* tick.h -
*
*
*
* Copyright (C) 2016 kontais@aliyun.com
*
*-----------------------------------------------------------------------*/
#ifndef _TICK_H_
#define _TICK_H_

#include <types.h>

bool_t time_out(uint32_t *start, uint32_t interval);
uint32_t time_elapsed(uint32_t *before);

#endif    /* _TICK_H_ */

