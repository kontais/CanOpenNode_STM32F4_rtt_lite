#ifndef __OS_HW_SERIAL_H__
#define __OS_HW_SERIAL_H__

#include <rtcpu.h>
#include <os.h>

#include "s3c24x0.h"

#define    USTAT_RCV_READY        0x01       /* receive data ready */
#define    USTAT_TXB_EMPTY        0x02       /* tx buffer empty */
#define BPS                    115200    /* serial baudrate */

#define UART_RX_BUFFER_SIZE        64
#define UART_TX_BUFFER_SIZE        64

struct serial_int_rx
{
    uint8_t  rx_buffer[UART_RX_BUFFER_SIZE];
    uint32_t read_index, save_index;
};

struct serial_int_tx
{
    uint8_t  tx_buffer[UART_TX_BUFFER_SIZE];
    uint32_t write_index, save_index;
};

typedef struct uartport
{
    volatile uint32_t ulcon;
    volatile uint32_t ucon;
    volatile uint32_t ufcon;
    volatile uint32_t umcon;
    volatile uint32_t ustat;
    volatile uint32_t urxb;
    volatile uint32_t ufstat;
    volatile uint32_t umstat;
    volatile uint32_t utxh;
    volatile uint32_t urxh;
    volatile uint32_t ubrd;
}uartport;

struct serial_device
{
    uartport* uart_device;

    /* rx structure */
    struct serial_int_rx* int_rx;

    /* tx structure */
    struct serial_int_tx* int_tx;
};

os_err_t os_arch_serial_register(device_t device, const char* name, uint32_t flag, struct serial_device *serial);

void os_arch_serial_isr(device_t device);

#endif
