/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE IS A REFERENCE CODE FOR MARVELL SWITCH PRODUCTS.  IT IS PROVIDED   *
* "AS IS" WITH NO WARRANTIES, EXPRESSED, IMPLIED OR OTHERWISE, REGARDING ITS   *
* ACCURACY, COMPLETENESS OR PERFORMANCE.                                       *
* CUSTOMERS ARE FREE TO MODIFY IT AND USE IT ONLY IN THEIR PRODUCTION          *
* SOFTWARE RELEASES WITH MARVELL SWITCH CHIPSETS.                              *
*******************************************************************************/

#ifndef __gtDbDxBc3TxQDynamicThresholdingTask_H
#define __gtDbDxBc3TxQDynamicThresholdingTask_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/*--------------------------------------------------------------------------------------------------
 *  TAIL-DROP profile
 *
 *  GT <guaranteeThreshold> :  0 - 48K  : UINT16
 *  GW <greedinessWeight>   :  . 8 integer + 8 - fractional part  GT_U8
 *  
 *  Update operation:
 *  
 *      TD_FreeBuffers  = max(0, TD_MaxBufferThr - AllocatedBuffers);  AllocatedBuffers is read from the device
 *      TD threshold[i] = GT[i] + GW[i]*TD_FreeBuffers,  i = 0:24 (8 priority queues x 3 drop precedences + port)
 *  
 *  Read :
 *                   0-15  0x40000200          <BM_IP> BM_IP TLU/Units/BM_IP Units/BM Counters/BM Global Buffer Counter
 *  Write : 
 *    +-------------+-------+-------+-------------------+-------------------------------------------------------
 *    |  greediness | field | field | addrees           |  cider
 *    |    index:   |       | bits  |                   |
 *    +-------------+-------+-------+-------------------+-------------------------------------------------------
 *    |      0-7    | Queue | 32-51 | 0x550a1800 + n*8  |  <TXQ_IP>TXQ_IP/<TXQ_IP> TXQ_QUEUE/Tables/TxQ Queue/Tail Drop/Queue Limits DP0 - Enqueue/Queue Limits DP0 - Enqueue
 *    |             | limit |       |                   |  where n (0-127) 16 profile x 8 represents limit 
      |             | DP0   |       |                   |
 *    +-------------+-------+-------+-------------------+-------------------------------------------------------
 *    |      8-15   | DP1   | 32-51 | 0x550a1C00 + n*8  |
 *    |     16-23   | DP2   |       | 0x550a2000 + n*8  |
 *    +-------------+-------+-------+-------------------+------------------------------------------------------- 
 *    |        24   | Port  |  0-19 | 0x550A0900 + 0x4*n|  <TXQ_IP>TXQ_IP/<TXQ_IP> TXQ_QUEUE/Units/TXQ_IP_queue/Tail Drop/Tail Drop Limits/Profile<%n> Port Buffer Limits
 *    |             | buffer|       |                   |  where n represnts profile N [0-15]
 *    |             | limit |       |                   |
 *    +-------------+-------+-------+-------------------+-------------------------------------------------------
 * 
 *     From CPSS point of view, access to 
 *          tableType = CPSS_DXCH_SIP5_TABLE_TAIL_DROP_EQ_QUEUE_LIMITS_DP0_E
 *                    CPSS_DXCH_SIP5_TABLE_TAIL_DROP_EQ_QUEUE_LIMITS_DP1_E
 *                    CPSS_DXCH_SIP5_TABLE_TAIL_DROP_EQ_QUEUE_LIMITS_DP2_E
 *       
 *           tableIndex = profileId*DYN_THRESH_QUEUE_NUM_CNS + queueId;
 *       
 *           U32_SET_FIELD_IN_ENTRY_MAC(hwArray, 0,  32, 0xFFFF);
 *           U32_SET_FIELD_IN_ENTRY_MAC(hwArray, 32, 19, value);
 *           prvCpssDxChWriteTableEntry(g_dynThreshTaskData.devNum, tableType, tableIndex, hwArray);
 *
 *-------------------------------------------------------------------------------------------------*/

#define HOST      1
#define CM3       2

#define TARGET    HOST /* HOST CM3  */

#define  BC3_DYN_THRESH_TASK_TEST_CNS     1
#if (BC3_DYN_THRESH_TASK_TEST_CNS == 1)
    #define BC3_DYN_THRESH_TASK_UNIT_TEST             0
    #define BC3_DYN_THRESH_TASK_DEBUG_PRINT_EN        1
    #define BC3_DYN_THRESH_TASK_FREE_BUF_NUM_UPDATE   1
#else
    #define  BC3_DYN_THRESH_TASK_UNIT_TEST            0
    #define BC3_DYN_THRESH_TASK_DEBUG_PRINT_EN        0
    #define BC3_DYN_THRESH_TASK_FREE_BUF_NUM_UPDATE   0
#endif

#if (TARGET == HOST)
    #include <cpssCommon/cpssPresteraDefs.h>
    #include <cpss/extServices/private/prvCpssBindFunc.h>
    #include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>
    #include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChRegsVer1.h>
    #include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>
    #include <appDemo/sysHwConfig/appDemoDb.h>
    #include <appDemo/sysHwConfig/gtAppDemoSysConfig.h>
    #include <appDemo/boardConfig/gtDbDxBobcat2Mappings.h>
    #include <appDemo/boardConfig/gtDbDxBobcat2PhyConfig.h>
    #include <appDemo/boardConfig/gtDbDxBobcat2GenUtils.h>


    #include <gtOs/gtOsTask.h>
    #include <gtOs/gtOsMsgQ.h>
    #include <gtOs/gtOsSem.h>

    typedef GT_MSGQ_ID xQueueHandle;
    typedef GT_TASK    xTaskHandle;
    typedef GT_U32     UBaseType_t;
    typedef unsigned (__TASKCONV *TaskFunction_t)(GT_VOID*);

    typedef GT_STATUS portBASE_TYPE;

    #define pdFALSE 0
    #define pdTRUE  1


    typedef portBASE_TYPE (*pdCOMMAND_LINE_CALLBACK)( GT_CHAR *pcWriteBuffer, size_t xWriteBufferLen, const GT_CHAR * pcCommandString );

    typedef struct xCOMMAND_LINE_INPUT
    {
        const GT_CHAR * const pcCommand;            /* The command that causes pxCommandInterpreter to be executed.  For example "help".  Must be all lower case. */
        const GT_CHAR * const pcHelpString;         /* String that describes how to use the command.  Should start with the command itself, and end with "\r\n".  For example "help: Returns a list of all the commands\r\n". */
        const pdCOMMAND_LINE_CALLBACK pxCommandInterpreter;	/* A pointer to the callback function that will return the output generated by the command. */
        GT_U8 cExpectedNumberOfParameters;        /* Commands expect a fixed number of parameters, which may be zero. */
    } CLI_Command_Definition_t;

#endif

#if (TARGET == CM3)
    #include <FreeRTOS.h>
    #include <task.h>
    #include <queue.h>
    #include <semphr.h>
    /* Demo app includes. */
    #include <portmacro.h>
    #include <hw.h>
    #include <ic.h>
    #include <uart.h>
    #include <msys/sysmap.h>
    #include "cli.h"
    #include "printf.h"
    #include <stdint.h>
    #include <string.h>
    #include <FreeRTOS_CLI.h>
    #include <mvHwsPortCtrlInc.h>
	
    #define cpssOsPrintf printf
#endif


typedef enum 
{
     DYN_THRESH_Task_EVENT_PROFILE_UPDATE_E = 0
    ,DYN_THRESH_Task_EVENT_START_E
    ,DYN_THRESH_Task_EVENT_THRESH_UPDATE_TIMEOUT_E
    ,DYN_THRESH_Task_EVENT_CHANGE_UPDATE_PERIOD_E
    ,DYN_THRESH_Task_EVENT_STOP_E
    ,DYN_THRESH_Task_EVENT_PRINT_PROFILE_E
    ,DYN_THRESH_Task_EVENT_PRINT_LOG_E
    ,DYN_THRESH_Task_EVENT_MAX_E                 /* never cross this boundary */
}DYN_THRESH_TASK_EVENT_ENT;

typedef GT_U16  DYN_THRESH_PROFILE_ID;
typedef GT_U16  DYN_THRESH_THRESHOLD_ID;
typedef GT_U16  DYN_THRESHOLD_GARANTEE_TH;
typedef GT_U16  DYN_THRESHOLD_GREEDINESS_p8q8;
typedef GT_U32  DYN_THRESH_UPDATE_TIME_uSec;
typedef GT_U32  DYN_THRESH_UPDATE_VALUE;

#define DYN_THRESH_BAD_PROFILE_ID_CNS  (DYN_THRESH_PROFILE_ID)(~0)


#define DYN_THRESH_QUEUE_NUM_CNS           8
#define DYN_THRESH_DROP_PRECEDENCE_NUM_CNS 3
#define DYN_THRESH_TOTAL_THREHSHOLDS_CNS   (DYN_THRESH_QUEUE_NUM_CNS * DYN_THRESH_DROP_PRECEDENCE_NUM_CNS + 1)


#define DYN_THRESH_TOTAL_PROFILE_CNS       16


typedef struct 
{
    DYN_THRESH_PROFILE_ID        profileId;
    GT_U32                       MaxBufferThr;
    DYN_THRESH_UPDATE_TIME_uSec  updatePerioud_uSec;
}DYN_THRESH_START_STC;

typedef struct 
{
    DYN_THRESH_UPDATE_TIME_uSec  updatePerioud_uSec;
}DYN_THRESH_CHANGE_UPDATE_PERIOD_STC;


typedef struct 
{
    DYN_THRESH_PROFILE_ID         profileId;
    DYN_THRESH_THRESHOLD_ID       thresholdId;
    DYN_THRESHOLD_GARANTEE_TH     guaranteeThreshold;
    DYN_THRESHOLD_GREEDINESS_p8q8 greedinessWeight_p8q8;    /* 8 bit integer part , 8 bit -- fraction */
}DYN_THRESH_PROFILE_UPDATE_STC;

typedef struct 
{
    DYN_THRESH_PROFILE_ID        profileId;
}DYN_THRESH_PRINT_PROFILE_STC;


typedef union 
{
    DYN_THRESH_PROFILE_UPDATE_STC         profileUpdate;
    DYN_THRESH_START_STC                  start;
    DYN_THRESH_CHANGE_UPDATE_PERIOD_STC   updatePeriod;
    DYN_THRESH_PRINT_PROFILE_STC          profilePrint;
}DYN_THRESH_USER_DATA_STC;

typedef struct
{
    DYN_THRESH_TASK_EVENT_ENT ev;
    DYN_THRESH_USER_DATA_STC  user_data;
}DYN_THRESH_Task_Msg_STC;

#define DYN_THRESH_Task_MsgQ_SIZE 200



/* used for quick access to event processing function */
typedef GT_STATUS (*DYN_THRESH_TASK_EV_PROC)
(
    DYN_THRESH_USER_DATA_STC  *msgDataPtr
);

typedef struct 
{
    DYN_THRESH_TASK_EVENT_ENT ev;
    DYN_THRESH_TASK_EV_PROC   proc;
}DYN_THRESH_TASK_EV_2_PROC_REL_STC;

typedef struct 
{
    DYN_THRESH_TASK_EVENT_ENT ev;
    GT_CHAR                   *str;
}DYN_THRESH_EV_STR_STC;


typedef struct 
{
    DYN_THRESH_THRESHOLD_ID firstThresh;
    DYN_THRESH_THRESHOLD_ID lastThresh;
    GT_STATUS               (*threshUpdateFun)(DYN_THRESH_PROFILE_ID profileId, GT_U32 queueId, GT_U32 value);
}DYN_THRESH_PROFILE_THRESH_x_UPDATE_FUN_STC;

/* data base */
typedef struct 
{
    DYN_THRESH_PROFILE_ID profileId;
    DYN_THRESHOLD_GARANTEE_TH      guaranteeThreshold   [DYN_THRESH_TOTAL_THREHSHOLDS_CNS];
    DYN_THRESHOLD_GREEDINESS_p8q8  greedinessWeight_p8q8[DYN_THRESH_TOTAL_THREHSHOLDS_CNS];
}DYN_THRESH_PROFILE_PARAMS_STC;

#define DYN_THRESH_PROFILE_N_CNS 16


typedef struct 
{
    DYN_THRESH_PROFILE_PARAMS_STC      dynThreshProfileParams[DYN_THRESH_PROFILE_N_CNS];
}AN_WA_PP_CONFIG_PARAMS_STC;

typedef struct 
{
    DYN_THRESH_PROFILE_ID          selectedProfileId;
    DYN_THRESH_UPDATE_TIME_uSec    updatePerioud_uSec;
    GT_U32                         MaxBufferThr;
    GT_U32                         regArr[DYN_THRESH_TOTAL_THREHSHOLDS_CNS];
}DYN_THRESH_OPER_PARAMS_STC;

#define DYN_TRESH_HISTORY_SIZE 5
typedef struct 
{
    GT_U16 lastIdx;
    GT_U16 len;
    GT_U32                  allocatedBuffersHist     [DYN_TRESH_HISTORY_SIZE];
    DYN_THRESH_UPDATE_VALUE thresholdUpdatedValueHist[DYN_TRESH_HISTORY_SIZE][DYN_THRESH_TOTAL_THREHSHOLDS_CNS];
}DYN_THRESH_HISTORY_STC; 


typedef enum 
{
     DYN_THRESH_STATE_IDLE_E = 0
    ,DYN_THRESH_STATE_WORK_E
    ,DYN_THRESH_STATE_MAX_E        /* don't pass the boundary */
}DYN_THRESH_STATE_ENT; 

typedef struct
{
    GT_U8                          devNum;
    GT_BOOL                        taskCreated;
    xQueueHandle                   msgQId;
    xTaskHandle                   taskId;
#if (BC3_DYN_THRESH_TASK_UNIT_TEST == 1)
    GT_SEM                     semId;
#endif
    DYN_THRESH_STATE_ENT           state;
    DYN_THRESH_TASK_EV_PROC        eventProcArr[DYN_THRESH_Task_EVENT_MAX_E];
    GT_CHAR                       *eventStrArr [DYN_THRESH_Task_EVENT_MAX_E];
    GT_BOOL                        TASK_DEBUG_PRINT;
    DYN_THRESH_PROFILE_PARAMS_STC  dynTheshProfileParams[DYN_THRESH_TOTAL_PROFILE_CNS];
    GT_STATUS                      (*threshUpdateFunArr[DYN_THRESH_TOTAL_THREHSHOLDS_CNS])(DYN_THRESH_PROFILE_ID profileId, GT_U32 queueId, GT_U32 value);
    DYN_THRESH_OPER_PARAMS_STC     operatParams;
    DYN_THRESH_HISTORY_STC         history;
}DYN_THRESH_Task_STC;

/*-------------------------------------------------------*
 *  DynThresh Timer Task                                 *
 *-------------------------------------------------------*/

typedef struct
{
    DYN_THRESH_UPDATE_TIME_uSec timeout;
}DYN_THRESH_Task_Timer_Msg_STC;

#define DYN_THRESH_Task_TIMER_MsgQ_SIZE 20


extern DYN_THRESH_Task_STC        g_dynThreshTaskData;

/*---------------------------------------------------------------------------------------------------------*
 * interface to task                                                                                       *
 *    - DYN_THRESH_Init()                                                                                  *
 *    - appDemoDynThreshProfileUpdateMsgSend    (profileId,thresholdId,guaranteeThreshold,greedinessWeight_p8q8)
 *    - appDemoDynThreshProfileUpdateExtMsgSend(profileId,thresholdId,guaranteeThreshold,greedinessWeight_intPart, greedinessWeight_fracPart)
 *    - appDemoDynThreshStartMsgSend             (profileId,MaxBufferThr,updatePerioud_uSec)
 *
 *---------------------------------------------------------------------------------------------------------*/
GT_STATUS appDemoDynThreshInit
(
    GT_VOID
);

GT_STATUS appDemoDynThreshProfileUpdateMsgSend
(
    DYN_THRESH_PROFILE_ID         profileId,
    DYN_THRESH_THRESHOLD_ID       thresholdId,
    DYN_THRESHOLD_GARANTEE_TH     guaranteeThreshold,
    DYN_THRESHOLD_GREEDINESS_p8q8 greedinessWeight_p8q8
);

GT_STATUS appDemoDynThreshProfileUpdateExtMsgSend
(
    DYN_THRESH_PROFILE_ID         profileId,
    DYN_THRESH_THRESHOLD_ID       thresholdId,
    DYN_THRESHOLD_GARANTEE_TH     guaranteeThreshold,
    GT_U32                        greedinessWeight_intPart,
    GT_U32                        greedinessWeight_fracPart
);

GT_STATUS appDemoDynThreshStartMsgSend
(
    DYN_THRESH_PROFILE_ID        profileId,
    GT_U32                       MaxBufferThr,
    DYN_THRESH_UPDATE_TIME_uSec  updatePerioud_uSec
);

GT_STATUS appDemoDynThreshUpdatePeriodMsgSend
(
    DYN_THRESH_UPDATE_TIME_uSec  updatePerioud_uSec
);

GT_STATUS appDemoDynThreshUpdateTimeoutMsgSend
(
    GT_VOID
);

GT_STATUS appDemoDynThreshStopMsgSend
(
    GT_VOID
);

GT_STATUS appDemoDynThreshProfilePrintMsgSend
(
    DYN_THRESH_PROFILE_ID        profileId
);

GT_STATUS appDemoDynThreshLogPrintMsgSend
(
    GT_VOID
);

#if (TARGET == CM3)
    extern const CLI_Command_Definition_t dyThreshProfileUpdate_Command;
    extern const CLI_Command_Definition_t dyThreshProfileUpdateExt_Command;
    extern const CLI_Command_Definition_t dyThreshStart_Command;
    extern const CLI_Command_Definition_t dyThreshStop_Command;
    extern const CLI_Command_Definition_t dyThreshPrintProfile_Command;
    extern const CLI_Command_Definition_t dyThreshPrintLog_Command;
#endif

#ifdef __cplusplus
}
#endif /* __cplusplus */


#endif



