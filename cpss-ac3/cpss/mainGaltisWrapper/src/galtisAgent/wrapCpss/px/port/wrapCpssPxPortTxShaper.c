/*******************************************************************************
*              (c), Copyright 2017, Marvell International Ltd.                 *
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
* @file wrapCpssPxPortTxShaper.c
*
* @brief Wrapper functions for
* cpss/px/port/cpssPxPortTxShaper.h API's
*
* @version   1
********************************************************************************
*/

/* Common galtis includes */
#include <galtisAgent/wrapUtil/cmdCpssPresteraUtils.h>
#include <cmdShell/cmdDb/cmdBase.h>
#include <cmdShell/common/cmdWrapUtils.h>
#include <cpss/px/port/cpssPxPortTxShaper.h>

/* Dump for Shaper */

/*
 * typedef: struct PORT_TX_SHAPER_TOKEN_BUCKET_STC
 *
 * Description:  structure to hold port's shaper info (per port/tc)
 *
 * Enumerations:
 *         tokenBucketEn - token Bucket Enable
 *         slowRateEn    - slow Rate Enable
 *         tbUpdateRatio - token bucket update ratio
 *         tokens - number of tokens
 *         maxBucketSize - max Bucket Size
 *         currentBucketSize - current Bucket Size
 *
 * Comments:
 *          for TXQ version 1 and above
 */
typedef struct{
    GT_BOOL tokenBucketEn;
    GT_BOOL slowRateEn;
    GT_U32  tbUpdateRatio;
    GT_U32  tokens;
    GT_U32  maxBucketSize;
    GT_U32  currentBucketSize;
    GT_BOOL tokenBucketAvbEnabled;
}PORT_TX_SHAPER_TOKEN_BUCKET_STC;

GT_STATUS prvPxPortTxShaperTokenBucketEntryRead
(
    IN  GT_U8   devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    IN  GT_BOOL usePerTc,
    IN  GT_U32  tc,
    OUT PORT_TX_SHAPER_TOKEN_BUCKET_STC *subEntryPtr
);


/**
* @internal prvCpssPxPortShaperInfoGet function
* @endinternal
*
* @brief   Gets shaper's configuration and state
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - port number
* @param[in] usePerTc                 - are we use 'per port' or 'per port per TC'
* @param[in] tc                       - traffic class , relevant when usePerTc = GT_TRUE
*
* @param[out] subEntryPtr              - (pointer to) dump information.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on failure.
* @retval GT_OUT_OF_RANGE          - parameter not in valid range
* @retval GT_TIMEOUT               - after max number of retries checking if PP ready
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Only Txq revision 1 is supported by this function.
*
*/
static GT_STATUS prvCpssPxPortShaperInfoGet
(
    IN  GT_SW_DEV_NUM   devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    IN  GT_BOOL usePerTc,
    IN  GT_U32  tc,
    OUT PORT_TX_SHAPER_TOKEN_BUCKET_STC *subEntryPtr
)
{
    GT_STATUS                       rc;

    rc = prvPxPortTxShaperTokenBucketEntryRead(devNum, portNum, usePerTc, tc, subEntryPtr);
    if (rc != GT_OK)
    {
        cmdOsPrintf("prvPxPortTxShaperTokenBucketEntryRead failed %d\n", rc);
        return rc;
    }

    if (usePerTc)
    {
        cmdOsPrintf("Shaper Dump for Port %d TC %d\n", portNum, tc);
    }
    else
    {
        cmdOsPrintf("Shaper Dump for Port %d\n", portNum);
    }

    return GT_OK;
}

/**
* @internal wrCpssPxPortShaperInfoGet function
* @endinternal
*
* @brief   Gets port shaper configuration and state.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on failure.
* @retval GT_OUT_OF_RANGE          - parameter not in valid range
* @retval GT_TIMEOUT               - after max number of retries checking if PP ready
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssPxPortShaperInfoGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS  result;

    GT_SW_DEV_NUM         devNum;
    GT_PHYSICAL_PORT_NUM  portNum;
    GT_BOOL               usePerTc;
    GT_U32                tc;
    PORT_TX_SHAPER_TOKEN_BUCKET_STC subEntry;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum      = (GT_SW_DEV_NUM)inArgs[0];
    portNum     = (GT_PHYSICAL_PORT_NUM)inArgs[1];
    usePerTc    = (GT_BOOL)inArgs[2];
    tc          = (GT_U32)inArgs[3];

    /* Override Device and Port */
    CONVERT_SW_DEV_PHYSICAL_PORT_MAC(devNum, portNum);

    result = prvCpssPxPortShaperInfoGet(devNum, portNum, usePerTc, tc, &subEntry);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d %d %d %d %d %d %d",
                 subEntry.tokenBucketEn, 
                 subEntry.slowRateEn, 
                 subEntry.tbUpdateRatio,
                 subEntry.tokens,
                 subEntry.maxBucketSize,
                 subEntry.currentBucketSize,
                 subEntry.tokenBucketAvbEnabled);

    return CMD_OK;

}

/**
* @internal wrCpssPxPortShaperDump function
* @endinternal
*
* @brief   Dump shaper's configuration and state
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - port number
* @param[in] usePerTc                 - are we use 'per port' or 'per port per TC'
* @param[in] tc                       - traffic class , relevant when usePerTc = GT_TRUE
*                                       GT_OK
*
* @note Only Txq revision 1 is supported by this function.
*
*/
GT_STATUS wrCpssPxPortShaperDump
(
    IN  GT_U8   devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    IN  GT_BOOL usePerTc,
    IN  GT_U32  tc
)
{
    PORT_TX_SHAPER_TOKEN_BUCKET_STC subEntry;
    GT_STATUS                       rc;

    rc = prvPxPortTxShaperTokenBucketEntryRead(devNum, portNum, usePerTc, tc, &subEntry);
    if (rc != GT_OK)
    {
        cmdOsPrintf("wrCpssPxPortShaperDump failed %d\n", rc);
        return rc;
    }

    if (usePerTc)
    {
        cmdOsPrintf("Shaper Dump for Port %d TC %d\n", portNum, tc);
    }
    else
    {
        cmdOsPrintf("Shaper Dump for Port %d\n", portNum);
    }

    cmdOsPrintf("tokenBucketEn        ; %d\n", subEntry.tokenBucketEn);
    cmdOsPrintf("slowRateEn           ; %d\n", subEntry.slowRateEn);
    cmdOsPrintf("tbUpdateRatio        ; %d\n", subEntry.tbUpdateRatio);
    cmdOsPrintf("tokens               ; %d\n", subEntry.tokens);
    cmdOsPrintf("maxBucketSize        ; %d*4K=%d=0x%06x\n",
                                                subEntry.maxBucketSize,
                                                _4K*subEntry.maxBucketSize,
                                                _4K*subEntry.maxBucketSize);
    cmdOsPrintf("currentBucketSize    ; %d=0x%06x\n",
                                                subEntry.currentBucketSize,
                                                subEntry.currentBucketSize);
    cmdOsPrintf("tokenBucketAvbEnabled; %d\n", subEntry.tokenBucketAvbEnabled);
    cmdOsPrintf("\n", portNum);
    return GT_OK;
}

/**
* @internal wrCpssPxPortTxShaperEnableSet function
* @endinternal
*
* @brief   Enable/Disable Token Bucket rate shaping on specified port of
*         specified device.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssPxPortTxShaperEnableSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS  result;

    GT_SW_DEV_NUM         devNum;
    GT_PHYSICAL_PORT_NUM  portNum;
    GT_BOOL               enable;
    GT_BOOL               avbEnable;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum    = (GT_SW_DEV_NUM)inArgs[0];
    portNum   = (GT_PHYSICAL_PORT_NUM)inArgs[1];
    enable    = (GT_BOOL)inArgs[2];
    avbEnable = (GT_BOOL)inArgs[3];

    /* Override Device and Port */
    CONVERT_SW_DEV_PHYSICAL_PORT_MAC(devNum, portNum);

    /* call cpss api function */
    result = cpssPxPortTxShaperEnableSet(devNum, portNum, enable, avbEnable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}

/**
* @internal wrCpssPxPortTxShaperEnableGet function
* @endinternal
*
* @brief   Get Enable/Disable Token Bucket rate shaping status on specified port of
*         specified device.
*
* @note   APPLICABLE DEVICES:      Pipe,
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters in NULL pointer.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssPxPortTxShaperEnableGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS               result;
    GT_SW_DEV_NUM           devNum;
    GT_PHYSICAL_PORT_NUM    portNum;
    GT_BOOL                 enable;
    GT_BOOL                 avbEnable;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_SW_DEV_NUM)inArgs[0];
    portNum = (GT_PHYSICAL_PORT_NUM)inArgs[1];

    /* Override Device and Port */
    CONVERT_SW_DEV_PHYSICAL_PORT_MAC(devNum, portNum);

    /* call cpss api function */
    result = cpssPxPortTxShaperEnableGet(devNum, portNum, &enable, &avbEnable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d %d", enable, avbEnable);
    return CMD_OK;
}

/**
* @internal wrCpssPxPortTxShaperProfileSet function
* @endinternal
*
* @brief   Set Token Bucket Shaper Profile on specified port of specified device.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssPxPortTxShaperProfileSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS               result;
    GT_SW_DEV_NUM           devNum;
    GT_PHYSICAL_PORT_NUM    portNum;
    GT_U32                  burstSize;
    GT_U32                  maxRate;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_SW_DEV_NUM)inArgs[0];
    portNum = (GT_PHYSICAL_PORT_NUM)inArgs[1];
    burstSize = (GT_U32)inArgs[2];
    maxRate = (GT_U32)inArgs[3];

    /* Override Device and Port */
    CONVERT_SW_DEV_PHYSICAL_PORT_MAC(devNum, portNum);

    /* call cpss api function */
    result = cpssPxPortTxShaperProfileSet(devNum, portNum, burstSize, &maxRate);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", maxRate);
    return CMD_OK;
}

/**
* @internal wrCpssPxPortTxShaperProfileGet function
* @endinternal
*
* @brief   Get Token Bucket Shaper Profile on specified port of specified device.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssPxPortTxShaperProfileGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS               result;
    GT_SW_DEV_NUM           devNum;
    GT_PHYSICAL_PORT_NUM    portNum;
    GT_U32                  burstSize;
    GT_U32                  maxRate = 0xFFFFFFFF;
    GT_BOOL                 enable = 0xFFFFFFFF;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_SW_DEV_NUM)inArgs[0];
    portNum = (GT_PHYSICAL_PORT_NUM)inArgs[1];

    /* Override Device and Port */
    CONVERT_SW_DEV_PHYSICAL_PORT_MAC(devNum, portNum);

    /* call cpss api function */
    result = cpssPxPortTxShaperProfileGet(devNum, portNum, &burstSize, &maxRate, &enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d %d %d", burstSize, maxRate, enable);

    return CMD_OK;
}

/**
* @internal wrCpssPxPortTxShaperModeSet function
* @endinternal
*
* @brief   Set Shaper mode, packet or byte based for given port.
*         Shapers are configured by cpssPxPortTxShaperProfileSet or
*         cpssPxPortTxShaperQueueProfileSet
*
* @note   APPLICABLE DEVICES:      Pipe.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssPxPortTxShaperModeSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS  result;

    GT_SW_DEV_NUM                   devNum;
    GT_PHYSICAL_PORT_NUM            portNum;
    CPSS_PX_PORT_TX_SHAPER_MODE_ENT mode;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum    = (GT_SW_DEV_NUM)inArgs[0];
    portNum   = (GT_PHYSICAL_PORT_NUM)inArgs[1];
    mode      = (CPSS_PX_PORT_TX_SHAPER_MODE_ENT)inArgs[2];

    /* Override Device and Port */
    CONVERT_SW_DEV_PHYSICAL_PORT_MAC(devNum, portNum);

    /* call cpss api function */
    result = cpssPxPortTxShaperModeSet(devNum, portNum, mode);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}

/**
* @internal wrCpssPxPortTxShaperModeGet function
* @endinternal
*
* @brief   Get Shaper mode, packet or byte based for given port.
*
* @note   APPLICABLE DEVICES:      Pipe.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssPxPortTxShaperModeGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS  result;

    GT_SW_DEV_NUM                   devNum;
    GT_PHYSICAL_PORT_NUM            portNum;
    CPSS_PX_PORT_TX_SHAPER_MODE_ENT mode;


    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum    = (GT_SW_DEV_NUM)inArgs[0];
    portNum   = (GT_PHYSICAL_PORT_NUM)inArgs[1];

    /* Override Device and Port */
    CONVERT_SW_DEV_PHYSICAL_PORT_MAC(devNum, portNum);

    /* call cpss api function */
    result = cpssPxPortTxShaperModeGet(devNum, portNum, &mode);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", mode);
    return CMD_OK;
}

/**
* @internal wrCpssPxPortTxShaperBaselineSet function
* @endinternal
*
* @brief   Set Token Bucket Baseline.
*         The Token Bucket Baseline is the "zero" level of the token bucket.
*         When the token bucket fill level < Baseline, the respective queue/port
*         is not served.
*
* @note   APPLICABLE DEVICES:      Pipe.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_OUT_OF_RANGE          - on out of range baseline
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Token Bucket Baseline must be configured as follows:
*       1. At least MTU.
*       2. If PFC response is enabled, the Baseline must be
*       at least 0x3FFFC0, see:
*       cpssPxPortPfcEnableSet.
*       cpssPxPortTxSchedulerDeficitModeEnableSet.
*       3. When packet based shaping is enabled, the following used as
*       shaper's MTU:
*       CPSS_PX_PORT_TX_SHAPER_CONFIG_STC. portsPacketLength
*       CPSS_PX_PORT_TX_SHAPER_CONFIG_STC. cpuPacketLength
*       see:
*       cpssPxPortTxShaperConfigurationSet.
*
*/
static CMD_STATUS wrCpssPxPortTxShaperBaselineSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS  result;

    GT_SW_DEV_NUM        devNum;
    GT_PHYSICAL_PORT_NUM baseline;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum    = (GT_SW_DEV_NUM)inArgs[0];
    baseline  = (GT_U32)inArgs[1];

    /* call cpss api function */
    result = cpssPxPortTxShaperBaselineSet(devNum, baseline);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}

/**
* @internal wrCpssPxPortTxShaperBaselineGet function
* @endinternal
*
* @brief   Get Token Bucket Baseline.
*         The Token Bucket Baseline is the "zero" level of the token bucket.
*         When the token bucket fill level < Baseline, the respective queue/port
*         is not served.
*
* @note   APPLICABLE DEVICES:      Pipe.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssPxPortTxShaperBaselineGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS  result;

    GT_SW_DEV_NUM        devNum;
    GT_PHYSICAL_PORT_NUM baseline;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum    = (GT_SW_DEV_NUM)inArgs[0];

    /* call cpss api function */
    result = cpssPxPortTxShaperBaselineGet(devNum, &baseline);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", baseline);
    return CMD_OK;
}

/**
* @internal wrCpssPxPortTxShaperConfigurationSet function
* @endinternal
*
* @brief   Set global configuration for shaper.
*
* @note   APPLICABLE DEVICES:      Pipe.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_OUT_OF_RANGE          - on out of range value
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note When packet based shaping is enabled, the following used as shaper's MTU:
*       configsPtr->portsPacketLength
*       configsPtr->cpuPacketLength
*       see:
*       cpssPxPortTxShaperBaselineSet.
*
*/
static CMD_STATUS wrCpssPxPortTxShaperConfigurationSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS  result;

    GT_SW_DEV_NUM        devNum;
    CPSS_PX_PORT_TX_SHAPER_CONFIG_STC configs;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum    = (GT_SW_DEV_NUM)inArgs[0];
    configs.tokensRate = (GT_U32)inArgs[1];
    configs.slowRateRatio = (GT_U32)inArgs[2];
    configs.tokensRateGran = (CPSS_PX_PORT_TX_SHAPER_GRANULARITY_ENT)inArgs[3];
    configs.portsPacketLength = (GT_U32)inArgs[4];
    configs.cpuPacketLength = (GT_U32)inArgs[5];

    /* call cpss api function */
    result = cpssPxPortTxShaperConfigurationSet(devNum, &configs);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}

/**
* @internal wrCpssPxPortTxShaperConfigurationGet function
* @endinternal
*
* @brief   Get global configuration for shaper.
*
* @note   APPLICABLE DEVICES:      Pipe.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note none.
*
*/
static CMD_STATUS wrCpssPxPortTxShaperConfigurationGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS  result;

    GT_SW_DEV_NUM        devNum;
    CPSS_PX_PORT_TX_SHAPER_CONFIG_STC configs;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum    = (GT_SW_DEV_NUM)inArgs[0];

    /* call cpss api function */
    result = cpssPxPortTxShaperConfigurationGet(devNum, &configs);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d %d %d %d %d",
                                        configs.tokensRate,
                                        configs.slowRateRatio,
                                        configs.tokensRateGran,
                                        configs.portsPacketLength,
                                        configs.cpuPacketLength);
    return CMD_OK;
}

/**
* @internal wrCpssPxPortTxShaperQueueEnableSet function
* @endinternal
*
* @brief   Enable/Disable shaping of transmitted traffic from a specified Traffic
*         Class Queue and specified port of specified device.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssPxPortTxShaperQueueEnableSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS  result;

    GT_SW_DEV_NUM         devNum;
    GT_PHYSICAL_PORT_NUM  portNum;
    GT_BOOL               enable;
    GT_BOOL               avbEnable;
    GT_U32                tcQueue;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum    = (GT_SW_DEV_NUM)inArgs[0];
    portNum   = (GT_PHYSICAL_PORT_NUM)inArgs[1];
    tcQueue   = (GT_U32)inArgs[2];
    enable    = (GT_BOOL)inArgs[3];
    avbEnable = (GT_BOOL)inArgs[4];

    /* Override Device and Port */
    CONVERT_SW_DEV_PHYSICAL_PORT_MAC(devNum, portNum);

    /* call cpss api function */
    result = cpssPxPortTxShaperQueueEnableSet(devNum, portNum, tcQueue, enable,
        avbEnable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}

/**
* @internal wrCpssPxPortTxShaperQueueEnableGet function
* @endinternal
*
* @brief   Get Enable/Disable shaping status
*         of transmitted traffic from a specified Traffic
*         Class Queue and specified port of specified device.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssPxPortTxShaperQueueEnableGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS  result;

    GT_SW_DEV_NUM         devNum;
    GT_PHYSICAL_PORT_NUM  portNum;
    GT_BOOL               enable;
    GT_BOOL               avbEnable;
    GT_U32                tcQueue;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum    = (GT_SW_DEV_NUM)inArgs[0];
    portNum   = (GT_PHYSICAL_PORT_NUM)inArgs[1];
    tcQueue   = (GT_U32)inArgs[2];

    /* Override Device and Port */
    CONVERT_SW_DEV_PHYSICAL_PORT_MAC(devNum, portNum);

    /* call cpss api function */
    result = cpssPxPortTxShaperQueueEnableGet(devNum, portNum, tcQueue, &enable,
        &avbEnable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d %d", enable, avbEnable);
    return CMD_OK;
}

/**
* @internal wrCpssPxPortTxShaperQueueProfileSet function
* @endinternal
*
* @brief   Set Shaper Profile for Traffic Class Queue of specified port on
*         specified device.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssPxPortTxShaperQueueProfileSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS               result;
    GT_SW_DEV_NUM           devNum;
    GT_PHYSICAL_PORT_NUM    portNum;
    GT_U32                  burstSize;
    GT_U32                  maxRate;
    GT_U32                  tcQueue;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_SW_DEV_NUM)inArgs[0];
    portNum = (GT_PHYSICAL_PORT_NUM)inArgs[1];
    tcQueue = (GT_U32)inArgs[2];
    burstSize = (GT_U32)inArgs[3];
    maxRate = (GT_U32)inArgs[4];

    /* Override Device and Port */
    CONVERT_SW_DEV_PHYSICAL_PORT_MAC(devNum, portNum);

    /* call cpss api function */
    result = cpssPxPortTxShaperQueueProfileSet(devNum, portNum, tcQueue,
        burstSize, &maxRate);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", maxRate);
    return CMD_OK;
}

/**
* @internal wrCpssPxPortTxShaperQueueProfileGet function
* @endinternal
*
* @brief   Get Shaper Profile for Traffic Class Queue of specified port on
*         specified device.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssPxPortTxShaperQueueProfileGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS               result;
    GT_SW_DEV_NUM           devNum;
    GT_PHYSICAL_PORT_NUM    portNum;
    GT_U32                  burstSize = 0xFFFFFFFF;
    GT_U32                  maxRate = 0xFFFFFFFF;
    GT_BOOL                 enable;
    GT_U32                  tcQueue;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum  = (GT_SW_DEV_NUM)inArgs[0];
    portNum = (GT_PHYSICAL_PORT_NUM)inArgs[1];
    tcQueue = (GT_U32)inArgs[2];

    /* Override Device and Port */
    CONVERT_SW_DEV_PHYSICAL_PORT_MAC(devNum, portNum);

    /* call cpss api function */
    result = cpssPxPortTxShaperQueueProfileGet(devNum, portNum, tcQueue,
        &burstSize, &maxRate, &enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d %d %d", burstSize, maxRate, enable);
    return CMD_OK;
}

/**** database initialization **************************************/
static CMD_COMMAND dbCommands[] =
{
    /* commandName  funcReference   funcArgs    funcFields */
    {"cpssPxPortTxShaperEnableSet",
     &wrCpssPxPortTxShaperEnableSet,
        4, 0},
    {"cpssPxPortTxShaperEnableGet",
     &wrCpssPxPortTxShaperEnableGet,
        2, 0},
    {"cpssPxPortTxShaperProfileSet",
     &wrCpssPxPortTxShaperProfileSet,
        4, 0},
    {"cpssPxPortTxShaperProfileGet",
     &wrCpssPxPortTxShaperProfileGet,
        2, 0},
    {"cpssPxPortTxShaperModeSet",
     &wrCpssPxPortTxShaperModeSet,
        3, 0},
    {"cpssPxPortTxShaperModeGet",
     &wrCpssPxPortTxShaperModeGet,
        2, 0},
    {"cpssPxPortTxShaperBaselineSet",
     &wrCpssPxPortTxShaperBaselineSet,
        2, 0},
    {"cpssPxPortTxShaperBaselineGet",
     &wrCpssPxPortTxShaperBaselineGet,
        1, 0},
    {"cpssPxPortTxShaperConfigurationSet",
     &wrCpssPxPortTxShaperConfigurationSet,
        6, 0},
    {"cpssPxPortTxShaperConfigurationGet",
     &wrCpssPxPortTxShaperConfigurationGet,
        1, 0},
    {"cpssPxPortTxShaperQueueEnableSet",
     &wrCpssPxPortTxShaperQueueEnableSet,
        5, 0},
    {"cpssPxPortTxShaperQueueEnableGet",
     &wrCpssPxPortTxShaperQueueEnableGet,
        3, 0},
    {"cpssPxPortTxShaperQueueProfileSet",
     &wrCpssPxPortTxShaperQueueProfileSet,
        5, 0},
    {"cpssPxPortTxShaperQueueProfileGet",
     &wrCpssPxPortTxShaperQueueProfileGet,
        3, 0},
    {"cpssPxPortShaperDump",
     &wrCpssPxPortShaperInfoGet,
        4, 0}
};


#define  numCommands     (sizeof(dbCommands) / sizeof(CMD_COMMAND))


/**
* @internal cmdLibInitCpssPxPortTxShaper function
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
GT_STATUS cmdLibInitCpssPxPortTxShaper
(
    GT_VOID
)
{
    return cmdInitLibrary(dbCommands, numCommands);
}

