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

/*--------------------------------------------*
 * procedures that sends message to task
 *        AN Enable
 *        AN Params Set
 *        AN Intr SyncChange
 *        AN Intr AN_Complete
 *        AN Disable
 *--------------------------------------------*/

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

extern GT_STATUS cpssInitSystem
(
    IN  GT_U32  boardIdx,
    IN  GT_U32  boardRevId,
    IN  GT_U32  reloadEeprom
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#if (BC3_DYN_THRESH_TASK_TEST_CNS == 1)

    #if (BC3_DYN_THRESH_TASK_UNIT_TEST == 1)

        GT_STATUS DYN_THRESH_UnitTest()
        {
            GT_STATUS rc;
            DYN_THRESH_PROFILE_PARAMS_STC profile;
            DYN_THRESH_THRESHOLD_ID thresholdId;


            rc = appDemoDbEntryAdd("initSerdesDefaults", 0);
            if (rc != GT_OK)
            {
                return rc;
            }


            rc = cpssInitSystem(29,1,0);
            if (rc != GT_OK)
            {
                return rc;
            }

            rc = DYN_THRESH_Init();
            if (rc != GT_OK)
            {
                return rc;
            }

            g_dynThreshTaskData.TASK_DEBUG_PRINT = GT_TRUE;

            cpssOsMemSet(&profile,0,sizeof(profile));
            profile.profileId = 0;
            profile.guaranteeThreshold[0] = 100;
            profile.greedinessWeight_p8q8  [0] = 2*256;
            profile.guaranteeThreshold[1] = 100;
            profile.greedinessWeight_p8q8  [1] = 4*256;
            profile.guaranteeThreshold[2] = 100;
            profile.greedinessWeight_p8q8  [2] = 256/2;

            for (thresholdId = 0 ; thresholdId < 3; thresholdId++)
            {
                rc = appDemoDynThreshProfileUpdateMsgSend(profile.profileId,thresholdId,
                                                       profile.guaranteeThreshold[thresholdId],
                                                       profile.greedinessWeight_p8q8[thresholdId]);
                if (rc != GT_OK)
                {
                    return rc;
                }
            }

            rc = appDemoDynThreshProfilePrintMsgSend(0);
            if (rc != GT_OK)
            {
                return rc;
            }

            rc = appDemoDynThreshProfileUpdateMsgSend(0,0,200,4);
            if (rc != GT_OK)
            {
                return rc;
            }

            rc = appDemoDynThreshProfilePrintMsgSend(0);
            if (rc != GT_OK)
            {
                return rc;
            }


            rc = appDemoDynThreshStartMsgSend(10,100,10000);
            if (rc != GT_OK)
            {
                return rc;
            }

            rc = appDemoDynThreshStartMsgSend(0,100,10000);
            if (rc != GT_OK)
            {
                return rc;
            }

            rc = appDemoDynThreshUpdatePeriodMsgSend(20000);
            if (rc != GT_OK)
            {
                return rc;
            }
            rc = appDemoDynThreshStartMsgSend(10,100,1000000);
            if (rc != GT_OK)
            {
                return rc;
            }
            cpssOsTimerWkAfter(10000);
            rc = appDemoDynThreshUpdatePeriodMsgSend(2000000);
            if (rc != GT_OK)
            {
                return rc;
            }
            cpssOsTimerWkAfter(20000);
            appDemoDynThreshStopMsgSend();


            rc = appDemoDynThreshProfilePrintMsgSend(0);
            if (rc != GT_OK)
            {
                return rc;
            }

            rc = appDemoDynThreshLogPrintMsgSend();
            if (rc != GT_OK)
            {
                return rc;
            }


            return GT_OK;
        }
    #endif
#endif


GT_STATUS appDemoDynThreshSysInit(GT_U16 GT, GT_U16 GW)
{
    GT_STATUS rc;
    DYN_THRESH_PROFILE_PARAMS_STC profile;
    DYN_THRESH_THRESHOLD_ID thresholdId;

    rc = appDemoDynThreshInit();
    if (rc != GT_OK)
    {
        return rc;
    }

    g_dynThreshTaskData.TASK_DEBUG_PRINT = GT_TRUE;

    cpssOsMemSet(&profile,0,sizeof(profile));
    profile.profileId = 2;
    profile.guaranteeThreshold   [0] = GT;
    profile.greedinessWeight_p8q8[0] = GW;
    profile.guaranteeThreshold   [1] = GT;
    profile.greedinessWeight_p8q8[1] = GW;

    for (thresholdId = 0 ; thresholdId < 2; thresholdId++)
    {
        rc = appDemoDynThreshProfileUpdateMsgSend(profile.profileId,thresholdId,
                                                profile.guaranteeThreshold[thresholdId],
                                                profile.greedinessWeight_p8q8[thresholdId]);
        if (rc != GT_OK)
        {
            return rc;
        }
    }

    rc = appDemoDynThreshProfilePrintMsgSend(profile.profileId);
    if (rc != GT_OK)
    {
        return rc;
    }
    return GT_OK;
}

GT_STATUS appDemoDynThreshSysRun(GT_U32 totalBuf, GT_U32 updatePer_ms, GT_U32 totalTime_sec)
{
    GT_STATUS rc;

    rc = appDemoDynThreshStartMsgSend(2,totalBuf,(DYN_THRESH_UPDATE_TIME_uSec)(updatePer_ms*1000));
    if (rc != GT_OK)
    {
        return rc;
    }

    if (updatePer_ms > 0)
    {
        if (totalTime_sec > 0)
        {
            cpssOsTimerWkAfter(totalTime_sec*1000);

            rc = appDemoDynThreshStopMsgSend();
            if (rc != GT_OK)
            {
                return rc;
            }


            rc = appDemoDynThreshLogPrintMsgSend();
            if (rc != GT_OK)
            {
                return rc;
            }
        }
    }
    else
    {
        GT_U32 i;
        for (i = 0 ; i < totalTime_sec; i++)
        {
            rc = appDemoDynThreshUpdateTimeoutMsgSend();
            if (rc != GT_OK)
            {
                return rc;
            }
        }
        rc = appDemoDynThreshLogPrintMsgSend();
        if (rc != GT_OK)
        {
            return rc;
        }
    }
    return GT_OK;
}



GT_STATUS appDemoDynThreshSysStop()
{
    GT_STATUS rc;

    rc = appDemoDynThreshStopMsgSend();
    if (rc != GT_OK)
    {
        return rc;
    }


    rc = appDemoDynThreshLogPrintMsgSend();
    if (rc != GT_OK)
    {
        return rc;
    }
    return GT_OK;
}


GT_STATUS DYN_THRESH_UnitTest()
{
    GT_STATUS rc;

    rc = cpssInitSystem(29,1,0);
    if (rc != GT_OK)
    {
        return rc;
    }
    rc = appDemoDynThreshSysInit(100,101);
    if (rc != GT_OK)
    {
        return rc;
    }
    rc = appDemoDynThreshSysRun(1024,0,7);
    if (rc != GT_OK)
    {
        return rc;
    }
    rc = appDemoDynThreshSysStop();
    if (rc != GT_OK)
    {
        return rc;
    }
    return GT_OK;
}



/*------------------------------------------------*
 *  New interface with strings                    *
 *------------------------------------------------*/
extern const CLI_Command_Definition_t dyThreshProfileUpdate_Command;
extern const CLI_Command_Definition_t dyThreshProfileUpdateExt_Command;
extern const CLI_Command_Definition_t dyThreshStart_Command;
extern const CLI_Command_Definition_t dyThreshUpdate_Command;
extern const CLI_Command_Definition_t dyThreshStop_Command;
extern const CLI_Command_Definition_t tgodyThreshPrintProfile_Command;
extern const CLI_Command_Definition_t dyThreshPrintLog_Command;

const CLI_Command_Definition_t * dynThreshArr[] = 
{
     &dyThreshProfileUpdate_Command
    ,&dyThreshProfileUpdateExt_Command
    ,&dyThreshStart_Command
    ,&dyThreshUpdate_Command
    ,&dyThreshStop_Command
    ,&tgodyThreshPrintProfile_Command
    ,&dyThreshPrintLog_Command
    ,(CLI_Command_Definition_t *)NULL
};

static GT_STATUS prvDynThreshCheckToken(GT_CHAR * buf, GT_CHAR * token)
{
    GT_CHAR *bufPtr;
    GT_U32   itemLen;
    GT_32    rc;
    GT_CHAR  word[30];

    bufPtr = osStrChr(buf,' ');
    if (bufPtr == NULL)
    {
        bufPtr = osStrChr(buf,0); /* end of string */
    }
    if (bufPtr != NULL)
    {
        itemLen = (GT_U32)(bufPtr - buf);
        osStrNCpy(&word[0],buf,itemLen);
        word[itemLen] = 0;
        rc = osStrCmp(word,token);
        if (rc == 0)
        {
            return GT_OK;
        }
    }
    return GT_NOT_FOUND;
}


static GT_STATUS DYN_THRESH_SendCommand(GT_CHAR * command)
{
    GT_STATUS rc;
    GT_U32 i;
    static GT_CHAR buf2print[100];

    for (i = 0; dynThreshArr[i] != NULL; i++)
    {
        rc = prvDynThreshCheckToken(command,(GT_CHAR*)dynThreshArr[i]->pcCommand);
        if (rc == GT_OK)
        {
            dynThreshArr[i]->pxCommandInterpreter(&buf2print[0],100,command);
            cpssOsPrintf("\nCommand : %s",buf2print);
            return GT_OK;
        }
    }
    return GT_NOT_FOUND;
}

static GT_STATUS DYN_THRESH_PROFILE_UPDATE_StrMsgSend
(
    DYN_THRESH_PROFILE_ID         profileId,
    DYN_THRESH_THRESHOLD_ID       thresholdId,
    DYN_THRESHOLD_GARANTEE_TH     guaranteeThreshold,
    DYN_THRESHOLD_GREEDINESS_p8q8 greedinessWeight_p8q8
)
{
    GT_CHAR cmd[100];

    osSprintf(&cmd[0],"dtpu %d %d %d %d",profileId,thresholdId,guaranteeThreshold,greedinessWeight_p8q8);
    return DYN_THRESH_SendCommand(&cmd[0]);
}


GT_STATUS appDemoDynThreshStartStrMsgSend
(
    DYN_THRESH_PROFILE_ID        profileId,
    GT_U32                       MaxBufferThr,
    DYN_THRESH_UPDATE_TIME_uSec  updatePerioud_uSec
)
{
    GT_CHAR cmd[100];

    osSprintf(&cmd[0],"dtstart %d %d %d %d",profileId,MaxBufferThr,updatePerioud_uSec);
    return DYN_THRESH_SendCommand(&cmd[0]);

}


GT_STATUS appDemoDynThreshUpdateTimeoutStrMsgSend
(
    GT_VOID
)
{
    GT_CHAR cmd[100];

    osSprintf(&cmd[0],"dtupdate");
    return DYN_THRESH_SendCommand(&cmd[0]);
}


GT_STATUS appDemoDynThreshStopStrMsgSend
(
    GT_VOID
)
{
    GT_CHAR cmd[100];

    osSprintf(&cmd[0],"dtstop");
    return DYN_THRESH_SendCommand(&cmd[0]);
}

GT_STATUS appDemoDynThreshProfilePrintStrMsgSend
(
    DYN_THRESH_PROFILE_ID        profileId
)
{
    GT_CHAR cmd[100];

    osSprintf(&cmd[0],"dtprofprint %d",profileId);
    return DYN_THRESH_SendCommand(&cmd[0]);
}

GT_STATUS appDemoDynThreshLogPrintStrMsgSend
(
    GT_VOID
)
{
    GT_CHAR cmd[100];

    osSprintf(&cmd[0],"dtlogprint");
    return DYN_THRESH_SendCommand(&cmd[0]);
}


/*----------------------------------------------------*/
GT_STATUS appDemoDynThreshSysInitStr(GT_U16 GT, GT_U16 GW)
{
    GT_STATUS rc;
    DYN_THRESH_PROFILE_PARAMS_STC profile;
    DYN_THRESH_THRESHOLD_ID thresholdId;

    rc = appDemoDynThreshInit();
    if (rc != GT_OK)
    {
        return rc;
    }

    g_dynThreshTaskData.TASK_DEBUG_PRINT = GT_TRUE;

    cpssOsMemSet(&profile,0,sizeof(profile));
    profile.profileId = 2;
    profile.guaranteeThreshold   [0] = GT;
    profile.greedinessWeight_p8q8[0] = GW;
    profile.guaranteeThreshold   [1] = GT;
    profile.greedinessWeight_p8q8[1] = GW;

    for (thresholdId = 0 ; thresholdId < 2; thresholdId++)
    {
        rc = DYN_THRESH_PROFILE_UPDATE_StrMsgSend(profile.profileId,thresholdId,
                                                  profile.guaranteeThreshold[thresholdId],
                                                  profile.greedinessWeight_p8q8[thresholdId]);
        if (rc != GT_OK)
        {
            return rc;
        }
    }

    rc = appDemoDynThreshProfilePrintStrMsgSend(profile.profileId);
    if (rc != GT_OK)
    {
        return rc;
    }
    return GT_OK;
}


GT_STATUS appDemoDynThreshSysRunStr(GT_U32 totalBuf, GT_U32 updatePer_ms, GT_U32 totalTime_sec)
{
    GT_STATUS rc;

    rc = appDemoDynThreshStartStrMsgSend(2,totalBuf,(DYN_THRESH_UPDATE_TIME_uSec)(updatePer_ms*1000));
    if (rc != GT_OK)
    {
        return rc;
    }

    if (updatePer_ms > 0)
    {
        if (totalTime_sec > 0)
        {
            cpssOsTimerWkAfter(totalTime_sec*1000);

            rc = appDemoDynThreshStopStrMsgSend();
            if (rc != GT_OK)
            {
                return rc;
            }


            rc = appDemoDynThreshLogPrintStrMsgSend();
            if (rc != GT_OK)
            {
                return rc;
            }
        }
    }
    else
    {
        GT_U32 i;
        for (i = 0 ; i < totalTime_sec; i++)
        {
            rc = appDemoDynThreshUpdateTimeoutStrMsgSend();
            if (rc != GT_OK)
            {
                return rc;
            }
        }
        rc = appDemoDynThreshLogPrintStrMsgSend();
        if (rc != GT_OK)
        {
            return rc;
        }
    }
    return GT_OK;
}

GT_STATUS appDemoDynThreshSysStopStr()
{
    GT_STATUS rc;

    rc = appDemoDynThreshStopStrMsgSend();
    if (rc != GT_OK)
    {
        return rc;
    }


    rc = appDemoDynThreshLogPrintStrMsgSend();
    if (rc != GT_OK)
    {
        return rc;
    }
    return GT_OK;
}


GT_STATUS appDemoDynThreshLogPrintStrMsgSendStr()
{
    GT_STATUS rc;

    rc = cpssInitSystem(29,1,0);
    if (rc != GT_OK)
    {
        return rc;
    }
    rc = appDemoDynThreshSysInitStr(100,101);
    if (rc != GT_OK)
    {
        return rc;
    }
    rc = appDemoDynThreshSysRunStr(1024,0,7);
    if (rc != GT_OK)
    {
        return rc;
    }
    rc = appDemoDynThreshSysStop();
    if (rc != GT_OK)
    {
        return rc;
    }
    return GT_OK;
}


