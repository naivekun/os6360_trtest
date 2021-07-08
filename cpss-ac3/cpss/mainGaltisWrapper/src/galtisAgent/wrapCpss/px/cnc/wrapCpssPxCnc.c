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
* @file wrapCpssPxCnc.c
*
* @brief Wrapper functions for
* PIPE CNC API's
*
* @version   1
********************************************************************************
*/

/* Common galtis includes */
#include <galtisAgent/wrapUtil/cmdCpssPresteraUtils.h>
#include <cmdShell/cmdDb/cmdBase.h>
#include <cmdShell/common/cmdWrapUtils.h>

/* Feature specific includes. */
#include <cpss/common/cpssTypes.h>
#include <cpss/px/cnc/cpssPxCnc.h>

static CPSS_PX_CNC_COUNTER_FORMAT_ENT  last_format =
    CPSS_PX_CNC_COUNTER_FORMAT_MODE_0_E;

#define PIPE_WR_INDEXES_SIZE (8)

/**
* @internal wrCpssPxCncUploadInit function
* @endinternal
*
* @brief   Initialize the CNC upload configuration, including DMA queue. Should be
*         used once before first cpssPxCncBlockUploadTrigger call.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success,
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_PARAM             - invalid cncCfgPtr.
* @retval GT_OUT_OF_RANGE          - bad address
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/
static CMD_STATUS wrCpssPxCncUploadInit
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_SW_DEV_NUM          devNum;
    CPSS_DMA_QUEUE_CFG_STC cncCfgPtr;
    GT_STATUS result;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_SW_DEV_NUM)inArgs[0];
    cncCfgPtr.dmaDescBlock = (GT_U8 *)inArgs[1];
    cncCfgPtr.dmaDescBlockSize = (CPSS_PX_CNC_CLIENT_ENT)inArgs[2];

    /* call cpss api function */
    result = cpssPxCncUploadInit(devNum, &cncCfgPtr);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssPxCncBlockClientEnableSet function
* @endinternal
*
* @brief   The function binds/unbinds the selected client to/from a counter block.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssPxCncBlockClientEnableSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_SW_DEV_NUM devNum;
    GT_U32 blockNum;
    CPSS_PX_CNC_CLIENT_ENT client;
    GT_BOOL updateEnable;


    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */

    devNum = (GT_U8)inArgs[0];
    blockNum = (GT_U32)inArgs[1];
    client = (CPSS_PX_CNC_CLIENT_ENT)inArgs[2];
    updateEnable = (GT_BOOL)inArgs[3];

    /* call cpss api function */
    result = cpssPxCncBlockClientEnableSet(
        devNum, blockNum, client, updateEnable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssPxCncBlockClientEnableGet function
* @endinternal
*
* @brief   The function gets bind/unbind of the selected client to a counter block.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssPxCncBlockClientEnableGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_SW_DEV_NUM devNum;
    GT_U32 blockNum;
    CPSS_PX_CNC_CLIENT_ENT client;
    GT_BOOL updateEnable;


    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */

    devNum = (GT_U8)inArgs[0];
    blockNum = (GT_U32)inArgs[1];
    client = (CPSS_PX_CNC_CLIENT_ENT)inArgs[2];

    /* call cpss api function */
    result = cpssPxCncBlockClientEnableGet(
        devNum, blockNum, client, &updateEnable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", updateEnable);

    return CMD_OK;
}

/**
* @internal wrCpssPxCncBlockClientRangesSet function
* @endinternal
*
* @brief   The function sets index ranges per CNC client and Block
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssPxCncBlockClientRangesSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_SW_DEV_NUM devNum;
    GT_U32 blockNum;
    GT_U64 indexRangesBmp;


    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */

    devNum = (GT_U8)inArgs[0];
    blockNum = (GT_U32)inArgs[1];
    indexRangesBmp.l[0] = (GT_U32)inArgs[2];
    indexRangesBmp.l[1] = (GT_U32)inArgs[3];

    /* call cpss api function */
    result = cpssPxCncBlockClientRangesSet(
        devNum, blockNum, indexRangesBmp);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssPxCncBlockClientRangesGet function
* @endinternal
*
* @brief   The function gets index ranges per CNC client and Block
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssPxCncBlockClientRangesGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_SW_DEV_NUM devNum;
    GT_U32 blockNum;
    GT_U64 indexRangesBmp;


    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */

    devNum = (GT_U8)inArgs[0];
    blockNum = (GT_U32)inArgs[1];

    /* call cpss api function */
    result = cpssPxCncBlockClientRangesGet(
        devNum, blockNum, &indexRangesBmp);

    /* pack output arguments to galtis string */
    galtisOutput(
        outArgs, result, "%d%d",
        indexRangesBmp.l[0], indexRangesBmp.l[1]);

    return CMD_OK;
}

/**
* @internal wrCpssPxCncClientByteCountModeSet function
* @endinternal
*
* @brief   The function sets format of CNC counter
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssPxCncClientByteCountModeSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_SW_DEV_NUM devNum;
    CPSS_PX_CNC_CLIENT_ENT client;
    CPSS_PX_CNC_BYTE_COUNT_MODE_ENT countMode;


    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */

    devNum = (GT_U8)inArgs[0];
    client = (CPSS_PX_CNC_CLIENT_ENT)inArgs[1];
    countMode = (CPSS_PX_CNC_BYTE_COUNT_MODE_ENT)inArgs[2];

    /* call cpss api function */
    result = cpssPxCncClientByteCountModeSet(devNum, client, countMode);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssPxCncClientByteCountModeGet function
* @endinternal
*
* @brief   The function gets format of CNC counter
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_BAD_STATE             - on wrong counting mode read
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*                                       on not supported client for device.
*/
static CMD_STATUS wrCpssPxCncClientByteCountModeGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_SW_DEV_NUM devNum;
    CPSS_PX_CNC_CLIENT_ENT client;
    CPSS_PX_CNC_BYTE_COUNT_MODE_ENT countMode;


    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */

    devNum = (GT_U8)inArgs[0];
    client = (CPSS_PX_CNC_CLIENT_ENT)inArgs[1];

    /* call cpss api function */
    result = cpssPxCncClientByteCountModeGet(devNum, client, &countMode);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", countMode);

    return CMD_OK;
}

/**
* @internal wrCpssPxCncCounterClearByReadEnableSet function
* @endinternal
*
* @brief   The function enable/disables clear by read mode of CNC counters read
*         operation.
*         If clear by read mode is disable the counters after read
*         keep the current value and continue to count normally.
*         If clear by read mode is enable the counters load a globally configured
*         value instead of the current value and continue to count normally.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssPxCncCounterClearByReadEnableSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_SW_DEV_NUM devNum;
    GT_BOOL enable;


    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */

    devNum = (GT_U8)inArgs[0];
    enable = (GT_BOOL)inArgs[1];

    /* call cpss api function */
    result = cpssPxCncCounterClearByReadEnableSet(devNum, enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssPxCncCounterClearByReadEnableGet function
* @endinternal
*
* @brief   The function gets clear by read mode status of CNC counters read
*         operation.
*         If clear by read mode is disable the counters after read
*         keep the current value and continue to count normally.
*         If clear by read mode is enable the counters load a globally configured
*         value instead of the current value and continue to count normally.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssPxCncCounterClearByReadEnableGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_SW_DEV_NUM devNum;
    GT_BOOL enable;


    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */

    devNum = (GT_U8)inArgs[0];

    /* call cpss api function */
    result = cpssPxCncCounterClearByReadEnableGet(devNum, &enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", enable);

    return CMD_OK;
}

/**
* @internal wrCpssPxCncCounterClearByReadValueSet function
* @endinternal
*
* @brief   The function sets the counter clear by read globally configured value.
*         If clear by read mode is enable the counters load a globally configured
*         value instead of the current value and continue to count normally.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssPxCncCounterClearByReadValueSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                         result;
    GT_SW_DEV_NUM devNum;
    CPSS_PX_CNC_COUNTER_FORMAT_ENT  format;
    CPSS_PX_CNC_COUNTER_STC         counter;


    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */

    devNum = (GT_U8)inArgs[0];
    format = (CPSS_PX_CNC_COUNTER_FORMAT_ENT)inArgs[1];
    counter.byteCount.l[0]=(GT_U32)inArgs[2];
    counter.byteCount.l[1]=(GT_U32)inArgs[3];
    counter.packetCount.l[0]=(GT_U32)inArgs[4];
    counter.packetCount.l[1]=(GT_U32)inArgs[5];

    last_format = format;

    /* call cpss api function */
    result = cpssPxCncCounterClearByReadValueSet(
        devNum, format, &counter);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssPxCncCounterClearByReadValueGet function
* @endinternal
*
* @brief   The function gets the counter clear by read globally configured value.
*         If clear by read mode is enable the counters load a globally configured
*         value instead of the current value and continue to count normally.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssPxCncCounterClearByReadValueGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                         result;
    GT_SW_DEV_NUM devNum;
    CPSS_PX_CNC_COUNTER_FORMAT_ENT  format;
    CPSS_PX_CNC_COUNTER_STC         counter;


    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */

     devNum = (GT_U8)inArgs[0];
     format = (CPSS_PX_CNC_COUNTER_FORMAT_ENT)inArgs[1];

     last_format = format;

    /* call cpss api function */
    result = cpssPxCncCounterClearByReadValueGet(
        devNum, format, &counter);

    /* pack output arguments to galtis string */
    galtisOutput(
        outArgs, result, "%d%d%d%d",
        counter.byteCount.l[0],counter.byteCount.l[1],
        counter.packetCount.l[0], counter.packetCount.l[1]);

    return CMD_OK;
}

/**
* @internal wrCpssPxCncCounterWraparoundEnableSet function
* @endinternal
*
* @brief   The function enables/disables wraparound for all CNC counters
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssPxCncCounterWraparoundEnableSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_SW_DEV_NUM devNum;
    GT_BOOL enable;


    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */

    devNum = (GT_U8)inArgs[0];
    enable = (GT_BOOL)inArgs[1];

    /* call cpss api function */
    result = cpssPxCncCounterWraparoundEnableSet(devNum, enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssPxCncCounterWraparoundEnableGet function
* @endinternal
*
* @brief   The function gets status of wraparound for all CNC counters
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssPxCncCounterWraparoundEnableGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_SW_DEV_NUM devNum;
    GT_BOOL enable;


    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */

    devNum = (GT_U8)inArgs[0];

    /* call cpss api function */
    result = cpssPxCncCounterWraparoundEnableGet(devNum, &enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", enable);

    return CMD_OK;
}
/***********Table(Refreash only):cpssPxCncCounterWraparoundIndexes************/

static  GT_U32  counterWraparoundIndexesArr[PIPE_WR_INDEXES_SIZE];
static  GT_U32 counterWraparoundIndexNum;
static  GT_U32 counterWraparoundIndex;

/**
* @internal wrCpssPxCncCounterWraparoundIndexesGetFirst function
* @endinternal
*
* @brief   The function gets the counter Wrap Around last 8 indexes
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The wraparound indexes are cleared on read.
*       Since in the HW each pair of wraparound indexes resides in one register,
*       cleared by read, in case not all the 8 possible indexes are read at once,
*       some data may be lost.
*       Recommendation: Always request 8 indexes => indexNumPtr=8 as input.
*
*/
static CMD_STATUS wrCpssPxCncCounterWraparoundIndexesGetFirst
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_SW_DEV_NUM devNum;
    GT_U32 blockNum;


    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    blockNum= (GT_U8)inArgs[1];
    counterWraparoundIndexNum=(GT_U8)inArgs[2];

    /* number of wraparound indexes signaled is limited to 8 - as the array size */
    if(counterWraparoundIndexNum > PIPE_WR_INDEXES_SIZE)
    {
        return CMD_ARG_OVERFLOW;
    }

    /* call cpss api function */
    result = cpssPxCncCounterWraparoundIndexesGet(
        devNum, blockNum, &counterWraparoundIndexNum, counterWraparoundIndexesArr);

    if (result != GT_OK)
    {
        galtisOutput(outArgs, result, "%d", -1);
        return CMD_OK;
    }

    if (counterWraparoundIndexNum == 0)
    {
       galtisOutput(outArgs, result, "%d", -1);
       return CMD_OK;
    }
    counterWraparoundIndex=0;

    inFields[0]=counterWraparoundIndex;
    inFields[1]=counterWraparoundIndexesArr[counterWraparoundIndex];



    /* pack output arguments to galtis string */
    fieldOutput("%d%d", inFields[0],inFields[1]);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%f");

    return CMD_OK;
}
/*****************************************************************************************************/

static CMD_STATUS wrCpssPxCncCounterWraparoundIndexesGetNext
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    counterWraparoundIndex++;

    if(counterWraparoundIndex > counterWraparoundIndexNum-1)
    {
        galtisOutput(outArgs, GT_OK, "%d", -1);
        return CMD_OK;
    }
    inFields[0]=counterWraparoundIndex;
    inFields[1]=counterWraparoundIndexesArr[counterWraparoundIndex];



    /* pack output arguments to galtis string */
   fieldOutput("%d%d", inFields[0],inFields[1]);


    /* pack output arguments to galtis string */
    galtisOutput(outArgs, GT_OK, "%f");

    return CMD_OK;
}

/**
* @internal wrCpssPxCncCounterGet function
* @endinternal
*
* @brief   The function gets the counter contents
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssPxCncCounterGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                         result;
    GT_SW_DEV_NUM devNum;
    GT_U32                            blockNum;
    GT_U32                            index;
    CPSS_PX_CNC_COUNTER_STC         counter;
    CPSS_PX_CNC_COUNTER_FORMAT_ENT  format;


    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */

    devNum   = (GT_U8)inArgs[0];
    blockNum = (GT_U32)inArgs[1];
    index    = (GT_U32)inArgs[2];
    format   = (CPSS_PX_CNC_COUNTER_FORMAT_ENT)inArgs[3];

    last_format = format;

    /* call cpss api function */
    result = cpssPxCncCounterGet(
        devNum, blockNum, index,
        last_format, &counter);

    /* pack output arguments to galtis string */
     galtisOutput(
         outArgs, result, "%d%d%d%d",
         counter.byteCount.l[0],counter.byteCount.l[1],
         counter.packetCount.l[0], counter.packetCount.l[1]);

    return CMD_OK;
}

/**
* @internal wrCpssPxCncBlockUploadTrigger function
* @endinternal
*
* @brief   The function triggers the Upload of the given counters block.
*         In order to initialize storage needed for uploaded counters,
*         cpssPxCncUploadInit must be called first.
*         An application may check that CNC upload finished by
*         cpssPxCncBlockUploadInProcessGet.
*         An application may sequentially upload several CNC blocks before start
*         to retrieve uploaded counters.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_NOT_INITIALIZED       - cpssPxCncUploadInit not called
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_STATE             - if the previous CNC upload in process.
* @retval GT_NOT_SUPPORTED         - block upload operation not supported.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssPxCncBlockUploadTrigger
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_SW_DEV_NUM devNum;
    GT_U32 blockNum;


    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */

    devNum = (GT_U8)inArgs[0];
    blockNum = (GT_U32)inArgs[1];

    /* call cpss api function */
    result = cpssPxCncBlockUploadTrigger(devNum, blockNum);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssPxCncBlockUploadInProcessGet function
* @endinternal
*
* @brief   The function gets bitmap of numbers of such counters blocks that upload
*         of them yet in process. The HW cannot keep more then one block in such
*         state, but an API matches the original HW representation of the state.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_NOT_SUPPORTED         - block upload operation not supported.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssPxCncBlockUploadInProcessGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_SW_DEV_NUM devNum;
    GT_U32 inProcessBlocksBmp;


    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */

    devNum = (GT_U8)inArgs[0];

    /* call cpss api function */
    result = cpssPxCncBlockUploadInProcessGet(
        devNum, &inProcessBlocksBmp);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", inProcessBlocksBmp);

    return CMD_OK;
}

/***********Table(Refreash only):cpssPxCncUploadedBlock************/
#define WRR_CNC_UPLOAD_MAX_INDEX_MAC        2048
static  CPSS_PX_CNC_COUNTER_STC  uploadedBlockIndexesArr[WRR_CNC_UPLOAD_MAX_INDEX_MAC];
static  GT_U32                     numOfCounterValues;
static  GT_U32                     uploadedBlockIndex;

/**
* @internal wrCpssPxCncUploadedBlockGetFirst function
* @endinternal
*
* @brief   The function return a block (array) of CNC counter values,
*         the maximal number of elements defined by the caller.
*         The CNC upload may triggered by cpssPxCncBlockUploadTrigger.
*         The CNC upload transfers whole CNC block (1K CNC counters)
*         to address upload queue. An application must get all transferred
*         counters. An application may sequentially upload several CNC blocks
*         before start to retrieve uploaded counters.
*         The device may transfer only part of CNC block because of DMA queue
*         full. In this case the cpssPxCncUploadedBlockGet may return only part of
*         the CNC block with return GT_OK. An application must to call
*         cpssPxCncUploadedBlockGet one more time to get rest of the block.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success
* @retval GT_NO_MORE               - the action succeeded and there are no more
*                                       waiting
*                                       CNC counter value
* @retval GT_NOT_READY             - Upload started after upload trigger or
*                                       continued after queue rewind but yet
*                                       not paused due to queue full and yet not finished.
*                                       The part of queue memory does not contain uploaded counters yet.
*                                       No counters retrieved.
* @retval GT_FAIL                  - on failure
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_SUPPORTED         - block upload operation not supported
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssPxCncUploadedBlockGetFirst
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                         result;
    GT_SW_DEV_NUM devNum;
    CPSS_PX_CNC_COUNTER_FORMAT_ENT  format;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */

    devNum = (GT_U8)inArgs[0];
    numOfCounterValues=(GT_U32)inArgs[1];
    format = (CPSS_PX_CNC_COUNTER_FORMAT_ENT)inArgs[2];

    last_format = format;

    /* number of requested indexes greater then array size */
    if(numOfCounterValues > WRR_CNC_UPLOAD_MAX_INDEX_MAC)
    {
        return CMD_ARG_OVERFLOW;
    }
    /* call cpss api function */
    result = cpssPxCncUploadedBlockGet(
        devNum, &numOfCounterValues,
        format, uploadedBlockIndexesArr);

    if ( ((result != GT_OK) && (result != GT_NO_MORE)) ||
         (numOfCounterValues == 0) )
        {
            galtisOutput(outArgs, result, "%d", -1);
            return CMD_OK;
    }

    uploadedBlockIndex=0;

    inFields[0]=uploadedBlockIndex;
    inFields[1]=uploadedBlockIndexesArr[uploadedBlockIndex].byteCount.l[0];
    inFields[2]=uploadedBlockIndexesArr[uploadedBlockIndex].byteCount.l[1];
    inFields[3]=uploadedBlockIndexesArr[uploadedBlockIndex].packetCount.l[0];
    inFields[4]=uploadedBlockIndexesArr[uploadedBlockIndex].packetCount.l[1];


     /* pack output arguments to galtis string */
    fieldOutput(
        "%d%d%d%d%d",
        inFields[0],inFields[1],inFields[2],inFields[3],inFields[4]);


    /* pack output arguments to galtis string */
    galtisOutput(outArgs, GT_OK, "%f");

    return CMD_OK;
}

/*********************************************************************/
static CMD_STATUS wrCpssPxCncUploadedBlockGetNext
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{

    uploadedBlockIndex++;

    if (uploadedBlockIndex > numOfCounterValues-1)
    {
        galtisOutput(outArgs, GT_OK, "%d", -1);
        return CMD_OK;
    }
    inFields[0]=uploadedBlockIndex;
    inFields[1]=uploadedBlockIndexesArr[uploadedBlockIndex].byteCount.l[0];
    inFields[2]=uploadedBlockIndexesArr[uploadedBlockIndex].byteCount.l[1];
    inFields[3]=uploadedBlockIndexesArr[uploadedBlockIndex].packetCount.l[0];
    inFields[4]=uploadedBlockIndexesArr[uploadedBlockIndex].packetCount.l[1];



    /* pack output arguments to galtis string */
    fieldOutput(
        "%d%d%d%d%d",
        inFields[0],inFields[1],inFields[2],inFields[3],inFields[4]);


    /* pack output arguments to galtis string */
    galtisOutput(outArgs, GT_OK, "%f");

    return CMD_OK;
}

/**
* @internal wrCpssPxCncEgressQueueClientModeSet function
* @endinternal
*
* @brief   The function sets Egress Queue client counting mode
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssPxCncEgressQueueClientModeSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                                   result;
    GT_SW_DEV_NUM devNum;
    CPSS_PX_CNC_EGRESS_QUEUE_CLIENT_MODE_ENT  mode;


    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */

    devNum = (GT_U8)inArgs[0];
    mode = (CPSS_PX_CNC_EGRESS_QUEUE_CLIENT_MODE_ENT)inArgs[1];

    /* call cpss api function */
    result = cpssPxCncEgressQueueClientModeSet(
        devNum, mode);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssPxCncEgressQueueClientModeGet function
* @endinternal
*
* @brief   The function gets Egress Queue client counting mode
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssPxCncEgressQueueClientModeGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                                   result;
    GT_SW_DEV_NUM devNum;
    CPSS_PX_CNC_EGRESS_QUEUE_CLIENT_MODE_ENT  mode;


    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */

    devNum = (GT_U8)inArgs[0];

    /* call cpss api function */
    result = cpssPxCncEgressQueueClientModeGet(
        devNum, &mode);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", mode);

    return CMD_OK;
}

/**
* @internal wrCpssPxCncCounterFormatSet function
* @endinternal
*
* @brief   The function sets byte count mode.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssPxCncCounterFormatSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                         result;
    GT_SW_DEV_NUM devNum;
    GT_U32                            block;
    CPSS_PX_CNC_COUNTER_FORMAT_ENT  format;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */

    devNum  = (GT_U8)inArgs[0];
    block   = (GT_U32)inArgs[1];
    format  = (CPSS_PX_CNC_COUNTER_FORMAT_ENT)inArgs[2];

    /* call cpss api function */
    result = cpssPxCncCounterFormatSet(
        devNum, block, format);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssPxCncCounterFormatGet function
* @endinternal
*
* @brief   The function gets byte count mode.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssPxCncCounterFormatGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                         result;
    GT_SW_DEV_NUM devNum;
    GT_U32                            block;
    CPSS_PX_CNC_COUNTER_FORMAT_ENT  format;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */

    devNum  = (GT_U8)inArgs[0];
    block   = (GT_U32)inArgs[1];

    /* call cpss api function */
    result = cpssPxCncCounterFormatGet(
        devNum, block, &format);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", format);

    return CMD_OK;
}


/**** database initialization **************************************/



static CMD_COMMAND dbCommands[] =
{
        {"cpssPxCncUploadInit",
         &wrCpssPxCncUploadInit,
         3, 0},
        {"cpssPxCncBlockClientEnableSet",
         &wrCpssPxCncBlockClientEnableSet,
         4, 0},
        {"cpssPxCncBlockClientEnableGet",
         &wrCpssPxCncBlockClientEnableGet,
         3, 0},
        {"cpssPxCncBlockClientRangesSet",
         &wrCpssPxCncBlockClientRangesSet,
         4, 0},
        {"cpssPxCncBlockClientRangesGet",
         &wrCpssPxCncBlockClientRangesGet,
         2, 0},
        {"cpssPxCncClientByteCountModeSet",
         &wrCpssPxCncClientByteCountModeSet,
         3, 0},
        {"cpssPxCncClientByteCountModeGet",
         &wrCpssPxCncClientByteCountModeGet,
         2, 0},
        {"cpssPxCncCounterClearByReadEnableSet",
         &wrCpssPxCncCounterClearByReadEnableSet,
         2, 0},
        {"cpssPxCncCounterClearByReadEnableGet",
         &wrCpssPxCncCounterClearByReadEnableGet,
         1, 0},
        {"cpssPxCncCounterClearByReadValueSet",
         &wrCpssPxCncCounterClearByReadValueSet,
         6, 0},
        {"cpssPxCncCounterClearByReadValueGet",
         &wrCpssPxCncCounterClearByReadValueGet,
         2, 0},
        {"cpssPxCncCounterWraparoundEnableSet",
         &wrCpssPxCncCounterWraparoundEnableSet,
         2, 0},
        {"cpssPxCncCounterWraparoundEnableGet",
         &wrCpssPxCncCounterWraparoundEnableGet,
         1, 0},
        {"cpssPxCncCounterWraparoundIndexesGetFirst",
         &wrCpssPxCncCounterWraparoundIndexesGetFirst,
         3, 0},
        {"cpssPxCncCounterWraparoundIndexesGetNext",
         &wrCpssPxCncCounterWraparoundIndexesGetNext,
         0, 0},
        {"cpssPxCncCounterGet",
         &wrCpssPxCncCounterGet,
         4, 0},
        {"cpssPxCncBlockUploadTrigger",
         &wrCpssPxCncBlockUploadTrigger,
         2, 0},
        {"cpssPxCncBlockUploadInProcessGet",
         &wrCpssPxCncBlockUploadInProcessGet,
         1, 0},
        {"cpssPxCncUploadedBlockGetFirst",
         &wrCpssPxCncUploadedBlockGetFirst,
         3, 0},
        {"cpssPxCncUploadedBlockGetNext",
         &wrCpssPxCncUploadedBlockGetNext,
         0, 0},
        {"cpssPxCncEgressQueueClientModeSet",
         &wrCpssPxCncEgressQueueClientModeSet,
         2, 0},
        {"cpssPxCncEgressQueueClientModeGet",
         &wrCpssPxCncEgressQueueClientModeGet,
         1, 0},
        {"cpssPxCncCounterFormatSet",
         &wrCpssPxCncCounterFormatSet,
         3, 0},
        {"cpssPxCncCounterFormatGet",
         &wrCpssPxCncCounterFormatGet,
         2, 0}
};

#define numCommands (sizeof(dbCommands) / sizeof(CMD_COMMAND))



/**
* @internal cmdLibInitCpssPxCnc function
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
GT_STATUS cmdLibInitCpssPxCnc
(
    GT_VOID
)
{
    return  cmdInitLibrary(dbCommands, numCommands);
}
