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
* @file wrapCpssLog.c
*
* @brief Wrapper functions for CPSS LOG API
*
* @version   1
********************************************************************************
*/

/* Common galtis includes */
#include <galtisAgent/wrapUtil/cmdCpssPresteraUtils.h>
#include <cmdShell/cmdDb/cmdBase.h>
#include <cmdShell/common/cmdWrapUtils.h>

/* Feature specific includes. */
#include <cpss/generic/log/cpssLog.h>

/**
* @internal wrCpssLogEnableSet function
* @endinternal
*
* @brief   The function enables/disables Log feature.
*/
static CMD_STATUS wrCpssLogEnableSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_BOOL enable;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    enable = (GT_BOOL)inArgs[0];

    cpssLogEnableSet(enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, GT_OK, "");

    return CMD_OK;
}

typedef enum {
    WR_CPSS_LOG_LIB_APP_DRIVER_CALL_E,
    WR_CPSS_LOG_LIB_BRIDGE_E,
    WR_CPSS_LOG_LIB_CNC_E,
    WR_CPSS_LOG_LIB_CONFIG_E,
    WR_CPSS_LOG_LIB_COS_E,
    WR_CPSS_LOG_LIB_HW_INIT_E,
    WR_CPSS_LOG_LIB_CSCD_E,
    WR_CPSS_LOG_LIB_CUT_THROUGH_E,
    WR_CPSS_LOG_LIB_DIAG_E,
    WR_CPSS_LOG_LIB_FABRIC_E,
    WR_CPSS_LOG_LIB_IP_E,
    WR_CPSS_LOG_LIB_IPFIX_E,
    WR_CPSS_LOG_LIB_IP_LPM_E,
    WR_CPSS_LOG_LIB_L2_MLL_E,
    WR_CPSS_LOG_LIB_LOGICAL_TARGET_E,
    WR_CPSS_LOG_LIB_LPM_E,
    WR_CPSS_LOG_LIB_MIRROR_E,
    WR_CPSS_LOG_LIB_MULTI_PORT_GROUP_E,
    WR_CPSS_LOG_LIB_NETWORK_IF_E,
    WR_CPSS_LOG_LIB_NST_E,
    WR_CPSS_LOG_LIB_OAM_E,
    WR_CPSS_LOG_LIB_PCL_E,
    WR_CPSS_LOG_LIB_PHY_E,
    WR_CPSS_LOG_LIB_POLICER_E,
    WR_CPSS_LOG_LIB_PORT_E,
    WR_CPSS_LOG_LIB_PROTECTION_E,
    WR_CPSS_LOG_LIB_PTP_E,
    WR_CPSS_LOG_LIB_SYSTEM_RECOVERY_E,
    WR_CPSS_LOG_LIB_TCAM_E,
    WR_CPSS_LOG_LIB_TM_GLUE_E,
    WR_CPSS_LOG_LIB_TRUNK_E,
    WR_CPSS_LOG_LIB_TTI_E,
    WR_CPSS_LOG_LIB_TUNNEL_E,
    WR_CPSS_LOG_LIB_VNT_E,
    WR_CPSS_LOG_LIB_RESOURCE_MANAGER_E,
    WR_CPSS_LOG_LIB_VERSION_E,
    WR_CPSS_LOG_LIB_TM_E,
    WR_CPSS_LOG_LIB_SMI_E,
    WR_CPSS_LOG_LIB_INIT_E,
    WR_CPSS_LOG_LIB_DRAGONITE_E,
    WR_CPSS_LOG_LIB_VIRTUAL_TCAM_E,

    WR_CPSS_LOG_LIB_ALL_E,

    WR_CPSS_LOG_LIB_INGRESS_E,
    WR_CPSS_LOG_LIB_EGRESS_E

}WR_CPSS_LOG_LIB_ENT;

/**
* @internal wrCpssLogLibEnableSet function
* @endinternal
*
* @brief   The function enables/disables specific type of log information for
*         specific or all libs.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - bad parameter lib or type
*/
static CMD_STATUS wrCpssLogLibEnableSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS               result;
    WR_CPSS_LOG_LIB_ENT     wrLib;
    CPSS_LOG_LIB_ENT        cpssLib;
    CPSS_LOG_TYPE_ENT       type;
    GT_BOOL                 enable;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    wrLib = (WR_CPSS_LOG_LIB_ENT)inArgs[0];
    type = (CPSS_LOG_TYPE_ENT)inArgs[1];
    enable = (GT_BOOL)inArgs[2];

    /* convert wrapper values to CPSS values */
    switch(wrLib)
    {
        case WR_CPSS_LOG_LIB_ALL_E:     cpssLib = CPSS_LOG_LIB_ALL_E; break;
        case WR_CPSS_LOG_LIB_INGRESS_E: cpssLib = CPSS_LOG_LIB_INGRESS_E; break;
        case WR_CPSS_LOG_LIB_EGRESS_E:  cpssLib = CPSS_LOG_LIB_EGRESS_E; break;
        default: cpssLib = (CPSS_LOG_LIB_ENT)wrLib; break;
    }

    result = cpssLogLibEnableSet(cpssLib,type,enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}
/**
* @internal wrCpssLogApiFormatSet function
* @endinternal
*
* @brief   The function set format for API related log.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - bad parameter format
*/
static CMD_STATUS wrCpssLogApiFormatSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    CPSS_LOG_API_FORMAT_ENT format;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    format = (CPSS_LOG_API_FORMAT_ENT)inArgs[0];

    result = cpssLogApiFormatSet(format);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}
/**
* @internal wrCpssLogStackSizeSet function
* @endinternal
*
* @brief   Defines the size of the stack, how many function will be logged before
*         an error occurred.
*
* @retval GT_OK                    - on success
*/
static CMD_STATUS wrCpssLogStackSizeSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U32    size;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    size = (GT_U32)inArgs[0];

    result = cpssLogStackSizeSet(size);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssLogLineNumberEnableSet function
* @endinternal
*
* @brief   The function enables/disables using of a source code line number by
*         some logging functions
*/
static CMD_STATUS wrCpssLogLineNumberEnableSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result = GT_OK;
    GT_BOOL   enable;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    enable = (GT_BOOL)inArgs[0];

    cpssLogLineNumberEnableSet(enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}


/**** database initialization **************************************/

static CMD_COMMAND dbCommands[] =
{
    {"cpssLogEnableSet",
        &wrCpssLogEnableSet,
        1, 0},
    {"cpssLogLibEnableSet",
        &wrCpssLogLibEnableSet,
        3, 0},
    {"cpssLogApiFormatSet",
        &wrCpssLogApiFormatSet,
        1, 0},
    {"cpssLogStackSizeSet",
        &wrCpssLogStackSizeSet,
        1, 0},
    {"cpssLogLineNumberEnableSet",
        &wrCpssLogLineNumberEnableSet,
        1, 0}
};


#define numCommands (sizeof(dbCommands) / sizeof(CMD_COMMAND))

/**
* @internal cmdLibInitCpssLog function
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
GT_STATUS cmdLibInitCpssLog
(
    GT_VOID
)
{
    return cmdInitLibrary(dbCommands, numCommands);
}


