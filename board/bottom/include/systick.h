/*-----------------------------------------------------------------------
* systick.h  -
*
*
*
* Copyright (C) 2017 kontais@aliyun.com
*
*-----------------------------------------------------------------------*/
#ifndef _SYSTICK_H_
#define _SYSTICK_H_

void systick_init(void);
void systick_stall_us(uint32_t us);
void systick_stall_ns(uint32_t ns);

#endif  /* _SYSTICK_H_ */
