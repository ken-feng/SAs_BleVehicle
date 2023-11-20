#include "serial.h"
#include "UART_Serial_Adapter.h"
#include "EM000401.h"
#if SERIAL_LOG_USING_TASK
#include "fsl_os_abstraction.h"
#endif
#define DEBUG_QUEUE_DEPTH       512//1024//512
#define DEBUG_QUEUE_SIZE        1

static u8 debug_tx_queue[NEW_QUEUE_SIZE(DEBUG_QUEUE_DEPTH, DEBUG_QUEUE_SIZE)];
static boolean debug_tx_flag = FALSE;
static uartState_t debug_state;


#if SERIAL_LOG_USING_TASK
void SerialTask(void* argument);
osaTaskId_t gSerialTaskId = 0;
osaEventId_t gSerialTaskEvent;
void BleMsgProcess_Task(void* argument);
//OSA_TASK_DEFINE(SerialTask, 4, 1, 1024, FALSE );
OSA_TASK_DEFINE(SerialTask, 1, 1, 512, FALSE );
#define gSerialTaskEvt              (1 << 0)
void Serial_task_init()
{
    gSerialTaskEvent = OSA_EventCreate(TRUE);
    if( NULL == gSerialTaskEvent )
    {
        panic(0,0,0,0);
        return;
    }
    gSerialTaskId = OSA_TaskCreate(OSA_TASK(SerialTask), NULL);
    if( NULL == gSerialTaskId )
    {
         panic(0,0,0,0);
         return;
    }
}

#endif

static int serial_debug_send(void)
{
    u32 len = 0;
    static u8 debug_tx_buff[256];

    do 
    {
        len = bdk_queue_get_use_size((bdk_queue_t *)debug_tx_queue);
        if (len > sizeof(debug_tx_buff))
        {
            len = sizeof(debug_tx_buff);
        }
        else
        {
            if (len == 0)
            {
                break;
            }
        }
        
        bdk_queue_pop((bdk_queue_t *)debug_tx_queue, debug_tx_buff, len, DEBUG_QUEUE_SIZE);
        LPUART_SendData(DEBUG_INSTANCE, debug_tx_buff, len);
    } while (0);

    return len;

}

static int serial_debug_printf(u8 *pdata, u16 length)
{
    bdk_queue_push((bdk_queue_t *)debug_tx_queue, pdata, length, DEBUG_QUEUE_SIZE);
    if (debug_tx_flag == FALSE)
    {
#if SERIAL_LOG_USING_TASK        
        OSA_EventSet(gSerialTaskEvent, gSerialTaskEvt);
#else        
        serial_debug_send();
        debug_tx_flag = TRUE;
#endif        
    }

    return length;
}

static void serial_uart_tx_cb(uartState_t* state)
{
#if SERIAL_LOG_USING_TASK
    OSA_EventSet(gSerialTaskEvent, gSerialTaskEvt);
#else    
    if (serial_debug_send() == 0)
    {
        debug_tx_flag = FALSE;
    }
#endif    
}


void serial_debug_init(void)
{
    LPUART_Initialize(DEBUG_INSTANCE, &debug_state);
    debug_state.pRxData = NULL;
    LPUART_InstallRxCalback(DEBUG_INSTANCE, NULL, 0);
    LPUART_InstallTxCalback(DEBUG_INSTANCE, serial_uart_tx_cb, 0);
    core_mm_set(debug_tx_queue, 0, sizeof(debug_tx_queue));
    bdk_queue_init((bdk_queue_t *)debug_tx_queue, DEBUG_QUEUE_DEPTH, DEBUG_QUEUE_SIZE);

    LPUART_SetBaudrate(DEBUG_INSTANCE, UART_BPS);
    bdk_log_redirect(serial_debug_printf);
}

void serial_init(void)
{
    serial_debug_init();
    
#if SERIAL_LOG_USING_TASK   
    Serial_task_init();
#endif
}

#if SERIAL_LOG_USING_TASK

void SerialTask(void* argument)
{
    osaEventFlags_t event;  
    while(1)
    {
        OSA_EventWait(gSerialTaskEvent, gSerialTaskEvt, FALSE, osaWaitForever_c , &event);
        if (event & gSerialTaskEvt) 
        {   
            if (serial_debug_send() == 0)
            {
                debug_tx_flag = FALSE;
            }
            else
            {
                debug_tx_flag = TRUE;
            }
                
        }
    }
}
#endif
