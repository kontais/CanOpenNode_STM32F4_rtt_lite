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
 * 2006-08-23     Bernard      first version
 * 2009-05-14     Bernard      add RT-THread device interface
 */

#include <rtcpu.h>
#include <os.h>

#include "AT91SAM7S.h"
#include "serial.h"

/**
 * @addtogroup AT91SAM7
 */
/*@{*/
typedef volatile uint32_t REG32;
struct rt_at91serial_hw
{
    REG32     US_CR;     // Control Register
    REG32     US_MR;     // Mode Register
    REG32     US_IER;     // Interrupt Enable Register
    REG32     US_IDR;     // Interrupt Disable Register
    REG32     US_IMR;     // Interrupt Mask Register
    REG32     US_CSR;     // Channel Status Register
    REG32     US_RHR;     // Receiver Holding Register
    REG32     US_THR;     // Transmitter Holding Register
    REG32     US_BRGR;     // Baud Rate Generator Register
    REG32     US_RTOR;     // Receiver Time-out Register
    REG32     US_TTGR;     // Transmitter Time-guard Register
    REG32     Reserved0[5];     //
    REG32     US_FIDI;     // FI_DI_Ratio Register
    REG32     US_NER;     // Nb Errors Register
    REG32     Reserved1[1];     //
    REG32     US_IF;     // IRDA_FILTER Register
    REG32     Reserved2[44];     //
    REG32     US_RPR;     // Receive Pointer Register
    REG32     US_RCR;     // Receive Counter Register
    REG32     US_TPR;     // Transmit Pointer Register
    REG32     US_TCR;     // Transmit Counter Register
    REG32     US_RNPR;     // Receive Next Pointer Register
    REG32     US_RNCR;     // Receive Next Counter Register
    REG32     US_TNPR;     // Transmit Next Pointer Register
    REG32     US_TNCR;     // Transmit Next Counter Register
    REG32     US_PTCR;     // PDC Transfer Control Register
    REG32     US_PTSR;     // PDC Transfer Status Register
};

struct rt_at91serial
{
    struct rt_device parent;

    struct rt_at91serial_hw* hw_base;
    uint16_t peripheral_id;
    uint32_t baudrate;

    /* reception field */
    uint16_t save_index, read_index;
    uint8_t  rx_buffer[RT_UART_RX_BUFFER_SIZE];
};
#ifdef RT_USING_UART1
struct rt_at91serial serial1;
#endif
#ifdef RT_USING_UART2
struct rt_at91serial serial2;
#endif

static void os_arch_serial_isr(int irqno)
{
    os_sr_t sr;
    struct rt_device* device;
    struct rt_at91serial* serial = NULL;

    if (irqno == AT91C_ID_US0) {
#ifdef RT_USING_UART1
        /* serial 1 */
        serial = &serial1;
#endif
    } else if (irqno == AT91C_ID_US1) {
#ifdef RT_USING_UART2
        /* serial 2 */
        serial = &serial2;
#endif
    }
    OS_ASSERT(serial != NULL);

    /* get generic device object */
    device = (device_t)serial;

    sr = os_enter_critical();

    /* get received character */
    serial->rx_buffer[serial->save_index] = serial->hw_base->US_RHR;

    /* move to next position */
    serial->save_index++;
    if (serial->save_index >= RT_UART_RX_BUFFER_SIZE)
        serial->save_index = 0;

    /* if the next position is read index, discard this 'read char' */
    if (serial->save_index == serial->read_index) {
        serial->read_index++;
        if (serial->read_index >= RT_UART_RX_BUFFER_SIZE)
            serial->read_index = 0;
    }

    os_exit_critical(sr);

    /* indicate to upper layer application */
    if (device->rx_indicate != NULL)
        device->rx_indicate(device, 1);

    /* ack interrupt */
    AT91C_AIC_EOICR = 1;
}

static os_err_t rt_serial_init (device_t dev)
{
    uint32_t bd;
    struct rt_at91serial* serial = (struct rt_at91serial*) dev;

    OS_ASSERT(serial != NULL);
    /* must be US0 or US1 */
    OS_ASSERT(((serial->peripheral_id == AT91C_ID_US0) ||
        (serial->peripheral_id == AT91C_ID_US1)));

    /* Enable Clock for USART */
    AT91C_PMC_PCER = 1 << serial->peripheral_id;

    /* Enable RxD0 and TxDO Pin */
    if (serial->peripheral_id == AT91C_ID_US0) {
        /* set pinmux */
        AT91C_PIO_PDR = (1 << 5) | (1 << 6);
    } else if (serial->peripheral_id == AT91C_ID_US1) {
        /* set pinmux */
        AT91C_PIO_PDR = (1 << 21) | (1 << 22);
    }

    serial->hw_base->US_CR = AT91C_US_RSTRX    |     /* Reset Receiver      */
                    AT91C_US_RSTTX        |        /* Reset Transmitter   */
                    AT91C_US_RXDIS        |        /* Receiver Disable    */
                    AT91C_US_TXDIS;                /* Transmitter Disable */

    serial->hw_base->US_MR = AT91C_US_USMODE_NORMAL |    /* Normal Mode */
                    AT91C_US_CLKS_CLOCK        |        /* Clock = MCK */
                    AT91C_US_CHRL_8_BITS    |        /* 8-bit Data  */
                    AT91C_US_PAR_NONE        |        /* No Parity   */
                    AT91C_US_NBSTOP_1_BIT;            /* 1 Stop Bit  */

    /* set baud rate divisor */
    bd =  ((MCK*10)/(serial->baudrate * 16));
    if ((bd % 10) >= 5) bd = (bd / 10) + 1;
    else bd /= 10;

    serial->hw_base->US_BRGR = bd;
    serial->hw_base->US_CR = AT91C_US_RXEN |        /* Receiver Enable     */
                    AT91C_US_TXEN;                    /* Transmitter Enable  */

    /* reset rx index */
    serial->save_index = 0;
    serial->read_index = 0;

    /* reset rx buffer */
    os_memset(serial->rx_buffer, 0, RT_UART_RX_BUFFER_SIZE);

    return OS_OK;
}

static os_err_t rt_serial_open(device_t dev, uint16_t oflag)
{
    struct rt_at91serial *serial = (struct rt_at91serial*)dev;
    OS_ASSERT(serial != NULL);

    if (dev->flag & RT_DEVICE_FLAG_INT_RX) {
        /* enable UART rx interrupt */
        serial->hw_base->US_IER = 1 << 0;         /* RxReady interrupt */
        serial->hw_base->US_IMR |= 1 << 0;         /* umask RxReady interrupt */

        /* install UART handler */
        os_arch_interrupt_install(serial->peripheral_id, os_arch_serial_isr, NULL);
        AT91C_AIC_SMR(serial->peripheral_id) = 5 | (0x01 << 5);
        os_arch_interrupt_umask(serial->peripheral_id);
    }

    return OS_OK;
}

static os_err_t rt_serial_close(device_t dev)
{
    struct rt_at91serial *serial = (struct rt_at91serial*)dev;
    OS_ASSERT(serial != NULL);

    if (dev->flag & RT_DEVICE_FLAG_INT_RX) {
        /* disable interrupt */
        serial->hw_base->US_IDR = 1 << 0;         /* RxReady interrupt */
        serial->hw_base->US_IMR &= ~(1 << 0);     /* mask RxReady interrupt */
    }

    return OS_OK;
}

static size_t rt_serial_read (device_t dev, offset_t pos, void* buffer, size_t size)
{
    uint8_t* ptr;
    struct rt_at91serial *serial = (struct rt_at91serial*)dev;
    OS_ASSERT(serial != NULL);

    /* point to buffer */
    ptr = (uint8_t*) buffer;

    if (dev->flag & RT_DEVICE_FLAG_INT_RX) {
        while (size) {
            /* interrupt receive */
            os_sr_t sr;

            sr = os_enter_critical();
            if (serial->read_index != serial->save_index) {
                *ptr = serial->rx_buffer[serial->read_index];

                serial->read_index++;
                if (serial->read_index >= RT_UART_RX_BUFFER_SIZE)
                    serial->read_index = 0;
            } else {
                /* no data in rx buffer */

                os_exit_critical(sr);
                break;
            }

            os_exit_critical(sr);

            ptr++; size--;
        }

        return (uint32_t)ptr - (uint32_t)buffer;
    } else if (dev->flag & RT_DEVICE_FLAG_DMA_RX) {
        /* not support right now */
        OS_ASSERT(0);
    } else {
        /* poll mode */
        while (size) {
            /* Wait for Full Rx Buffer */
            while (!(serial->hw_base->US_CSR & AT91C_US_RXRDY));

            /* Read Character */
            *ptr = serial->hw_base->US_RHR;
            ptr++;
            size--;
        }

        return (size_t)ptr - (size_t)buffer;
    }

    return 0;
}

static size_t rt_serial_write (device_t dev, offset_t pos, const void* buffer, size_t size)
{
    uint8_t* ptr;
    struct rt_at91serial *serial = (struct rt_at91serial*)dev;
    OS_ASSERT(serial != NULL);

    ptr = (uint8_t*) buffer;
    if (dev->open_flag & RT_DEVICE_OFLAG_WRONLY) {
        if (dev->flag & RT_DEVICE_FLAG_STREAM) {
            /* it's a stream mode device */
            while (size) {
                /* stream mode */
                if (*ptr == '\n') {
                    while (!(serial->hw_base->US_CSR & AT91C_US_TXRDY));
                    serial->hw_base->US_THR = '\r';
                }

                /* Wait for Empty Tx Buffer */
                while (!(serial->hw_base->US_CSR & AT91C_US_TXRDY));

                /* Transmit Character */
                serial->hw_base->US_THR = *ptr;
                ptr++; size--;
            }
        } else {
            while (size) {
                /* Wait for Empty Tx Buffer */
                while (!(serial->hw_base->US_CSR & AT91C_US_TXRDY));

                /* Transmit Character */
                serial->hw_base->US_THR = *ptr;
                ptr++; size--;
            }
        }
    }

    return (size_t)ptr - (size_t)buffer;
}

static os_err_t rt_serial_control (device_t dev, uint8_t cmd, void *args)
{
    return OS_OK;
}

os_err_t os_arch_serial_init()
{
    device_t device;

#ifdef RT_USING_UART1
    device = (device_t) &serial1;

    /* init serial device private data */
    serial1.hw_base         = (struct rt_at91serial_hw*)AT91C_BASE_US0;
    serial1.peripheral_id     = AT91C_ID_US0;
    serial1.baudrate        = 115200;

    /* set device virtual interface */
    device->init     = rt_serial_init;
    device->open     = rt_serial_open;
    device->close     = rt_serial_close;
    device->read     = rt_serial_read;
    device->write     = rt_serial_write;
    device->control = rt_serial_control;

    /* register uart1 on device subsystem */
    rt_device_register(device, "uart1", RT_DEVICE_FLAG_RDWR | RT_DEVICE_FLAG_INT_RX);
#endif

#ifdef RT_USING_UART2
    device = (device_t) &serial2;

    serial2.hw_base         = (struct rt_at91serial_hw*)AT91C_BASE_US1;
    serial2.peripheral_id     = AT91C_ID_US1;
    serial2.baudrate        = 115200;

    /* set device virtual interface */
    device->init     = rt_serial_init;
    device->open     = rt_serial_open;
    device->close     = rt_serial_close;
    device->read     = rt_serial_read;
    device->write     = rt_serial_write;
    device->control = rt_serial_control;

    /* register uart2 on device subsystem */
    rt_device_register(device, "uart2", RT_DEVICE_FLAG_RDWR | RT_DEVICE_FLAG_INT_RX);
#endif

    return OS_OK;
}

/*@}*/
