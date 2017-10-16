#ifndef _CO_OS_H_
#define _CO_OS_H_

void can_task_init(void *parameter);
uint32_t can_send(CanTxMsg *msg);
void can_recv_callback_install(void (*callback)(void* parameter, CanRxMsg *recv_msg));
uint32_t CO_timer1ms(void);

#endif /* _CO_OS_H_ */
