/*
 * File      : serial.c
 * This file is part of RT-Thread RTOS
 * COPYRIGHT (C) 2006, RT-Thread Development Team
 *
 * The license and distribution terms for this file may be
 * found in the file LICENSE in this distribution or at
 * http://openlab.rt-thread.com/license/LICENSE
 *
 * Change Logs:
 * Date           Author       Notes
 * 2006-03-13     Bernard      first version
 * 2009-04-20     yi.qiu       modified according bernard's stm32 version
 * 2012-02-17     mojingxian   modified for bf53x
 */

#include <os.h>

#include "serial.h"

/**
 * @addtogroup BF53X
 */
/*@{*/

/* RT-Thread Device Interface */
/**
 * This function initializes serial
 */
static os_err_t rt_serial_init (device_t dev)
{
    struct serial_device* uart = (struct serial_device*) dev->user_data;

    if (!(dev->flag & RT_DEVICE_FLAG_ACTIVATED)) {

        if (dev->flag & RT_DEVICE_FLAG_INT_RX) {
            os_memset(uart->int_rx->rx_buffer, 0,
                sizeof(uart->int_rx->rx_buffer));
            uart->int_rx->read_index = uart->int_rx->save_index = 0;
        }

        if (dev->flag & RT_DEVICE_FLAG_INT_TX) {
            os_memset(uart->int_tx->tx_buffer, 0,
                sizeof(uart->int_tx->tx_buffer));
            uart->int_tx->write_index = uart->int_tx->save_index = 0;
        }

        dev->flag |= RT_DEVICE_FLAG_ACTIVATED;
    }

    return OS_OK;
}

/* save a char to serial buffer */
static void rt_serial_savechar(struct serial_device* uart, char ch)
{
    os_sr_t sr;

    sr = os_enter_critical();

    uart->int_rx->rx_buffer[uart->int_rx->save_index] = ch;
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
}

static os_err_t rt_serial_open(device_t dev, uint16_t oflag)
{
    OS_ASSERT(dev != NULL);

    return OS_OK;
}

static os_err_t rt_serial_close(device_t dev)
{
    OS_ASSERT(dev != NULL);

    return OS_OK;
}

static size_t rt_serial_read (device_t dev, offset_t pos, void* buffer, size_t size)
{
    uint8_t* ptr;
    os_err_t err_code;
    struct serial_device* uart;

    ptr = buffer;
    err_code = OS_OK;
    uart = (struct serial_device*)dev->user_data;

    if (dev->flag & RT_DEVICE_FLAG_INT_RX) {
        os_sr_t sr;

        /* interrupt mode Rx */
        while (size) {
            if (uart->int_rx->read_index != uart->int_rx->save_index) {
                *ptr++ = uart->int_rx->rx_buffer[uart->int_rx->read_index];
                size--;

                sr = os_enter_critical();

                uart->int_rx->read_index++;
                if (uart->int_rx->read_index >= UART_RX_BUFFER_SIZE)
                    uart->int_rx->read_index = 0;

                os_exit_critical(sr);
            } else {
                /* set error code */
                err_code = OS_EEMPTY;
                break;
            }
        }
    } else {
        /* polling mode */
        while ((uint32_t)ptr - (uint32_t)buffer < size) {
            //while (uart->uart_device->ustat & USTAT_RCV_READY) {
                *ptr = uart->uart_device->rbr_thr & 0xff;
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
    struct serial_device* uart;

    err_code = OS_OK;
    ptr = (uint8_t*)buffer;
    uart = (struct serial_device*)dev->user_data;

    if (dev->flag & RT_DEVICE_FLAG_INT_TX) {
        /* interrupt mode Tx */
        while (uart->int_tx->save_index != uart->int_tx->write_index) {
            /* save on tx buffer */
            uart->int_tx->tx_buffer[uart->int_tx->save_index] = *ptr++;

            --size;

            /* move to next position */
            uart->int_tx->save_index++;

            /* wrap save index */
            if (uart->int_tx->save_index >= UART_TX_BUFFER_SIZE)
                uart->int_tx->save_index = 0;
        }

        /* set error code */
        if (size > 0)
            err_code = OS_EFULL;
    } else {
        /* polling mode */
        while (size) {
            /*
             * to be polite with serial console add a line feed
             * to the carriage return character
             */
            if (*ptr == '\n' && (dev->flag & RT_DEVICE_FLAG_STREAM)) {
                while (!(uart->uart_device->lsr & 0x20));
                uart->uart_device->rbr_thr = '\r';
            }

            while (!(uart->uart_device->lsr & 0x20));
            uart->uart_device->rbr_thr = (*ptr & 0xFF);

           ptr++;
           size--;
        }
    }

    /* set error code */
    rt_set_errno(err_code);

    return (uint32_t)ptr - (uint32_t)buffer;
}

static os_err_t rt_serial_control (device_t dev, uint8_t cmd, void *args)
{
    OS_ASSERT(dev != NULL);

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
 * serial register
 */
os_err_t os_arch_serial_register(device_t device, const char* name, uint32_t flag, struct serial_device *serial)
{
    OS_ASSERT(device != NULL);

    device->type         = RT_Device_Class_Char;
    device->rx_indicate = NULL;
    device->tx_complete = NULL;
    device->init         = rt_serial_init;
    device->open        = rt_serial_open;
    device->close        = rt_serial_close;
    device->read         = rt_serial_read;
    device->write         = rt_serial_write;
    device->control     = rt_serial_control;
    device->user_data   = serial;

    /* register a character device */
    return rt_device_register(device, name, RT_DEVICE_FLAG_RDWR | flag);
}

/* ISR for serial interrupt */
void os_arch_serial_isr(device_t device)
{
    struct serial_device* uart = (struct serial_device*) device->user_data;

    /* interrupt mode receive */
    OS_ASSERT(device->flag & RT_DEVICE_FLAG_INT_RX);

    /* save on rx buffer */
    //while (uart->uart_device->ustat & USTAT_RCV_READY) {
        rt_serial_savechar(uart, uart->uart_device->rbr_thr & 0xff);
    }

    /* invoke callback */
    if (device->rx_indicate != NULL) {
        size_t rx_length;

        /* get rx length */
        rx_length = uart->int_rx->read_index > uart->int_rx->save_index ?
            UART_RX_BUFFER_SIZE - uart->int_rx->read_index + uart->int_rx->save_index :
            uart->int_rx->save_index - uart->int_rx->read_index;

        device->rx_indicate(device, rx_length);
    }
}

/*@}*/
