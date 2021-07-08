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
* @file wrapCpssHwInit.c
* @version   2
********************************************************************************
*/

/* Common galtis includes */
#include <galtisAgent/wrapUtil/cmdCpssPresteraUtils.h>
#include <cmdShell/cmdDb/cmdBase.h>
#include <cmdShell/common/cmdWrapUtils.h>

/* Feature specific includes. */
#include <cpss/common/cpssTypes.h>
#include <cpss/common/cpssHwInit/cpssHwInit.h>


/****************cpssPpConfigDevData Table*************************************/
static PRV_CPSS_PP_HA_CONFIG_INFO_STC *configDevDataBuffer;
static GT_U32                          configDevDataBufferSize;
static GT_U32                          indexGet = 0;

/**
* @internal wrCpssPpConfigDevDataImportSetFirst function
* @endinternal
*
* @brief   Imports data for device specific information. Used in catch up process
*         during initialization as the first step in the catch up process for
*         configuring the standby CPU.
*         NOTE: for a device that will be initialized in "pp phase 1" with
*         High availability mode of "standby" , this function MUST be called prior
*         to the "pp phase 1"
*
* @note   APPLICABLE DEVICES:      ALL
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device Number or configDevDataBufferSize
*/
static CMD_STATUS wrCpssPpConfigDevDataImportSetFirst
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_U32   ind;

    /*reset on first*/
    configDevDataBufferSize = 0;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    if(configDevDataBuffer == NULL)
    {
        configDevDataBuffer = (PRV_CPSS_PP_HA_CONFIG_INFO_STC*)cmdOsMalloc(
                                        sizeof(PRV_CPSS_PP_HA_CONFIG_INFO_STC));
    }

    if(configDevDataBuffer == NULL)
    {
        galtisOutput(outArgs, GT_NO_RESOURCE, "");
        return CMD_OK;
    }

    ind = (GT_U32)inFields[0];

    if(ind > configDevDataBufferSize / sizeof(PRV_CPSS_PP_HA_CONFIG_INFO_STC) )
    {
        galtisOutput(outArgs, GT_BAD_VALUE, "");
        return CMD_OK;
    }

    configDevDataBuffer[ind].deviceId = (GT_U32)inFields[1];
    configDevDataBuffer[ind].revision = (GT_U8)inFields[2];

    configDevDataBufferSize += sizeof(PRV_CPSS_PP_HA_CONFIG_INFO_STC);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, GT_OK, "");

    return CMD_OK;
}

/**
* @internal wrCpssPpConfigDevDataImportSetNext function
* @endinternal
*
* @brief   Imports data for device specific information. Used in catch up process
*         during initialization as the first step in the catch up process for
*         configuring the standby CPU.
*         NOTE: for a device that will be initialized in "pp phase 1" with
*         High availability mode of "standby" , this function MUST be called prior
*         to the "pp phase 1"
*
* @note   APPLICABLE DEVICES:      ALL
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device Number or configDevDataBufferSize
*/
static CMD_STATUS wrCpssPpConfigDevDataImportSetNext
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_U32   ind;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    configDevDataBuffer = (PRV_CPSS_PP_HA_CONFIG_INFO_STC*)
                          cmdOsRealloc(configDevDataBuffer,
                          sizeof(PRV_CPSS_PP_HA_CONFIG_INFO_STC) + configDevDataBufferSize);

    if(configDevDataBuffer == NULL)
    {
        configDevDataBuffer = (PRV_CPSS_PP_HA_CONFIG_INFO_STC*)cmdOsMalloc(
                                        sizeof(PRV_CPSS_PP_HA_CONFIG_INFO_STC));
    }

    if(configDevDataBuffer == NULL)
    {
        galtisOutput(outArgs, GT_NO_RESOURCE, "");
        return CMD_OK;
    }

    ind = (GT_U32)inFields[0];

    if(ind > configDevDataBufferSize / sizeof(PRV_CPSS_PP_HA_CONFIG_INFO_STC))
    {
        galtisOutput(outArgs, GT_BAD_VALUE, "");
        return CMD_OK;
    }

    configDevDataBuffer[ind].deviceId = (GT_U32)inFields[1];
    configDevDataBuffer[ind].revision = (GT_U8)inFields[2];

    configDevDataBufferSize += sizeof(PRV_CPSS_PP_HA_CONFIG_INFO_STC);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, GT_OK, "");

    return CMD_OK;
}

/**
* @internal wrCpssPpConfigDevDataImportEndSet function
* @endinternal
*
* @brief   Imports data for device specific information. Used in catch up process
*         during initialization as the first step in the catch up process for
*         configuring the standby CPU.
*         NOTE: for a device that will be initialized in "pp phase 1" with
*         High availability mode of "standby" , this function MUST be called prior
*         to the "pp phase 1"
*
* @note   APPLICABLE DEVICES:      ALL
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device Number or configDevDataBufferSize
*/
static CMD_STATUS wrCpssPpConfigDevDataImportEndSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8     devNum;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];

    /* call cpss api function */
    result = cpssPpConfigDevDataImport(devNum, configDevDataBuffer,
                                               configDevDataBufferSize);
    if(configDevDataBuffer)
    {
        cmdOsFree(configDevDataBuffer);
        configDevDataBuffer = NULL;
    }

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssPpConfigDevDataExportGetFirst function
* @endinternal
*
* @brief   Imports data for device specific information. Used in catch up process
*         during initialization as the first step in the catch up process for
*         configuring the standby CPU.
*
* @note   APPLICABLE DEVICES:      ALL
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_SIZE              - block is not big enough to hold all the data
* @retval GT_BAD_PARAM             - wrong device Number
*/
static CMD_STATUS wrCpssPpConfigDevDataExportGetFirst
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8     devNum;

    indexGet = 0;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    configDevDataBufferSize = (GT_U32)inArgs[1];

    /* map input arguments to locals */
    if(configDevDataBuffer == NULL)
    {
        configDevDataBuffer = (PRV_CPSS_PP_HA_CONFIG_INFO_STC*)
                              cmdOsMalloc(configDevDataBufferSize);
    }

    if(configDevDataBuffer == NULL)
    {
        galtisOutput(outArgs, GT_NO_RESOURCE, "");
        return CMD_OK;
    }

    /* call cpss api function */
    result = cpssPpConfigDevDataExport(devNum, configDevDataBuffer,
                                               &configDevDataBufferSize);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssPpConfigDevDataExportGetNext function
* @endinternal
*
* @brief   Imports data for device specific information. Used in catch up process
*         during initialization as the first step in the catch up process for
*         configuring the standby CPU.
*
* @note   APPLICABLE DEVICES:      ALL
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_SIZE              - block is not big enough to hold all the data
* @retval GT_BAD_PARAM             - wrong device Number
*/
static CMD_STATUS wrCpssPpConfigDevDataExportGetNext
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    if (indexGet > configDevDataBufferSize / sizeof(PRV_CPSS_PP_HA_CONFIG_INFO_STC) )
    {
        /* notify "no records more" */
        galtisOutput(outArgs, GT_OK, "%d", -1);

        cmdOsFree(configDevDataBuffer);
        configDevDataBuffer = NULL;

        return CMD_OK;
    }

    fieldOutput("%d%d%d", indexGet,
                        configDevDataBuffer[indexGet].deviceId,
                        configDevDataBuffer[indexGet].revision);
    galtisOutput(outArgs, CMD_OK, "%f");

    /* increment index */
    indexGet++;

    return CMD_OK;
}

/**************** cpssPpConfigDevData Table End *******************************/


/**** database initialization **************************************/

static CMD_COMMAND dbCommands[] =
{
        {"cpssPpConfigDevDataSetFirst",
         &wrCpssPpConfigDevDataImportSetFirst,
         0, 3},
        {"cpssPpConfigDevDataSetNext",
         &wrCpssPpConfigDevDataImportSetNext,
         0, 3},
        {"cpssPpConfigDevDataEndSet",
         &wrCpssPpConfigDevDataImportEndSet,
         1, 0},

        {"cpssPpConfigDevDataGetFirst",
         &wrCpssPpConfigDevDataExportGetFirst,
         2, 0},
        {"cpssPpConfigDevDataGetNext",
         &wrCpssPpConfigDevDataExportGetNext,
         0, 3}
};

#define numCommands (sizeof(dbCommands) / sizeof(CMD_COMMAND))

/**
* @internal cmdLibInitCpssGenHwInit function
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
GT_STATUS cmdLibInitCpssGenHwInit
(
    GT_VOID
)
{
    return cmdInitLibrary(dbCommands, numCommands);
}


