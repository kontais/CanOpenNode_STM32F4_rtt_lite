/*
 * File      : serial.c
 * This file is part of RT-Thread RTOS
 * COPYRIGHT (C) 2010, RT-Thread Development Team
 *
 * The license and distribution terms for this file may be
 * found in the file LICENSE in this distribution or at
 * http://www.rt-thread.org/license/LICENSE
 *
 * Change Logs:
 * Date           Author       Notes
 * 2009-02-05     Bernard      first version
 * 2009-10-25     Bernard      fix rt_serial_read bug when there is no data
 *                             in the buffer.
 * 2010-03-29     Bernard      cleanup code.
 * 2010-03-30     Kyle         Ported from STM32 to AVR32.
 */

#include "serial.h"
#include "compiler.h"
#include "usart.h"

struct rt_device _rt_usart_device;
struct avr32_serial_int_rx _rt_usart_rx;
struct avr32_serial_device uart =
{
    .uart_device = (avr32_usart_t *) &AVR32_USART1,
    .int_rx = &_rt_usart_rx
};

/**
 * @addtogroup AVR32UC3
 */
/*@{*/

/* RT-Thread Device Interface */
static os_err_t rt_serial_init (device_t dev)
{
    struct avr32_serial_device* uart = (struct avr32_serial_device*) dev->user_data;

    if (!(dev->flag & RT_DEVICE_FLAG_ACTIVATED)) {
        if (dev->flag & RT_DEVICE_FLAG_INT_RX) {
            os_memset(uart->int_rx->rx_buffer, 0, sizeof(uart->int_rx->rx_buffer));
            uart->int_rx->read_index = 0;
            uart->int_rx->save_index = 0;
        }

        dev->flag |= RT_DEVICE_FLAG_ACTIVATED;
    }

    return OS_OK;
}

static os_err_t rt_serial_open(device_t dev, uint16_t oflag)
{
    return OS_OK;
}

static os_err_t rt_serial_close(device_t dev)
{
    return OS_OK;
}

static size_t rt_serial_read (device_t dev, offset_t pos, void* buffer, size_t size)
{
    uint8_t* ptr;
    os_err_t err_code;
    struct avr32_serial_device* uart;

    ptr = buffer;
    err_code = OS_OK;
    uart = (struct avr32_serial_device*)dev->user_data;

    if (dev->flag & RT_DEVICE_FLAG_INT_RX) {
        /* interrupt mode Rx */
        while (size) {
            os_sr_t sr;

            sr = os_enter_critical();

            if (uart->int_rx->read_index != uart->int_rx->save_index) {
                /* read a character */
                *ptr++ = uart->int_rx->rx_buffer[uart->int_rx->read_index];
                size--;

                /* move to next position */
                uart->int_rx->read_index++;
                if (uart->int_rx->read_index >= UART_RX_BUFFER_SIZE)
                    uart->int_rx->read_index = 0;
            } else {
                /* set error code */
                err_code = OS_EEMPTY;

                os_exit_critical(sr);
                break;
            }

            os_exit_critical(sr);
        }
    } else {
        /* polling mode */
        while ((uint32_t)ptr - (uint32_t)buffer < size) {
            while (usart_test_hit(uart->uart_device)) {
                *ptr = uart->uart_device->rhr & 0xff;
                ptr++;
            }
        }
    }

    /* set error code */
    rt_set_errno(err_code);
    return (uint32_t)ptr - (uint32_t)buffer;
}

static size_t rt_serial_write (device_t dev, offset_t pos, const void* buffer, size_t size)
{
    uint8_t* ptr;
    os_err_t err_code;
    struct avr32_serial_device* uart;

    err_code = OS_OK;
    ptr = (uint8_t*)buffer;
    uart = (struct avr32_serial_device*)dev->user_data;

    if (dev->flag & RT_DEVICE_FLAG_INT_TX) {
        /* interrupt mode Tx, does not support */
        OS_ASSERT(0);
    } else {
        /* polling mode */
        if (dev->flag & RT_DEVICE_FLAG_STREAM) {
            /* stream mode */
            while (size) {
                usart_putchar(uart->uart_device, (int) *ptr);

               ptr++;
               size--;
            }
        } else {
            /* write data directly */
            while (size) {
                usart_bw_write_char(uart->uart_device, (int) *ptr);

               ptr++;
               size--;
            }
        }
    }

    /* set error code */
    rt_set_errno(err_code);

    return (uint32_t)ptr - (uint32_t)buffer;
}

static os_err_t rt_serial_control (device_t dev, uint8_t cmd, void *args)
{
    struct avr32_serial_device* uart;

    OS_ASSERT(dev != NULL);

    uart = (struct avr32_serial_device*)dev->user_data;
    switch (cmd) {
    case RT_DEVICE_CTRL_SUSPEND:
        /* suspend device */
        dev->flag |= RT_DEVICE_FLAG_SUSPENDED;
        break;

    case RT_DEVICE_CTRL_RESUME:
        /* resume device */
        dev->flag &= ~RT_DEVICE_FLAG_SUSPENDED;
        break;
    }

    return OS_OK;
}

/*
 * serial register for STM32
 * support STM32F103VB and STM32F103ZE
 */
os_err_t os_arch_serial_register(device_t device, const char* name, uint32_t flag, struct avr32_serial_device *serial)
{
    OS_ASSERT(device != NULL);

    if ((flag & RT_DEVICE_FLAG_DMA_RX) ||
        (flag & RT_DEVICE_FLAG_INT_TX)) {
        OS_ASSERT(0);
    }

    device->type         = RT_Device_Class_Char;
    device->rx_indicate = NULL;
    device->tx_complete = NULL;
    device->init         = rt_serial_init;
    device->open        = rt_serial_open;
    device->close        = rt_serial_close;
    device->read         = rt_serial_read;
    device->write         = rt_serial_write;
    device->control     = rt_serial_control;
    device->user_data    = serial;

    /* register a character device */
    return rt_device_register(device, name, RT_DEVICE_FLAG_RDWR | flag);
}

/* ISR for serial interrupt */
void os_arch_serial_isr(void)
{
    struct avr32_serial_device* uart = (struct avr32_serial_device*) _rt_usart_device.user_data;
    os_sr_t sr;

    if (usart_test_hit(uart->uart_device)) {
        /* interrupt mode receive */
        OS_ASSERT(device->flag & RT_DEVICE_FLAG_INT_RX);

        sr = os_enter_critical();

        /* save character */
        uart->int_rx->rx_buffer[uart->int_rx->save_index] = uart->uart_device->rhr & 0xff;
        uart->int_rx->save_index++;
        if (uart->int_rx->save_index >= UART_RX_BUFFER_SIZE)
            uart->int_rx->save_index = 0;

        /* if the next position is read index, discard this 'read char' */
        if (uart->int_rx->save_index == uart->int_rx->read_index) {
            uart->int_rx->read_index++;
            if (uart->int_rx->read_index >= UART_RX_BUFFER_SIZE)
                uart->int_rx->read_index = 0;
        }

        os_exit_critical(sr);

        /* invoke callback */
        if (_rt_usart_device.rx_indicate != NULL) {
            size_t rx_length;

            /* get rx length */
            rx_length = uart->int_rx->read_index > uart->int_rx->save_index ?
                UART_RX_BUFFER_SIZE - uart->int_rx->read_index + uart->int_rx->save_index :
                uart->int_rx->save_index - uart->int_rx->read_index;

            _rt_usart_device.rx_indicate(&_rt_usart_device, rx_length);
        }
    } else {
        usart_reset_status(uart->uart_device);
    }
}

/*@}*/
