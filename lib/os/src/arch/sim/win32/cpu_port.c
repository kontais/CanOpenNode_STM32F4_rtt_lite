/*
************************************************************************************************************************
* File    : os_arch_port.c
* By      : xyou
* Version : V1.00.00
*
* By      : prife
* Version : V1.00.01
************************************************************************************************************************
*/

/*
*********************************************************************************************************
*                                             INCLUDE FILES
*********************************************************************************************************
*/
#include  <os.h>
#include  <windows.h>
#include  <mmsystem.h>
#include  <stdio.h>
#include  "os_arch_port.h"

/*
*********************************************************************************************************
*                                             WinThread STRUCTURE
*  Windows runs each task in a task.
*  The context switch is managed by the tasks.So the task stack does not have to be managed directly,
*  although the stack stack is still used to hold an WinThreadState structure this is the only thing it
*  will be ever hold.
*  the structure indirectly maps the task handle to a task handle
*********************************************************************************************************
*/
typedef struct
{
    void            *Param;                     //Thread param
    void            (*Entry)(void *);           //Thread entry
    void            (*Exit)(void);                      //Thread exit
    HANDLE          ThreadHandle;
    DWORD           ThreadID;
}win_task_t;

const DWORD MS_VC_EXCEPTION=0x406D1388;

#pragma pack(push,8)
typedef struct tagTHREADNAME_INFO
{
    DWORD dwType; // Must be 0x1000.
    LPCSTR szName; // Pointer to name (in user addr space).
    DWORD dwThreadID; // Thread ID (-1=caller task).
    DWORD dwFlags; // Reserved for future use, must be zero.
} THREADNAME_INFO;
#pragma pack(pop)

/*
*********************************************************************************************************
*                                             LOCAL DEFINES
*********************************************************************************************************
*/
#define MAX_INTERRUPT_NUM       ((uint32_t)sizeof(uint32_t) * 8)

/*
 * Simulated interrupt waiting to be processed.this is a bit mask where each bit represent one interrupt
 * so a maximum of 32 interrupts can be simulated
 */
static volatile uint32_t  CpuPendingInterrupts = 0;

/*
 * An event used to inform the simulated interrupt processing task (a high priority task
 *      that simulated interrupt processing) that an interrupt is pending
 */
static HANDLE   hInterruptEventHandle = NULL;

/*
 * Mutex used to protect all the simulated interrupt variables that are accessed by multiple tasks
 */
static HANDLE   hInterruptEventMutex = NULL;

/*
 * Handler for all the simulate software interrupts.
 * The first two positions are used the Yield and Tick interrupt so are handled slightly differently
 * all the other interrupts can be user defined
*/
static uint32_t (*CpuIsrHandler[MAX_INTERRUPT_NUM])(void) = {0};

/*
 * Handler for OSTick Thread
 */
static HANDLE       OSTick_Thread;
static DWORD        OSTick_ThreadID;
static HANDLE       OSTick_SignalPtr;
static TIMECAPS     OSTick_TimerCap;
static MMRESULT     OSTick_TimerID;

/*
 * flag in interrupt handling
 */
uint32_t interrupt_switch_task_from, interrupt_switch_task_to;
uint32_t interrupt_switch_flag;

/*
*********************************************************************************************************
*                                             PRIVATE FUNCTION PROTOTYPES
*********************************************************************************************************
*/
//static void WinThreadScheduler(void);
void WinThreadScheduler(void);
uint32_t YieldInterruptHandle(void);
uint32_t SysTickInterruptHandle(void);
static DWORD WINAPI ThreadforSysTickTimer(LPVOID lpParam);
static DWORD WINAPI ThreadforKeyGet(LPVOID lpParam);

static void SetThreadName(DWORD dwThreadID, char* threadName)
{
#if defined(_MSC_VER)
    THREADNAME_INFO info;
    info.dwType = 0x1000;
    info.szName = threadName;
    info.dwThreadID = dwThreadID;
    info.dwFlags = 0;

    __try
    {
        RaiseException(MS_VC_EXCEPTION, 0, sizeof(info)/sizeof(ULONG_PTR), (ULONG_PTR*)&info);
    }
    __except(EXCEPTION_EXECUTE_HANDLER) {
    }
#endif
}

/*
*********************************************************************************************************
*                                            os_arch_stack_init()
* Description : Initialize stack of task
* Argument(s) : void *pvEntry,void *pvParam,uint8_t *pStackAddr,void *pvExit
* Return(s)   : uint8_t*
* Caller(s)   : os_task_init or os_task_create
* Note(s)     : none
*********************************************************************************************************
*/

static DWORD WINAPI thread_run(LPVOID lpThreadParameter)
{
    os_task_t tid = os_task_self();
    win_task_t  *pWinThread = (win_task_t *)lpThreadParameter;

    SetThreadName(GetCurrentThreadId(), tid->name);

    pWinThread->Entry(pWinThread->Param);

    pWinThread->Exit();
    return 0;
}

uint8_t* os_arch_stack_init(void *pEntry,void *pParam,uint8_t *pStackAddr,void *pExit)
{
    win_task_t    *pWinThread = NULL;

    /*
     * In this simulated case a stack is not initialized
     * The task handles the context switching itself. The WinThreadState object is placed onto the stack
     * that was created for the task
     * so the stack buffer is still used,just not in the conventional way.
     */
    pWinThread = (win_task_t *)(pStackAddr - sizeof(win_task_t));

    pWinThread->Entry = pEntry;
    pWinThread->Param = pParam;
    pWinThread->Exit = pExit;

    pWinThread->ThreadHandle = NULL;
    pWinThread->ThreadID = 0;

    /* Create the winthread */
    pWinThread->ThreadHandle = CreateThread(NULL,
                                            0,
                                            (LPTHREAD_START_ROUTINE) thread_run,
                                            pWinThread,
                                            CREATE_SUSPENDED,
                                            &(pWinThread->ThreadID));
    SetThreadAffinityMask(pWinThread->ThreadHandle,
                          0x01);
    SetThreadPriorityBoost(pWinThread->ThreadHandle,
                           TRUE);
    SetThreadPriority(pWinThread->ThreadHandle,
                      THREAD_PRIORITY_IDLE);

    return (uint8_t*)pWinThread;
} /*** os_arch_stack_init ***/

/*
*********************************************************************************************************
*                                            os_enter_critical()
* Description : disable cpu interrupts
* Argument(s) : void
* Return(s)   : int32_t
* Caller(s)   : Applicatios or os_kernel
* Note(s)     : none
*********************************************************************************************************
*/
os_sr_t os_enter_critical(void)
{
    if (hInterruptEventMutex != NULL) {
        WaitForSingleObject(hInterruptEventMutex,INFINITE);
    }

    return 0;
} /*** os_enter_critical ***/

/*
*********************************************************************************************************
*                                            os_exit_critical()
* Description : enable cpu interrupts
* Argument(s) : os_sr_t sr
* Return(s)   : void
* Caller(s)   : Applications or os_kernel
* Note(s)     : none
*********************************************************************************************************
*/
void os_exit_critical(os_sr_t sr)
{
    level = level;

    if (hInterruptEventMutex != NULL) {
        ReleaseMutex(hInterruptEventMutex);
    }

} /*** os_exit_critical ***/

/*
*********************************************************************************************************
*                                            os_arch_context_switch_interrupt()
* Description : switch task's contex
* Argument(s) : void
* Return(s)   : void
* Caller(s)   : os kernel
* Note(s)     : none
*********************************************************************************************************
*/
void os_arch_context_switch_interrupt(uint32_t from,
                                    uint32_t to)
{
    if (interrupt_switch_flag != 1) {
        interrupt_switch_flag = 1;

        // set interrupt_switch_task_from
        interrupt_switch_task_from = *((uint32_t *)(from));
    }

    interrupt_switch_task_to = *((uint32_t *)(to));

    //trigger YIELD exception(cause context switch)
    TriggerSimulateInterrupt(CPU_INTERRUPT_YIELD);
} /*** os_arch_context_switch_interrupt ***/


void os_arch_context_switch(uint32_t from,
                          uint32_t to)
{
    if (interrupt_switch_flag != 1) {
        interrupt_switch_flag  = 1;

        // set interrupt_switch_task_from
        interrupt_switch_task_from = *((uint32_t *)(from));

    }

    // set interrupt_switch_task_to
    interrupt_switch_task_to = *((uint32_t *)(to));

    //trigger YIELD exception(cause contex switch)
    TriggerSimulateInterrupt(CPU_INTERRUPT_YIELD);

} /*** os_arch_context_switch ***/

/*
*********************************************************************************************************
*                                            os_arch_context_switch_to()
* Description : switch to new task
* Argument(s) : uint32_t to              //the stack address of the task which will switch to
* Return(s)   : void
* Caller(s)   : os_task schecale
* Note(s)     : this function is used to perform the first task switch
*********************************************************************************************************
*/
void os_arch_context_switch_to(uint32_t to)
{
    //set to task
    interrupt_switch_task_to = *((uint32_t *)(to));

    //clear from task
    interrupt_switch_task_from = 0;

    //set interrupt to 1
    interrupt_switch_flag = 1;

    //start WinThreadScheduler
    WinThreadScheduler();

    //never reach here!
    return;

} /*** os_arch_context_switch_to ***/


/*
*********************************************************************************************************
*                                            TriggerSimulateInterrupt()
* Description : Trigger a simulated interrupts handle
* Argument(s) : t_uint32_t IntIndex
* Return(s)   : void
* Caller(s)   : Applications
* Note(s)     : none
*********************************************************************************************************
*/
void TriggerSimulateInterrupt(uint32_t IntIndex)
{
    if ((IntIndex < MAX_INTERRUPT_NUM) && (hInterruptEventMutex != NULL)) {
        /* Yield interrupts are processed even when critical nesting is non-zero  */
        WaitForSingleObject(hInterruptEventMutex,
                            INFINITE);

        CpuPendingInterrupts |= (1 << IntIndex);

        SetEvent(hInterruptEventHandle);

        ReleaseMutex(hInterruptEventMutex);
    }
} /*** TriggerSimulateInterrupt ***/

/*
*********************************************************************************************************
*                                            RegisterSimulateInterrupt()
* Description : Register a interrupt handle to simulate paltform
* Argument(s) : uint32_t IntIndex,uint32_t (*IntHandler)(void)
* Return(s)   : void
* Caller(s)   : Applications
* Note(s)     : none
*********************************************************************************************************
*/
void RegisterSimulateInterrupt(uint32_t IntIndex,uint32_t (*IntHandler)(void))
{
    if (IntIndex < MAX_INTERRUPT_NUM) {
        if (hInterruptEventMutex != NULL) {
            WaitForSingleObject(hInterruptEventMutex,
                                INFINITE);

            CpuIsrHandler[IntIndex] = IntHandler;

            ReleaseMutex(hInterruptEventMutex);
        } else {
            CpuIsrHandler[IntIndex] = IntHandler;
        }
    }

} /*** RegisterSimulateInterrupt ***/


/*
*********************************************************************************************************
*                                             PRIVATE FUNCTION
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*                                            WinThreadScheduler()
* Description : Handle all simulate interrupts
* Argument(s) : void
* Return(s)   : static void
* Caller(s)   : os scachle
* Note(s)     : none
*********************************************************************************************************
*/
#define WIN_WM_MIN_RES      (1)
 void WinThreadScheduler(void)
{
    HANDLE          hInterruptObjectList[2];
    HANDLE          hThreadHandle;
    uint32_t     SwitchRequiredMask;
    uint32_t     i;

    win_task_t    *WinThreadFrom;
    win_task_t    *WinThreadTo;

    /*
     * Install the interrupt handlers used bye scheduler itself
     */
    RegisterSimulateInterrupt(CPU_INTERRUPT_YIELD,
                              YieldInterruptHandle);
    RegisterSimulateInterrupt(CPU_INTERRUPT_TICK,
                              SysTickInterruptHandle);

    /*
     * Create the events and mutex that are used to synchronise all the WinThreads
     */
    hInterruptEventMutex = CreateMutex(NULL,
                                       FALSE,
                                       NULL);
    hInterruptEventHandle = CreateEvent(NULL,
                                        FALSE,
                                        FALSE,
                                        NULL);

    if ((hInterruptEventMutex == NULL) || (hInterruptEventHandle == NULL)) {
        return;
    }

    /*
     * Set the priority of this WinThread such that it is above the priority of the WinThreads
     * that run rt-threads.
     * This is higher priority is required to ensure simulate interrupts take priority over rt-threads
     */
    hThreadHandle = GetCurrentThread();
    if (hThreadHandle == NULL) {
        return;
    }

    if (SetThreadPriority(hThreadHandle,
                          THREAD_PRIORITY_HIGHEST) == 0) {
        return;
    }
    SetThreadPriorityBoost(hThreadHandle,
                           TRUE);
    SetThreadAffinityMask(hThreadHandle,
                          0x01);

    /*
     * Start the task that simulates the timer peripheral to generate tick interrupts.
     */
    OSTick_Thread = CreateThread(NULL,
                                 0,
                                 ThreadforSysTickTimer,
                                 0,
                                 CREATE_SUSPENDED,
                                 &OSTick_ThreadID);
    if (OSTick_Thread == NULL) {
        //Display Error Message

        return;
    }
    SetThreadPriority(OSTick_Thread,
                      THREAD_PRIORITY_NORMAL);
    SetThreadPriorityBoost(OSTick_Thread,
                           TRUE);
    SetThreadAffinityMask(OSTick_Thread,
                          0x01);

    /*
     * Set timer Caps
     */
    if (timeGetDevCaps(&OSTick_TimerCap,
                       sizeof(OSTick_TimerCap)) != TIMERR_NOERROR) {

        CloseHandle(OSTick_Thread);

        return;
    }
    if (OSTick_TimerCap.wPeriodMin < WIN_WM_MIN_RES) {
        OSTick_TimerCap.wPeriodMin = WIN_WM_MIN_RES;
    }

    if (timeBeginPeriod(OSTick_TimerCap.wPeriodMin) != TIMERR_NOERROR) {
        CloseHandle(OSTick_Thread);

        return;
    }

    OSTick_SignalPtr = CreateEvent(NULL,TRUE,FALSE,NULL);
    if (OSTick_SignalPtr == NULL) {
        // disp error message

        timeEndPeriod(OSTick_TimerCap.wPeriodMin);
        CloseHandle(OSTick_Thread);

        return;
    }

    OSTick_TimerID = timeSetEvent((UINT             )   (1000 / OS_TICKS_PER_SEC) ,
                                  (UINT             )   OSTick_TimerCap.wPeriodMin,
                                  (LPTIMECALLBACK   )   OSTick_SignalPtr,
                                  (DWORD_PTR        )   NULL,
                                  (UINT             )   (TIME_PERIODIC | TIME_CALLBACK_EVENT_SET));

    if (OSTick_TimerID == 0) {
        //disp

        CloseHandle(OSTick_SignalPtr);
        timeEndPeriod(OSTick_TimerCap.wPeriodMin);
        CloseHandle(OSTick_Thread);

        return;
    }

    /*
     * Start OS Tick Thread an release Interrupt Mutex
     */
    ResumeThread(OSTick_Thread);
    ReleaseMutex(hInterruptEventMutex);

    //trigger YEILD INTERRUPT
    TriggerSimulateInterrupt(CPU_INTERRUPT_YIELD);

    /*
     * block on the mutex that ensure exclusive access to the simulated interrupt objects
     *  and the events that signals that a simulated interrupt should be processed.
     */

    hInterruptObjectList[0] = hInterruptEventHandle;
    hInterruptObjectList[1] = hInterruptEventMutex;

    while (1) {
        WaitForMultipleObjects(sizeof(hInterruptObjectList) / sizeof(HANDLE),
                               hInterruptObjectList,
                               TRUE,
                               INFINITE);

        /*
         * Used to indicate whether the simulate interrupt processing has necessitated a contex
         * switch to another task
         */
        SwitchRequiredMask = 0;

        /*
         * For each interrupt we are interested in processing ,each of which is represented
         * by a bit in the 32bit CpuPendingInterrupts variable.
         */
        for (i = 0; i < MAX_INTERRUPT_NUM; i++) {
            /* is the simulated interrupt pending ? */
            if (CpuPendingInterrupts & (1UL << i)) {
                /* Is a handler installed ?*/
                if (CpuIsrHandler[i] != NULL) {
                    /* Run the actual handler */
                    if (CpuIsrHandler[i]() != 0) {
                        SwitchRequiredMask |= (1UL << i);
                    }
                }

                /* Clear the interrupt pending bit */
                CpuPendingInterrupts &= ~(1UL << i);
            }
        }

        if (SwitchRequiredMask != 0) {
            WinThreadFrom = (win_task_t *)interrupt_switch_task_from;
            WinThreadTo = (win_task_t *)interrupt_switch_task_to;

            if ((WinThreadFrom != NULL) && (WinThreadFrom->ThreadHandle != NULL)) {
                SuspendThread(WinThreadFrom->ThreadHandle);
            }

            ResumeThread(WinThreadTo->ThreadHandle);

        }

        ReleaseMutex(hInterruptEventMutex);
    }
} /*** WinThreadScheduler ***/


/*
*********************************************************************************************************
*                                            ThreadforSysTickTimer()
* Description : win task to simulate a systick timer
* Argument(s) : LPVOID lpParam
* Return(s)   : static DWORD WINAPI
* Caller(s)   : none
* Note(s)     : This is not a real time way of generating tick events as the next wake time should be relative
*               to the previous wake time,not the time Sleep() is called.
*               It is done this way to prevent overruns in this very non real time simulated/emulated environment
*********************************************************************************************************
*/
static DWORD WINAPI ThreadforSysTickTimer(LPVOID lpParam)
{

    (void)lpParam;              //prevent compiler warnings

    for (;;) {
        /*
         * Wait until the timer expires and we can access the simulated interrupt variables.
         */
        WaitForSingleObject(OSTick_SignalPtr,INFINITE);

        ResetEvent(OSTick_SignalPtr);

        /*
         * Trigger a systick interrupt
         */
        TriggerSimulateInterrupt(CPU_INTERRUPT_TICK);

    }

    return 0;

} /*** prvThreadforSysTickTimer ***/

/*
*********************************************************************************************************
*                                            SysTickInterruptHandle()
* Description : Interrupt handle for systick
* Argument(s) : void
* Return(s)   : uint32_t
* Caller(s)   : none
* Note(s)     : none
*********************************************************************************************************
*/
uint32_t SysTickInterruptHandle(void)
{

    /* enter interrupt */
    os_isr_enter();

    os_tick_increase();

    /* leave interrupt */
    os_isr_leave();

    return 0;
} /*** SysTickInterruptHandle ***/

/*
*********************************************************************************************************
*                                            YieldInterruptHandle()
* Description : Interrupt handle for Yield
* Argument(s) : void
* Return(s)   : uint32_t
* Caller(s)   : none
* Note(s)     : none
*********************************************************************************************************
*/
uint32_t YieldInterruptHandle(void)
{

    /*
     * if interrupt_switch_flag = 1 yield already handled
     */
    if (interrupt_switch_flag != 0) {
        interrupt_switch_flag = 0;

        /* return task switch request = 1 */
        return 1;
    }

    return 0;
} /*** YieldInterruptHandle ***/
