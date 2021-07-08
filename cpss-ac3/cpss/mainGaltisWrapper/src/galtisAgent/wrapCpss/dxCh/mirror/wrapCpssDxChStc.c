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
* @file wrapCpssDxChStc.c
*
* @brief TODO: Add proper description of this file here
*
* @version   4
********************************************************************************
*/

/* Common galtis includes */
#include <galtisAgent/wrapUtil/cmdCpssPresteraUtils.h>
#include <cmdShell/cmdDb/cmdBase.h>
#include <cmdShell/common/cmdWrapUtils.h>

/* Feature specific includes. */
#include <cpss/common/cpssTypes.h>
#include <cpss/dxCh/dxChxGen/mirror/cpssDxChStc.h>

/**
* @internal wrCpssDxChStcIngressCountModeSet function
* @endinternal
*
* @brief   Set the type of packets subject to Ingress Sampling to CPU.
*
* @note   APPLICABLE DEVICES:      All DXCH devices
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device number.
* @retval GT_HW_ERROR              - on writing to HW error.
*/
static CMD_STATUS wrCpssDxChStcIngressCountModeSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8 devNum;
    CPSS_DXCH_STC_COUNT_MODE_ENT mode;


    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */

    devNum = (GT_U8)inArgs[0];
    mode = (CPSS_DXCH_STC_COUNT_MODE_ENT)inArgs[1];

    /* call cpss api function */
    result = cpssDxChStcIngressCountModeSet(devNum, mode);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxChStcIngressCountModeGet function
* @endinternal
*
* @brief   Get the type of packets that are subject to Ingress Sampling to CPU.
*
* @note   APPLICABLE DEVICES:      All DXCH devices
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
*/
static CMD_STATUS wrCpssDxChStcIngressCountModeGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8 devNum;
    CPSS_DXCH_STC_COUNT_MODE_ENT mode;


    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */

    devNum = (GT_U8)inArgs[0];

    /* call cpss api function */
    result = cpssDxChStcIngressCountModeGet(devNum, &mode);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", mode);

    return CMD_OK;
}

/**
* @internal wrCpssDxChStcReloadModeSet function
* @endinternal
*
* @brief   Set the type of Sampling To CPU (STC) count reload mode.
*
* @note   APPLICABLE DEVICES:      All DXCH devices
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device number,STC type or mode.
* @retval GT_HW_ERROR              - on writing to HW error.
*/
static CMD_STATUS wrCpssDxChStcReloadModeSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8 devNum;
    CPSS_DXCH_STC_TYPE_ENT stcType;
    CPSS_DXCH_STC_COUNT_RELOAD_MODE_ENT mode;


    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */

    devNum = (GT_U8)inArgs[0];
    stcType = (CPSS_DXCH_STC_TYPE_ENT)inArgs[1];
    mode = (CPSS_DXCH_STC_COUNT_RELOAD_MODE_ENT)inArgs[2];

    /* call cpss api function */
    result = cpssDxChStcReloadModeSet(devNum, stcType, mode);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxChStcReloadModeGet function
* @endinternal
*
* @brief   Get the type of Sampling To CPU (STC) count reload mode.
*
* @note   APPLICABLE DEVICES:      All DXCH devices
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device number or STC type.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
*/
static CMD_STATUS wrCpssDxChStcReloadModeGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8 devNum;
    CPSS_DXCH_STC_TYPE_ENT stcType;
    CPSS_DXCH_STC_COUNT_RELOAD_MODE_ENT mode;


    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */

    devNum = (GT_U8)inArgs[0];
    stcType = (CPSS_DXCH_STC_TYPE_ENT)inArgs[1];

    /* call cpss api function */
    result = cpssDxChStcReloadModeGet(devNum, stcType, &mode);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", mode);

    return CMD_OK;
}

/**
* @internal wrCpssDxChStcEnableSet function
* @endinternal
*
* @brief   Global Enable/Disable Sampling To CPU (STC).
*
* @note   APPLICABLE DEVICES:      All DXCH devices
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device number or STC type.
* @retval GT_HW_ERROR              - on writing to HW error.
*/
static CMD_STATUS wrCpssDxChStcEnableSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8 devNum;
    CPSS_DXCH_STC_TYPE_ENT stcType;
    GT_BOOL enable;


    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */

    devNum = (GT_U8)inArgs[0];
    stcType = (CPSS_DXCH_STC_TYPE_ENT)inArgs[1];
    enable = (GT_BOOL)inArgs[2];

    /* call cpss api function */
    result = cpssDxChStcEnableSet(devNum, stcType, enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxChStcEnableGet function
* @endinternal
*
* @brief   Get the global status of Sampling To CPU (STC) (Enabled/Disabled).
*
* @note   APPLICABLE DEVICES:      All DXCH devices
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device number or STC type.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
*/
static CMD_STATUS wrCpssDxChStcEnableGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8 devNum;
    CPSS_DXCH_STC_TYPE_ENT stcType;
    GT_BOOL enable;


    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */

    devNum = (GT_U8)inArgs[0];
    stcType = (CPSS_DXCH_STC_TYPE_ENT)inArgs[1];

    /* call cpss api function */
    result = cpssDxChStcEnableGet(devNum, stcType, &enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", enable);

    return CMD_OK;
}

/**
* @internal wrCpssDxChStcPortLimitSet function
* @endinternal
*
* @brief   Set Sampling to CPU (STC) limit per port.
*         The limit to be loaded into the Count Down Counter.
*         This counter is decremented for each packet received on this port and is
*         subject to sampling according to the setting of STC Count mode.
*         When this counter is decremented from 1 to 0, the packet causing this
*         decrement is sampled to the CPU.
*
* @note   APPLICABLE DEVICES:      All DXCH devices
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device number,port number or STC type.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_OUT_OF_RANGE          - limit is out of range
*/
static CMD_STATUS wrCpssDxChStcPortLimitSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8 devNum;
    GT_PHYSICAL_PORT_NUM port;
    CPSS_DXCH_STC_TYPE_ENT stcType;
    GT_U32 limit;


    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */

    devNum = (GT_U8)inArgs[0];
    port = (GT_PHYSICAL_PORT_NUM)inArgs[1];
    stcType = (CPSS_DXCH_STC_TYPE_ENT)inArgs[2];
    limit = (GT_U32)inArgs[3];

    /* Override Device and Port */
    CONVERT_DEV_PHYSICAL_PORT_MAC(devNum, port);

    /* call cpss api function */
    result = cpssDxChStcPortLimitSet(devNum, port, stcType, limit);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxChStcPortLimitGet function
* @endinternal
*
* @brief   Get Sampling to CPU (STC) limit per port.
*         The limit to be loaded into the Count Down Counter.
*         This counter is decremented for each packet received on this port and is
*         subject to sampling according to the setting of STC Count mode.
*         When this counter is decremented from 1 to 0, the packet causing this
*         decrement is sampled to the CPU.
*
* @note   APPLICABLE DEVICES:      All DXCH devices
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device number,port number or STC type.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
*/
static CMD_STATUS wrCpssDxChStcPortLimitGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8 devNum;
    GT_PHYSICAL_PORT_NUM port;
    CPSS_DXCH_STC_TYPE_ENT stcType;
    GT_U32 limit;


    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */

    devNum = (GT_U8)inArgs[0];
    port = (GT_PHYSICAL_PORT_NUM)inArgs[1];
    stcType = (CPSS_DXCH_STC_TYPE_ENT)inArgs[2];

    /* Override Device and Port */
    CONVERT_DEV_PHYSICAL_PORT_MAC(devNum, port);

    /* call cpss api function */
    result = cpssDxChStcPortLimitGet(devNum, port, stcType, &limit);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", limit);

    return CMD_OK;
}

/**
* @internal wrCpssDxChStcPortReadyForNewLimitGet function
* @endinternal
*
* @brief   Check whether Sampling to CPU is ready to get new STC Limit Value per
*         port.
*         The function cpssDxChStcPortLimitSet sets new limit value.
*         But only after device finishes handling of new limit value the
*         cpssDxChStcPortLimitSet may be called once more.
*
* @note   APPLICABLE DEVICES:      All DXCH devices
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device number,port number or STC type.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
*/
static CMD_STATUS wrCpssDxChStcPortReadyForNewLimitGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8 devNum;
    GT_PHYSICAL_PORT_NUM port;
    CPSS_DXCH_STC_TYPE_ENT stcType;
    GT_BOOL isReady;


    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */

    devNum = (GT_U8)inArgs[0];
    port = (GT_PHYSICAL_PORT_NUM)inArgs[1];
    stcType = (CPSS_DXCH_STC_TYPE_ENT)inArgs[2];

    /* Override Device and Port */
    CONVERT_DEV_PHYSICAL_PORT_MAC(devNum, port);

    /* call cpss api function */
    result = cpssDxChStcPortReadyForNewLimitGet(devNum, port, stcType, &isReady);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", isReady);

    return CMD_OK;
}

/**
* @internal wrCpssDxChStcPortCountdownCntrGet function
* @endinternal
*
* @brief   Get STC Countdown Counter per port.
*         This counter is decremented for each packet received on this port and is
*         subject to sampling according to the setting of STC Count mode.
*         When this counter is decremented from 1 to 0, the packet causing this
*         decrement is sampled to the CPU.
*
* @note   APPLICABLE DEVICES:      All DXCH devices
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device number,port number or STC type.
* @retval GT_HW_ERROR              - on writing to HW error.
*/
static CMD_STATUS wrCpssDxChStcPortCountdownCntrGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8 devNum;
    GT_PHYSICAL_PORT_NUM port;
    CPSS_DXCH_STC_TYPE_ENT stcType;
    GT_U32 cntr;


    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */

    devNum = (GT_U8)inArgs[0];
    port = (GT_PHYSICAL_PORT_NUM)inArgs[1];
    stcType = (CPSS_DXCH_STC_TYPE_ENT)inArgs[2];

    /* Override Device and Port */
    CONVERT_DEV_PHYSICAL_PORT_MAC(devNum, port);

    /* call cpss api function */
    result = cpssDxChStcPortCountdownCntrGet(devNum, port, stcType, &cntr);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", cntr);

    return CMD_OK;
}

/**
* @internal wrCpssDxChStcPortSampledPacketsCntrSet function
* @endinternal
*
* @brief   Set the number of packets Sampled to CPU per port.
*
* @note   APPLICABLE DEVICES:      All DXCH devices
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device number,port number or STC type.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_OUT_OF_RANGE          - on wrong cntr
*/
static CMD_STATUS wrCpssDxChStcPortSampledPacketsCntrSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8 devNum;
    GT_PHYSICAL_PORT_NUM port;
    CPSS_DXCH_STC_TYPE_ENT stcType;
    GT_U32 cntr;


    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */

    devNum = (GT_U8)inArgs[0];
    port = (GT_PHYSICAL_PORT_NUM)inArgs[1];
    stcType = (CPSS_DXCH_STC_TYPE_ENT)inArgs[2];
    cntr = (GT_U32)inArgs[3];

    /* Override Device and Port */
    CONVERT_DEV_PHYSICAL_PORT_MAC(devNum, port);

    /* call cpss api function */
    result = cpssDxChStcPortSampledPacketsCntrSet(devNum, port, stcType, cntr);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxChStcPortSampledPacketsCntrGet function
* @endinternal
*
* @brief   Get the number of packets Sampled to CPU per port.
*
* @note   APPLICABLE DEVICES:      All DXCH devices
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device number,port number or STC type.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
*/
static CMD_STATUS wrCpssDxChStcPortSampledPacketsCntrGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8 devNum;
    GT_PHYSICAL_PORT_NUM port;
    CPSS_DXCH_STC_TYPE_ENT stcType;
    GT_U32 cntr;


    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */

    devNum = (GT_U8)inArgs[0];
    port = (GT_PHYSICAL_PORT_NUM)inArgs[1];
    stcType = (CPSS_DXCH_STC_TYPE_ENT)inArgs[2];

    /* Override Device and Port */
    CONVERT_DEV_PHYSICAL_PORT_MAC(devNum, port);

    /* call cpss api function */
    result = cpssDxChStcPortSampledPacketsCntrGet(devNum, port, stcType, &cntr);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", cntr);

    return CMD_OK;
}


/**
* @internal wrCpssDxChStcEgressAnalyzerIndexSet function
* @endinternal
*
* @brief   Set the egress STC analyzer index.
*         feature behavior:
*         If a packet is marked for both egress STC and egress mirroring , the
*         packet is only replicated once, depending on a configurable priority
*         between the mechanisms. The configurable <STC analyzer index> determines
*         the priority of STC compared to mirroring to the analyzer.
*         Note:
*         1. that STC does not actually use this index to determine the target
*         of the mirrored packet.
*         2. the indexes used by egress mirroring engines are in range 0..6.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device.
* @retval GT_OUT_OF_RANGE          - index is out of range.
* @retval GT_HW_ERROR              - on HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note function uses same parameters as mirroring functions like
*       cpssDxChMirrorTxGlobalAnalyzerInterfaceIndexSet(devNum,enable,index).
*
*/
static CMD_STATUS wrCpssDxChStcEgressAnalyzerIndexSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;

    GT_U8       devNum;
    GT_BOOL   enable;
    GT_U32    index;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum =  (GT_U8)inArgs[0];
    enable =  (GT_BOOL)inArgs[1];
    index =  (GT_U32)inArgs[2];

    /* call cpss api function */
    result = cpssDxChStcEgressAnalyzerIndexSet(devNum, enable , index);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}

/**
* @internal wrCpssDxChStcEgressAnalyzerIndexGet function
* @endinternal
*
* @brief   Get the egress STC analyzer index.
*         feature behavior:
*         If a packet is marked for both egress STC and egress mirroring , the
*         packet is only replicated once, depending on a configurable priority
*         between the mechanisms. The configurable <STC analyzer index> determines
*         the priority of STC compared to mirroring to the analyzer.
*         Note:
*         1. that STC does not actually use this index to determine the target
*         of the mirrored packet.
*         2. the indexes used by egress mirroring engines are in range 0..6.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device.
* @retval GT_OUT_OF_RANGE          - index is out of range.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_HW_ERROR              - on HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note function uses same parameters as mirroring functions like
*       cpssDxChMirrorTxGlobalAnalyzerInterfaceIndexGet(devNum,enablePtr,indexPtr).
*
*/
static CMD_STATUS wrCpssDxChStcEgressAnalyzerIndexGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;

    GT_U8       devNum;
    GT_BOOL   enable;
    GT_U32    index;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum =  (GT_U8)inArgs[0];

    /* call cpss api function */
    result = cpssDxChStcEgressAnalyzerIndexGet(devNum, &enable ,&index );

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d%d", enable , index);
    return CMD_OK;
}


/**** database initialization **************************************/



static CMD_COMMAND dbCommands[] =
{
        {"cpssDxChStcIngressCountModeSet",
         &wrCpssDxChStcIngressCountModeSet,
         2, 0},
        {"cpssDxChStcIngressCountModeGet",
         &wrCpssDxChStcIngressCountModeGet,
         1, 0},
        {"cpssDxChStcReloadModeSet",
         &wrCpssDxChStcReloadModeSet,
         3, 0},
        {"cpssDxChStcReloadModeGet",
         &wrCpssDxChStcReloadModeGet,
         2, 0},
        {"cpssDxChStcEnableSet",
         &wrCpssDxChStcEnableSet,
         3, 0},
        {"cpssDxChStcEnableGet",
         &wrCpssDxChStcEnableGet,
         2, 0},
        {"cpssDxChStcPortLimitSet",
         &wrCpssDxChStcPortLimitSet,
         4, 0},
        {"cpssDxChStcPortLimitGet",
         &wrCpssDxChStcPortLimitGet,
         3, 0},
        {"cpssDxChStcPortReadyForNewLimitGet",
         &wrCpssDxChStcPortReadyForNewLimitGet,
         3, 0},
        {"cpssDxChStcPortCountdownCntrGet",
         &wrCpssDxChStcPortCountdownCntrGet,
         3, 0},
        {"cpssDxChStcPortSampledPacketsCntrSet",
         &wrCpssDxChStcPortSampledPacketsCntrSet,
         4, 0},
        {"cpssDxChStcPortSampledPacketsCntrGet",
         &wrCpssDxChStcPortSampledPacketsCntrGet,
         3, 0},
        {"cpssDxChStcEgressAnalyzerIndexSet",
         &wrCpssDxChStcEgressAnalyzerIndexSet,
         3, 0},
        {"cpssDxChStcEgressAnalyzerIndexGet",
         &wrCpssDxChStcEgressAnalyzerIndexGet,
         1, 0}
};

#define numCommands (sizeof(dbCommands) / sizeof(CMD_COMMAND))

/**
* @internal cmdLibInitCpssDxChStc function
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
GT_STATUS cmdLibInitCpssDxChStc
(
    GT_VOID
)
{
    return cmdInitLibrary(dbCommands, numCommands);
}


