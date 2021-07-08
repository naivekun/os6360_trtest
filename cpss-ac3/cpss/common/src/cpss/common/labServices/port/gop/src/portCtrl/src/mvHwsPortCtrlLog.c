/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
*/
/**
********************************************************************************
* @file mvHwsPortCtrlLog.c
*
* @brief Port Control Log
*
* @version   1
********************************************************************************
*/
#include <mvHwsPortCtrlInc.h>

/* Port Control Log */
MV_HWS_PORT_CTRL_LOG_ENTRY portLog[MV_PORT_CTRL_LOG_SIZE];

/* FW Hws Log */
MV_HWS_LOG_ENTRY hwsLog[MV_FW_HWS_LOG_SIZE];

/* Port Control Log pointer */
GT_U32 logPointer;

/* Port Control Log count */
GT_U32 logCount;

/* Port Control Host Log reset */
GT_U32 logReset;

/* Port Control Log reset */
GT_U32 logLocalReset;

/***** HWS Log Parameters ******/
/* Port Control Log pointer */
GT_U32 hwsLogPointer;

/* Port Control Log count */
GT_U32 hwsLogCount;

#ifdef FREE_RTOS_HWS_ENHANCED_PRINT_MODE

/* AP Registers description */
char *regType[PORT_CTRL_MAX_AP_REGS] =
{
    [PORT_CTRL_AP_REG_CFG_0]                                   "Cfg-0",
    [PORT_CTRL_AP_REG_CFG_1]                                   "Cfg-1",
    [PORT_CTRL_AP_REG_ST_0]                                    "St-0",
    [PORT_CTRL_AP_INT_REG_802_3_AP_CTRL]                       "Ctrl",
    [PORT_CTRL_AP_INT_REG_802_3_AP_ST]                         "St",
    [PORT_CTRL_AP_INT_REG_802_3_AP_ADV_REG_1]                  "AdvReg1",
    [PORT_CTRL_AP_INT_REG_802_3_AP_ADV_REG_2]                  "AdvReg2",
    [PORT_CTRL_AP_INT_REG_802_3_AP_ADV_REG_3]                  "AdvReg3",
    [PORT_CTRL_AP_INT_REG_802_3_LP_BASE_ABILITY_REG_1]         "LpReg1",
    [PORT_CTRL_AP_INT_REG_802_3_LP_BASE_ABILITY_REG_2]         "LpReg2",
    [PORT_CTRL_AP_INT_REG_802_3_LP_BASE_ABILITY_REG_3]         "LpReg3",
    [PORT_CTRL_AP_INT_REG_ANEG_CTRL_0]                         "Aneg0",
    [PORT_CTRL_AP_INT_REG_ANEG_CTRL_1]                         "Aneg1",
    [PORT_CTRL_AP_INT_REG_802_3_NEXT_PG_TX_REG]                "NxtPg",
    [PORT_CTRL_AP_INT_REG_802_3_NEXT_PG_TX_CODE_0_15]          "NxtPg-0",
    [PORT_CTRL_AP_INT_REG_802_3_NEXT_PG_TX_CODE_16_31]         "NxtPg-16",
    [PORT_CTRL_AP_INT_REG_802_3_LP_NEXT_PG_ABILITY_REG]        "LpNxtPg",
    [PORT_CTRL_AP_INT_REG_802_3_LP_NEXT_PG_ABILITY_CODE_0_15]  "LpNxtPg-0",
    [PORT_CTRL_AP_INT_REG_802_3_LP_NEXT_PG_ABILITY_CODE_16_31] "LpNxtPg-16",
    [PORT_CTRL_AP_INT_REG_802_3_BP_ETH_ST_REG]                 "BpSt",
    [PORT_CTRL_AP_INT_REG_802_3_LP_ADV_REG]                    "LpAdv",
    [PORT_CTRL_AP_REG_ST_1]                                    "St-1"
};

/* Message Type description */
char *msgType[MV_HWS_IPC_LAST_CTRL_MSG_TYPE] =
{
   [MV_HWS_IPC_PORT_AP_ENABLE_MSG]                    "AP Enable",
   [MV_HWS_IPC_PORT_AP_DISABLE_MSG]                   "AP Disable",
   [MV_HWS_IPC_PORT_AP_SYS_CFG_VALID_MSG]             "AP Sys Cfg",
   [MV_HWS_IPC_PORT_AP_CFG_GET_MSG]                   "AP Cfg Get",
   [MV_HWS_IPC_PORT_AP_STATUS_MSG]                    "AP Status",
   [MV_HWS_IPC_PORT_AP_STATS_MSG]                     "AP Stats",
   [MV_HWS_IPC_PORT_AP_STATS_RESET_MSG]               "AP Reset",
   [MV_HWS_IPC_PORT_AP_INTROP_GET_MSG]                "AP Introp Get",
   [MV_HWS_IPC_PORT_AP_INTROP_SET_MSG]                "AP Introp Set",
   [MV_HWS_IPC_PORT_AP_DEBUG_GET_MSG]                 "AP Debug",
   [MV_HWS_IPC_PORT_PARAMS_MSG]                       "Params Set",
   [MV_HWS_IPC_PORT_AVAGO_SERDES_INIT_MSG]            "Avago Serdes Init",
   [MV_HWS_IPC_PORT_AVAGO_GUI_SET_MSG]                "Avago GUI Set",
   [MV_HWS_IPC_PORT_AVAGO_SERDES_RESET_DATA_MSG]      "Avago Appl Data Reset",
   [MV_HWS_IPC_VOS_OVERRIDE_PARAMS_DATA_MSG]          "VOS Override MSG",
   [MV_HWS_IPC_VOS_OVERRIDE_MODE_SET_MSG]             "VOS Override Mode Set",
   [MV_HWS_IPC_NOOP_MSG]                              "Noop",
   [MV_HWS_IPC_PORT_INIT_MSG]                         "Init",
   [MV_HWS_IPC_PORT_RESET_MSG]                        "Reset",
   [MV_HWS_IPC_PORT_RESET_EXT_MSG]                    "Reset Ext",
   [MV_HWS_IPC_HWS_LOG_GET_MSG]                       "Hws LOG Get",
   [MV_HWS_IPC_PORT_AUTO_TUNE_SET_EXT_MSG]            "Auto Tune Set",
   [MV_HWS_IPC_PORT_AUTO_TUNE_STOP_MSG]               "Auto Tune Stop",
   [MV_HWS_IPC_PORT_AUTO_TUNE_STATE_CHK_MSG]          "Auto Tune Check",
   [MV_HWS_IPC_PORT_LINK_STATUS_GET_MSG]              "Link Status",
   [MV_HWS_IPC_PORT_FEC_CONFIG_GET_MSG]               "Fec Get",
   [MV_HWS_IPC_PORT_TX_ENABLE_GET_MSG]                "Tx Enable Get",
   [MV_HWS_IPC_PORT_SERDES_RESET_MSG]                 "Serdes Reset",
   [MV_HWS_IPC_PORT_POLARITY_SET_MSG]                 "Polarity",
   [MV_HWS_IPC_PORT_FEC_CONFIG_MSG]                   "Fec Config",
   [MV_HWS_IPC_PORT_TX_ENABLE_MSG]                    "Tx Enable",
   [MV_HWS_IPC_PORT_SIGNAL_DETECT_GET_MSG]            "Signal Detect",
   [MV_HWS_IPC_PORT_CDR_LOCK_STATUS_GET_MSG]          "CDR Lock Status",
   [MV_HWS_IPC_PORT_LOOPBACK_SET_MSG]                 "Loopback",
   [MV_HWS_IPC_PORT_LOOPBACK_STATUS_GET_MSG]          "Loopback Status",
   [MV_HWS_IPC_PORT_PPM_SET_MSG]                      "PPM Set",
   [MV_HWS_IPC_PORT_PPM_GET_MSG]                      "PPM Get",
   [MV_HWS_IPC_PORT_IF_GET_MSG]                       "If Get",
   [MV_HWS_IPC_PORT_SERDES_MANUAL_RX_CONFIG_MSG]      "Serdes Man Rx",
   [MV_HWS_IPC_PORT_SERDES_MANUAL_TX_CONFIG_MSG]      "Serdes Man Tx",
   [MV_HWS_IPC_PORT_PCS_ACTIVE_STATUS_GET_CONFIG_MSG] "PCS Act Status",
   [MV_HWS_IPC_PORT_FC_STATE_SET_MSG]                 "FC State Set",
   [MV_HWS_IPC_PORT_ENABLE_CTRL_SET_MSG]              "Port Enable Ctrl Set",
   [MV_HWS_IPC_PORT_ENABLE_CTRL_GET_MSG]              "Port Enable Ctrl Get",
   [MV_HWS_IPC_PORT_SERDES_RX_PARAMETERS_OFFSET_CONFIG_MSG] "Port Serdes TX Parameters Offset Set",
   [MV_HWS_IPC_PORT_SERDES_RX_CONFIG_OVERRIDE_MSG]    "Port Serdes RX Parameters Override Set"

};

/* Port Type description */
char *portType[] =
{
    "Reg",
    "AP"
};

/* Supervisour state description */
char *superState[] =
{
    "High Pri Msg",
    "Low Pri Msg",
    "Delay"
};

/* Port Management state description */
/* Code not relevant for BobK CM3 due to space limit */
char *portMngState[] =
{
    "Msg Process",
    "Delay"
};

/* Port State machine state description */
char *portMngSmState[] =
{
    "Idle",
    "Serdes Config",
    "Serdes Training",
    "Mac/Pcs Config",
    "Active",
    "Delete"
};

/* Port State machine status description */
char *portMngSmStatus[] =
{
    "Not Running",
    "Start Execute",
    "Serdes Config In Progress",
    "Serdes Config Success",
    "Serdes Config Failure",
    "Serdes Training In Progress",
    "Serdes Training Success",
    "Serdes Training Failure",
    "Serdes Training Config Failure",
    "Serdes Training End Wait for Ack",
    "Mac/Pcs Config In Progress",
    "Mac/Pcs Config Success",
    "Mac/Pcs Config Failure",
    "Delete In Progress",
    "Delete Success",
    "Delete Failure",
    "FEC Config In Progress",
    "FEC Config Success",
    "FEC Config Failure",
    "FC State Set In Progress",
    "FC State Set Success",
    "FC State Set Failure",
    "Serdes Training Not Ready",
    "Serdes Polarity Set In Progress",
    "Serdes Polarity Set Success",
    "Serdes Polarity Set Failure",
    "Serdes Training End Received Ack",
    "Port Enable Failure",
    "Serdes Adaptive Training Start Failure",
    "Serdes Enhance Tune Lite Failure",
    "Serdes Enhance Tune Lite In Progress",
    "Serdes Enhance Tune Lite Success",
    "Serdes CTLE or ETL config Failure"

};

/* AP HCD Found description */
char* apPortMngHcdFound[] =
{
    "Not Found",
    "Found"
};

/* AP HCD Type description */
char* apPortMngHcdType[Port_AP_LAST] =
{
    "1000Base KX",
    "10GBase KX4",
    "10GBase KR",
    "25GBASE_KR_S",
    "25GBASE_KR",
    "40GBase KR4",
    "40GBASE_CR4",
    "100GBASE_CR10",
    "100GBASE_KP4",
    "100GBASE_KR4",
    "100GBASE_CR4",
    "25GBASE_KR1_CONSORTIUM",
    "25GBASE_CR1_CONSORTIUM",
    "50GBASE_KR2_CONSORTIUM",
    "50GBASE_CR2_CONSORTIUM"
};

/* AP General filed description */
char* apPortMngGen[] =
{
    "Off",
    "On"
};

/* AP FEC Type description */
char* apPortMngHcdFecType[] =
{
    "NO FEC",
    "FC FEC",
    "RS FEC"
};

#endif /* FREE_RTOS_HWS_ENHANCED_PRINT_MODE */

#ifndef DISABLE_CLI
/* AP Port State machine description */
char *apPortMngSmState[] =
{
    "Idle",
    "Init",
    "Tx Disable",
    "Resolution",
    "Active",
    "Delete",
    "Disable",
    "Max" /* since state machine enum has values 0-6 the description should contain matching list */
};

/* AP Port State machine status description */
char *apPortMngSmStatus[] =
{
    "Start Execute",
    "Serdes Init Failure",
    "Init Success",
    "Tx Disable In Progress",
    "Tx Disable Failure",
    "Tx Disable Success",
    "Resolution In Progress",
    "Parallel Detect Resolution In Progress",
    "Parallel Detect Resolution Failure",
    "AP Resolution In Progress",
    "AP No Resolution",
    "AP Resolution Timer Failure",
    "AP Resolution Max Interval Failure",
    "Resolution Success",
    "Link Check Start",
    "Link Check Validate",
    "Link Check Failure",
    "Link Check Success",
    "Delete In Progress",
    "Delete Success",
    "Unknown HCD Found",
    "AP Resolution check consortium",
    "AP Resolution check ACK2",
    "AP Resolution no more pages",
    "Msg Send Failure"
};
#endif /* DISABLE_CLI */

/**
* @internal mvPortCtrlLogInit function
* @endinternal
*
* @brief   Initialize and clear real time log
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvPortCtrlLogInit(void)
{
    osMemSet(portLog, 0, sizeof(MV_HWS_PORT_CTRL_LOG_ENTRY) * MV_PORT_CTRL_LOG_SIZE);
    logPointer    = 0;
    logCount      = 0;
    logReset      = 0;
    logLocalReset = 0;

    /* Invalidate cache for memory coherence */
    FW_CACHE_INVALIDATE(&portLog[0], sizeof(portLog));
    FW_CACHE_INVALIDATE(&logPointer, (sizeof(logPointer)));
    FW_CACHE_INVALIDATE(&logCount, (sizeof(logCount)));
    FW_CACHE_INVALIDATE(&logReset, (sizeof(logReset)));

    return GT_OK;
}

/**
* @internal mvHwsLogInit function
* @endinternal
*
* @brief   Initialize and clear real time hws log
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsLogInit(void)
{
    GT_U32 idx;

    osMemSet(hwsLog, 0, MV_FW_HWS_LOG_SIZE * sizeof(MV_HWS_LOG_ENTRY));

    /* Initializing every log entry string */
    for (idx=0; idx < MV_FW_HWS_LOG_SIZE; idx++)
    {
        osMemSet(hwsLog[idx].info, '\0', sizeof(hwsLog[0].info));
    }

    hwsLogPointer    = 0;
    hwsLogCount      = 0;

    /* Invalidate cache for memory coherence */
    FW_CACHE_INVALIDATE(&hwsLog[0], sizeof(hwsLog));
    FW_CACHE_INVALIDATE(&hwsLogPointer, (sizeof(hwsLogPointer)));
    FW_CACHE_INVALIDATE(&hwsLogCount, (sizeof(hwsLogCount)));

    return GT_OK;
}

/**
* @internal mvHwsLogInfoGet function
* @endinternal
*
* @brief   Return real time log info
*/
void mvHwsLogInfoGet(GT_U32 *fwBaseAddr,
                          GT_U32 *hwsLogBaseAddr,
                          GT_U32 *hwsLogPointerAddr,
                          GT_U32 *hwsLogCountAddr,
                          GT_U32 *hwsLogResetAddr)
{
    *fwBaseAddr     = LOAD_ADDR;
    *hwsLogBaseAddr    = (GT_U32)hwsLog;
    *hwsLogPointerAddr = (GT_U32)&hwsLogPointer;
    *hwsLogCountAddr   = (GT_U32)&hwsLogCount;
}

/**
* @internal mvPortCtrlLogInfoGet function
* @endinternal
*
* @brief   Return real time log info
*/
void mvPortCtrlLogInfoGet(GT_U32 *fwBaseAddr,
                          GT_U32 *logBaseAddr,
                          GT_U32 *logPointerAddr,
                          GT_U32 *logCountAddr,
                          GT_U32 *logResetAddr)
{
    *fwBaseAddr     = LOAD_ADDR;
    *logBaseAddr    = (GT_U32)portLog;
    *logPointerAddr = (GT_U32)&logPointer;
    *logCountAddr   = (GT_U32)&logCount;
    *logResetAddr   = (GT_U32)&logReset;
}

/**
* @internal mvPortCtrlLogAdd function
* @endinternal
*
* @brief   Add Log entry to the Log and override oldest Log entry in case
*         the Log is full
*/
void mvPortCtrlLogAdd
(
    GT_U32 entry
)
{
    GT_U32 timestamp = mvPortCtrlCurrentTs();

    if (mvPortCtrlLogLock() == GT_OK)
    {
        /* The first time we add a new entry after host reads the log.
           Only those new lines will be availble to read */
        if (logLocalReset < logReset)
        {
            logCount      = 0;
            logLocalReset = logReset;
        }

        portLog[logPointer].info      = entry;
        portLog[logPointer].timestamp = timestamp;

        if (logPointer + 1 >= MV_PORT_CTRL_LOG_SIZE)
        {
            logPointer = 0;
        }
        else
        {
            logPointer++;
        }

        if (logCount + 1 >= MV_PORT_CTRL_LOG_SIZE)
        {
            logCount = MV_PORT_CTRL_LOG_SIZE - 1;
        }
        else
        {
            logCount++;
        }

        /* Invalidate cache for memory coherence */
        FW_CACHE_INVALIDATE(&portLog[0], sizeof(portLog));
        FW_CACHE_INVALIDATE(&logPointer, (sizeof(logPointer)));
        FW_CACHE_INVALIDATE(&logCount, (sizeof(logCount)));
        FW_CACHE_INVALIDATE(&logReset, (sizeof(logReset)));

        mvPortCtrlLogUnlock();
    }
}

/**
* @internal mvHwsLogAdd function
* @endinternal
*
* @brief   Add Log string to the Hws Log and override oldest Log content in
*         case the Log is full
*/
void mvHwsLogAdd
(
    char *msgPtr
)
{
    MV_HWS_LOG_ENTRY entry;

    if (mvPortCtrlLogLock() == GT_OK)
    {
        /* Building the entry */
        entry.timestamp = mvPortCtrlCurrentTs();
        osMemCpy(entry.info, msgPtr, MV_FW_HWS_LOG_STRING_SIZE);

        /* Adding the message entry */
        osMemCpy( &(hwsLog[hwsLogPointer]), &entry, sizeof(MV_HWS_LOG_ENTRY));

        /* Cyclic pointer behaviour */
        if (hwsLogPointer+1 >= MV_FW_HWS_LOG_SIZE)
        {
            hwsLogPointer = 0;
        }
        else
        {
            /* Incrementing the pointer */
            hwsLogPointer +=1 ;
        }

        /* Incrementing the counter and handling counter overflow.
           We will allow counter with MV_FW_HWS_LOG_SIZE+1 value
           for the host to know about overlap */
        if (hwsLogCount <= MV_FW_HWS_LOG_SIZE)
        {
            hwsLogCount += 1;
        }

        /* Invalidate cache for memory coherence */
        FW_CACHE_INVALIDATE(&hwsLog[0], sizeof(hwsLog));
        FW_CACHE_INVALIDATE(&hwsLogPointer, (sizeof(hwsLogPointer)));
        FW_CACHE_INVALIDATE(&hwsLogCount, (sizeof(hwsLogCount)));

        mvPortCtrlLogUnlock();
    }
}

#ifndef DISABLE_CLI
/**
* @internal arbSmStateDesc function
* @endinternal
*
* @brief   Print AP ARB State machine status description
*/
void arbSmStateDesc(GT_U32 mask, GT_U32 statusId)
{
    if(statusId == AP_PORT_MSG_SEND_FAILURE)
    {
        if(mask == MV_HWS_IPC_PORT_RESET_MSG)
        {
            mvPcPrintf("RESET\n");
        }
        else if (mask == MV_HWS_IPC_PORT_INIT_MSG)
        {
            mvPcPrintf("INIT\n");
        }
        else
        {
            mvPcPrintf("\n");
        }
    }
    else
    {
        if (mask != 0)
        {
            if      (mask & ST_AN_ENABLE)    mvPcPrintf("ARB Status AN_ENABLE\n");
            else if (mask & ST_TX_DISABLE)   mvPcPrintf("ARB Status TX_DISABLE\n");
            else if (mask & ST_LINK_STAT_CK) mvPcPrintf("ARB Status LINK_STATE_CHECK\n");
            else if (mask & ST_PARALLEL_FLT) mvPcPrintf("ARB Status PARALLEL_FLT\n");
            else if (mask & ST_ABILITY_DET)  mvPcPrintf("ARB Status ABILITY_DET\n");
            else if (mask & ST_ACK_DETECT)   mvPcPrintf("ARB Status ST_ACK_DETECT\n");
            else if (mask & ST_COMPLETE_ACK) mvPcPrintf("ARB Status ST_COMPLETE_ACK\n");
            else if (mask & ST_NP_WAIT)      mvPcPrintf("ARB Status ST_NP_WAIT\n");
            else if (mask & ST_AN_GOOD_CK)   mvPcPrintf("ARB Status AN_GOOD_CK\n");
            else if (mask & ST_AN_GOOD)      mvPcPrintf("ARB Status AN_GOOD\n");
            else if (mask & ST_SERDES_WAIT)  mvPcPrintf("ARB Status SERDES WAIT\n");
        }
        else
        {
            mvPcPrintf("\n");
        }
    }
}
#endif /* DISABLE_CLI */

#ifdef FREE_RTOS_HWS_ENHANCED_PRINT_MODE
/**
* @internal mvPortCtrlLogDump function
* @endinternal
*
* @brief   Print Log content according to Log count
*         The order of print is from the oldest to the newest message
*/
void mvPortCtrlLogDump(GT_U32 port)
{
    GT_U32 logIndex;
    GT_U32 msgId;
    GT_U32 stateId;
    GT_U32 portId;
    GT_U32 regId;
    GT_U32 regVal;
    GT_U32 hcdId;
    GT_U32 arbId;
    GT_U32 statusId;
    GT_U32 logActiveCount;
    GT_U32 logActivePointer;
    GT_U32 logInfo;
    GT_U32 timestamp;
    GT_U8 portNumBase = 0;

#ifdef BC2_DEV_SUPPORT
    if(BobcatA0 == hwsDeviceSpecInfo[0].devType)
    {
        portNumBase = MV_PORT_CTRL_BC2_AP_PORT_NUM_BASE;
    }
    else if(Alleycat3A0 == hwsDeviceSpecInfo[0].devType)
    {
        portNumBase = MV_PORT_CTRL_AC3_AP_PORT_NUM_BASE;
    }
    else
    {
        portNumBase = 0;
    }
#else
    portNumBase = MV_PORT_CTRL_AP_PORT_NUM_BASE;
#endif

    logActiveCount   = logCount;
    logActivePointer = logPointer;

    if (logActiveCount >= MV_PORT_CTRL_LOG_SIZE)
    {
        logActiveCount = MV_PORT_CTRL_LOG_SIZE - 1;
    }

    if (logActivePointer >= MV_PORT_CTRL_LOG_SIZE)
    {
        logActivePointer = 0;
    }

    if (mvPortCtrlLogLock() == GT_OK)
    {
        logCount = 0;
        mvPortCtrlLogUnlock();
    }

    if (logActiveCount > logActivePointer)
    {
        logActivePointer = MV_PORT_CTRL_LOG_SIZE - (logActiveCount - logActivePointer);
    }
    else
    {
        logActivePointer = logActivePointer - logActiveCount;
    }

    mvPcPrintf("Port Control Realtime Log\n"
               "=========================\n"
               "Num of log entries  %03d\n"
               "Current entry index %03d\n",
               logActiveCount, logActivePointer);

    /* Dump log */
    for (logIndex = 0; logIndex < logActiveCount; logIndex++)
    {
        if (mvPortCtrlLogLock() != GT_OK)
            return;

        logInfo   = portLog[logActivePointer].info;
        timestamp = portLog[logActivePointer].timestamp;

        mvPortCtrlLogUnlock();

        if ((logIndex % 10) == 0)
        {
            mvPortCtrlProcessDelay(2);
        }

        /* Update next entry log pointer */
        logActivePointer++;
        if (logActivePointer >= MV_PORT_CTRL_LOG_SIZE)
        {
            logActivePointer = 0;
        }

        switch (LOG_MOD_GET(logInfo))
        {
        case M0_DEBUG:
            portId  = REG_LOG_PORT_GET(logInfo);
            stateId = REG_LOG_STATE_GET(logInfo);
            regId   = REG_LOG_IDX_GET(logInfo);
            regVal  = REG_LOG_VAL_GET(logInfo);

            /* Validate inputs params */
            if (((port != LOG_ALL_PORT_DUMP) && (port != portId)) ||
                 (stateId >= AP_PORT_SM_MAX_STATE) ||
                 (regId >= PORT_CTRL_MAX_AP_REGS))
            {
                break;
            }

            /* Process message
            ** ===============
            ** Print registers in sections:
            **   Cfg-0, Cfg-1, St-0
            **   Ctrl, St, AdvReg1, AdvReg2, AdvReg3
            **   LpReg1, LpReg2, LpReg3, Aneg0, Aneg1
            **   NxtPg, NxtPg-0, NxtPg-16
            **   LpNxtPg, LpNxtPg-0, LpNxtPg-16, BpSt, LpAdv
            */
            if (regId == PORT_CTRL_AP_REG_CFG_0)
            {
                mvPcPrintf("%08d: AP REG  Port %d, State O%d: ", timestamp, MV_HWS_PORT_CTRL_AP_PHYSICAL_PORT_GET(portId, portNumBase), stateId);
            }
            else if ((regId == PORT_CTRL_AP_INT_REG_802_3_AP_CTRL) ||
                     (regId == PORT_CTRL_AP_INT_REG_802_3_LP_BASE_ABILITY_REG_1) ||
                     (regId == PORT_CTRL_AP_INT_REG_802_3_NEXT_PG_TX_REG) ||
                     (regId == PORT_CTRL_AP_INT_REG_802_3_LP_NEXT_PG_ABILITY_REG))
            {
                mvPcPrintf("%08d: AP REG  Port %d, State O%d, AP Int: ", timestamp, MV_HWS_PORT_CTRL_AP_PHYSICAL_PORT_GET(portId, portNumBase), stateId);
            }

            mvPcPrintf("%s 0x%04x ", regType[regId], regVal);

            if ((regId == PORT_CTRL_AP_REG_ST_0) ||
                (regId == PORT_CTRL_AP_INT_REG_802_3_AP_ADV_REG_3) ||
                (regId == PORT_CTRL_AP_INT_REG_ANEG_CTRL_1) ||
                (regId == PORT_CTRL_AP_INT_REG_802_3_NEXT_PG_TX_CODE_16_31) ||
                (regId == PORT_CTRL_AP_INT_REG_802_3_LP_ADV_REG))
            {
                mvPcPrintf("\n");
            }
            break;

        case M1_SUPERVISOR:
            portId  = SPV_LOG_PORT_GET(logInfo);
            msgId   = SPV_LOG_MSG_GET(logInfo);
            stateId = SPV_LOG_STATE_GET(logInfo);

            /* Validate inputs params */
            if (((port != LOG_ALL_PORT_DUMP) && (port != portId)) ||
                 (msgId >= MV_HWS_IPC_LAST_CTRL_MSG_TYPE) ||
                 (stateId >= SPV_MAX_STATE))
            {
                break;
            }

            /* Process message */
            mvPcPrintf("%08d: Super   Port %02d, %s, %s Msg Sent to execute, O%d(%s)\n",
                       timestamp,
                       portId,
                       portType[SPV_LOG_TYPE_GET(logInfo)],
                       msgType[msgId],
                       stateId + 1,
                       superState[stateId]);

            break;
#ifdef REG_PORT_TASK
        case M2_PORT_MNG:
            portId  = PORT_MNG_LOG_PORT_GET(logInfo);
            msgId   = PORT_MNG_MSG_GET(logInfo);
            stateId = PORT_MNG_LOG_STATE_GET(logInfo);

            /* Validate inputs params */
            if (((port != LOG_ALL_PORT_DUMP) && (port != portId)) ||
                 (msgId >= MV_HWS_IPC_LAST_CTRL_MSG_TYPE) ||
                 (stateId >= PORT_MNG_MAX_STATE))
            {
                break;
            }

            /* Process message */
            mvPcPrintf("%08d: PortMng Port %02d, %s Msg start execute, O%d(%s)\n",
                       timestamp,
                       portId,
                       msgType[msgId],
                       stateId,
                       portMngState[stateId]);
            break;
#endif
        case M3_PORT_SM:
            portId   = PORT_SM_LOG_PORT_GET(logInfo);
            statusId = PORT_SM_LOG_STATUS_GET(logInfo);
            stateId  = PORT_SM_LOG_STATE_GET(logInfo);

            /* Validate inputs params */
            if (((port != LOG_ALL_PORT_DUMP) && (port != portId)) ||
                 (statusId >= PORT_SM_MAX_STATUS) ||
                 (stateId >= PORT_SM_MAX_STATE))
            {
                break;
            }

            /* Process message */
            mvPcPrintf("%08d: Port SM Port %02d, %s, O%d(%s)\n",
                       timestamp,
                       portId,
                       portMngSmStatus[statusId],
                       stateId,
                       portMngSmState[stateId]);
            break;

        case M4_AP_PORT_MNG:
            portId   = AP_PORT_MNG_LOG_PORT_GET(logInfo);
            statusId = AP_PORT_MNG_LOG_STATUS_GET(logInfo);
            stateId  = AP_PORT_MNG_LOG_STATE_GET(logInfo);

            /* Validate inputs params */
            if (((port != LOG_ALL_PORT_DUMP) && (port != portId)) ||
                 (statusId >= PORT_SM_MAX_STATUS) ||
                 (stateId >= PORT_SM_MAX_STATE))
            {
                break;
            }

            /* Process message */
            mvPcPrintf("%08d: Port SM Port %02d, %s, O%d(%s)\n",
                       timestamp,
                       portId,
                       portMngSmStatus[statusId],
                       stateId,
                       portMngSmState[stateId]);
            break;

        case M5_AP_PORT_DET:
            portId   = AP_PORT_DET_LOG_PORT_GET(logInfo);
            statusId = AP_PORT_DET_LOG_STATUS_GET(logInfo);
            stateId  = AP_PORT_DET_LOG_STATE_GET(logInfo);
            arbId    = AP_PORT_DET_LOG_HW_SM_GET(logInfo);

            /* Validate inputs params */
            if (((port != LOG_ALL_PORT_DUMP) && (port != portId)) ||
                 (statusId >= AP_PORT_MAX_STATUS) ||
                 (stateId >= AP_PORT_SM_MAX_STATE))
            {
                break;
            }

            /* Process message */
            mvPcPrintf("%08d: AP SM   Port %02d, status %d %s, O%d(%s) ",
                       timestamp,
                       portId,
                       statusId,
                       apPortMngSmStatus[statusId],
                       stateId,
                       apPortMngSmState[stateId]);

            arbSmStateDesc(arbId, statusId);
            break;

        case M5_AP_PORT_DET_EXT:
            portId = AP_PORT_DET_EXT_LOG_PORT_GET(logInfo);
            hcdId  = AP_PORT_DET_EXT_LOG_HCD_GET(logInfo);

            /* Validate inputs params */
            if (((port != LOG_ALL_PORT_DUMP) && (port != portId)) ||
                 (hcdId >= 4))
            {
                break;
            }

            if(AP_PORT_DET_EXT_LOG_HCD_GET(logInfo) >= Port_AP_LAST)
            {
                mvPcPrintf("hcdType not supported %d\n", AP_PORT_DET_EXT_LOG_HCD_GET(logInfo));
                return;
            }

            /* Process message */
            mvPcPrintf("%08d: AP SM   Port %02d, HCD[%s %s Link %s, FEC Result %s] FC[Tx %s, RX %s] Int[HCD %s, Link %s] \n",
                       timestamp,
                       portId,
                       apPortMngHcdFound[AP_PORT_DET_EXT_LOG_FOUND_GET(logInfo)],
                       apPortMngHcdType[AP_PORT_DET_EXT_LOG_HCD_GET(logInfo)],
                       apPortMngGen[AP_PORT_DET_EXT_LOG_LINK_GET(logInfo)],
                       apPortMngHcdFecType[AP_PORT_DET_EXT_LOG_FEC_RES_GET(logInfo)],
                       apPortMngGen[AP_PORT_DET_EXT_LOG_TX_FC_GET(logInfo)],
                       apPortMngGen[AP_PORT_DET_EXT_LOG_RX_FC_GET(logInfo)],
                       apPortMngGen[AP_PORT_DET_EXT_LOG_HCD_INT_GET(logInfo)],
                       apPortMngGen[AP_PORT_DET_EXT_LOG_LINK_INT_GET(logInfo)]);
            break;
#ifdef AP_GENERAL_TASK
        case M6_GENERAL:
            portId = PORT_GENERAL_LOG_PORT_GET(logInfo);
            msgId  = PORT_GENERAL_LOG_MSG_GET(logInfo);

            /* Validate inputs params */
            if (((port != LOG_ALL_PORT_DUMP) && (port != portId)) ||
                 (msgId >= MV_HWS_IPC_LAST_CTRL_MSG_TYPE))
            {
                break;
            }

            /* Process message */
            mvPcPrintf("%08d: General Port %02d, Executing %s Msg",
                       timestamp,
                       portId,
                       msgType[msgId]);
        /* Code not relevant for BobK CM3 due to space limit */
            switch (msgId)
            {
                case MV_HWS_IPC_PORT_LINK_STATUS_GET_MSG:
                    if (PORT_GENERAL_LOG_RES_GET(logInfo))
                    {
                        mvPcPrintf(" %s", (PORT_GENERAL_LOG_DETAIL_GET(logInfo) == MV_GEN_LINK_DOWN) ? "Fail" :"OK");
                    }
                    break;
                case MV_HWS_IPC_PORT_AUTO_TUNE_SET_EXT_MSG:
                    if (PORT_GENERAL_LOG_RES_GET(logInfo))
                    {
                        mvPcPrintf(" %s", (PORT_GENERAL_LOG_DETAIL_GET(logInfo) == MV_GEN_TUNE_FAIL) ? "Fail" :"Pass");
                    }
                    break;
            }
            mvPcPrintf("\n");
            break;
#endif
        }
    }
}
#endif /* FREE_RTOS_HWS_ENHANCED_PRINT_MODE */

#ifndef DISABLE_CLI
/**
* @internal mvPortCtrlApPortStatus function
* @endinternal
*
* @brief   AP Port state & status show
*/
void mvPortCtrlApPortStatus(void)
{
    GT_U8 portIndex;

    MV_HWS_AP_SM_INFO *apSm;
    GT_U8 maxApPortNum = 0;

#ifdef BC2_DEV_SUPPORT
    if(BobcatA0 == hwsDeviceSpecInfo[0].devType)
    {
        maxApPortNum = MV_PORT_CTRL_BC2_MAX_AP_PORT_NUM;
    }
    else if(Alleycat3A0 == hwsDeviceSpecInfo[0].devType)
    {
        maxApPortNum = MV_PORT_CTRL_AC3_MAX_AP_PORT_NUM;
    }
    else
    {
        hwsOsPrintf("unknown packet processor\n");
    }
#else
    maxApPortNum = MV_PORT_CTRL_MAX_AP_PORT_NUM;
#endif

    for (portIndex = 0; portIndex < maxApPortNum; portIndex++)
    {
        apSm = &(mvHwsPortCtrlApPortDetect.apPortSm[portIndex].info);

        if (apSm->state != AP_PORT_SM_IDLE_STATE)
        {
            mvPcPrintf("AP Port %02d, %s, O%d(%s) ",
                       apSm->portNum, apPortMngSmStatus[apSm->status],
                       apSm->state, apPortMngSmState[apSm->state]);

            mvPcPrintf("HCD[%d %d, FEC Result %d] capability 0x%x options 0x%x ",
                      /* apPortMngHcdFound[*/AP_ST_HCD_FOUND_GET(apSm->hcdStatus)/*]*/,
                       /*apPortMngHcdType[*/AP_ST_HCD_TYPE_GET(apSm->hcdStatus)/*]*/,
                       /*apPortMngHcdFecType[*/AP_ST_HCD_FEC_RES_GET(apSm->hcdStatus)/*]*/,
                       apSm->capability, apSm->options);
            arbSmStateDesc(apSm->ARMSmStatus, 0);
        }
    }
}
#endif /* DISABLE_CLI */

