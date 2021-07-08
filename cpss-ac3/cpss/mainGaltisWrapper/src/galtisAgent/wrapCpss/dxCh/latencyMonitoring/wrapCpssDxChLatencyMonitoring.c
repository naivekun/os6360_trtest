/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
* wrapCpssDxChLatencyMonitoring.c
*
* DESCRIPTION:
*       Wrapper functions for Cpss DxCh Latency monitoring
*
* DEPENDENCIES:
*       None.
*
* FILE REVISION NUMBER:
*       $Revision: 1 $
*
*******************************************************************************/

/* Common galtis includes */
#include <galtisAgent/wrapUtil/cmdCpssPresteraUtils.h>
#include <cmdShell/cmdDb/cmdBase.h>
#include <cmdShell/common/cmdWrapUtils.h>


/* Feature specific includes. */
#include <cpss/common/cpssTypes.h>
#include <cpss/dxCh/dxChxGen/latencyMonitoring/cpssDxChLatencyMonitoring.h>

/*****************************Commands*******************************/

/**
* @internal wrCpssDxChLatencyMonitoringPortCfgSet function
* @endinternal
*
* @brief   Set index and profile for physical port for port latency monitoring.
*          Combination {source index, target index} used for indexing.
*
* @note   APPLICABLE DEVICES:      Falcon
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChLatencyMonitoringPortCfgSet

(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS               result;
    GT_U8                   devNum;
    GT_PHYSICAL_PORT_NUM    port;
    CPSS_DIRECTION_ENT      portType;
    GT_U32                  index;
    GT_U32                  profile;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    port = (GT_PHYSICAL_PORT_NUM)inArgs[1];
    portType = (CPSS_DIRECTION_ENT)inArgs[2];
    index = (GT_U32)inArgs[3];
    profile = (GT_U32)inArgs[4];

    /* call cpss api function */
    result = cpssDxChLatencyMonitoringPortCfgSet(devNum, port, portType, index,
        profile);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}

/**
* @internal wrCpssDxChLatencyMonitoringPortCfgGet function
* @endinternal
*
* @brief   Get index and profile for physical port for port latency monitoring.
*          Combination {source index, target index} used for indexing.
*
* @note   APPLICABLE DEVICES:      Falcon
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChLatencyMonitoringPortCfgGet

(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS               result;
    GT_U8                   devNum;
    GT_PHYSICAL_PORT_NUM    port;
    CPSS_DIRECTION_ENT      portType;
    GT_U32                  index;
    GT_U32                  profile;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    port = (GT_PHYSICAL_PORT_NUM)inArgs[1];
    portType = (CPSS_DIRECTION_ENT)inArgs[2];

    /* call cpss api function */
    result = cpssDxChLatencyMonitoringPortCfgGet(devNum, port, portType, &index,
        &profile);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d%d", index, profile);
    return CMD_OK;
}

/**
* @internal wrCpssDxChLatencyMonitoringEnableSet function
* @endinternal
*
* @brief   Set latency monitoring enabled state.
*
* @note   APPLICABLE DEVICES:      Falcon
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChLatencyMonitoringEnableSet

(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS               result;
    GT_U8                   devNum;
    GT_PHYSICAL_PORT_NUM    srcPortNum;
    GT_PHYSICAL_PORT_NUM    trgPortNum;
    GT_BOOL                 enabled;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    srcPortNum = (GT_PHYSICAL_PORT_NUM)inArgs[1];
    trgPortNum = (GT_PHYSICAL_PORT_NUM)inArgs[2];
    enabled = (GT_BOOL)inArgs[3];

    /* call cpss api function */
    result = cpssDxChLatencyMonitoringEnableSet(devNum, srcPortNum, trgPortNum,
        enabled);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}

/**
* @internal wrCpssDxChLatencyMonitoringEnableGet function
* @endinternal
*
* @brief   Get latency monitoring enabled state.
*
* @note   APPLICABLE DEVICES:      Falcon
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChLatencyMonitoringEnableGet

(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS               result;
    GT_U8                   devNum;
    GT_PHYSICAL_PORT_NUM    srcPortNum;
    GT_PHYSICAL_PORT_NUM    trgPortNum;
    GT_BOOL                 enabled;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    srcPortNum = (GT_PHYSICAL_PORT_NUM)inArgs[1];
    trgPortNum = (GT_PHYSICAL_PORT_NUM)inArgs[2];

    /* call cpss api function */
    result = cpssDxChLatencyMonitoringEnableGet(devNum, srcPortNum, trgPortNum,
        &enabled);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", enabled);
    return CMD_OK;
}

/**
* @internal wrCpssDxChLatencyMonitoringSamplingProfileSet
* @endinternal
*
* @brief   Set latency monitoring sampling profile.
*
* @note   APPLICABLE DEVICES:      Falcon
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChLatencyMonitoringSamplingProfileSet

(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS               result;
    GT_U8                   devNum;
    GT_U32                  latencyProfile;
    GT_U32                  samplingProfile;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    latencyProfile = (GT_U32)inArgs[1];
    samplingProfile = (GT_U32)inArgs[2];

    /* call cpss api function */
    result = cpssDxChLatencyMonitoringSamplingProfileSet(devNum, latencyProfile,
        samplingProfile);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}

/**
* @internal wrCpssDxChLatencyMonitoringSamplingProfileGet
* @endinternal
*
* @brief   Set latency monitoring sampling profile.
*
* @note   APPLICABLE DEVICES:      Falcon
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChLatencyMonitoringSamplingProfileGet

(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS               result;
    GT_U8                   devNum;
    GT_U32                  latencyProfile;
    GT_U32                  samplingProfile;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    latencyProfile = (GT_U32)inArgs[1];

    /* call cpss api function */
    result = cpssDxChLatencyMonitoringSamplingProfileGet(devNum, latencyProfile,
        &samplingProfile);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", samplingProfile);
    return CMD_OK;
}

/**
* @internal wrCpssDxChLatencyMonitoringSamplingConfigurationSet
* @endinternal
*
* @brief   Set latency monitoring sampling configuration.
*
* @note   APPLICABLE DEVICES:      Falcon
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChLatencyMonitoringSamplingConfigurationSet

(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS               result;
    GT_U8                   devNum;
    GT_U32                  samplingProfile;
    GT_U32                  samplingThreshold;
    CPSS_DXCH_LATENCY_MONITORING_SAMPLING_MODE_ENT samplingMode;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    samplingProfile = (GT_U32)inArgs[1];
    samplingThreshold = (GT_U32)inArgs[2];
    samplingMode = (GT_U32)inArgs[3];

    /* call cpss api function */
    result = cpssDxChLatencyMonitoringSamplingConfigurationSet(devNum,
        samplingProfile, samplingMode, samplingThreshold);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}

/**
* @internal wrCpssDxChLatencyMonitoringSamplingConfigurationGet
* @endinternal
*
* @brief   Get latency monitoring sampling configuration.
*
* @note   APPLICABLE DEVICES:      Falcon
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChLatencyMonitoringSamplingConfigurationGet

(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS               result;
    GT_U8                   devNum;
    GT_U32                  samplingProfile;
    GT_U32                  samplingThreshold;
    CPSS_DXCH_LATENCY_MONITORING_SAMPLING_MODE_ENT samplingMode;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    samplingProfile = (GT_U32)inArgs[1];

    /* call cpss api function */
    result = cpssDxChLatencyMonitoringSamplingConfigurationGet(devNum,
        samplingProfile, &samplingMode, &samplingThreshold);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d%d", samplingMode, samplingThreshold);
    return CMD_OK;
}

/**
* @internal wrCpssDxChLatencyMonitoringCfgSet
* @endinternal
*
* @brief   Set latency monitoring configuration.
*
* @note   APPLICABLE DEVICES:      Falcon
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChLatencyMonitoringCfgSet

(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                               result;
    GT_U8                                   devNum;
    GT_LATENCY_MONITORING_UNIT_BMP          lmuBmp;
    GT_U32                                  latencyProfile;
    CPSS_DXCH_LATENCY_MONITORING_CFG_STC    lmuConfig;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    lmuBmp = (GT_LATENCY_MONITORING_UNIT_BMP)inArgs[1];
    latencyProfile = (GT_U32)inFields[0];
    lmuConfig.rangeMax = (GT_U32)inFields[1];
    lmuConfig.rangeMin = (GT_U32)inFields[2];
    lmuConfig.notificationThresh = (GT_U32)inFields[3];

    /* call cpss api function */
    result = cpssDxChLatencyMonitoringCfgSet(devNum, lmuBmp, latencyProfile,
        &lmuConfig);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}

/**
* @internal wrCpssDxChLatencyMonitoringPortEnableSet function
* @endinternal
*
* @brief   Enable/Disable egress port for latency monitoring.
*
* @note   APPLICABLE DEVICES:      Falcon
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChLatencyMonitoringPortEnableSet

(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS               result;
    GT_U8                   devNum;
    GT_PHYSICAL_PORT_NUM    portNum;
    GT_BOOL                 enabled;


    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    portNum = (GT_PHYSICAL_PORT_NUM)inArgs[1];
    enabled = (GT_BOOL)inArgs[2];

    /* call cpss api function */
    result = cpssDxChLatencyMonitoringPortEnableSet(devNum, portNum, enabled);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}

/**
* @internal wrCpssDxChLatencyMonitoringPortEnableGet function
* @endinternal
*
* @brief   Get enabled state of latency monitoring for port.
*
* @note   APPLICABLE DEVICES:      Falcon
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChLatencyMonitoringPortEnableGet

(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS               result;
    GT_U8                   devNum;
    GT_PHYSICAL_PORT_NUM    portNum;
    GT_BOOL                 enabled;


    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    portNum = (GT_PHYSICAL_PORT_NUM)inArgs[1];

    /* call cpss api function */
    result = cpssDxChLatencyMonitoringPortEnableGet(devNum, portNum, &enabled);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", enabled);
    return CMD_OK;
}


/******** Tables */
static GT_U32                                 wrLmu_currentLatencyProfile;

/**
* @internal cpssDxChLatencyMonitoringStatGet function
* @endinternal
*
* @brief  Get latency monitoring statistics.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                - device number
* @param[in] lmuBmp                - LMU bitmap. LMU# selected depending on
*                                    egress MAC range of 8 MACs it corresponds. Bit
*                                    offset in the LMU bitmap equals LMU#. LMU#
*                                    calculated as egress MAC# >> 3. 
* @param[in] latencyProfile        - latency profile (APPLICABLE RANGES: 0..511)
* @param[out] statisticsPtr        - (pointer to) latency monitoring statistics
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - one of the input parameters is not valid.
* @retval GT_BAD_PTR               - null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
*
* @note In case if multiple LMUs selected, packet counts are summed, average
* latency calculated as average of average latency values for all selected LMUs.
* Minimal latency is the lowest value of minimal latency values for all
* selected LMUs. Maximal latency is the highest value of maximal latency values
* for all selected LMUs.
* 
*/
static CMD_STATUS wrCpssDxChLatencyMonitoringStatGetNext
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_U8          dev;
    GT_LATENCY_MONITORING_UNIT_BMP         lmuBmp;
    GT_U32                                 firstLatencyProfile;
    GT_U32                                 numOfProfiles;
    CPSS_DXCH_LATENCY_MONITORING_STAT_STC  statistics;
    GT_STATUS      rc;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    dev = (GT_U8)inArgs[0];
    lmuBmp = (GT_LATENCY_MONITORING_UNIT_BMP)inArgs[1];
    firstLatencyProfile = (GT_U32)inArgs[2];
    numOfProfiles       = (GT_U32)inArgs[3];

    if (wrLmu_currentLatencyProfile >= (firstLatencyProfile + numOfProfiles))
    {
        galtisOutput(outArgs, GT_OK, "%d", -1);
        return CMD_OK;
    }

    /* clean information */
    cpssOsBzero((GT_CHAR *)&statistics, sizeof(statistics));
    rc = cpssDxChLatencyMonitoringStatGet(dev,lmuBmp,wrLmu_currentLatencyProfile,&statistics);
    if (rc != GT_OK)
    {
        galtisOutput(outArgs, rc, "%d", -1);
        return CMD_OK;
    }

    inFields[0] = wrLmu_currentLatencyProfile;
    inFields[1] = statistics.minLatency;
    inFields[2] = statistics.maxLatency;
    inFields[3] = statistics.avgLatency;
    inFields[4] = statistics.packetsInRange.l[0];
    inFields[5] = statistics.packetsInRange.l[1];
    inFields[6] = statistics.packetsOutOfRange.l[0];
    inFields[7] = statistics.packetsOutOfRange.l[1];

    wrLmu_currentLatencyProfile++;

   /* pack and output table fields */
    fieldOutput("%d%d%d%d%d%d%d%d", inFields[0], inFields[1], inFields[2], inFields[3], inFields[4], inFields[5], inFields[6], inFields[7]);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, GT_OK, "%f");

    return CMD_OK;
}

/**
* @internal cpssDxChLatencyMonitoringStatGet function
* @endinternal
*
* @brief  Get latency monitoring statistics.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                - device number
* @param[in] lmuBmp                - LMU bitmap. LMU# selected depending on
*                                    egress MAC range of 8 MACs it corresponds. Bit
*                                    offset in the LMU bitmap equals LMU#. LMU#
*                                    calculated as egress MAC# >> 3. 
* @param[in] latencyProfile        - latency profile (APPLICABLE RANGES: 0..511)
* @param[out] statisticsPtr        - (pointer to) latency monitoring statistics
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - one of the input parameters is not valid.
* @retval GT_BAD_PTR               - null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
*
* @note In case if multiple LMUs selected, packet counts are summed, average
* latency calculated as average of average latency values for all selected LMUs.
* Minimal latency is the lowest value of minimal latency values for all
* selected LMUs. Maximal latency is the highest value of maximal latency values
* for all selected LMUs.
* 
*/
static CMD_STATUS wrCpssDxChLatencyMonitoringStatGetFirst
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

     wrLmu_currentLatencyProfile = (GT_U32)inArgs[2];

     return wrCpssDxChLatencyMonitoringStatGetNext(inArgs, inFields, numFields, outArgs);
}

/**
* @internal cpssDxChLatencyMonitoringCfgGet function
* @endinternal
*
* @brief  Get latency monitoring configuration.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                - device number
* @param[in] lmuBmp                - LMU bitmap. LMU# selected depending on
*                                    egress MAC range of 8 MACs it corresponds. Bit
*                                    offset in the LMU bitmap equals LMU#. LMU#
*                                    calculated as egress MAC# >> 3. Only one LMU
*                                    can be selected.
* @param[in] latencyProfile        - latency profile (APPLICABLE RANGES: 0..511)
*
* @param[out] lmuConfigPtr         - (pointer to) latency monitoring configuration
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - one of the input parameters is not valid.
* @retval GT_OUT_OF_RANGE          - parameter is out of range.
* @retval GT_BAD_PTR               - null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
*/
static CMD_STATUS wrCpssDxChLatencyMonitoringCfgGetNext
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_U8          dev;
    GT_LATENCY_MONITORING_UNIT_BMP         lmuBmp;
    GT_U32                                 firstLatencyProfile;
    GT_U32                                 numOfProfiles;
    CPSS_DXCH_LATENCY_MONITORING_CFG_STC   lmuConfig;
    GT_STATUS      rc;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    dev = (GT_U8)inArgs[0];
    lmuBmp = (GT_LATENCY_MONITORING_UNIT_BMP)inArgs[1];
    firstLatencyProfile = (GT_U32)inArgs[2];
    numOfProfiles       = (GT_U32)inArgs[3];

    if (wrLmu_currentLatencyProfile >= (firstLatencyProfile + numOfProfiles))
    {
        galtisOutput(outArgs, GT_OK, "%d", -1);
        return CMD_OK;
    }

    /* clean information */
    cpssOsBzero((GT_CHAR *)&lmuConfig, sizeof(lmuConfig));
    rc = cpssDxChLatencyMonitoringCfgGet(dev,lmuBmp,wrLmu_currentLatencyProfile,&lmuConfig);
    if (rc != GT_OK)
    {
        galtisOutput(outArgs, rc, "%d", -1);
        return CMD_OK;
    }

    inFields[0] = wrLmu_currentLatencyProfile;
    inFields[1] = lmuConfig.rangeMax;
    inFields[2] = lmuConfig.rangeMin;
    inFields[3] = lmuConfig.notificationThresh;

    wrLmu_currentLatencyProfile++;

   /* pack and output table fields */
    fieldOutput("%d%d%d%d", inFields[0], inFields[1], inFields[2], inFields[3]);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, GT_OK, "%f");

    return CMD_OK;
}

/**
* @internal cpssDxChLatencyMonitoringCfgGet function
* @endinternal
*
* @brief  Get latency monitoring configuration.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                - device number
* @param[in] lmuBmp                - LMU bitmap. LMU# selected depending on
*                                    egress MAC range of 8 MACs it corresponds. Bit
*                                    offset in the LMU bitmap equals LMU#. LMU#
*                                    calculated as egress MAC# >> 3. Only one LMU
*                                    can be selected.
* @param[in] latencyProfile        - latency profile (APPLICABLE RANGES: 0..511)
*
* @param[out] lmuConfigPtr         - (pointer to) latency monitoring configuration
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - one of the input parameters is not valid.
* @retval GT_OUT_OF_RANGE          - parameter is out of range.
* @retval GT_BAD_PTR               - null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
*/
static CMD_STATUS wrCpssDxChLatencyMonitoringCfgGetFirst
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

     wrLmu_currentLatencyProfile = (GT_U32)inArgs[2];

     return wrCpssDxChLatencyMonitoringCfgGetNext(inArgs, inFields, numFields, outArgs);
}

/**** database initialization **************************************/

static CMD_COMMAND dbCommands[] =
{
    {"cpssDxChLatencyMonitoringPortCfgSet",
        &wrCpssDxChLatencyMonitoringPortCfgSet,
        5, 0},
    {"cpssDxChLatencyMonitoringPortCfgGet",
        &wrCpssDxChLatencyMonitoringPortCfgGet,
        3, 0},
    {"cpssDxChLatencyMonitoringEnableSet",
        &wrCpssDxChLatencyMonitoringEnableSet,
        4, 0},
    {"cpssDxChLatencyMonitoringEnableGet",
        &wrCpssDxChLatencyMonitoringEnableGet,
        3, 0},
    {"cpssDxChLatencyMonitoringSamplingProfileSet",
        &wrCpssDxChLatencyMonitoringSamplingProfileSet,
        3, 0},
    {"cpssDxChLatencyMonitoringSamplingProfileGet",
        &wrCpssDxChLatencyMonitoringSamplingProfileGet,
        2, 0},
    {"cpssDxChLatencyMonitoringSamplingConfigurationSet",
        &wrCpssDxChLatencyMonitoringSamplingConfigurationSet,
        4, 0},
    {"cpssDxChLatencyMonitoringSamplingConfigurationGet",
        &wrCpssDxChLatencyMonitoringSamplingConfigurationGet,
        2, 0},
    {"cpssDxChLatencyMonitoringStatGetFirst",
        &wrCpssDxChLatencyMonitoringStatGetFirst,
        4, 0},
    {"cpssDxChLatencyMonitoringStatGetNext",
        &wrCpssDxChLatencyMonitoringStatGetNext,
        4, 0},
    {"cpssDxChLatencyMonitoringCfgGetFirst",
        &wrCpssDxChLatencyMonitoringCfgGetFirst,
        4, 0},
    {"cpssDxChLatencyMonitoringCfgGetNext",
        &wrCpssDxChLatencyMonitoringCfgGetNext,
        4, 0},
    {"cpssDxChLatencyMonitoringCfgSet",
        &wrCpssDxChLatencyMonitoringCfgSet,
        4, 4},
    {"cpssDxChLatencyMonitoringPortEnableSet",
        &wrCpssDxChLatencyMonitoringPortEnableSet,
        3, 0},
    {"cpssDxChLatencyMonitoringPortEnableGet",
        &wrCpssDxChLatencyMonitoringPortEnableGet,
        2, 0}
};


#define numCommands (sizeof(dbCommands) / sizeof(CMD_COMMAND))



/**
* @internal cmdLibInitCpssDxChLatencyMonitoring function
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
GT_STATUS cmdLibInitCpssDxChLatencyMonitoring
(
    GT_VOID
)
{
    return cmdInitLibrary(dbCommands, numCommands);
}
