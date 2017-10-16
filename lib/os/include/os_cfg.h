/* RT-Thread config file */
#ifndef _OS_CFG_H_
#define _OS_CFG_H_

#define OS_NAME_MAX                   8
#define OS_ALIGN_SIZE                 8

#define OS_TICKS_PER_SEC              1000     // 1000Hz, 1ms/Tick

#define OS_TASK_PRIORITY_MAX          256     // 256 max


#define IDLE_TASK_STACK_SIZE           512

/* DEBUG */
#define OS_CFG_DEBUG
#define OS_CFG_OVERFLOW_CHECK

/* TIMER_TASK */
#define OS_CFG_SOFT_TIMER
#define OS_TIMER_TASK_PRIO            4
#define OS_TIMER_TASK_STACK_SIZE      1024

/* HEAP */
#define OS_CFG_HEAP
#define OS_HEAP_SIZE                  64
#define OS_HEAP_END                   (0x20000000 + OS_HEAP_SIZE * 1024)

#define OS_CONSOLE_BUF_SIZE           128

#define OS_CFG_CPU_FFS

#endif /* _OS_CFG_H_ */
