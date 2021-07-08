/*******************************************************************************
*              Copyright 2001, GALILEO TECHNOLOGY, LTD.
*
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL. NO RIGHTS ARE GRANTED
* HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT OF MARVELL OR ANY THIRD
* PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE DISCRETION TO REQUEST THAT THIS
* CODE BE IMMEDIATELY RETURNED TO MARVELL. THIS CODE IS PROVIDED "AS IS".
* MARVELL MAKES NO WARRANTIES, EXPRESS, IMPLIED OR OTHERWISE, REGARDING ITS
* ACCURACY, COMPLETENESS OR PERFORMANCE. MARVELL COMPRISES MARVELL TECHNOLOGY
* GROUP LTD. (MTGL) AND ITS SUBSIDIARIES, MARVELL INTERNATIONAL LTD. (MIL),
* MARVELL TECHNOLOGY, INC. (MTI), MARVELL SEMICONDUCTOR, INC. (MSI), MARVELL
* ASIA PTE LTD. (MAPL), MARVELL JAPAN K.K. (MJKK), GALILEO TECHNOLOGY LTD. (GTL)
* AND GALILEO TECHNOLOGY, INC. (GTI).
********************************************************************************
*/
/**
********************************************************************************
* @file wrapCpssDxChOamSrv.c
*
* @brief Wrapper functions for oam service cpss.dxCh functions
*
* @version   3
********************************************************************************
*/

/* Common galtis includes */
#include <galtisAgent/wrapUtil/cmdCpssPresteraUtils.h>
#include <cmdShell/cmdDb/cmdBase.h>
#include <cmdShell/common/cmdWrapUtils.h>


/* Feature specific includes. */
#include <cpss/dxCh/dxChxGen/oam/cpssDxChOamSrv.h>

/**
* @internal wrCpssDxChOamSrvLoopbackConfigSet function
* @endinternal
*
* @brief   Set OAM service CPU Loopback configuration per flow ID.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - if parameters are bad
*
* @note wrapper parameters:
*       devNum        = (GT_U8)inArgs[0];
*       flowId        = (GT_U32)inArgs[1];
*       outOfSequenceEnable  = (GT_BOOL)inArgs[2];
*       invalidTlvEnable   = (GT_BOOL)inArgs[3];
*       tlvDataCrc32Val    = (GT_U32)inArgs[4];
*
*/
static CMD_STATUS wrCpssDxChOamSrvLoopbackConfigSet

(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                           result;
    GT_U8                               devNum;
    GT_U32                              flowId;
    CPSS_DXCH_OAM_SRV_LB_CONFIG_STC     lbConfig;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    cpssOsMemSet(&lbConfig, '\0', sizeof(CPSS_DXCH_OAM_SRV_LB_CONFIG_STC));

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    flowId = (GT_U32)inArgs[1];
    lbConfig.outOfSequenceEnable = (GT_BOOL)inArgs[2];
    lbConfig.invalidTlvEnable = (GT_BOOL)inArgs[3];
    lbConfig.tlvDataCrc32Val = (GT_U32)inArgs[4];
    
    /* call cpss api function */
    result = cpssDxChOamSrvLoopbackConfigSet(devNum, flowId, &lbConfig);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxChOamSrvLoopbackConfigGet function
* @endinternal
*
* @brief   Get OAM Loopback configuration per flow ID from service CPU.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - if parameters are bad
*
* @note wrapper parameters:
*       devNum        = (GT_U8)inArgs[0];
*       flowId        = (GT_U32)inArgs[1];
*       wrapper OUTPUT parameters:
*       outOfSequenceEnable  = (GT_BOOL)inFields[0];
*       invalidTlvEnable   = (GT_BOOL)inFields[1];
*       tlvDataCrc32Val    = (GT_U32)inFields[2];
*
*/
static CMD_STATUS wrCpssDxChOamSrvLoopbackConfigGet

(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                           result;
    GT_U8                               devNum;
    GT_U32                              flowId;
    CPSS_DXCH_OAM_SRV_LB_CONFIG_STC     lbConfig;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    cpssOsMemSet(&lbConfig, '\0', sizeof(CPSS_DXCH_OAM_SRV_LB_CONFIG_STC));

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    flowId = (GT_U32)inArgs[1];
    
    /* call cpss api function */
    result = cpssDxChOamSrvLoopbackConfigGet(devNum, flowId, &lbConfig);

    inFields[0] = lbConfig.outOfSequenceEnable;
    inFields[1] = lbConfig.invalidTlvEnable;
    inFields[2] = lbConfig.tlvDataCrc32Val;

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d%d%d", 
                 inFields[0], inFields[1], inFields[2]);

    return CMD_OK;
}

/**
* @internal wrCpssDxChOamSrvLoopbackStatisticalCountersGet function
* @endinternal
*
* @brief   Get the OAM LB statistical counters from service CPU per flow ID.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - if parameters are bad
*
* @note wrapper parameters:
*       devNum        = (GT_U8)inArgs[0];
*       flowId        = (GT_U32)inArgs[1];
*       clearOnRead      = (GT_BOOL)inArgs[2];
*       wrapper OUTPUT parameters:
*       outOfSequenceCount  = (GT_U32)inFields[0];
*       totalCheckedDataTlv  = (GT_U32)inFields[1];
*       totalInvalidDataTlv  = (GT_U32)inFields[2];
*
*/
static CMD_STATUS wrCpssDxChOamSrvLoopbackStatisticalCountersGet

(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                           result;
    GT_U8                               devNum;
    GT_U32                              flowId;
    GT_BOOL                             clearOnRead;
    GT_U32                              timeout = 0;
    CPSS_DXCH_OAM_SRV_LB_COUNTERS_STC   lbCounters;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    cpssOsMemSet(&lbCounters, '\0', sizeof(CPSS_DXCH_OAM_SRV_LB_COUNTERS_STC));

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    flowId = (GT_U32)inArgs[1];
    clearOnRead = (GT_BOOL)inArgs[2];
    
    /* call cpss api function */
    result = cpssDxChOamSrvLoopbackStatisticalCountersGet(devNum,
                                                          flowId,
                                                          clearOnRead,
                                                          timeout,
                                                          &lbCounters);

    inFields[0] = lbCounters.outOfSequenceCount;
    inFields[1] = lbCounters.totalCheckedDataTlv;
    inFields[2] = lbCounters.totalInvalidDataTlv;

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d%d%d", 
                 inFields[0], inFields[1], inFields[2]);

    return CMD_OK;
}

/**
* @internal wrCpssDxChOamSrvLoopbackStatisticalCountersClear function
* @endinternal
*
* @brief   Clear the LB statistical counters in service CPU per flow ID.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - if parameters are bad
*
* @note wrapper parameters:
*       devNum        = (GT_U8)inArgs[0];
*       flowId        = (GT_U32)inArgs[1];
*
*/
static CMD_STATUS wrCpssDxChOamSrvLoopbackStatisticalCountersClear

(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                           result;
    GT_U8                               devNum;
    GT_U32                              flowId;
    
    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    flowId = (GT_U32)inArgs[1];
    
    /* call cpss api function */
    result = cpssDxChOamSrvLoopbackStatisticalCountersClear(devNum, flowId);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxChOamSrvDmConfigSet function
* @endinternal
*
* @brief   Set OAM service CPU Delay Measurement configuration per flow ID.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - if parameters are bad
*
* @note wrapper parameters:
*       devNum        = (GT_U8)inArgs[0];
*       flowId        = (GT_U32)inArgs[1];
*       countersEnable    = (GT_BOOL)inArgs[2];
*
*/
static CMD_STATUS wrCpssDxChOamSrvDmConfigSet

(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                           result;
    GT_U8                               devNum;
    GT_U32                              flowId;
    CPSS_DXCH_OAM_SRV_DM_CONFIG_STC     dmConfig;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    cpssOsMemSet(&dmConfig, '\0', sizeof(CPSS_DXCH_OAM_SRV_DM_CONFIG_STC));

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    flowId = (GT_U32)inArgs[1];
    dmConfig.countersEnable = (GT_BOOL)inArgs[2];
    
    /* call cpss api function */
    result = cpssDxChOamSrvDmConfigSet(devNum, flowId, &dmConfig);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxChOamSrvDmConfigGet function
* @endinternal
*
* @brief   Get OAM service CPU Delay Measurement configuration per flow ID.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - if parameters are bad
*
* @note wrapper parameters:
*       devNum        = (GT_U8)inArgs[0];
*       flowId        = (GT_U32)inArgs[1];
*       wrapper OUTPUT parameters:
*       countersEnable      = (GT_BOOL)inFields[0];
*
*/
static CMD_STATUS wrCpssDxChOamSrvDmConfigGet

(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                           result;
    GT_U8                               devNum;
    GT_U32                              flowId;
    CPSS_DXCH_OAM_SRV_DM_CONFIG_STC     dmConfig;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    cpssOsMemSet(&dmConfig, '\0', sizeof(CPSS_DXCH_OAM_SRV_DM_CONFIG_STC));

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    flowId = (GT_U32)inArgs[1];
    
    /* call cpss api function */
    result = cpssDxChOamSrvDmConfigGet(devNum, flowId, &dmConfig);

    inFields[0] = dmConfig.countersEnable;
    
    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", inFields[0]);

    return CMD_OK;
}

/**
* @internal wrCpssDxChOamSrvDmStatisticalCountersGet function
* @endinternal
*
* @brief   Get the Delay Measurement statistical counters per flow ID from service CPU.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - if parameters are bad
*
* @note wrapper parameters:
*       devNum        = (GT_U8)inArgs[0];
*       flowId        = (GT_U32)inArgs[1];
*       clearOnRead      = (GT_BOOL)inArgs[2];
*       wrapper OUTPUT parameters:
*       twoWayFrameDelay     = (GT_U32)inFields[0];
*       oneWayFrameDelayForward  = (GT_U32)inFields[1];
*       oneWayFrameDelayBackward = (GT_U32)inFields[2];
*
*/
static CMD_STATUS wrCpssDxChOamSrvDmStatisticalCountersGet

(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                           result;
    GT_U8                               devNum;
    GT_U32                              flowId;
    GT_BOOL                             clearOnRead;
    CPSS_DXCH_OAM_SRV_DM_COUNTERS_STC   dmCounters;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    cpssOsMemSet(&dmCounters, '\0', sizeof(CPSS_DXCH_OAM_SRV_LB_COUNTERS_STC));

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    flowId = (GT_U32)inArgs[1];
    clearOnRead = (GT_BOOL)inArgs[2];
    
    /* call cpss api function */
    result = cpssDxChOamSrvDmStatisticalCountersGet(devNum,
                                                    flowId,
                                                    clearOnRead,
                                                    &dmCounters);

    inFields[0] = dmCounters.basicCounters.twoWayFrameDelay;
    inFields[1] = dmCounters.basicCounters.oneWayFrameDelayForward;
    inFields[2] = dmCounters.basicCounters.oneWayFrameDelayBackward;
    
    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d%d%d", 
                 inFields[0], inFields[1], inFields[2]);

    return CMD_OK;
}

/**
* @internal wrCpssDxChOamSrvDmStatisticalCountersClear function
* @endinternal
*
* @brief   Clear the Delay Measurement statistical counters per flow ID in service CPU.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - if parameters are bad
*
* @note wrapper parameters:
*       devNum        = (GT_U8)inArgs[0];
*       flowId        = (GT_U32)inArgs[1];
*
*/
static CMD_STATUS wrCpssDxChOamSrvDmStatisticalCountersClear

(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                           result;
    GT_U8                               devNum;
    GT_U32                              flowId;
    
    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    flowId = (GT_U32)inArgs[1];
    
    /* call cpss api function */
    result = cpssDxChOamSrvDmStatisticalCountersClear(devNum, flowId);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxChOamSrvLmConfigSet function
* @endinternal
*
* @brief   Set OAM service CPU Loss Measurement configuration per flow ID.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - if parameters are bad
*
* @note wrapper parameters:
*       devNum        = (GT_U8)inArgs[0];
*       flowId        = (GT_U32)inArgs[1];
*       countersEnable    = (GT_BOOL)inArgs[2];
*
*/
static CMD_STATUS wrCpssDxChOamSrvLmConfigSet

(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                           result;
    GT_U8                               devNum;
    GT_U32                              flowId;
    CPSS_DXCH_OAM_SRV_LM_CONFIG_STC     lmConfig;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    cpssOsMemSet(&lmConfig, '\0', sizeof(CPSS_DXCH_OAM_SRV_LM_CONFIG_STC));

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    flowId = (GT_U32)inArgs[1];
    lmConfig.countersEnable = (GT_BOOL)inArgs[2];
    
    /* call cpss api function */
    result = cpssDxChOamSrvLmConfigSet(devNum, flowId, &lmConfig);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxChOamSrvLmConfigGet function
* @endinternal
*
* @brief   Get OAM service CPU Loss Measurement configuration per flow ID.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - if parameters are bad
*
* @note wrapper parameters:
*       devNum        = (GT_U8)inArgs[0];
*       flowId        = (GT_U32)inArgs[1];
*       wrapper OUTPUT parameters:
*       countersEnable    = (GT_BOOL)inFields[0];
*
*/
static CMD_STATUS wrCpssDxChOamSrvLmConfigGet

(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                           result;
    GT_U8                               devNum;
    GT_U32                              flowId;
    CPSS_DXCH_OAM_SRV_LM_CONFIG_STC     lmConfig;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    cpssOsMemSet(&lmConfig, '\0', sizeof(CPSS_DXCH_OAM_SRV_LM_CONFIG_STC));

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    flowId = (GT_U32)inArgs[1];
    
    /* call cpss api function */
    result = cpssDxChOamSrvLmConfigGet(devNum, flowId, &lmConfig);

    inFields[0] = lmConfig.countersEnable;
    
    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", inFields[0]);

    return CMD_OK;
}

/**
* @internal wrCpssDxChOamSrvLmStatisticalCountersGet function
* @endinternal
*
* @brief   Get the Loss Measurement statistical counters per flow ID from service CPU.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - if parameters are bad
*
* @note wrapper parameters:
*       devNum        = (GT_U8)inArgs[0];
*       flowId        = (GT_U32)inArgs[1];
*       clearOnRead      = (GT_BOOL)inArgs[2];
*       wrapper OUTPUT parameters:
*       twoWayFrameDelay     = (GT_U32)inFields[0];
*       oneWayFrameDelayForward  = (GT_U32)inFields[1];
*       oneWayFrameDelayBackward = (GT_U32)inFields[2];
*
*/
static CMD_STATUS wrCpssDxChOamSrvLmStatisticalCountersGet

(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                           result;
    GT_U8                               devNum;
    GT_U32                              flowId;
    GT_BOOL                             clearOnRead;
    CPSS_DXCH_OAM_SRV_LM_COUNTERS_STC   lmCounters;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    cpssOsMemSet(&lmCounters, '\0', sizeof(CPSS_DXCH_OAM_SRV_LM_COUNTERS_STC));

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    flowId = (GT_U32)inArgs[1];
    clearOnRead = (GT_BOOL)inArgs[2];
    
    /* call cpss api function */
    result = cpssDxChOamSrvLmStatisticalCountersGet(devNum,
                                                    flowId,
                                                    clearOnRead,
                                                    &lmCounters);

    inFields[0] = lmCounters.basicCounters.txFrameCntForward;
    inFields[1] = lmCounters.basicCounters.rxFrameCntForward;
    inFields[2] = lmCounters.basicCounters.txFrameCntBackward;
    inFields[3] = lmCounters.basicCounters.rxFrameCntBackward;
    inFields[4] = lmCounters.basicCounters.farEndFrameLoss;
    inFields[5] = lmCounters.basicCounters.nearEndFrameLoss;
    
    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d%d%d%d%d%d", 
                 inFields[0], inFields[1], inFields[2],
                 inFields[3], inFields[4], inFields[5]);

    return CMD_OK;
}

/**
* @internal wrCpssDxChOamSrvLmStatisticalCountersClear function
* @endinternal
*
* @brief   Clear the Loss Measurement statistical counters per flow ID in service CPU.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - if parameters are bad
*
* @note wrapper parameters:
*       devNum        = (GT_U8)inArgs[0];
*       flowId        = (GT_U32)inArgs[1];
*
*/
static CMD_STATUS wrCpssDxChOamSrvLmStatisticalCountersClear

(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                           result;
    GT_U8                               devNum;
    GT_U32                              flowId;
    
    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    flowId = (GT_U32)inArgs[1];
    
    /* call cpss api function */
    result = cpssDxChOamSrvLmStatisticalCountersClear(devNum, flowId);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxChOamSrvLocalMacAddressSet function
* @endinternal
*
* @brief   Globally configure the MAC address table in the service CPU.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - if parameters are bad
*
* @note wrapper parameters:
*       devNum        = (GT_U8)inArgs[0];
*       index         = (GT_U32)inArgs[1];
*       macAddr        = (MAC)(H)inArgs[2];
*
*/
static CMD_STATUS wrCpssDxChOamSrvLocalMacAddressSet

(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                           result;
    GT_U8                               devNum;
    GT_U32                              index;
    GT_ETHERADDR     		            macAddr;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    cpssOsMemSet(&macAddr, '\0', sizeof(GT_ETHERADDR));

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    index = (GT_U32)inArgs[1];
    galtisMacAddr(&macAddr, (GT_U8*)inArgs[2]);
    
    /* call cpss api function */
    result = cpssDxChOamSrvLocalMacAddressSet(devNum, index, &macAddr);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxChOamSrvLocalMacAddressGet function
* @endinternal
*
* @brief   Globally get the MAC address table in the service CPU.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - if parameters are bad
*
* @note wrapper parameters:
*       devNum        = (GT_U8)inArgs[0];
*       index         = (GT_U32)inArgs[1];
*       wrapper OUTPUT parameters:
*       macAddr        = (MAC)(H)inFields[0];
*
*/
static CMD_STATUS wrCpssDxChOamSrvLocalMacAddressGet

(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                           result;
    GT_U8                               devNum;
    GT_U32                              index;
    GT_ETHERADDR     		            macAddr;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    cpssOsMemSet(&macAddr, '\0', sizeof(GT_ETHERADDR));

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    index = (GT_U32)inArgs[1];
    
    /* call cpss api function */
    result = cpssDxChOamSrvLocalMacAddressGet(devNum, index, &macAddr);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%6b", macAddr.arEther);

    return CMD_OK;
}

/**
* @internal wrCpssDxChOamSrvLocalInterfaceMacIndexSet function
* @endinternal
*
* @brief   Configure the MAC address index of a local interface.
*         If OAM packet that come to service CPU is multicast packet, and this packet
*         need to be send back from the CPU - the source mac for this packet will be
*         taken from this table according to source interface that packet comes from.
*         the destination mac will be the incoming source mac.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - if parameters are bad
*
* @note wrapper parameters:
*       devNum        = (GT_U8)inArgs[0];
*       portNum        = (GT_U32)inArgs[1];
*       isPhysicalPort    = (GT_BOOL)inArgs[2];
*       macAddrIndex     = (GT_U32)inArgs[3];
*
*/
static CMD_STATUS wrCpssDxChOamSrvLocalInterfaceMacIndexSet

(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                                   result;
    GT_U8                                       devNum;
    GT_PORT_NUM                                 portNum;
    GT_BOOL                                     isPhysicalPort;
    GT_U32	     		   	                    macAddIndex;
    
    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    portNum = (GT_PORT_NUM)inArgs[1];
    isPhysicalPort = (GT_BOOL)inArgs[2];
    macAddIndex = (GT_U32)inArgs[3];
    
    /* call cpss api function */
    result = cpssDxChOamSrvLocalInterfaceMacIndexSet(devNum, portNum, isPhysicalPort, macAddIndex);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxChOamSrvLocalInterfaceMacIndexGet function
* @endinternal
*
* @brief   Configure the MAC address index of a local interface.
*         If OAM packet that come to service CPU is multicast packet, and this packet
*         need to be send back from the CPU - the source mac for this packet will be
*         taken from this table according to source interface that packet comes from.
*         the destination mac will be the incoming source mac.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - if parameters are bad
*
* @note wrapper parameters:
*       devNum        = (GT_U8)inArgs[0];
*       portNum        = (GT_U32)inArgs[1];
*       isPhysicalPort    = (GT_BOOL)inArgs[2];
*       wrapper OUTPUT parameters:
*       macAddrIndex     = (GT_U32)inFields[0];
*
*/
static CMD_STATUS wrCpssDxChOamSrvLocalInterfaceMacIndexGet

(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                                   result;
    GT_U8                                       devNum;
    GT_PORT_NUM                                 portNum;
    GT_BOOL                                     isPhysicalPort;
    GT_U32	     		   	                    macAddIndex;
    
    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    portNum = (GT_PORT_NUM)inArgs[1];
    isPhysicalPort = (GT_BOOL)inArgs[2];
    
    /* call cpss api function */
    result = cpssDxChOamSrvLocalInterfaceMacIndexGet(devNum, portNum, isPhysicalPort, &macAddIndex);

    inFields[0] = macAddIndex;
    
    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", inFields[0]);

    return CMD_OK;
}

/**
* @internal wrCpssDxChOamSrvRxFlowEntry1731FwdSet function
* @endinternal
*
* @brief   Globally configure the MAC address table in the service CPU. for Y.1731 protocol.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - if parameters are bad
*
* @note wrapper parameters:
*       input args:
*       devNum        = (GT_U8)inArgs[0];
*       flowId        = (GT_U32)inArgs[1];
*       mpType        = (CPSS_DXCH_OAM_SRV_MP_TYPE_ENT)inArgs[2];
*       -CPSS_DXCH_OAM_SRV_MP_TYPE_UP_MEP_E
*       -CPSS_DXCH_OAM_SRV_MP_TYPE_DOWN_MEP_E
*       -CPSS_DXCH_OAM_SRV_MP_TYPE_MIP_E
*       -CPSS_DXCH_OAM_SRV_MP_TYPE_OTHER_E
*       cfmOffset       = (GT_U32)inArgs[3];
*       dsaParamValid     = (GT_BOOL)inArgs[4];
*       dsaTagType      = (CPSS_DXCH_NET_DSA_TYPE_ENT)inArgs[5];
*       vpt          = (GT_U8)inArgs[6];
*       cfiBit        = (GT_8)inArgs[7];
*       vid          = (GT_U16)inArgs[8];
*       dropOnSource     = (GT_BOOL)inArgs[9];
*       packetIsLooped    = (GT_BOOL)inArgs[10];
*       srcIsTagged      = (GT_BOOL)inFields[11];
*       srcHwDev       = (GT_U32)inFields[12];
*       srcIsTrunk      = (GT_BOOL)inFields[13];
*       trunkId        = (GT_U16)inFields[14];
*       srcPortNum      = (GT_U32)inFields[15];
*       srcId         = (GT_U32)inFields[16];
*       egrFilterRegistered  = (GT_BOOL)inFields[17];
*       wasRouted       = (GT_BOOL)inFields[18];
*       qosProfileIndex    = (GT_U32)inFields[19];
*       dstHwDevNum      = (GT_U32)inFields[20];
*       dstPortNum      = (GT_U32)inFields[21];
*       isTrgPhyPortValid   = (GT_BOOL)inFields[22];
*       dstEport       = (GT_U32)inFields[23];
*       tag0TpidIndex     = (GT_U32)inFields[24];
*
*/
static CMD_STATUS wrCpssDxChOamSrvRxFlowEntry1731FwdSet

(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                                   result;
    GT_U8                                       devNum;
    GT_U32                                      flowId;
    CPSS_DXCH_OAM_SRV_RX_FLOW_FRAME_PARAMS_STC  frameParams;
    CPSS_DXCH_NET_DSA_PARAMS_STC                dsaParam;
    GT_PORT_NUM                                 __Port; /* Dummy for converting. */
    GT_HW_DEV_NUM                               __HwDev; /* Dummy for converting. */
    
    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    cpssOsMemSet(&frameParams, '\0', sizeof(CPSS_DXCH_OAM_SRV_RX_FLOW_FRAME_PARAMS_STC));
    cpssOsMemSet(&dsaParam, '\0', sizeof(CPSS_DXCH_NET_DSA_PARAMS_STC));

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    flowId = (GT_U32)inArgs[1];
    
    frameParams.frameType = CPSS_DXCH_OAM_SRV_FRAME_TYPE_1731_E;

    frameParams.frame.frame1731.mpType = (CPSS_DXCH_OAM_SRV_MP_TYPE_ENT)inArgs[2];
    frameParams.frame.frame1731.cfmOffset = (GT_U32)inArgs[3];
    frameParams.frame.frame1731.transmitParams.dsaParamValid = (GT_U32)inArgs[4];

    if(frameParams.frame.frame1731.transmitParams.dsaParamValid)
    {
        dsaParam.dsaType = CPSS_DXCH_NET_DSA_CMD_FORWARD_E;

        dsaParam.commonParams.dsaTagType = (CPSS_DXCH_NET_DSA_TYPE_ENT)inArgs[5];
        dsaParam.commonParams.vpt = (GT_U8)inArgs[6];
        dsaParam.commonParams.cfiBit = (GT_8)inArgs[7];
        dsaParam.commonParams.vid = (GT_U16)inArgs[8];
        dsaParam.commonParams.dropOnSource = (GT_BOOL)inArgs[9];
        dsaParam.commonParams.packetIsLooped = (GT_BOOL)inArgs[10];

        dsaParam.dsaInfo.forward.srcIsTagged = (GT_BOOL)inArgs[11];
        dsaParam.dsaInfo.forward.srcHwDev = (GT_HW_DEV_NUM)inArgs[12];
        dsaParam.dsaInfo.forward.srcIsTrunk = (GT_BOOL)inArgs[13];
        if(dsaParam.dsaInfo.forward.srcIsTrunk == GT_TRUE)
        {
            dsaParam.dsaInfo.forward.source.trunkId = (GT_TRUNK_ID)inArgs[14];
            CONVERT_TRUNK_ID_TEST_TO_CPSS_MAC(dsaParam.dsaInfo.forward.source.trunkId);
        
            __HwDev = dsaParam.dsaInfo.forward.srcHwDev;
            CONVERT_DEV_PORT_DATA_MAC(__HwDev,__Port);
            dsaParam.dsaInfo.forward.srcHwDev = __HwDev;
        }
        else
        {
            dsaParam.dsaInfo.forward.source.portNum = (GT_PORT_NUM)inArgs[15];
        
            __HwDev = dsaParam.dsaInfo.forward.srcHwDev;
            __Port = dsaParam.dsaInfo.forward.source.portNum;
            CONVERT_DEV_PORT_DATA_MAC(__HwDev,__Port);
            dsaParam.dsaInfo.forward.srcHwDev = __HwDev;
            dsaParam.dsaInfo.forward.source.portNum = __Port;
        }
        
        dsaParam.dsaInfo.forward.srcId = (GT_U32)inArgs[16];
        dsaParam.dsaInfo.forward.egrFilterRegistered = (GT_BOOL)inArgs[17];
        dsaParam.dsaInfo.forward.wasRouted = (GT_BOOL)inArgs[18];
        dsaParam.dsaInfo.forward.qosProfileIndex = (GT_U32)inArgs[19];
        
        dsaParam.dsaInfo.forward.dstInterface.type = CPSS_INTERFACE_PORT_E;
        dsaParam.dsaInfo.forward.dstInterface.devPort.hwDevNum = (GT_HW_DEV_NUM)inArgs[20];
        dsaParam.dsaInfo.forward.dstInterface.devPort.portNum = (GT_PORT_NUM)inArgs[21];

        __HwDev = dsaParam.dsaInfo.forward.dstInterface.devPort.hwDevNum;
        __Port = dsaParam.dsaInfo.forward.dstInterface.devPort.portNum;
        CONVERT_DEV_PORT_DATA_MAC(__HwDev,__Port);
        dsaParam.dsaInfo.forward.dstInterface.devPort.hwDevNum = __HwDev;
        dsaParam.dsaInfo.forward.dstInterface.devPort.portNum = __Port;

        dsaParam.dsaInfo.forward.isTrgPhyPortValid = (GT_BOOL)inArgs[22];
        dsaParam.dsaInfo.forward.dstEport = (GT_PORT_NUM)inArgs[23];
        dsaParam.dsaInfo.forward.tag0TpidIndex = (GT_U32)inArgs[24];
    
        cpssOsMemCpy(&frameParams.frame.frame1731.transmitParams.dsaParam, &dsaParam, sizeof(CPSS_DXCH_NET_DSA_PARAMS_STC));        
    }
    
    /* call cpss api function */
    result = cpssDxChOamSrvRxFlowEntrySet(devNum, flowId, NULL, &frameParams);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxChOamSrvRxFlowEntry1731FromCpuSet function
* @endinternal
*
* @brief   Globally configure the MAC address table in the service CPU. for Y.1731 protocol.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - if parameters are bad
*
* @note wrapper parameters:
*       input args:
*       devNum        = (GT_U8)inArgs[0];
*       flowId        = (GT_U32)inArgs[1];
*       mpType        = (CPSS_DXCH_OAM_SRV_MP_TYPE_ENT)inArgs[2];
*       -CPSS_DXCH_OAM_SRV_MP_TYPE_UP_MEP_E
*       -CPSS_DXCH_OAM_SRV_MP_TYPE_DOWN_MEP_E
*       -CPSS_DXCH_OAM_SRV_MP_TYPE_MIP_E
*       -CPSS_DXCH_OAM_SRV_MP_TYPE_OTHER_E
*       cfmOffset       = (GT_U32)inArgs[3];
*       dsaParamValid     = (GT_BOOL)inArgs[4];
*       dsaTagType      = (CPSS_DXCH_NET_DSA_TYPE_ENT)inArgs[5];
*       vpt          = (GT_U8)inArgs[6];
*       cfiBit        = (GT_8)inArgs[7];
*       vid          = (GT_U16)inArgs[8];
*       dropOnSource     = (GT_BOOL)inArgs[9];
*       packetIsLooped    = (GT_BOOL)inArgs[10];
*       tc          = (GT_U8)inArgs[11];
*       dp          = (CPSS_DP_LEVEL_ENT)inArgs[12];
*       egrFilterEn      = (GT_BOOL)inArgs[13];
*       cascadeControl    = (GT_BOOL)inArgs[14];
*       egrFilterRegistered  = (GT_BOOL)inArgs[15];
*       srcId         = (GT_U32)inArgs[16];
*       srcHwDev       = (GT_U32)inArgs[17];
*       dstHwDevNum      = (GT_U32)inArgs[18];
*       dstPortNum      = (GT_U32)inArgs[19];
*       dstIsTagged      = (GT_BOOL)inArgs[20];
*       mailBoxToNeighborCPU = (GT_BOOL)inArgs[21];
*       isTrgPhyPortValid   = (GT_BOOL)inArgs[22];
*       dstEport       = (GT_PORT_NUM)inArgs[23];
*       tag0TpidIndex     = (GT_U32)inArgs[24];
*
*/
static CMD_STATUS wrCpssDxChOamSrvRxFlowEntry1731FromCpuSet

(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                                   result;
    GT_U8                                       devNum;
    GT_U32                                      flowId;
    CPSS_DXCH_OAM_SRV_RX_FLOW_FRAME_PARAMS_STC  frameParams;
    CPSS_DXCH_NET_DSA_PARAMS_STC                dsaParam;
    GT_PORT_NUM                                 __Port; /* Dummy for converting. */
    GT_HW_DEV_NUM                               __HwDev; /* Dummy for converting. */
    
    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    cpssOsMemSet(&frameParams, '\0', sizeof(CPSS_DXCH_OAM_SRV_RX_FLOW_FRAME_PARAMS_STC));
    cpssOsMemSet(&dsaParam, '\0', sizeof(CPSS_DXCH_NET_DSA_PARAMS_STC));

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    flowId = (GT_U32)inArgs[1];
    
    frameParams.frameType = CPSS_DXCH_OAM_SRV_FRAME_TYPE_1731_E;

    frameParams.frame.frame1731.mpType = (CPSS_DXCH_OAM_SRV_MP_TYPE_ENT)inArgs[2];
    frameParams.frame.frame1731.cfmOffset = (GT_U32)inArgs[3];
    frameParams.frame.frame1731.transmitParams.dsaParamValid = (GT_U32)inArgs[4];
    
    if(frameParams.frame.frame1731.transmitParams.dsaParamValid)
    {
        dsaParam.dsaType = CPSS_DXCH_NET_DSA_CMD_FROM_CPU_E;

        dsaParam.commonParams.dsaTagType = (CPSS_DXCH_NET_DSA_TYPE_ENT)inArgs[5];
        dsaParam.commonParams.vpt = (GT_U8)inArgs[6];
        dsaParam.commonParams.cfiBit = (GT_8)inArgs[7];
        dsaParam.commonParams.vid = (GT_U16)inArgs[8];
        dsaParam.commonParams.dropOnSource = (GT_BOOL)inArgs[9];
        dsaParam.commonParams.packetIsLooped = (GT_BOOL)inArgs[10];

        dsaParam.dsaInfo.fromCpu.tc = (GT_U8)inArgs[11];
        dsaParam.dsaInfo.fromCpu.dp = (CPSS_DP_LEVEL_ENT)inArgs[12];
        dsaParam.dsaInfo.fromCpu.egrFilterEn = (GT_BOOL)inArgs[13];
        dsaParam.dsaInfo.fromCpu.cascadeControl = (GT_BOOL)inArgs[14];
        dsaParam.dsaInfo.fromCpu.egrFilterRegistered = (GT_BOOL)inArgs[15];
        dsaParam.dsaInfo.fromCpu.srcId = (GT_U32)inArgs[16];
        dsaParam.dsaInfo.fromCpu.srcHwDev = (GT_HW_DEV_NUM)inArgs[17];
        
        __HwDev = dsaParam.dsaInfo.fromCpu.srcHwDev;
        CONVERT_DEV_PORT_DATA_MAC(__HwDev,__Port);
        dsaParam.dsaInfo.fromCpu.srcHwDev = __HwDev;
        
        dsaParam.dsaInfo.fromCpu.dstInterface.type = CPSS_INTERFACE_PORT_E;
        dsaParam.dsaInfo.fromCpu.dstInterface.devPort.hwDevNum = (GT_HW_DEV_NUM)inArgs[18];
        dsaParam.dsaInfo.fromCpu.dstInterface.devPort.portNum = (GT_PORT_NUM)inArgs[19];
        
        __HwDev = dsaParam.dsaInfo.fromCpu.dstInterface.devPort.hwDevNum;
        __Port = dsaParam.dsaInfo.fromCpu.dstInterface.devPort.portNum;
        CONVERT_DEV_PORT_DATA_MAC(__HwDev,__Port);
        dsaParam.dsaInfo.fromCpu.dstInterface.devPort.hwDevNum = __HwDev;
        dsaParam.dsaInfo.fromCpu.dstInterface.devPort.portNum = __Port;
        
        dsaParam.dsaInfo.fromCpu.extDestInfo.devPort.dstIsTagged = (GT_BOOL)inArgs[20];
        dsaParam.dsaInfo.fromCpu.extDestInfo.devPort.mailBoxToNeighborCPU = (GT_BOOL)inArgs[21];
        
        dsaParam.dsaInfo.fromCpu.isTrgPhyPortValid = (GT_BOOL)inArgs[22];
        dsaParam.dsaInfo.fromCpu.dstEport = (GT_PORT_NUM)inArgs[23];
        dsaParam.dsaInfo.fromCpu.tag0TpidIndex = (GT_U32)inArgs[24];
        
        cpssOsMemCpy(&frameParams.frame.frame1731.transmitParams.dsaParam, &dsaParam, sizeof(CPSS_DXCH_NET_DSA_PARAMS_STC));        
    }
    
    /* call cpss api function */
    result = cpssDxChOamSrvRxFlowEntrySet(devNum, flowId, NULL, &frameParams);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxChOamSrvSystemInit function
* @endinternal
*
* @brief   Initialize OAM service CPU.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - if parameters are bad
*
* @note wrapper parameters:
*       devNum = (GT_U8)inArgs[0];
*
*/
static CMD_STATUS wrCpssDxChOamSrvSystemInit

(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                           result;
    GT_U8                               devNum;
    CPSS_DXCH_OAM_SRV_SYS_PARAMS_STC    sysParams;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    
    /* call cpss api function */
    cpssOsMemSet(&sysParams, '\0', sizeof(CPSS_DXCH_OAM_SRV_SYS_PARAMS_STC));
    result = cpssDxChOamSrvSystemInit(devNum, &sysParams);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}

/**** database initialization **************************************/

static CMD_COMMAND dbCommands[] =
{
    {"cpssDxChOamSrvLoopbackConfigSet",
        &wrCpssDxChOamSrvLoopbackConfigSet,
        5, 0},
    {"cpssDxChOamSrvLoopbackConfigGet",
        &wrCpssDxChOamSrvLoopbackConfigGet,
        2, 0},
    {"cpssDxChOamSrvLoopbackStatisticalCountersGet",
        &wrCpssDxChOamSrvLoopbackStatisticalCountersGet,
        3, 0},
    {"cpssDxChOamSrvLoopbackStatisticalCountersClear",
        &wrCpssDxChOamSrvLoopbackStatisticalCountersClear,
        2, 0},

    {"cpssDxChOamSrvDmConfigSet",
        &wrCpssDxChOamSrvDmConfigSet,
        3, 0},
    {"cpssDxChOamSrvDmConfigGet",
        &wrCpssDxChOamSrvDmConfigGet,
        2, 0},
    {"cpssDxChOamSrvDmStatisticalCountersGet",
        &wrCpssDxChOamSrvDmStatisticalCountersGet,
        3, 0},
    {"cpssDxChOamSrvDmStatisticalCountersClear",
        &wrCpssDxChOamSrvDmStatisticalCountersClear,
        2, 0},

    {"cpssDxChOamSrvLmConfigSet",
        &wrCpssDxChOamSrvLmConfigSet,
        3, 0},
    {"cpssDxChOamSrvLmConfigGet",
        &wrCpssDxChOamSrvLmConfigGet,
        2, 0},
    {"cpssDxChOamSrvLmStatisticalCountersGet",
        &wrCpssDxChOamSrvLmStatisticalCountersGet,
        3, 0},
    {"cpssDxChOamSrvLmStatisticalCountersClear",
        &wrCpssDxChOamSrvLmStatisticalCountersClear,
        2, 0},

    {"cpssDxChOamSrvLocalMacAddressSet",
        &wrCpssDxChOamSrvLocalMacAddressSet,
        3, 0},
    {"cpssDxChOamSrvLocalMacAddressGet",
        &wrCpssDxChOamSrvLocalMacAddressGet,
        2, 0},
    {"cpssDxChOamSrvLocalInterfaceMacIndexSet",
        &wrCpssDxChOamSrvLocalInterfaceMacIndexSet,
        4, 0},
    {"cpssDxChOamSrvLocalInterfaceMacIndexGet",
        &wrCpssDxChOamSrvLocalInterfaceMacIndexGet,
        3, 0},

    {"cpssDxChOamSrvRxFlowEntry1731FwdSet",
        &wrCpssDxChOamSrvRxFlowEntry1731FwdSet,
        25, 0},
    {"cpssDxChOamSrvRxFlowEntry1731FromCpuSet",
        &wrCpssDxChOamSrvRxFlowEntry1731FromCpuSet,
        25, 0},
		
    {"cpssDxChOamSrvSystemInit",
        &wrCpssDxChOamSrvSystemInit,
        1, 0}
    
};

#define numCommands (sizeof(dbCommands) / sizeof(CMD_COMMAND))

/**
* @internal cmdLibInitCpssDxChOamSrv function
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
GT_STATUS cmdLibInitCpssDxChOamSrv
(
    GT_VOID
)
{
    return cmdInitLibrary(dbCommands, numCommands);
}



