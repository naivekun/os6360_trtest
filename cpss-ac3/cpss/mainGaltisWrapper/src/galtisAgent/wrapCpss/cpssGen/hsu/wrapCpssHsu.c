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
* @file wrapCpssHsu.c
* @version   9
********************************************************************************
*/

/* Common galtis includes */
#include <galtisAgent/wrapUtil/cmdCpssPresteraUtils.h>
#include <cmdShell/cmdDb/cmdBase.h>
#include <cmdShell/common/cmdWrapUtils.h>

/* Feature specific includes. */
#include <cpss/common/cpssTypes.h>
#include <cpss/generic/systemRecovery/hsu/cpssGenHsu.h>
#include <cpss/common/systemRecovery/cpssGenSystemRecovery.h>


/**
* @internal wrCpssHsuStateSet function
* @endinternal
*
* @brief   This function inform cpss about HSU state
*
* @note   APPLICABLE DEVICES:      ALL
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_PARAM             - on bad hsuState
*/
static CMD_STATUS wrCpssHsuStateSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    CPSS_SYSTEM_RECOVERY_INFO_STC hsuInfo;
    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    hsuInfo.systemRecoveryState = (GT_U32)inArgs[0];
    hsuInfo.systemRecoveryProcess = CPSS_SYSTEM_RECOVERY_PROCESS_HSU_E;
    switch ((GT_U32)inArgs[1])
    {
    case 0:
        hsuInfo.systemRecoveryMode.continuousAuMessages = GT_TRUE;
        hsuInfo.systemRecoveryMode.continuousFuMessages = GT_TRUE;
        hsuInfo.systemRecoveryMode.continuousRx = GT_TRUE;
        hsuInfo.systemRecoveryMode.continuousTx = GT_TRUE;
        break;
    case 1:
        hsuInfo.systemRecoveryMode.continuousAuMessages = GT_FALSE;
        hsuInfo.systemRecoveryMode.continuousFuMessages = GT_FALSE;
        hsuInfo.systemRecoveryMode.continuousRx = GT_FALSE;
        hsuInfo.systemRecoveryMode.continuousTx = GT_FALSE;
        break;
    default:
        return GT_BAD_PARAM;
    }
    hsuInfo.systemRecoveryMode.haCpuMemoryAccessBlocked = GT_FALSE;
    /* call cpss api function */
    result = cpssSystemRecoveryStateSet(&hsuInfo);
    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d%d", hsuInfo.systemRecoveryState,(GT_U32)inArgs[1]);

    return CMD_OK;
}
/**
* @internal wrCpssHsuEventHandleUpdate function
* @endinternal
*
* @brief   This function replace old event handle created after regular init
*         to new event handle that application got during HSU.
*
* @note   APPLICABLE DEVICES:      ALL
*
* @retval GT_OK                    - on success.
*/
CMD_STATUS wrCpssHsuEventHandleUpdate
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U32 oldEvReqHndl,newEvReqHndl;
    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */

    oldEvReqHndl = (GT_U32)inArgs[0];
    newEvReqHndl = (GT_U32)inArgs[1];

    result =  cpssHsuEventHandleUpdate(oldEvReqHndl, newEvReqHndl);
    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d%d", oldEvReqHndl, newEvReqHndl);

    return CMD_OK;
}

/**
* @internal wrCpssHsuWarmRestart function
* @endinternal
*
* @brief   Perform warm restart
*
* @note   APPLICABLE DEVICES:      ALL
*                                       None
*/
static CMD_STATUS wrCpssHsuWarmRestart
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;


#ifndef ASIC_SIMULATION
    cpssExtDrvHsuWarmRestart();
#endif
    return CMD_OK;
}


/**
* @internal wrCpssExtDrvSetIntLockUnlock function
* @endinternal
*
* @brief   Lock/unlock interrupts
*/
CMD_STATUS wrCpssExtDrvSetIntLockUnlock
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_U32 action;
    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */

    action = (GT_U32)inArgs[0];
    if (action == 0)
    {
        PRV_CPSS_LOCK_BY_MUTEX;
        PRV_CPSS_INT_SCAN_LOCK();
    }
    if (action == 1)
    {
        PRV_CPSS_INT_SCAN_UNLOCK();
        PRV_CPSS_UNLOCK_BY_MUTEX;
    }
     /* pack output arguments to galtis string */
    galtisOutput(outArgs, GT_OK, "0");
    return CMD_OK;
}

/**
* @internal wrCpssPpInterruptsDisable function
* @endinternal
*
* @brief   This API disable PP interrupts
*
* @note   APPLICABLE DEVICES:      ALL
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on hardware error
* @retval GT_NOT_INITIALIZED       - if the driver was not initialized
* @retval GT_HW_ERROR              - on hardware error
*/
CMD_STATUS wrCpssPpInterruptsDisable
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8 devNum;
    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */

    devNum = (GT_U8)inArgs[0];

    result = cpssPpInterruptsDisable(devNum);
    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**** database initialization **************************************/

static CMD_COMMAND dbCommands[] =
{
        {"cpssHsuStateSet",
         &wrCpssHsuStateSet,
         2, 0},
        {"cpssHsuEventHandlUpdate",
         &wrCpssHsuEventHandleUpdate,
         2, 0},
        {"extDrvHsuWarmRestart",
         &wrCpssHsuWarmRestart,
         0, 0},
        {"cpssExtDrvSetIntLockUnlock",
         &wrCpssExtDrvSetIntLockUnlock,
         2, 0},
        {"cpssPpInterruptsDisable",
         &wrCpssPpInterruptsDisable,
         1, 0},

};

#define numCommands (sizeof(dbCommands) / sizeof(CMD_COMMAND))

/**
* @internal cmdLibInitCpssGenHsu function
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
GT_STATUS cmdLibInitCpssGenHsu
(
    GT_VOID
)
{
    return cmdInitLibrary(dbCommands, numCommands);
}


