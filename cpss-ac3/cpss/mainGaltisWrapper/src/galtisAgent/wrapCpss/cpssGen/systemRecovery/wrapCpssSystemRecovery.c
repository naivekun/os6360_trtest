/*******************************************************************************
*              (c), Copyright 2006, Marvell International Ltd.                 *
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
* @file wrapCpssSystemRecovery.c
* @version   5
********************************************************************************
*/

/* Common galtis includes */
#include <galtisAgent/wrapUtil/cmdCpssPresteraUtils.h>
#include <cmdShell/cmdDb/cmdBase.h>
#include <cmdShell/common/cmdWrapUtils.h>

/* Feature specific includes. */
#include <cpss/common/cpssTypes.h>
#include <cpss/common/systemRecovery/cpssGenSystemRecovery.h>


/**
* @internal wrCpssSystemRecoveryStateSet function
* @endinternal
*
* @brief   This function inform cpss about System Recovery state
*
* @note   APPLICABLE DEVICES:      ALL
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_PARAM             - on bad system recovery state
*/
static CMD_STATUS wrCpssSystemRecoveryStateSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    CPSS_SYSTEM_RECOVERY_INFO_STC systemRecovery;
    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    systemRecovery.systemRecoveryState = (CPSS_SYSTEM_RECOVERY_STATE_ENT)inArgs[0];
    systemRecovery.systemRecoveryMode.continuousRx =  (GT_BOOL)inArgs[1];
    systemRecovery.systemRecoveryMode.continuousTx = (GT_BOOL)inArgs[2];
    systemRecovery.systemRecoveryMode.continuousAuMessages = (GT_BOOL)inArgs[3];
    systemRecovery.systemRecoveryMode.continuousFuMessages = (GT_BOOL)inArgs[4];
    systemRecovery.systemRecoveryMode.haCpuMemoryAccessBlocked = (GT_BOOL)inArgs[5];
    systemRecovery.systemRecoveryProcess = CPSS_SYSTEM_RECOVERY_PROCESS_NOT_ACTIVE_E;
    /* call cpss api function */
    result = cpssSystemRecoveryStateSet(&systemRecovery);
    cpssOsPrintf("systemRecoveryState = %d\n",systemRecovery.systemRecoveryState);
    cpssOsPrintf("systemRecoveryMode.continuousRx = %d\n",systemRecovery.systemRecoveryMode.continuousRx);
    cpssOsPrintf("systemRecoveryMode.continuousTx = %d\n",systemRecovery.systemRecoveryMode.continuousTx);
    cpssOsPrintf("systemRecoveryMode.continuousAuMessages = %d\n",systemRecovery.systemRecoveryMode.continuousAuMessages);
    cpssOsPrintf("systemRecoveryMode.continuousFuMessages = %d\n",systemRecovery.systemRecoveryMode.continuousFuMessages);
    cpssOsPrintf("systemRecoveryMode.haCpuMemoryAccessBlocked = %d\n",systemRecovery.systemRecoveryMode.haCpuMemoryAccessBlocked);
    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d%d%d%d", systemRecovery.systemRecoveryState,systemRecovery.systemRecoveryMode.continuousRx,
                 systemRecovery.systemRecoveryMode.continuousAuMessages,systemRecovery.systemRecoveryMode.haCpuMemoryAccessBlocked);

    return CMD_OK;
}

/**
* @internal wrCpssSystemRecoveryStateSet_1 function
* @endinternal
*
* @brief   This function inform cpss about System Recovery state
*
* @note   APPLICABLE DEVICES:      ALL
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_PARAM             - on bad system recovery state
*/
static CMD_STATUS wrCpssSystemRecoveryStateSet_1
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    CPSS_SYSTEM_RECOVERY_INFO_STC systemRecovery;
    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    systemRecovery.systemRecoveryState = (CPSS_SYSTEM_RECOVERY_STATE_ENT)inArgs[0];
    systemRecovery.systemRecoveryMode.continuousRx =  (GT_BOOL)inArgs[1];
    systemRecovery.systemRecoveryMode.continuousTx = (GT_BOOL)inArgs[2];
    systemRecovery.systemRecoveryMode.continuousAuMessages = (GT_BOOL)inArgs[3];
    systemRecovery.systemRecoveryMode.continuousFuMessages = (GT_BOOL)inArgs[4];
    systemRecovery.systemRecoveryMode.haCpuMemoryAccessBlocked = (GT_BOOL)inArgs[5];
    systemRecovery.systemRecoveryProcess = (CPSS_SYSTEM_RECOVERY_PROCESS_ENT)inArgs[6];
    /* call cpss api function */
    result = cpssSystemRecoveryStateSet(&systemRecovery);
    cpssOsPrintf("systemRecoveryState = %d\n",systemRecovery.systemRecoveryState);
    cpssOsPrintf("systemRecoveryMode.continuousRx = %d\n",systemRecovery.systemRecoveryMode.continuousRx);
    cpssOsPrintf("systemRecoveryMode.continuousTx = %d\n",systemRecovery.systemRecoveryMode.continuousTx);
    cpssOsPrintf("systemRecoveryMode.continuousAuMessages = %d\n",systemRecovery.systemRecoveryMode.continuousAuMessages);
    cpssOsPrintf("systemRecoveryMode.continuousFuMessages = %d\n",systemRecovery.systemRecoveryMode.continuousFuMessages);
    cpssOsPrintf("systemRecoveryMode.haCpuMemoryAccessBlocked = %d\n",systemRecovery.systemRecoveryMode.haCpuMemoryAccessBlocked);
    cpssOsPrintf("systemRecoveryProcess = %d\n",systemRecovery.systemRecoveryProcess);
    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d%d%d%d", systemRecovery.systemRecoveryState,systemRecovery.systemRecoveryMode.continuousRx,
                 systemRecovery.systemRecoveryMode.continuousAuMessages,systemRecovery.systemRecoveryMode.haCpuMemoryAccessBlocked);

    return CMD_OK;
}


/**
* @internal wrCpssInboundSdmaEnable function
* @endinternal
*
* @brief   Perform inbound sdma enable
*
* @note   APPLICABLE DEVICES:      ALL
*                                       None
*/
static CMD_STATUS wrCpssInboundSdmaEnable
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS   result = GT_OK;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

#if ((!defined ASIC_SIMULATION) && (defined GDA8548_DRV))
    result = cpssExtDrvInboundSdmaEnable();
#endif
    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssInboundSdmaDisable function
* @endinternal
*
* @brief   Perform inbound sdma enable
*
* @note   APPLICABLE DEVICES:      ALL
*                                       None
*/
static CMD_STATUS wrCpssInboundSdmaDisable
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS   result = GT_OK;
    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;
#if ((!defined ASIC_SIMULATION) && (defined GDA8548_DRV))
    result = cpssExtDrvInboundSdmaDisable();
#endif
    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}


/**** database initialization **************************************/

static CMD_COMMAND dbCommands[] =
{
        {"cpssSystemRecoveryStateSet",
         &wrCpssSystemRecoveryStateSet,
         6, 0},
        {"cpssSystemRecoveryStateSet_1",
         &wrCpssSystemRecoveryStateSet_1,
         7, 0},
        {"cpssInboundSdmaEnable",
         &wrCpssInboundSdmaEnable,
         0, 0},
        {"cpssInboundSdmaDisable",
         &wrCpssInboundSdmaDisable,
         0, 0},
};

#define numCommands (sizeof(dbCommands) / sizeof(CMD_COMMAND))

/**
* @internal cmdLibInitCpssGenSystemRecovery function
* @endinternal
*
* @brief   Library database initialization function.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on failure.
*
* @note none
*
*/
GT_STATUS cmdLibInitCpssGenSystemRecovery
(
    GT_VOID
)
{
    return cmdInitLibrary(dbCommands, numCommands);
}


