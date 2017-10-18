/*
 * File      : board.c
 * This file is part of RT-Thread RTOS
 * COPYRIGHT (C) 2006 - 2013 RT-Thread Develop Team
 *
 * The license and distribution terms for this file may be
 * found in the file LICENSE in this distribution or at
 * http://www.rt-thread.org/license/LICENSE
 *
 * Change Logs:
 * Date           Author       Notes
 * 2009-01-05     Bernard      first implementation
 */
#include <stdio.h>

#include <board.h>

#include <usart4.h>
#include <systick.h>

#include <at24c16.h>

/**
 * @addtogroup STM32
 */

/*******************************************************************************
* Function Name  : assert_failed
* Description    : Reports the name of the source file and the source line number
*                  where the assert error has occurred.
* Input          : - file: pointer to the source file name
*                  - line: assert error line source number
* Output         : None
* Return         : None
*******************************************************************************/
void assert_failed(uint8_t* file, uint32_t line)
{
    printf("\n\r Wrong parameter value detected on\r\n");
    printf("       file  %s\r\n", file);
    printf("       line  %d\r\n", line);

    while (1) ;
}

/**
 * This function will initial STM32 board.
 */
void board_init(void)
{
    systick_init();
    uart4_init();
}

void board_reset(void)
{
    NVIC_SystemReset();
    __disable_irq();
    while (1);          // wait for wdog reset
}
/*@}*/
