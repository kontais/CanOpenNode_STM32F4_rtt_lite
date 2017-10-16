/*-----------------------------------------------------------------------
* systick.c  -
*
*
*
* Copyright (C) 2017 kontais@aliyun.com
*
*-----------------------------------------------------------------------*/
#include <board.h>
#include <os.h>

void systick_init(void)
{
    RCC_ClocksTypeDef RCC_Clocks;

    /* Configure the SysTick */
    RCC_GetClocksFreq(&RCC_Clocks);
    SysTick_Config(RCC_Clocks.HCLK_Frequency / OS_TICKS_PER_SEC);    
}

/**
 * This is the timer interrupt service routine.
 *
 */
void SysTick_Handler(void)
{
    os_isr_enter();

    os_tick_increase();

    os_isr_leave();
}

void systick_stall_us(uint32_t us)
{
  uint32_t TickRemain, TickStart, TickElapsed, TickCurrent;
  uint32_t Load;

  Load   = SysTick->LOAD;

  TickRemain = (SystemCoreClock / 1000000) * us;

  TickStart = SysTick->VAL;
  while (1) {
    __NOP();
    __NOP();
    TickCurrent = SysTick->VAL;
    if (TickCurrent <= TickStart) {
      TickElapsed = TickStart - TickCurrent;
    } else {
      TickElapsed = Load - TickCurrent + TickStart;
    }

    TickStart   = TickCurrent;

    if (TickRemain > TickElapsed) {
      TickRemain -= TickElapsed;
    } else {
      break;  // TimeOut
    }
  }
}

/*
    此函数只针对168MHz STM32F4处理器
    最小延时30ns    
*/
void systick_stall_ns(uint32_t ns)
{
    uint32_t loop = ns / 30;
    do {
        __NOP();
    } while (loop--);
}
