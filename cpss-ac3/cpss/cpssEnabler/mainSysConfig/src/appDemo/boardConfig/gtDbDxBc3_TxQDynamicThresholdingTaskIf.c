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



/*------------------------------------------------*
 * For debug purposse                             *
 *------------------------------------------------*/
GT_STATUS DYN_THRESH_Task_StatusSet(GT_BOOL isEnabled)
{
    g_dynThreshTaskData.taskCreated = isEnabled;
    return GT_OK;
}

/*--------------------------------------------*
 * procedures that sends message to task
 *        AN Enable
 *        AN Params Set
 *        AN Intr SyncChange
 *        AN Intr AN_Complete
 *        AN Disable
 *--------------------------------------------*/

static GT_VOID prvDynThreshTask_MsgInit
(
    DYN_THRESH_Task_Msg_STC *msgPtr,
    DYN_THRESH_TASK_EVENT_ENT ev
)
{
    cpssOsMemSet(msgPtr,0,sizeof(*msgPtr));
    msgPtr->ev        = ev;
}

static GT_STATUS prvDynThreshTask_MsgSend
(
    DYN_THRESH_Task_STC     *dynThreshTaskDataPtr,
    DYN_THRESH_Task_Msg_STC *msgPtr
)
{
    GT_STATUS  rc;
    rc = osMsgQSend(dynThreshTaskDataPtr->msgQId,msgPtr,sizeof(*msgPtr),OS_MSGQ_WAIT_FOREVER);
    if (rc != GT_OK)
    {
        return rc;
    }
    return GT_OK;
}

GT_STATUS appDemoDynThreshProfileUpdateMsgSend
(
    DYN_THRESH_PROFILE_ID         profileId,
    DYN_THRESH_THRESHOLD_ID       thresholdId,
    DYN_THRESHOLD_GARANTEE_TH     guaranteeThreshold,
    DYN_THRESHOLD_GREEDINESS_p8q8 greedinessWeight_p8q8
)
{
    GT_STATUS rc;
    DYN_THRESH_Task_Msg_STC msg;


    prvDynThreshTask_MsgInit(&msg,DYN_THRESH_Task_EVENT_PROFILE_UPDATE_E);
    msg.user_data.profileUpdate.profileId = profileId;
    msg.user_data.profileUpdate.thresholdId = thresholdId;
    msg.user_data.profileUpdate.guaranteeThreshold = guaranteeThreshold;
    msg.user_data.profileUpdate.greedinessWeight_p8q8   = greedinessWeight_p8q8;

    rc = prvDynThreshTask_MsgSend(&g_dynThreshTaskData,&msg);
    if (rc != GT_OK)
    {
        return rc;
    }
    return GT_OK;
}


GT_STATUS appDemoDynThreshProfileUpdateExtMsgSend
(
    DYN_THRESH_PROFILE_ID         profileId,
    DYN_THRESH_THRESHOLD_ID       thresholdId,
    DYN_THRESHOLD_GARANTEE_TH     guaranteeThreshold,
    GT_U32                        greedinessWeight_intPart,
    GT_U32                        greedinessWeight_fracPart
)
{
    GT_STATUS rc;
    DYN_THRESHOLD_GREEDINESS_p8q8 greediness_q8p8;

    greediness_q8p8 = (DYN_THRESHOLD_GREEDINESS_p8q8)((greedinessWeight_intPart&0xFF) << 8 | (greedinessWeight_fracPart & 0xFF));
    rc = appDemoDynThreshProfileUpdateMsgSend(profileId,thresholdId,guaranteeThreshold,greediness_q8p8);
    if (rc != GT_OK)
    {
        return rc;
    }
    return GT_OK;
}


GT_STATUS appDemoDynThreshStartMsgSend
(
    DYN_THRESH_PROFILE_ID        profileId,
    GT_U32                       MaxBufferThr,
    DYN_THRESH_UPDATE_TIME_uSec  updatePerioud_uSec
)
{
    GT_STATUS rc;
    DYN_THRESH_Task_Msg_STC msg;

    prvDynThreshTask_MsgInit(&msg,DYN_THRESH_Task_EVENT_START_E);
    msg.user_data.start.profileId          = profileId;
    msg.user_data.start.MaxBufferThr       = MaxBufferThr;
    msg.user_data.start.updatePerioud_uSec = updatePerioud_uSec;

    rc = prvDynThreshTask_MsgSend(&g_dynThreshTaskData,&msg);
    if (rc != GT_OK)
    {
        return rc;
    }
    return GT_OK;
}


GT_STATUS appDemoDynThreshUpdatePeriodMsgSend
(
    DYN_THRESH_UPDATE_TIME_uSec  updatePerioud_uSec
)
{
    GT_STATUS rc;
    DYN_THRESH_Task_Msg_STC msg;

    prvDynThreshTask_MsgInit(&msg,DYN_THRESH_Task_EVENT_CHANGE_UPDATE_PERIOD_E);
    msg.user_data.updatePeriod.updatePerioud_uSec = updatePerioud_uSec;

    rc = prvDynThreshTask_MsgSend(&g_dynThreshTaskData,&msg);
    if (rc != GT_OK)
    {
        return rc;
    }
    return GT_OK;
}


GT_STATUS appDemoDynThreshUpdateTimeoutMsgSend
(
    GT_VOID
)
{
    GT_STATUS rc;
    DYN_THRESH_Task_Msg_STC msg;

    prvDynThreshTask_MsgInit(&msg,DYN_THRESH_Task_EVENT_THRESH_UPDATE_TIMEOUT_E);

    rc = prvDynThreshTask_MsgSend(&g_dynThreshTaskData,&msg);
    if (rc != GT_OK)
    {
        return rc;
    }
    return GT_OK;
}

GT_STATUS appDemoDynThreshStopMsgSend
(
    GT_VOID
)
{
    GT_STATUS rc;
    DYN_THRESH_Task_Msg_STC msg;

    prvDynThreshTask_MsgInit(&msg,DYN_THRESH_Task_EVENT_STOP_E);

    rc = prvDynThreshTask_MsgSend(&g_dynThreshTaskData,&msg);
    if (rc != GT_OK)
    {
        return rc;
    }
    return GT_OK;
}

GT_STATUS appDemoDynThreshProfilePrintMsgSend
(
    DYN_THRESH_PROFILE_ID        profileId
)
{
    GT_STATUS rc;
    DYN_THRESH_Task_Msg_STC msg;

    prvDynThreshTask_MsgInit(&msg,DYN_THRESH_Task_EVENT_PRINT_PROFILE_E);

    msg.user_data.profilePrint.profileId = profileId;
    rc = prvDynThreshTask_MsgSend(&g_dynThreshTaskData,&msg);
    if (rc != GT_OK)
    {
        return rc;
    }
    return GT_OK;
}

GT_STATUS appDemoDynThreshLogPrintMsgSend
(
    GT_VOID
)
{
    GT_STATUS rc;
    DYN_THRESH_Task_Msg_STC msg;

    prvDynThreshTask_MsgInit(&msg,DYN_THRESH_Task_EVENT_PRINT_LOG_E);

    rc = prvDynThreshTask_MsgSend(&g_dynThreshTaskData,&msg);
    if (rc != GT_OK)
    {
        return rc;
    }
    return GT_OK;
}

/*--------------------------------------*
    *    CLI interface to CM3              *
    *--------------------------------------*/

/*---------------------------------------------------------------- 
    *   update profile          : dtpu <profileid> <thresholdId> <guaranteeThreshiold> <greedinessWeight_p8q8>
    *   update profile extended : dtpuext <profileid> <thresholdId> <guaranteeThreshiold> <greedinessWeight_Int> <greedinessWeight_Frac>
    *   start thresholding      : dtstart <profileid> <maxBuf> <updatePerioud_usec>
    *   stop thresholding       : dtstop
    *   print profile           : dtprofprint <profileid>
    *   print log               : dtlogprint
    *---------------------------------------------------------------- */ 

static GT_CHAR *getToken(GT_CHAR * buf, GT_CHAR * token)
{
    GT_CHAR *bufPtr;
    GT_U32   len;
    GT_32    rc;

    bufPtr = osStrChr(buf,' ');
    if (bufPtr == NULL)
    {
        bufPtr = osStrChr(buf,0); /* search for end-of-string */
    }
    if (bufPtr != NULL)
    {
        len = (GT_U32)(bufPtr - buf);
        rc = osStrNCmp(buf,token,len);
        if (rc == 0)
        {
            return bufPtr+1;
        }
    }
    return (GT_CHAR *)NULL;
}


portBASE_TYPE prvDynThreshUpdateProfileCommand
( 
    GT_CHAR *pcWriteBuffer,
    size_t xWriteBufferLen, 
    const GT_CHAR *pcCommandString 
)
{
    GT_STATUS rc;
    DYN_THRESH_PROFILE_ID         profileId;
    DYN_THRESH_THRESHOLD_ID       thresholdId;
    DYN_THRESHOLD_GARANTEE_TH     guaranteeThreshold;
    DYN_THRESHOLD_GREEDINESS_p8q8 greedinessWeight_p8q8;
    GT_CHAR *ptr;


    ptr = getToken((GT_CHAR*)pcCommandString,"dtpu");
    if (ptr == NULL)
    {
        osSprintf(pcWriteBuffer, "unknown command : %s\n", pcCommandString );
        return pdFALSE;
    }

    profileId             = (DYN_THRESH_PROFILE_ID        ) osStrToU32( (const char * )ptr, &ptr, 0 );
    thresholdId           = (DYN_THRESH_THRESHOLD_ID      ) osStrToU32(( const char * )ptr, &ptr, 0 );
    guaranteeThreshold    = (DYN_THRESHOLD_GARANTEE_TH    ) osStrToU32( (const char * )ptr, &ptr, 0 );
    greedinessWeight_p8q8 = (DYN_THRESHOLD_GREEDINESS_p8q8) osStrToU32(( const char * )ptr, &ptr, 0 );



    rc =  appDemoDynThreshProfileUpdateMsgSend(profileId,thresholdId,guaranteeThreshold,greedinessWeight_p8q8);
    if (rc != GT_OK)
    {
        osSprintf(pcWriteBuffer, "\nERROR sending message %s\n");
        return pdFALSE;
    }

    osSprintf(pcWriteBuffer, "%s ... OK\n", pcCommandString );
    return pdFALSE;
}

portBASE_TYPE prvDynThreshUpdateProfileExtCommand
( 
    GT_CHAR *pcWriteBuffer,
    size_t xWriteBufferLen, 
    const GT_CHAR *pcCommandString 
)
{
    GT_STATUS rc;
    DYN_THRESH_PROFILE_ID         profileId;
    DYN_THRESH_THRESHOLD_ID       thresholdId;
    DYN_THRESHOLD_GARANTEE_TH     guaranteeThreshold;
    GT_U32                        greedinessWeight_intPart;
    GT_U32                        greedinessWeight_fracPart;
    GT_CHAR *ptr;

    ptr = getToken((GT_CHAR*)pcCommandString,"dtpuext");
    if (ptr == NULL)
    {
        osSprintf(pcWriteBuffer, "unknown command : %s\n", pcCommandString );
        return pdFALSE;
    }

    profileId                = (DYN_THRESH_PROFILE_ID        ) osStrToU32( (const char * )ptr, &ptr, 0 );
    thresholdId              = (DYN_THRESH_THRESHOLD_ID      ) osStrToU32(( const char * )ptr, &ptr, 0 );
    guaranteeThreshold       = (DYN_THRESHOLD_GARANTEE_TH    ) osStrToU32( (const char * )ptr, &ptr, 0 );
    greedinessWeight_intPart = osStrToU32(( const char * )ptr, &ptr, 0 );
    greedinessWeight_fracPart = osStrToU32(( const char * )ptr, &ptr, 0 );


    rc =  appDemoDynThreshProfileUpdateExtMsgSend(profileId,thresholdId,guaranteeThreshold,greedinessWeight_intPart,greedinessWeight_fracPart);
    if (rc != GT_OK)
    {
        osSprintf(pcWriteBuffer, "\nERROR sending message %s\n");
        return pdFALSE;
    }

    osSprintf(pcWriteBuffer, "%s ... OK\n", pcCommandString );
    return pdFALSE;
}

portBASE_TYPE prvDynThreshStart
( 
    GT_CHAR *pcWriteBuffer,
    size_t xWriteBufferLen, 
    const GT_CHAR *pcCommandString 
)
{
    GT_STATUS rc;
    GT_CHAR *ptr;
    DYN_THRESH_PROFILE_ID        profileId;
    GT_U32                       MaxBufferThr;
    DYN_THRESH_UPDATE_TIME_uSec  updatePerioud_uSec;

    ptr = getToken((GT_CHAR*)pcCommandString,"dtstart");
    if (ptr == NULL)
    {
        osSprintf(pcWriteBuffer, "unknown command : %s\n", pcCommandString );
        return pdFALSE;
    }

    profileId                = (DYN_THRESH_PROFILE_ID        ) osStrToU32( (const char * )ptr, &ptr, 0 );
    MaxBufferThr             =                                 osStrToU32(( const char * )ptr, &ptr, 0 );
    updatePerioud_uSec       = (DYN_THRESH_UPDATE_TIME_uSec  ) osStrToU32( (const char * )ptr, &ptr, 0 );


    rc =  appDemoDynThreshStartMsgSend(profileId,MaxBufferThr,updatePerioud_uSec);
    if (rc != GT_OK)
    {
        osSprintf(pcWriteBuffer, "\nERROR sending message %s\n");
        return pdFALSE;
    }

    osSprintf(pcWriteBuffer, "%s ... OK\n", pcCommandString );
    return pdFALSE;
}


portBASE_TYPE prvDynThreshUpdate
( 
    GT_CHAR *pcWriteBuffer,
    size_t xWriteBufferLen, 
    const GT_CHAR *pcCommandString 
)
{
    GT_STATUS rc;
    GT_CHAR *ptr;

    ptr = getToken((GT_CHAR*)pcCommandString,"dtupdate");
    if (ptr == NULL)
    {
        osSprintf(pcWriteBuffer, "unknown command : %s\n", pcCommandString );
        return pdFALSE;
    }

    rc =  appDemoDynThreshUpdateTimeoutMsgSend();
    if (rc != GT_OK)
    {
        osSprintf(pcWriteBuffer, "\nERROR sending message %s\n");
        return pdFALSE;
    }

    osSprintf(pcWriteBuffer, "%s ... OK\n", pcCommandString );
    return pdFALSE;
}


portBASE_TYPE prvDynThreshStop
( 
    GT_CHAR *pcWriteBuffer,
    size_t xWriteBufferLen, 
    const GT_CHAR *pcCommandString 
)
{
    GT_STATUS rc;
    GT_CHAR *ptr;

    ptr = getToken((GT_CHAR*)pcCommandString,"dtstop");
    if (ptr == NULL)
    {
        osSprintf(pcWriteBuffer, "unknown command : %s\n", pcCommandString );
        return pdFALSE;
    }


    rc =  appDemoDynThreshStopMsgSend();
    if (rc != GT_OK)
    {
        osSprintf(pcWriteBuffer, "\nERROR sending message %s\n");
        return pdFALSE;
    }

    osSprintf(pcWriteBuffer, "%s ... OK\n", pcCommandString );
    return pdFALSE;
}

portBASE_TYPE prvDynThreshProfilePrint
( 
    GT_CHAR *pcWriteBuffer,
    size_t xWriteBufferLen, 
    const GT_CHAR *pcCommandString 
)
{
    GT_STATUS rc;
    DYN_THRESH_PROFILE_ID         profileId;
    GT_CHAR *ptr;

    ptr = getToken((GT_CHAR*)pcCommandString,"dtprofprint");
    if (ptr == NULL)
    {
        osSprintf(pcWriteBuffer, "unknown command : %s\n", pcCommandString );
        return pdFALSE;
    }

    profileId                = (DYN_THRESH_PROFILE_ID        ) osStrToU32( (const char * )ptr, &ptr, 0 );


    rc =  appDemoDynThreshProfilePrintMsgSend(profileId);
    if (rc != GT_OK)
    {
        osSprintf(pcWriteBuffer, "\nERROR sending message %s\n");
        return pdFALSE;
    }

    osSprintf(pcWriteBuffer, "%s ... OK\n", pcCommandString );
    return pdFALSE;
}

portBASE_TYPE prvDynThreshLogPrint
(
    GT_CHAR *pcWriteBuffer,
    size_t xWriteBufferLen, 
    const GT_CHAR *pcCommandString 
)
{
    GT_STATUS rc;
    GT_CHAR *ptr;

    ptr = getToken((GT_CHAR*)pcCommandString,"dtlogprint");
    if (ptr == NULL)
    {
        osSprintf(pcWriteBuffer, "unknown command : %s\n", pcCommandString );
        return pdFALSE;
    }


    rc =  appDemoDynThreshLogPrintMsgSend();
    if (rc != GT_OK)
    {
        osSprintf(pcWriteBuffer, "\nERROR sending message %s\n");
        return pdFALSE;
    }

    osSprintf(pcWriteBuffer, "%s ... OK\n", pcCommandString );
    return pdFALSE;
}


const CLI_Command_Definition_t dyThreshProfileUpdate_Command =
{
    ( const GT_CHAR * const ) "dtpu",
    ( const GT_CHAR * const ) "dtpu <profileid> <thresholdId> <guaranteeThreshiold> <greedinessWeight_p8q8>:\n"
            "dynamic threshiolding update profile.\n\n",
    prvDynThreshUpdateProfileCommand,
    4
};

const CLI_Command_Definition_t dyThreshProfileUpdateExt_Command =
{
    ( const GT_CHAR * const ) "dtpuext",
    ( const GT_CHAR * const ) "dtpuext <profileid> <thresholdId> <guaranteeThreshiold> <greedinessWeight_Int> <greedinessWeight_Frac>:\n"
            "dynamic threshiolding update profile extended.\n\n",
    prvDynThreshUpdateProfileExtCommand,
    5
};

const CLI_Command_Definition_t dyThreshStart_Command =
{
    ( const GT_CHAR * const ) "dtstart",
    ( const GT_CHAR * const ) "dtstart <profileid> <maxBuf> <updatePerioud_usec>:\n"
            "dynamic threshiolding start.\n\n",
    prvDynThreshStart,
    3
};

const CLI_Command_Definition_t dyThreshUpdate_Command =
{
    ( const GT_CHAR * const ) "dtupdate",
    ( const GT_CHAR * const ) "dtupdate:\n"
            "dynamic threshiolding update.\n\n",
    prvDynThreshUpdate,
    3
};


const CLI_Command_Definition_t dyThreshStop_Command =
{
    ( const GT_CHAR * const ) "dtstop",
    ( const GT_CHAR * const ) "dtstop:\n"
            "dynamic threshiolding stop.\n\n",
    prvDynThreshStop,
    0
};


const CLI_Command_Definition_t tgodyThreshPrintProfile_Command =
{
    ( const GT_CHAR * const ) "dtprofprint",
    ( const GT_CHAR * const ) "dtprofprint <profileid>:\n"
            "dynamic threshiolding profile print.\n\n",
    prvDynThreshProfilePrint,
    1
};

const CLI_Command_Definition_t dyThreshPrintLog_Command =
{
    ( const GT_CHAR * const ) "dtlogprint",
    ( const GT_CHAR * const ) "dtlogprint:\n"
            "dynamic threshiolding log print.\n\n",
    prvDynThreshLogPrint,
    0
};

#if (TARGET == CM3)

#endif





