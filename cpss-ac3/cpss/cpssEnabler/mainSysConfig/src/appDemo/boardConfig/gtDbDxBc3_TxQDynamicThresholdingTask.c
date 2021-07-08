/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE IS A REFERENCE CODE FOR MARVELL SWITCH PRODUCTS.  IT IS PROVIDED   *
* "AS IS" WITH NO WARRANTIES, EXPRESSED, IMPLIED OR OTHERWISE, REGARDING ITS   *
* ACCURACY, COMPLETENESS OR PERFORMANCE.                                       *
* CUSTOMERS ARE FREE TO MODIFY IT AND USE IT ONLY IN THEIR PRODUCTION          *
* SOFTWARE RELEASES WITH MARVELL SWITCH CHIPSETS.                              *
*******************************************************************************/
#if (TARGET == HOST)
	#include <appDemo/boardConfig/gtDbDxBc3_TxQDynamicThresholdingTask.h>
#endif
#if (TARGET == CM3)
	#include <mvHwsBc3TxQDynamicThresholdingTask.h>
#endif

#define UNUSED_PARAM(x) x = x


/*---------------------------------------------*
 * OS onterface (either HOST / CM3 freeRTOS    *
 *---------------------------------------------*/

GT_STATUS DynThreshTaskMsgQueueCreate
(
    IN  UBaseType_t uxQueueLength,
    IN  UBaseType_t uxItemSize,
    OUT xQueueHandle *msgQIdPtr
)
{
    #if (TARGET == HOST)
        GT_STATUS rc;
        rc = osMsgQCreate("DynThreshTask_MsgQ", 
                           uxQueueLength,
                           uxItemSize, 
                           msgQIdPtr);
        if (rc != GT_OK)
        {
            return rc;
        }
        return GT_OK;
    #elif (TARGET == CM3)
        *msgQIdPtr =  xQueueCreate(uxQueueLength,uxItemSize );
        if (*msgQIdPtr == NULL)
        {
            return GT_FAIL;
        }
        return GT_OK;
    #else
        #error "undefined target"
        return GT_FAIL;
    #endif
}

GT_STATUS DynThreshTaskMsgQueueRcv
(
    IN xQueueHandle msgQId,
    OUT GT_PTR      messagePtr,
    OUT GT_U32      *messageSizePtr
)
{
    #if (TARGET == HOST)
        GT_STATUS rc;
        rc = osMsgQRecv(msgQId,messagePtr,messageSizePtr,OS_MSGQ_WAIT_FOREVER);
        if (rc != GT_OK)
        {
            return rc; /* nothing to do , something wrong */
        }
        return GT_OK;
    #elif (TARGET == CM3)
        if (xQueueReceive(msgQId,  message, portMAX_DELAY ))
        {
            return GT_OK;
        }
        return GT_FAIL;
    #else
        #error "undefined target"
        return GT_FAIL;
    #endif
}

GT_STATUS DynThreshTaskMsgQueueNumWaiting(IN xQueueHandle  msgQId,
                                          OUT UBaseType_t  *messageNumPtr)
{
    #if (TARGET == HOST)
        GT_STATUS rc;
        rc = osMsgQNumMsgs(msgQId,/*OUT*/messageNumPtr);
        if (rc != GT_OK)
        {
            return rc;
        }
        return GT_OK;
    #elif (TARGET == CM3)
        *messageNumPtr =  uxQueueMessagesWaiting( msgQId);
        return GT_OK;
    #else
        #error "undefined target"
        return GT_FAIL;
    #endif
}



GT_STATUS DynThreshTaskMsgTaskCreate(IN  const GT_CHAR *pcName,
                                     IN  GT_U32  uxPriority,
                                     IN  GT_U32  usStackDepth,
                                     IN  TaskFunction_t pvTaskCode,
                                     IN  GT_VOID *pvParameters,
                                     OUT GT_TASK *pxCreatedTask
)
{
    #if (TARGET == HOST)
        GT_STATUS rc;

        rc = osTaskCreate(/* IN */pcName
                          /* IN */,uxPriority
                          /* IN */,usStackDepth
                          /* IN */,pvTaskCode
                          /* IN */,pvParameters
                          /* OUT*/,pxCreatedTask);
        if (rc != GT_OK)
        {
            return rc;
        }
        return GT_OK;
    #elif (TARGET == CM3)
        BaseType_t xReturned;
        xreturned xTaskCreate(/* IN */pvTaskCode,
                               /* IN */pcName,
                               /* IN */usStackDepth,
                               /* IN */pvParameters,
                               /* IN */uxPriorityuxPriority,
                               /* OUT*/pxCreatedTask
                               );
        if( xReturned == pdPASS )
        {
            return GT_FAIL;
        }
        return GT_OK;
    #else
        #error "undefined target"
        return GT_FAIL;
    #endif
}

GT_VOID DynThreshTaskDelay(GT_U32 uSec)
{
    #if (TARGET == HOST)
        GT_U32 ms = (uSec+500)/1000;
        /* convert to ms */
        cpssOsTimerWkAfter(ms);
    #elif (TARGET == CM3)
        GT_U32 ticks;

        ticks = uSec /portTICK_PERIOD_USEC;
        vTaskDelay(ticks);
    #else
        #error "undefined target"
    #endif
}


/*-------------------------------------------------------*
 *  DynThresh Task   Implementation                      *
 *-------------------------------------------------------*/
extern GT_STATUS DynThreshTask_ProcUpon_ProfileUpdate           (DYN_THRESH_USER_DATA_STC *msgDataPtr);
extern GT_STATUS DynThreshTask_ProcUpon_Start                   (DYN_THRESH_USER_DATA_STC *msgDataPtr);
extern GT_STATUS DynThreshTask_ProcUpon_ThresholdUpdateTimeout  (DYN_THRESH_USER_DATA_STC *msgDataPtr);
extern GT_STATUS DynThreshTask_ProcUpon_ChangeUpdatePeriod      (DYN_THRESH_USER_DATA_STC *msgDataPtr);
extern GT_STATUS DynThreshTask_ProcUpon_Stop                    (DYN_THRESH_USER_DATA_STC *msgDataPtr);
extern GT_STATUS DynThreshTask_ProcUpon_PrintProfile            (DYN_THRESH_USER_DATA_STC *msgDataPtr);
extern GT_STATUS DynThreshTask_ProcUpon_PrintLog                (DYN_THRESH_USER_DATA_STC *msgDataPtr);

extern GT_STATUS DynThreshTask_Threshold_DP0_write (DYN_THRESH_PROFILE_ID profileId, GT_U32 queueId, GT_U32 value);
extern GT_STATUS DynThreshTask_Threshold_DP1_write (DYN_THRESH_PROFILE_ID profileId, GT_U32 queueId, GT_U32 value);
extern GT_STATUS DynThreshTask_Threshold_DP2_write (DYN_THRESH_PROFILE_ID profileId, GT_U32 queueId, GT_U32 value);
extern GT_STATUS DynThreshTask_Threshold_Port_write(DYN_THRESH_PROFILE_ID profileId, GT_U32 queueId, GT_U32 value);


DYN_THRESH_TASK_EV_2_PROC_REL_STC DYN_THRESH_EvProc_InitList[] = 
{
      { DYN_THRESH_Task_EVENT_PROFILE_UPDATE_E,             DynThreshTask_ProcUpon_ProfileUpdate          }
     ,{ DYN_THRESH_Task_EVENT_START_E,                      DynThreshTask_ProcUpon_Start                  }
     ,{ DYN_THRESH_Task_EVENT_THRESH_UPDATE_TIMEOUT_E,      DynThreshTask_ProcUpon_ThresholdUpdateTimeout }
     ,{ DYN_THRESH_Task_EVENT_CHANGE_UPDATE_PERIOD_E,       DynThreshTask_ProcUpon_ChangeUpdatePeriod     }
     ,{ DYN_THRESH_Task_EVENT_STOP_E,                       DynThreshTask_ProcUpon_Stop                   }
     ,{ DYN_THRESH_Task_EVENT_PRINT_PROFILE_E,              DynThreshTask_ProcUpon_PrintProfile           }
     ,{ DYN_THRESH_Task_EVENT_PRINT_LOG_E,                  DynThreshTask_ProcUpon_PrintLog               }
     ,{ DYN_THRESH_Task_EVENT_MAX_E,                        (DYN_THRESH_TASK_EV_PROC)NULL                 }
};

DYN_THRESH_EV_STR_STC DYN_THRESH_EvStr_InitList[] = 
{
      { DYN_THRESH_Task_EVENT_PROFILE_UPDATE_E,             "ProfileUpdate"                         }
     ,{ DYN_THRESH_Task_EVENT_START_E,                      "Start"                                 }
     ,{ DYN_THRESH_Task_EVENT_THRESH_UPDATE_TIMEOUT_E,      "ThresholdUpdateTimeout"                }
     ,{ DYN_THRESH_Task_EVENT_CHANGE_UPDATE_PERIOD_E,       "ChangeUpdatePeriod"                    }
     ,{ DYN_THRESH_Task_EVENT_STOP_E,                       "Stop"                                  }
     ,{ DYN_THRESH_Task_EVENT_PRINT_PROFILE_E,              "PrintProfile"                          }
     ,{ DYN_THRESH_Task_EVENT_PRINT_LOG_E,                  "PrintLog"                              }
     ,{ DYN_THRESH_Task_EVENT_MAX_E,                        (GT_CHAR *)NULL                         }
};

#define BAD_THRESH (DYN_THRESH_THRESHOLD_ID)(~0)

DYN_THRESH_PROFILE_THRESH_x_UPDATE_FUN_STC threshUpdateFun_InitList[] = 
{
    {         0 ,              7,  DynThreshTask_Threshold_DP0_write  }
   ,{         8 ,             15,  DynThreshTask_Threshold_DP1_write  }
   ,{        16 ,             23,  DynThreshTask_Threshold_DP2_write  }
   ,{        24 ,             24,  DynThreshTask_Threshold_Port_write }
   ,{ BAD_THRESH,     BAD_THRESH,  NULL                               }
};


DYN_THRESH_Task_STC        g_dynThreshTaskData;




/*-------------------------------------------------------*
 *   History                                             *
 *-------------------------------------------------------*/
GT_VOID DYN_THRESH_HISTORY_Init
(
    IN DYN_THRESH_HISTORY_STC *histPtr
)
{
    histPtr->len      = 0;
    histPtr->lastIdx  = 0;
    cpssOsMemSet(&histPtr->allocatedBuffersHist,     0,sizeof(histPtr->allocatedBuffersHist));
    cpssOsMemSet(&histPtr->thresholdUpdatedValueHist,0,sizeof(histPtr->thresholdUpdatedValueHist));
}

GT_VOID DYN_THRESH_HISTORY_AppendStart
(
    IN DYN_THRESH_HISTORY_STC *histPtr,
    IN GT_U32                  allocatedBuffers
)
{
    histPtr->allocatedBuffersHist[histPtr->lastIdx] = allocatedBuffers;
}

GT_VOID DYN_THRESH_HISTORY_AppendUpdatedThreshold
(
    IN DYN_THRESH_HISTORY_STC *histPtr,
    IN DYN_THRESH_THRESHOLD_ID thresholdId,
    IN DYN_THRESH_UPDATE_VALUE  value
)
{
    histPtr->thresholdUpdatedValueHist[histPtr->lastIdx][thresholdId] = value;
}

GT_VOID DYN_THRESH_HISTORY_AppendEnd
(
    IN DYN_THRESH_HISTORY_STC *histPtr
)
{
    if (histPtr->len < DYN_TRESH_HISTORY_SIZE-1)
    {
        histPtr->len++;
        histPtr->lastIdx++;
    }
    else /*  (histPtr->len >= DYN_TRESH_HISTORY_SIZE) */
    {
        if (histPtr->len < DYN_TRESH_HISTORY_SIZE)
        {
            histPtr->len++;
        }
        histPtr->lastIdx  = (histPtr->lastIdx +1) % DYN_TRESH_HISTORY_SIZE;
    }
}


/*-------------------------------------------------------*
 *   DYN_THRESH_Task_EVENT_PROFILE_UPDATE_E              *
 *-------------------------------------------------------*/
GT_STATUS DynThreshTask_ProcUpon_ProfileUpdate
(
    DYN_THRESH_USER_DATA_STC   *msgDataPtr
)
{
    GT_STATUS greediNessId;
    DYN_THRESH_PROFILE_PARAMS_STC * profilePtr;

    if (g_dynThreshTaskData.TASK_DEBUG_PRINT==GT_TRUE)
    {
        cpssOsPrintf("\n    PROFILE UPDATE : PROFILE_ID %d thresholdId %2d : GT=%d, GW=%d",
                             msgDataPtr->profileUpdate.profileId,
                             msgDataPtr->profileUpdate.thresholdId,
                             msgDataPtr->profileUpdate.guaranteeThreshold,
                             msgDataPtr->profileUpdate.greedinessWeight_p8q8);
    }
    if (msgDataPtr->profileUpdate.profileId > DYN_THRESH_TOTAL_PROFILE_CNS)
    {
        cpssOsPrintf("\n---> ERROR : bad profile %d",msgDataPtr->profileUpdate.profileId);
        return GT_OK;
    }
    if (msgDataPtr->profileUpdate.thresholdId > DYN_THRESH_TOTAL_THREHSHOLDS_CNS)
    {
        cpssOsPrintf("\n---> ERROR : bad thresholdId %d",msgDataPtr->profileUpdate.thresholdId);
        return GT_OK;
    }

    profilePtr = &g_dynThreshTaskData.dynTheshProfileParams[msgDataPtr->profileUpdate.profileId];
    greediNessId = msgDataPtr->profileUpdate.thresholdId;

    profilePtr->guaranteeThreshold    [greediNessId] = msgDataPtr->profileUpdate.guaranteeThreshold;
    profilePtr->greedinessWeight_p8q8 [greediNessId] = msgDataPtr->profileUpdate.greedinessWeight_p8q8;

    return GT_OK;
}

/*-------------------------------------------------------*
 *   DYN_THRESH_Task_EVENT_START_E                       *
 *-------------------------------------------------------*/
GT_STATUS DynThreshTask_ProcUpon_Start
(
    DYN_THRESH_USER_DATA_STC   *msgDataPtr
)
{
    if (g_dynThreshTaskData.TASK_DEBUG_PRINT==GT_TRUE)
    {
        cpssOsPrintf("\n    START : PROFILE_ID %d profileId = %d maxBuf = %6d updatePer = %d"
                                  ,msgDataPtr->start.profileId
                                  ,msgDataPtr->start.profileId
                                  ,msgDataPtr->start.MaxBufferThr
                                  ,msgDataPtr->start.updatePerioud_uSec);

    }
    if (msgDataPtr->start.profileId >= DYN_THRESH_TOTAL_PROFILE_CNS)
    {
        cpssOsPrintf("\n---> ERROR : bad profile %d",msgDataPtr->start.profileId);
        return GT_OK;
    }
    g_dynThreshTaskData.operatParams.selectedProfileId  = msgDataPtr->start.profileId;
    g_dynThreshTaskData.operatParams.MaxBufferThr       = msgDataPtr->start.MaxBufferThr;
    g_dynThreshTaskData.operatParams.updatePerioud_uSec = msgDataPtr->start.updatePerioud_uSec;

    DYN_THRESH_HISTORY_Init(&g_dynThreshTaskData.history);

    if (msgDataPtr->start.updatePerioud_uSec != 0)
    {
        g_dynThreshTaskData.state = DYN_THRESH_STATE_WORK_E;
    }
    return GT_OK;
}

/*-------------------------------------------------------*
 *   DYN_THRESH_Task_EVENT_CHANGE_UPDATE_PERIOD_E        *
 *-------------------------------------------------------*/
GT_STATUS DynThreshTask_ProcUpon_ChangeUpdatePeriod (DYN_THRESH_USER_DATA_STC *msgDataPtr)
{
    if (g_dynThreshTaskData.TASK_DEBUG_PRINT==GT_TRUE)
    {
        cpssOsPrintf("\n    CHANGE_UPDATE PERIOD :  %d uSec",msgDataPtr->start.profileId);
    }
    g_dynThreshTaskData.operatParams.updatePerioud_uSec = msgDataPtr->updatePeriod.updatePerioud_uSec;
    return GT_OK;
}


/*-------------------------------------------------------*
 *   DYN_THRESH_Task_EVENT_STOP_E                        *
 *-------------------------------------------------------*/
GT_STATUS DynThreshTask_ProcUpon_Stop
(
    DYN_THRESH_USER_DATA_STC   *msgDataPtr
)
{
    if (g_dynThreshTaskData.TASK_DEBUG_PRINT==GT_TRUE)
    {
        cpssOsPrintf("\n    STOP",msgDataPtr->start.profileId);
    }
    g_dynThreshTaskData.state = DYN_THRESH_STATE_IDLE_E;
    g_dynThreshTaskData.operatParams.selectedProfileId = DYN_THRESH_BAD_PROFILE_ID_CNS;


    g_dynThreshTaskData.state = DYN_THRESH_STATE_IDLE_E;
    return GT_OK;
}


/*-------------------------------------------------------*
 *   DYN_THRESH_Task_EVENT_THRESH_UPDATE_TIMEOUT_E       *
 *-------------------------------------------------------*/
#define BM_GLOBAL_BUFFER_CNTR_OFFS_CNS  0
#define BM_GLOBAL_BUFFER_CNTR_LEN_CNS  16

GT_STATUS DynThreshTask_allocatedBuffersNumGet
(
    GT_U32 * allocatedBuffersPtr
)
{
    GT_STATUS rc;
    GT_U32 regAddr;

    regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(g_dynThreshTaskData.devNum)->BM.BMCntrs.BMGlobalBufferCntr;
    rc = prvCpssDrvHwPpGetRegField(g_dynThreshTaskData.devNum, regAddr, 
                            BM_GLOBAL_BUFFER_CNTR_OFFS_CNS,BM_GLOBAL_BUFFER_CNTR_LEN_CNS, 
                            /*OUT*/allocatedBuffersPtr);

    #if (BC3_DYN_THRESH_TASK_FREE_BUF_NUM_UPDATE == 1)
        *allocatedBuffersPtr = g_dynThreshTaskData.operatParams.MaxBufferThr >> g_dynThreshTaskData.history.len;
    #endif
    return rc;
}

GT_STATUS DynThreshTask_Threshold_DP0_write
(
    DYN_THRESH_PROFILE_ID    profileId,
    GT_U32                   queueId,
    GT_U32                   value
)
{
    GT_STATUS rc;
    CPSS_DXCH_TABLE_ENT tableType; /*table type*/
    GT_U32      hwArray[2] = {0};   /* HW table data */
    GT_U32      tableIndex;         /* table index */

    tableIndex = profileId*DYN_THRESH_QUEUE_NUM_CNS + queueId;

    /* Queue Limits DP0 - Enqueue */
    tableType = CPSS_DXCH_SIP5_TABLE_TAIL_DROP_EQ_QUEUE_LIMITS_DP0_E;
    U32_SET_FIELD_IN_ENTRY_MAC(hwArray, 0,  32, 0xFFFF);
    U32_SET_FIELD_IN_ENTRY_MAC(hwArray, 32, 19, value);
    rc = prvCpssDxChWriteTableEntry(g_dynThreshTaskData.devNum, tableType, tableIndex, hwArray);
    if(rc != GT_OK)
    {
        return rc;
    }

    /* 
    hwArray[0] = hwArray[1] = 0;
    value = 0;
    rc = prvCpssDxChReadTableEntry(g_dynThreshTaskData.devNum, tableType, tableIndex, hwArray);

    U32_GET_FIELD_IN_ENTRY_MAC(hwArray,32,19,value);
    */
    return GT_OK;
}

GT_STATUS DynThreshTask_Threshold_DP1_write
(
    DYN_THRESH_PROFILE_ID    profileId,
    GT_U32                   queueId,
    GT_U32                   value
)
{
    GT_STATUS rc;
    CPSS_DXCH_TABLE_ENT tableType; /*table type*/
    GT_U32      hwArray[2] = {0};   /* HW table data */
    GT_U32      tableIndex;         /* table index */

    tableIndex = profileId*DYN_THRESH_QUEUE_NUM_CNS + queueId;

    /* Queue Limits DP0 - Enqueue */
    tableType = CPSS_DXCH_SIP5_TABLE_TAIL_DROP_EQ_QUEUE_LIMITS_DP1_E;
    U32_SET_FIELD_IN_ENTRY_MAC(hwArray, 0,  32, 0xFFFF);
    U32_SET_FIELD_IN_ENTRY_MAC(hwArray, 32, 19, value);
    rc = prvCpssDxChWriteTableEntry(g_dynThreshTaskData.devNum, tableType, tableIndex, hwArray);
    if(rc != GT_OK)
    {
        return rc;
    }
    return GT_OK;
}

GT_STATUS DynThreshTask_Threshold_DP2_write
(
    DYN_THRESH_PROFILE_ID    profileId,
    GT_U32                   queueId,
    GT_U32                   value
)
{
    GT_STATUS rc;
    CPSS_DXCH_TABLE_ENT tableType; /*table type*/
    GT_U32      hwArray[2] = {0};   /* HW table data */
    GT_U32      tableIndex;         /* table index */

    tableIndex = profileId*DYN_THRESH_QUEUE_NUM_CNS + queueId;

    /* Queue Limits DP0 - Enqueue */
    tableType = CPSS_DXCH_SIP5_TABLE_TAIL_DROP_EQ_QUEUE_LIMITS_DP1_E;
    U32_SET_FIELD_IN_ENTRY_MAC(hwArray, 0,  32, 0xFFFF);
    U32_SET_FIELD_IN_ENTRY_MAC(hwArray, 32, 19, value);
    rc = prvCpssDxChWriteTableEntry(g_dynThreshTaskData.devNum, tableType, tableIndex, hwArray);
    if(rc != GT_OK)
    {
        return rc;
    }
    return GT_OK;
}

GT_STATUS DynThreshTask_Threshold_Port_write
(
    DYN_THRESH_PROFILE_ID    profileId,
    GT_U32                   queueId,  /* unused parameter */
    GT_U32                   value
)
{
    GT_STATUS rc;
    GT_U32 regAddr;

    UNUSED_PARAM(queueId);

    regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(g_dynThreshTaskData.devNum)->TXQ.queue.tailDrop.tailDropLimits.profilePortBufferLimits[profileId];
    rc = prvCpssDrvHwPpSetRegField(g_dynThreshTaskData.devNum, regAddr, 
                                   0,19,value);
    return rc;
}


GT_U32 DynThreshTask_ThresholdCompute
(
    DYN_THRESHOLD_GARANTEE_TH      guaranteeThreshold,
    DYN_THRESHOLD_GREEDINESS_p8q8  greedinessWeight_p8q8,
    GT_U32 freeBuffers
)
{
    GT_U32 value;
    GT_U32 dynThr = (GT_U32)greedinessWeight_p8q8*(GT_U32)freeBuffers;

    dynThr = (dynThr + 128)>> 8;
    value = guaranteeThreshold + dynThr;
    return value;
}


GT_STATUS DynThreshTask_ProcUpon_ThresholdUpdateTimeout
(
    DYN_THRESH_USER_DATA_STC   *msgDataPtr
)
{
    GT_STATUS rc;
    GT_U32 allocatedBuffers;
    GT_U32 freeBuffers;
    DYN_THRESH_THRESHOLD_ID  thresholdId;
    GT_U32                   queueId;
    DYN_THRESH_UPDATE_VALUE   value;
    DYN_THRESH_PROFILE_PARAMS_STC *profilePtr;

    UNUSED_PARAM(msgDataPtr);


    if (g_dynThreshTaskData.TASK_DEBUG_PRINT==GT_TRUE)
    {
        cpssOsPrintf("\n    THRESH UPDATE TIMEOUT");
    }
    if (g_dynThreshTaskData.operatParams.selectedProfileId == DYN_THRESH_BAD_PROFILE_ID_CNS)
    {
        cpssOsPrintf("\n--->no operational profile");
        return GT_OK;
    }


    profilePtr = &g_dynThreshTaskData.dynTheshProfileParams[g_dynThreshTaskData.operatParams.selectedProfileId];

    rc = DynThreshTask_allocatedBuffersNumGet(&allocatedBuffers);
    if (rc != GT_OK)
    {
        if (g_dynThreshTaskData.TASK_DEBUG_PRINT==GT_TRUE)
        {
            cpssOsPrintf("\n---> ERROR : read allocated buffers fails : %d",rc);
        }
        return rc;
    }


    freeBuffers = 0;
    if (g_dynThreshTaskData.operatParams.MaxBufferThr > allocatedBuffers)
    {
        freeBuffers = g_dynThreshTaskData.operatParams.MaxBufferThr - allocatedBuffers;
    }

    DYN_THRESH_HISTORY_AppendStart(&g_dynThreshTaskData.history,allocatedBuffers);
    for (thresholdId = 0 ; thresholdId < 8/* DYN_THRESH_TOTAL_THREHSHOLDS_CNS */; thresholdId++)
    {
        queueId = thresholdId % 8;
        value = DynThreshTask_ThresholdCompute(profilePtr->guaranteeThreshold   [thresholdId],
                                               profilePtr->greedinessWeight_p8q8[thresholdId],
                                               freeBuffers);
        rc = g_dynThreshTaskData.threshUpdateFunArr[thresholdId](g_dynThreshTaskData.operatParams.selectedProfileId,
                                                            queueId, 
                                                            value);
        if (rc != GT_OK)
        {
            return rc;
        }
        DYN_THRESH_HISTORY_AppendUpdatedThreshold(&g_dynThreshTaskData.history,thresholdId,value);
    }

    DYN_THRESH_HISTORY_AppendEnd(&g_dynThreshTaskData.history);
    return GT_OK;
}

/*-------------------------------------------------------*
 *   DYN_THRESH_Task_EVENT_PRINT_LOG_E                   *
 *-------------------------------------------------------*/
GT_STATUS DynThreshTask_ProcUpon_PrintProfile
(
    DYN_THRESH_USER_DATA_STC   *msgDataPtr
)
{
    DYN_THRESH_THRESHOLD_ID thresholdId;
    if (g_dynThreshTaskData.TASK_DEBUG_PRINT==GT_TRUE)
    {
        cpssOsPrintf("\n    PrintProfile %d",msgDataPtr->profilePrint.profileId);
    }
    if (msgDataPtr->profilePrint.profileId >= DYN_THRESH_TOTAL_PROFILE_CNS)
    {
        cpssOsPrintf("\n---> ERROR Profile %d : out of boundary",msgDataPtr->profilePrint.profileId);
        return GT_OK;
    }
    cpssOsPrintf("\n    +-----------------------------------+");
    cpssOsPrintf("\n    | Profile %2d                       |",msgDataPtr->profilePrint.profileId);
    cpssOsPrintf("\n    +-----------------------------------+");
    cpssOsPrintf("\n    | %2s : %8s %8s ","#","GT","GW");
    for (thresholdId = 0; thresholdId < DYN_THRESH_TOTAL_THREHSHOLDS_CNS; thresholdId++)
    {
        GT_U32 intPart  = g_dynThreshTaskData.dynTheshProfileParams[msgDataPtr->profilePrint.profileId].greedinessWeight_p8q8[thresholdId]>>8;
        GT_U32 fracPart = g_dynThreshTaskData.dynTheshProfileParams[msgDataPtr->profilePrint.profileId].greedinessWeight_p8q8[thresholdId] & 0xFF;
        cpssOsPrintf("\n    | %2d : %8d %4d.%3d/256",thresholdId
                              ,g_dynThreshTaskData.dynTheshProfileParams[msgDataPtr->profilePrint.profileId].guaranteeThreshold[thresholdId]
                              ,intPart
                              ,fracPart);

    }
    return GT_OK;
}
/*-------------------------------------------------------*
 *   DYN_THRESH_Task_EVENT_PRINT_LOG_E                   *
 *-------------------------------------------------------*/
GT_STATUS DynThreshTask_ProcUpon_PrintLog
(
    DYN_THRESH_USER_DATA_STC   *msgDataPtr
)
{
    GT_U32 i;
    GT_U32 idx;
    DYN_THRESH_THRESHOLD_ID thresholdId;

    if (g_dynThreshTaskData.TASK_DEBUG_PRINT==GT_TRUE)
    {
        cpssOsPrintf("\n    PrintLog");
    }
    cpssOsPrintf("\n+----+---------+------------------------------------------------------------------------------------------------------------------------------+");
    cpssOsPrintf("\n| #  |  alloc  |                    DP0                                           DP1                                  DP3                    |");
    cpssOsPrintf("\n|    | buffers | port    0    1    2    3    4    5    6    7    0    1    2    3    4    5    6    7    0    1    2    3    4    5    6    7 |");
    cpssOsPrintf("\n+----+---------+------------------------------------------------------------------------------------------------------------------------------+");
    i = g_dynThreshTaskData.history.lastIdx;
    for (idx = 0 ;  idx < g_dynThreshTaskData.history.len; idx++, i = (i +1 ) % DYN_TRESH_HISTORY_SIZE)
    {
        cpssOsPrintf("\n| %2d | %7d |",i,g_dynThreshTaskData.history.allocatedBuffersHist[i]);
        cpssOsPrintf(" %4d", g_dynThreshTaskData.history.thresholdUpdatedValueHist[i][24]);
        for(thresholdId = 0 ; thresholdId <  DYN_THRESH_TOTAL_THREHSHOLDS_CNS-1; thresholdId++)
        {
            cpssOsPrintf(" %4d", g_dynThreshTaskData.history.thresholdUpdatedValueHist[i][thresholdId]);
        }
    }
    return GT_OK;
}



GT_STATUS DYN_THRESH_TASK_DEBUG_PRINT_Set(GT_U32 value)
{
    #if (BC3_DYN_THRESH_TASK_DEBUG_PRINT_EN == 1)
        if (value == 0)
        {
            g_dynThreshTaskData.TASK_DEBUG_PRINT = GT_FALSE;
        }
        else
        {
            g_dynThreshTaskData.TASK_DEBUG_PRINT = GT_TRUE;
        }
        return GT_OK;
    #else
        UNUSED_PARAM(value);
        cpssOsPrint("\nSet BC3_DYN_THRESH_TASK_DEBUG_PRINT_EN == 1\n");
        return GT_FAIL;
    #endif
}


static GT_STATUS DYN_THRESH_Task_MsgGetProcess(GT_VOID)
{
    GT_STATUS rc;
    DYN_THRESH_Task_Msg_STC msg;
    GT_U32                  msgSize;


    msgSize = (GT_U32)sizeof(msg);

    rc = DynThreshTaskMsgQueueRcv(g_dynThreshTaskData.msgQId,&msg,&msgSize);
    if (rc != GT_OK)
    {
        return rc; /* nothing to do , something wrong */
    }
    if (msg.ev >= DYN_THRESH_Task_EVENT_MAX_E)
    {
        if (g_dynThreshTaskData.TASK_DEBUG_PRINT==GT_TRUE)
        {
            cpssOsPrintf("\n--->ERROR : Undefined EVENT %d",msg.ev);
        }
        return rc; /* nothing to do , something wrong */
    }
    if (g_dynThreshTaskData.TASK_DEBUG_PRINT==GT_TRUE)
    {
        cpssOsPrintf("\nGet EVENT %2d = %s",msg.ev,g_dynThreshTaskData.eventStrArr[msg.ev]);
    }
    rc = (g_dynThreshTaskData.eventProcArr[msg.ev])((GT_VOID*)&msg.user_data);
    if (rc != GT_OK)
    {
        return rc; /* nothing to do , something wrong */
    }
    #if (BC3_DYN_THRESH_TASK_UNIT_TEST == 1)
        osSemSignal(g_dynThreshTaskData.semId);
    #endif
    return GT_OK;
}

#if (TARGET == HOST)
    static unsigned __TASKCONV DYN_THRESH_Task(GT_VOID * param)

#elif (TARGET == CM3)
    static void vTaskCode( void * param)
#else
    error "undefined target"
#endif
{
    GT_STATUS rc;
    GT_U32 msgNum;

    UNUSED_PARAM(param);
    g_dynThreshTaskData.state = DYN_THRESH_STATE_IDLE_E;
    for (;;)
    {
        switch (g_dynThreshTaskData.state)
        {
            case DYN_THRESH_STATE_IDLE_E:
            {
                rc = DYN_THRESH_Task_MsgGetProcess();
                break;
            }
            case DYN_THRESH_STATE_WORK_E:
            {
                rc = DynThreshTaskMsgQueueNumWaiting(g_dynThreshTaskData.msgQId,/*OUT*/&msgNum);
                if (rc != GT_OK)
                {
                    break; /* nothing to do , something wrong */
                }
                if (msgNum > 0)
                {
                    rc = DYN_THRESH_Task_MsgGetProcess();
                    break;
                }
                rc = DynThreshTask_ProcUpon_ThresholdUpdateTimeout(NULL);
                DynThreshTaskDelay(g_dynThreshTaskData.operatParams.updatePerioud_uSec*1000);
            }
            default:
            {
                return GT_FAIL; /* unknown state */
            }
        }
       
    }
    return GT_OK;
}



/*-----------------------------------------------*
 * DYN_THRESH_Task_Create()                      * 
 *-----------------------------------------------*/
static GT_STATUS DYN_THRESH_TASK_EventProcTblInit
(
    INOUT DYN_THRESH_Task_STC * taskDataPtr,
    IN DYN_THRESH_TASK_EV_2_PROC_REL_STC * evProc_InitList
)
{
    GT_U32 i;
    for (i = 0 ; i < sizeof(taskDataPtr->eventProcArr)/sizeof(taskDataPtr->eventProcArr[0]); i++)
    {
        taskDataPtr->eventProcArr[i] = (DYN_THRESH_TASK_EV_PROC)NULL;
    }
    for (i = 0 ; evProc_InitList[i].ev != DYN_THRESH_Task_EVENT_MAX_E; i++)
    {
        if (evProc_InitList[i].ev >= DYN_THRESH_Task_EVENT_MAX_E)
        {
            return GT_BAD_PARAM;
        }
        taskDataPtr->eventProcArr[evProc_InitList[i].ev] = evProc_InitList[i].proc;
    }
    /* check */
    for (i = 0 ; i < sizeof(taskDataPtr->eventProcArr)/sizeof(taskDataPtr->eventProcArr[0]); i++)
    {
        if (taskDataPtr->eventProcArr[i] ==(DYN_THRESH_TASK_EV_PROC)NULL)
        {
            cpssOsPrintf("\n--> ERROR : event %d has no function to process",i);
            return GT_BAD_PARAM;
        }
    }

    return GT_OK;
}

static GT_STATUS DYN_THRESH_TASK_EventStrInit
(
    INOUT DYN_THRESH_Task_STC * taskDataPtr,
    IN    DYN_THRESH_EV_STR_STC * evStr_InitList
)
{
    GT_U32 i;
    for (i = 0 ; i < sizeof(taskDataPtr->eventProcArr)/sizeof(taskDataPtr->eventProcArr[0]); i++)
    {
        taskDataPtr->eventStrArr[i] = (GT_CHAR*)NULL;
    }
    for (i = 0 ; evStr_InitList[i].ev != DYN_THRESH_Task_EVENT_MAX_E; i++)
    {
        if (evStr_InitList[i].ev >= DYN_THRESH_Task_EVENT_MAX_E)
        {
            return GT_BAD_PARAM;
        }
        taskDataPtr->eventStrArr[evStr_InitList[i].ev] = evStr_InitList[i].str;
    }
    /* check that all is filled */
    for (i = 0 ; i < sizeof(taskDataPtr->eventProcArr)/sizeof(taskDataPtr->eventProcArr[0]); i++)
    {
        if (taskDataPtr->eventStrArr[i] == (GT_CHAR*)NULL)
        {
            cpssOsPrintf("\n--> ERROR : event %d has no name",i);
            return GT_BAD_PARAM;
        }
    }

    return GT_OK;
}

static GT_STATUS DYN_THRESH_TASK_ThresholdUpdateFunTblInit
(
    INOUT DYN_THRESH_Task_STC * taskDataPtr,
    IN    DYN_THRESH_PROFILE_THRESH_x_UPDATE_FUN_STC * threshUpdateFun_InitListPtr
)
{
    GT_U32 i;
    DYN_THRESH_THRESHOLD_ID threshId;
    for (threshId = 0 ; threshId < sizeof(taskDataPtr->threshUpdateFunArr)/sizeof(taskDataPtr->threshUpdateFunArr[0]); threshId++)
    {
        taskDataPtr->threshUpdateFunArr[threshId] = NULL;
    }
    for (i = 0 ; threshUpdateFun_InitListPtr->threshUpdateFun != NULL; i++, threshUpdateFun_InitListPtr++)
    {
        for (threshId = threshUpdateFun_InitListPtr->firstThresh ; threshId <= threshUpdateFun_InitListPtr->lastThresh; threshId++)
        {
            taskDataPtr->threshUpdateFunArr[threshId] = threshUpdateFun_InitListPtr->threshUpdateFun;
        }
    }
    /* check that all is filled */
    for (threshId = 0 ; threshId < sizeof(taskDataPtr->threshUpdateFunArr)/sizeof(taskDataPtr->threshUpdateFunArr[0]); threshId++)
    {
        if (taskDataPtr->threshUpdateFunArr[threshId] == NULL)
        {
            cpssOsPrintf("\n--> ERROR : thresholId %d has has no function",threshId);
            return GT_BAD_PARAM;
        }
    }

    return GT_OK;

}


static GT_STATUS DYN_THRESH_Task_Create
(
    DYN_THRESH_Task_STC * taskDataPtr,
    GT_U32                prio

)
{
    GT_STATUS rc;
    DYN_THRESH_PROFILE_ID    profileId;
    DYN_THRESH_THRESHOLD_ID greediNessId;
    DYN_THRESH_PROFILE_PARAMS_STC * profilePtr;


    taskDataPtr->taskCreated = GT_FALSE;
    taskDataPtr->TASK_DEBUG_PRINT = GT_FALSE;


   /*-----------------------------------------*
    * init event --> proc table               *
    *-----------------------------------------*/
    rc = DYN_THRESH_TASK_EventProcTblInit(taskDataPtr,&DYN_THRESH_EvProc_InitList[0]);
    if (rc != GT_OK)
    {
        return rc;
    }
    rc = DYN_THRESH_TASK_EventStrInit(taskDataPtr,&DYN_THRESH_EvStr_InitList[0]);
    if (rc != GT_OK)
    {
        return rc;
    }

    rc = DYN_THRESH_TASK_ThresholdUpdateFunTblInit(taskDataPtr,&threshUpdateFun_InitList[0]);
    if (rc != GT_OK)
    {
        return rc;
    }


    for (profileId = 0; profileId < sizeof(taskDataPtr->dynTheshProfileParams)/sizeof(taskDataPtr->dynTheshProfileParams[0]); profileId++)
    {
        profilePtr = &taskDataPtr->dynTheshProfileParams[profileId];
        profilePtr->profileId = profileId;
        for (greediNessId = 0 ; greediNessId < sizeof(profilePtr->greedinessWeight_p8q8)/sizeof(profilePtr->greedinessWeight_p8q8[0]); greediNessId++)
        {
            profilePtr->guaranteeThreshold[greediNessId] = 0;
            profilePtr->guaranteeThreshold[greediNessId] = 0;
        }
    }
    g_dynThreshTaskData.operatParams.selectedProfileId  = DYN_THRESH_BAD_PROFILE_ID_CNS;
    g_dynThreshTaskData.operatParams.updatePerioud_uSec = 0;

    /*--------------------------------------------*
     * Init Message Queue                         *
     *--------------------------------------------*/
    rc = DynThreshTaskMsgQueueCreate(DYN_THRESH_Task_MsgQ_SIZE, sizeof(DYN_THRESH_Task_Msg_STC), /*OUT*/&taskDataPtr->msgQId);
    if (rc != GT_OK)
    {
        return rc;
    }

    #if (BC3_DYN_THRESH_TASK_UNIT_TEST == 1)
        rc = osSemBinCreate("DYN_THRESH_Task_Sem",OS_SEMB_EMPTY,&taskDataPtr->semId);
        if (rc != GT_OK)
        {
            return rc;
        }
    #endif


    /*--------------------------------------------*
     * Create Task                                *
     *--------------------------------------------*/
    rc = DynThreshTaskMsgTaskCreate("DYN_THRESH_Task",  /* Task Name      */
                        prio,             /* Task Priority  */
                        _4KB,             /* Stack Size     */
                        DYN_THRESH_Task,  /* Starting Point */
                        NULL,             /* Arguments list */
                        &taskDataPtr->taskId);     /* task ID        */
    if (rc != GT_OK)
    {
        return rc;
    }

    #ifdef ASIC_SIMULATION
    taskDataPtr->devNum = 0;
    #endif

    DynThreshTaskDelay(100);

    taskDataPtr->taskCreated = GT_TRUE;

    return GT_OK;
}


GT_STATUS appDemoDynThreshInit
(
    GT_VOID
)
{
    GT_STATUS rc;

    rc = DYN_THRESH_Task_Create(&g_dynThreshTaskData, 32);
    if (rc != GT_OK)
    {
        return rc;
    }
    return GT_OK;
}



