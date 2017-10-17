/*-----------------------------------------------------------------------
* CO_os.c  -
*
*
*
* Copyright (C) 2017 kontais@aliyun.com
*
*-----------------------------------------------------------------------*/
#include <stdio.h>  // for NULL
#include <board.h>
#include <os.h>

void bxCAN_init(void);
void can_recv_dispatch(CanRxMsg *msg);

#define CAN_RX_MSG_QUEUE_LEN 20
#define CAN_TX_MSG_QUEUE_LEN 20 

os_mqueue_t can_rx_mqueue;
os_mqueue_t can_tx_mqueue;
CanRxMsg can_rx_buffer[CAN_RX_MSG_QUEUE_LEN];
CanTxMsg can_tx_buffer[CAN_TX_MSG_QUEUE_LEN];

#define CAN_RECV_TASK_STACK_SIZE    2048
static os_task_t can_recv_task;
ALIGN(OS_ALIGN_SIZE) static uint8_t can_recv_task_stack[CAN_RECV_TASK_STACK_SIZE];

#define CAN_SEND_TASK_STACK_SIZE    2048
static os_task_t can_send_task;
ALIGN(OS_ALIGN_SIZE) static uint8_t can_send_task_stack[CAN_SEND_TASK_STACK_SIZE];

void (*can_recv_callback)(void* parameter, CanRxMsg *recv_msg) = NULL;

void can_recv_callback_install(void (*callback)(void* parameter, CanRxMsg *recv_msg))
{
    can_recv_callback = callback;
}

void can_recv_task_entry(void* parameter)
{
    CanRxMsg RxMessage;

    while (1) {
        os_mqueue_get(&can_rx_mqueue, (void*)&RxMessage, sizeof(CanRxMsg), OS_WAIT_FOREVER);
        if (can_recv_callback != NULL) {
            can_recv_callback(parameter, &RxMessage);
        }
    }
    
}

void can_send_task_entry(void* parameter)
{
    CanTxMsg TxMessage;
    
    while (1) {
        os_mqueue_get(&can_tx_mqueue, (void*)&TxMessage, sizeof(CanTxMsg), OS_WAIT_FOREVER);
        while (CAN_TxStatus_NoMailBox == CAN_Transmit(CAN1, &TxMessage)) {
            os_task_yield();
        }
    }
}

void can_task_init(void *parameter)
{
    os_mqueue_init(&can_rx_mqueue, (void*)can_rx_buffer, sizeof(CanRxMsg), sizeof(can_rx_buffer), OS_IPC_FIFO);
    os_mqueue_init(&can_tx_mqueue, (void*)can_tx_buffer, sizeof(CanTxMsg), sizeof(can_tx_buffer), OS_IPC_FIFO);

    os_task_init(&can_recv_task,
                    "can_rx",
                    can_recv_task_entry,
                    parameter,
                    &can_recv_task_stack[0],
                    CAN_RECV_TASK_STACK_SIZE,
                    128,
                    20);
    os_task_startup(&can_recv_task);

    os_task_init(&can_send_task,
                    "can_tx",
                    can_send_task_entry,
                    parameter,
                    &can_send_task_stack[0],
                    CAN_SEND_TASK_STACK_SIZE,
                    128,
                    20);
    os_task_startup(&can_send_task);

    bxCAN_init();
}

void CAN1_RX0_IRQHandler(void)
{
    CanRxMsg RxMessage;

    os_isr_enter();

    CAN_Receive(CAN1, CAN_FIFO0, &RxMessage);

    os_mqueue_put(&can_rx_mqueue, (void*)&RxMessage, sizeof(CanRxMsg));

    os_isr_leave();
}

uint32_t can_send(CanTxMsg *msg)
{
    os_err_t err;

    err = os_mqueue_put(&can_tx_mqueue, (void*)msg, sizeof(CanTxMsg));

    if (err == OS_OK) {
        return 0;
    }
    return 1;
}

uint32_t CO_timer1ms(void)
{
    return os_tick_get();
}
