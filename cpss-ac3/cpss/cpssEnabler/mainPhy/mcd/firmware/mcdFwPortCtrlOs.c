/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
*******************************************************************************/
/**
********************************************************************************
* @file mcdFwPortCtrlOs.c
*
* @brief Port Control OS Initialization
*
* @version   1
********************************************************************************
*/
#include <mcdFwPortCtrlInc.h>

/* Port Control Process Handlers */
xTaskHandle mvHwsSupervisorHandler;
xTaskHandle mvHwsGeneralHandler;
xTaskHandle mvHwsPortHandler;
xTaskHandle mvHwsApPortHandler;
xTaskHandle mvHwsApDetectHandler;
#ifdef FREE_RTOS_HWS_ENHANCED_PRINT_MODE
xTaskHandle mvHwsDebugHandler;
#endif /* FREE_RTOS_HWS_ENHANCED_PRINT_MODE */

/* Port Control Semaphore Handlers */

#ifdef MV_SERDES_AVAGO_IP
xSemaphoreHandle mvHwsAvagoSem;
#endif /* MV_SERDES_AVAGO_IP */
xSemaphoreHandle mvHwsSyncSem;
xSemaphoreHandle mvHwsLogSem;
xSemaphoreHandle mvHwsIpcSem;
#ifdef FREE_RTOS_HWS_ENHANCED_PRINT_MODE
xSemaphoreHandle mvHwsDebugSem;
#endif /* FREE_RTOS_HWS_ENHANCED_PRINT_MODE */

/* Port Control Queue Handlers */
xQueueHandle mvHwsGeneralQueue;
xQueueHandle mvHwsPortQueue;
xQueueHandle mvHwsApDetectQueue;
xQueueHandle mvHwsPortCtrlQueue[] =
{
    [M0_DEBUG]           NULL,
    [M1_SUPERVISOR]      NULL,
    [M2_PORT_MNG]        NULL,
    [M3_PORT_SM]         NULL,
    [M4_AP_PORT_MNG]     NULL,
    [M5_AP_PORT_DET]     NULL,
    [M5_AP_PORT_DET_EXT] NULL,
    [M6_GENERAL]         NULL
};

/**
* @internal mvPortCtrlOsResourcesInit function
* @endinternal
*
* @brief   Allocate and Initialize OS Resources
*         - Allocate system memoty queues
*         - Allocate system semaphores
*         - Allocate system processes
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvPortCtrlOsResourcesInit(void)
{
    /* Message Queue Section */
    /* ===================== */
    mvHwsGeneralQueue = xQueueCreate(MV_GENERAL_MSG_NUM, MV_GENERAL_MSG_SIZE);
    mvHwsPortCtrlQueue[M6_GENERAL] = mvHwsGeneralQueue;
    if(mvHwsGeneralQueue == 0)
    {
        mvPcPrintf("Port Ctrl Error, Failed to create General queue\n");
        return GT_FAIL;
    }

    mvHwsApDetectQueue = xQueueCreate(MV_AP_MSG_NUM, MV_AP_MSG_SIZE);
    mvHwsPortCtrlQueue[M5_AP_PORT_DET] = mvHwsApDetectQueue;
    if(mvHwsApDetectQueue == 0)
    {
        mvPcPrintf("Port Ctrl Error, Failed to create AP Detect queue\n");
        return GT_FAIL;
    }

    mvHwsPortQueue = xQueueCreate(MV_PORT_MSG_NUM, MV_PORT_MSG_SIZE);
    mvHwsPortCtrlQueue[M2_PORT_MNG] = mvHwsPortQueue;
    if(mvHwsPortQueue == 0)
    {
        mvPcPrintf("Port Ctrl Error, Failed to create Port Init queue\n");
        return GT_FAIL;
    }

    /* Semaphore Section */
    /* ================= */
#ifdef MV_SERDES_AVAGO_IP
    mvHwsAvagoSem = xSemaphoreCreateMutex();
    if(mvHwsAvagoSem == NULL)
    {
        mvPcPrintf("Port Ctrl Error, Failed to create Avago Semaphore\n");
        return GT_FAIL;
    }
#endif /* MV_SERDES_AVAGO_IP */

    vSemaphoreCreateBinary(mvHwsSyncSem);
    if(mvHwsSyncSem != NULL)
    {
        if(xSemaphoreTake(mvHwsSyncSem, (portTickType) 10) != pdTRUE)
        {
            mvPcPrintf("Port Ctrl Error, Failed to take Sync Semaphore\\n");
            return GT_FAIL;
        }
    }
    else
    {
        mvPcPrintf("Port Ctrl Error, Failed to create Sync Semaphore\\n");
        return GT_FAIL;
    }

    mvHwsLogSem = xSemaphoreCreateMutex();
    if(mvHwsLogSem == NULL)
    {
        mvPcPrintf("Port Ctrl Error, Failed to create Log Semaphore\n");
        return GT_FAIL;
    }

    vSemaphoreCreateBinary(mvHwsIpcSem);
    if(mvHwsIpcSem == NULL)
    {
        mvPcPrintf("Port Ctrl Error, Failed to create IPC Semaphore\n");
        return GT_FAIL;
    }

#ifdef FREE_RTOS_HWS_ENHANCED_PRINT_MODE
    vSemaphoreCreateBinary(mvHwsDebugSem);
    if(mvHwsDebugSem == NULL)
    {
        mvPcPrintf("Port Ctrl Error, Failed to create Debug Semaphore\n");
        return GT_FAIL;
    }

    xSemaphoreTake(mvHwsDebugSem, (portTickType) portMAX_DELAY);
#endif /* FREE_RTOS_HWS_ENHANCED_PRINT_MODE */

    /* Process Section */
    /* =============== */

    /* M1_SUPERVISOR Process */
    if (xTaskCreate(mvPortCtrlSpvRoutine,
                (signed char*) "Supervisor",
                MV_SUPERVISOR_STACK,
                NULL,
                MV_SUPERVISOR_PRIORITY,
                &mvHwsSupervisorHandler) != pdPASS)
    {
        mvPcPrintf("Info, Create mvPortCtrlSpvRoutine Failed\n");
        return GT_FAIL;
    }

    /* M2_PORT_MNG Process */
    if (xTaskCreate(mvPortCtrlPortRoutine,
                (signed char*) "Port Mng",
                MV_PORT_STACK,
                NULL,
                MV_PORT_PRIORITY,
                &mvHwsPortHandler) != pdPASS)
    {
        mvPcPrintf("Info, Create mvPortCtrlPortRoutine Failed\n");
        return GT_FAIL;
    }

    /* M4_AP_PORT_MNG Process */
    if (xTaskCreate(mvPortCtrlApPortRoutine,
                (signed char*) "AP Port Mng",
                MV_PORT_STACK,
                NULL,
                MV_PORT_PRIORITY,
                &mvHwsApPortHandler) != pdPASS)
    {
        mvPcPrintf("Info, Create mvPortCtrlApPortRoutine Failed\n");
        return GT_FAIL;
    }

    /* M5_AP_PORT_DET Process */
    if (xTaskCreate(mvHwsApDetectRoutine,
                (signed char*) "AP Detect",
                MV_AP_STACK,
                NULL,
                MV_AP_PRIORITY,
                &mvHwsApDetectHandler) != pdPASS)
    {
        mvPcPrintf("Info, Create mvHwsApRoutine Failed\n");
        return GT_FAIL;
    }

    /* M6_GENERAL Process */
    if (xTaskCreate(mvPortCtrlGenRoutine,
                (signed char*) "General",
                MV_GENERAL_STACK,
                NULL,
                MV_GENERAL_PRIORITY,
                &mvHwsGeneralHandler) != pdPASS)
    {
        mvPcPrintf("Info, Create mvPortCtrlGenRoutine Failed\n");
        return GT_FAIL;
    }

#ifdef FREE_RTOS_HWS_ENHANCED_PRINT_MODE
    if (xTaskCreate(mvPortCtrlDebugRoutine,
                (signed char*) "Debug",
                MV_DEBUG_STACK,
                NULL,
                MV_DEBUG_PRIORITY,
                &mvHwsDebugHandler) != pdPASS)
    {
        mvPcPrintf("Info, Create mvPortCtrlGenRoutine Failed\n");
        return GT_FAIL;
    }
#endif /* FREE_RTOS_HWS_ENHANCED_PRINT_MODE */

    return GT_OK;
}

/**
* @internal mvPortCtrlSyncLock function
* @endinternal
*
* @brief   These two functions are used to synchronize the start of
*         execution of al processes
*         All process are blocked on mvPortCtrlProcessWait
*         the main routine will enable the semaphore (Give)
*         the first process that successfully take the semaphore
*         immediately release it to the next process, untill all process
*         are active and running
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvPortCtrlSyncLock(void)
{
    if(xSemaphoreTake(mvHwsSyncSem, (portTickType) portMAX_DELAY) != pdTRUE)
    {
        mvPcPrintf("Port Ctrl Error, Failed to Take sync semaphore\n");
        return GT_FAIL;
    }

    return GT_OK;
}

GT_STATUS mvPortCtrlSyncUnlock(void)
{
    if(xSemaphoreGive(mvHwsSyncSem) != pdTRUE)
    {
        mvPcPrintf("Port Ctrl Error, Failed to Give sync semaphore\n");
        return GT_FAIL;
    }

    return GT_OK;
}

/**
* @internal mvPortCtrlLogLock function
* @endinternal
*
* @brief   These two functions are used to protect real time log
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvPortCtrlLogLock(void)
{
    if(xSemaphoreTake(mvHwsLogSem, (portTickType) portMAX_DELAY) != pdTRUE)
    {
        mvPcPrintf("Port Ctrl Error, Failed to Take log semaphore\n");
        return GT_FAIL;
    }

    return GT_OK;
}

GT_STATUS mvPortCtrlLogUnlock(void)
{
    if(xSemaphoreGive(mvHwsLogSem) != pdTRUE)
    {
        mvPcPrintf("Port Ctrl Error, Failed to Give log semaphore\n");
        return GT_FAIL;
    }

    return GT_OK;
}

/**
* @internal mvPortCtrlIpcLock function
* @endinternal
*
* @brief   These two functions are used to protect IPC
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvPortCtrlIpcLock(void)
{
    if(xSemaphoreTake(mvHwsIpcSem, (portTickType) portMAX_DELAY) != pdTRUE)
    {
        mvPcPrintf("Port Ctrl Error, Failed to Take IPC semaphore\n");
        return GT_FAIL;
    }

    return GT_OK;
}

GT_STATUS mvPortCtrlIpcUnlock(void)
{
    if(xSemaphoreGive(mvHwsIpcSem) != pdTRUE)
    {
        mvPcPrintf("Port Ctrl Error, Failed to Give IPC semaphore\n");
        return GT_FAIL;
    }

    return GT_OK;
}

#ifdef MV_SERDES_AVAGO_IP
/**
* @internal mvPortCtrlAvagoLock function
* @endinternal
*
* @brief   These two functions are used to protect Avago access
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvPortCtrlAvagoLock(void)
{
    if(xSemaphoreTake(mvHwsAvagoSem, (portTickType) portMAX_DELAY) != pdTRUE)
    {
        mvPcPrintf("Port Ctrl Error, Failed to Take Avago semaphore\n");
        return GT_FAIL;
    }

    return GT_OK;
}

GT_STATUS mvPortCtrlAvagoUnlock(void)
{
    if(xSemaphoreGive(mvHwsAvagoSem) != pdTRUE)
    {
        mvPcPrintf("Port Ctrl Error, Failed to Give Avago semaphore\n");
        return GT_FAIL;
    }

    return GT_OK;
}
#endif /* MV_SERDES_AVAGO_IP */

#ifdef FREE_RTOS_HWS_ENHANCED_PRINT_MODE
/**
* @internal mvPortCtrlDbgLock function
* @endinternal
*
* @brief   These two functions are sync debug print
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvPortCtrlDbgLock(void)
{
    if(xSemaphoreTake(mvHwsDebugSem, (portTickType) portMAX_DELAY) != pdTRUE)
    {
        mvPcPrintf("Port Ctrl Error, Failed to Take Debug semaphore\n");
        return GT_FAIL;
    }

    return GT_OK;
}

GT_STATUS mvPortCtrlDbgUnlock(void)
{
    if(xSemaphoreGive(mvHwsDebugSem) != pdTRUE)
    {
        mvPcPrintf("Port Ctrl Error, Failed to Give Debug semaphore\n");
        return GT_FAIL;
    }

    return GT_OK;
}
#endif /* FREE_RTOS_HWS_ENHANCED_PRINT_MODE */

/**
* @internal mvPortCtrlCurrentTs function
* @endinternal
*
* @brief   Return current time stamp
*/
GT_U32 mvPortCtrlCurrentTs(void)
{
    return (xTaskGetTickCount() / portTICK_RATE_MS);
}

/**
* @internal mvPortCtrlProcessDelay function
* @endinternal
*
* @brief   Delay process execution for [x] msec
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvPortCtrlProcessDelay(GT_U32 duration)
{
    const portTickType xDelay = duration / portTICK_RATE_MS;

    vTaskDelay(xDelay);

    return GT_OK;
}

/**
* @internal mvPortCtrlThresholdSet function
* @endinternal
*
* @brief   Set threshold for port
*/
void mvPortCtrlThresholdSet(GT_U32 delay, GT_U32 *thrershold)
{
    GT_U32 currentTimestamp;

    if (delay == PORT_CTRL_TIMER_DEFAULT)
    {
        *thrershold = 0;
    }
    else
    {
        currentTimestamp = xTaskGetTickCount() / portTICK_RATE_MS;
        *thrershold = currentTimestamp + (delay);
    }
}

/**
* @internal mvPortCtrlThresholdCheck function
* @endinternal
*
* @brief   Check if port threshold exceeded
*
* @retval 1                        - threshold exceeded
* @retval 0                        - threshold not exceeded
*/
GT_U32 mvPortCtrlThresholdCheck(GT_U32 thrershold)
{
    GT_U32 currentTimestamp;

    if (thrershold > PORT_CTRL_TIMER_DEFAULT)
    {
        currentTimestamp = xTaskGetTickCount() / portTICK_RATE_MS;

        if (currentTimestamp >= thrershold)
        {
            return 1;
        }
    }

    return 0;
}

/**
* @internal mvPortCtrlProcessPendMsgNum function
* @endinternal
*
* @brief   Receive number of messages pending in one of the system message
*         queues
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvPortCtrlProcessPendMsgNum(MV_PORT_CTRL_MODULE module, GT_U32 *msgNum)
{
    if (mvHwsPortCtrlQueue[module] == NULL)
    {
        return GT_FAIL;
    }

    *msgNum = (GT_U32)uxQueueMessagesWaiting(mvHwsPortCtrlQueue[module]);

    return GT_OK;
}

/**
* @internal mvPortCtrlProcessMsgRecv function
* @endinternal
*
* @brief   Receive a message from one of the system message queues
*         duration defines the time to wait until the message is received
*         in case the queue is empty.
*         duration == 0, return immediately
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvPortCtrlProcessMsgRecv(MV_PORT_CTRL_MODULE module,
                                   GT_U32 duration,
                                   MCD_IPC_CTRL_MSG_STRUCT *msgPtr)
{
    if (mvHwsPortCtrlQueue[module] == NULL)
    {
        return GT_FAIL;
    }

    if(xQueueReceive(mvHwsPortCtrlQueue[module], msgPtr, (portTickType) duration / portTICK_RATE_MS) != pdTRUE)
    {
        return GT_EMPTY;
    }

    return GT_OK;
}

/**
* @internal mvPortCtrlProcessMsgSend function
* @endinternal
*
* @brief   Send a message to a system message queues
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvPortCtrlProcessMsgSend(MV_PORT_CTRL_MODULE module,
                                   MCD_IPC_CTRL_MSG_STRUCT *msgPtr)
{
    if (mvHwsPortCtrlQueue[module] == NULL)
    {
        return GT_FAIL;
    }

    if((xQueueSend(mvHwsPortCtrlQueue[module], msgPtr, (portTickType) MV_PROCESS_MSG_TX_DELAY)) != pdTRUE)
    {
        mvPcPrintf("Port Ctrl Error, module %d Queue FULL\n", module);
        return GT_FAIL;
    }

    return GT_OK;
}




