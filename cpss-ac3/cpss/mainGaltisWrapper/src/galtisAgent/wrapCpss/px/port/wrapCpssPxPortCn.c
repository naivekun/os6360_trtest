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
* @file wrapCpssPxPortCn.c
*
* @brief Wrapper functions for
* cpss/px/port/cpssPxPortCn.h API's
*
* @version   1
********************************************************************************
*/

/* Common galtis includes */
#include <galtisAgent/wrapUtil/cmdCpssPresteraUtils.h>
#include <cmdShell/cmdDb/cmdBase.h>
#include <cmdShell/common/cmdWrapUtils.h>

/* Feature specific includes. */
#include <cpss/px/port/cpssPxPortCn.h>
#include <cpss/px/cpssHwInit/private/prvCpssPxRegsVer1.h>
/**
* @internal wrCpssPxPortCnModeEnableSet function
* @endinternal
*
* @brief   Enable/Disable Congestion Notification (CN) mechanism.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssPxPortCnModeEnableSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_SW_DEV_NUM  devNum;
    GT_BOOL enable;


    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_SW_DEV_NUM)inArgs[0];
    enable = (GT_BOOL)inArgs[1];

    /* call cpss api function */
    result = cpssPxPortCnModeEnableSet(devNum, enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssPxPortCnModeEnableGet function
* @endinternal
*
* @brief   Get the current status of Congestion Notification (CN) mode.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssPxPortCnModeEnableGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_SW_DEV_NUM  devNum;
    GT_BOOL enable;


    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_SW_DEV_NUM)inArgs[0];

    /* call cpss api function */
    result = cpssPxPortCnModeEnableGet(devNum, &enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", enable);

    return CMD_OK;
}

/**
* @internal wrCpssPxPortCnPacketLengthSet function
* @endinternal
*
* @brief   Set packet length used by the CN frames triggering logic.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device number or packet length.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Configuring a value other than CPSS_PX_PORT_CN_LENGTH_ORIG_PACKET_E
*       should be used when the packet length is unknown, e.g. Cut-through mode.
*
*/
static CMD_STATUS wrCpssPxPortCnPacketLengthSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_SW_DEV_NUM  devNum;
    CPSS_PX_PORT_CN_PACKET_LENGTH_ENT packetLength;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_SW_DEV_NUM)inArgs[0];
    packetLength = (CPSS_PX_PORT_CN_PACKET_LENGTH_ENT)inArgs[1];

    /* call cpss api function */
    result = cpssPxPortCnPacketLengthSet(devNum, packetLength);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssPxPortCnPacketLengthGet function
* @endinternal
*
* @brief   Get packet length used by the CN frames triggering logic.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssPxPortCnPacketLengthGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_SW_DEV_NUM  devNum;
    CPSS_PX_PORT_CN_PACKET_LENGTH_ENT packetLength;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_SW_DEV_NUM)inArgs[0];

    /* call cpss api function */
    result = cpssPxPortCnPacketLengthGet(devNum, &packetLength);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", packetLength);

    return CMD_OK;
}

/**
* @internal wrCpssPxPortCnQueueStatusModeEnableSet function
* @endinternal
*
* @brief   Enable/Disable queue status mode. When enabled, CNM is sent to a
*         configurable target that examines the queue status.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssPxPortCnQueueStatusModeEnableSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_SW_DEV_NUM  devNum;
    GT_BOOL enable;
    GT_PORT_NUM targetPort;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_SW_DEV_NUM)inArgs[0];
    enable = (GT_BOOL)inArgs[1];
    targetPort = (GT_PORT_NUM)inArgs[2];


    /* call cpss api function */
    result = cpssPxPortCnQueueStatusModeEnableSet(devNum, enable, targetPort);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssPxPortCnQueueStatusModeEnableGet function
* @endinternal
*
* @brief   Get the current status of queue status mode.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssPxPortCnQueueStatusModeEnableGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_SW_DEV_NUM  devNum;
    GT_BOOL enable;
    GT_PORT_NUM targetPort;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_SW_DEV_NUM)inArgs[0];

    /* call cpss api function */
    result = cpssPxPortCnQueueStatusModeEnableGet(devNum, &enable, &targetPort);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d%d", enable, targetPort);

    return CMD_OK;
}

/**
* @internal wrCpssPxPortCnSampleEntrySet function
* @endinternal
*
* @brief   Set CN sample interval entry.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device number, or entry index
* @retval GT_OUT_OF_RANGE          - on out of range interval
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note 1.To implement CCFC functionality set entryPtr fields to:
*       interval = (MTU+256b)/16
*       randBitmap = 0
*       2.To achieve uniform distribution of random values, clear the interval
*       to be randomized to 0. For example, if interval = 0xF0 and
*       randBitmap = 0x0F then the sampling interval is a random value
*       in [0xF0,0xFF].
*
*/
static CMD_STATUS wrCpssPxPortCnSampleEntrySet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_SW_DEV_NUM  devNum;
    GT_U8 entryIndex;
    CPSS_PX_PORT_CN_SAMPLE_INTERVAL_ENTRY_STC entry;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_SW_DEV_NUM)inArgs[0];
    entryIndex = (GT_U8)inArgs[1];
    entry.interval = (GT_U32)inArgs[2];
    entry.randBitmap = (GT_U32)inArgs[3];

    /* call cpss api function */
    result = cpssPxPortCnSampleEntrySet(devNum, entryIndex, &entry);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssPxPortCnSampleEntryGet function
* @endinternal
*
* @brief   Get CN sample interval entry.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device number, or entry index
* @retval GT_OUT_OF_RANGE          - on out of range interval
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssPxPortCnSampleEntryGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_SW_DEV_NUM  devNum;
    GT_U8 entryIndex;
    CPSS_PX_PORT_CN_SAMPLE_INTERVAL_ENTRY_STC entry;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_SW_DEV_NUM)inArgs[0];
    entryIndex = (GT_U8)inArgs[1];

    /* call cpss api function */
    result = cpssPxPortCnSampleEntryGet(devNum, entryIndex, &entry);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d%d", entry.interval, entry.randBitmap);

    return CMD_OK;
}

/**
* @internal wrCpssPxPortCnFbCalcConfigSet function
* @endinternal
*
* @brief   Set Fb (Feedback) calculation configuration.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_OUT_OF_RANGE          - on out of range value
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note To implement CCFC functionality set fbCalcCfgPtr fields to:
*       deltaEnable = 0
*
*/
static CMD_STATUS wrCpssPxPortCnFbCalcConfigSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_SW_DEV_NUM  devNum;
    CPSS_PX_PORT_CN_FB_CALCULATION_CONFIG_STC fbCalcCfg;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* init with zeros */
    cmdOsMemSet(&fbCalcCfg, 0, sizeof(fbCalcCfg));

    /* map input arguments to locals */
    devNum = (GT_SW_DEV_NUM)inArgs[0];
    fbCalcCfg.wExp = (GT_32)inFields[0];
    fbCalcCfg.fbLsb = (GT_U32)inFields[1];
    fbCalcCfg.deltaEnable = (GT_BOOL)inFields[2];
    fbCalcCfg.fbMin = (GT_U32)inFields[3];
    fbCalcCfg.fbMax = (GT_U32)inFields[4];

    /* call cpss api function */
    result = cpssPxPortCnFbCalcConfigSet(devNum, &fbCalcCfg);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssPxPortCnFbCalcConfigGetFirst function
* @endinternal
*
* @brief   Get Fb (Feedback) calculation configuration.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssPxPortCnFbCalcConfigGetFirst
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_SW_DEV_NUM  devNum;
    CPSS_PX_PORT_CN_FB_CALCULATION_CONFIG_STC fbCalcCfg;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_SW_DEV_NUM)inArgs[0];

    /* call cpss api function */
    result = cpssPxPortCnFbCalcConfigGet(devNum, &fbCalcCfg);

    /* pack output arguments to galtis string */
    inFields[0] = fbCalcCfg.wExp;
    inFields[1] = fbCalcCfg.fbLsb;
    inFields[2] = fbCalcCfg.deltaEnable;
    inFields[3] = fbCalcCfg.fbMin;
    inFields[4] = fbCalcCfg.fbMax;

    /* pack and output table fields */
    fieldOutput("%d%d%d%d%d", inFields[0], inFields[1], inFields[2],
                                           inFields[3], inFields[4]);

    galtisOutput(outArgs, result, "%f");

    return CMD_OK;
}
/*******************************************************************************/
static CMD_STATUS wrCpssPxPortCnFbCalcConfigGetNext
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    galtisOutput(outArgs, GT_OK, "%d", -1);
    return CMD_OK;
}

/**
* @internal wrCpssPxPortCnMessageTriggeringStateSet function
* @endinternal
*
* @brief   Set CNM Triggering State Variables
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device number,portNum,tc
* @retval GT_OUT_OF_RANGE          - on wrong qcnSampleInterval, qlenOld
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note qlenOld reset value and QcnSampleInterval init value must be set by the
*       application prior to configuring the queue to be CN aware
*
*/
static CMD_STATUS wrCpssPxPortCnMessageTriggeringStateSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_SW_DEV_NUM devNum;
    GT_PHYSICAL_PORT_NUM    portNum;
    GT_U8                   tcQueue;
    GT_U32                  qcnSampleInterval;
    GT_U32                  qlenOld;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_SW_DEV_NUM)inArgs[0];
    portNum = (GT_PHYSICAL_PORT_NUM)inArgs[1];
    tcQueue = (GT_U8)inArgs[2];
    qcnSampleInterval = (GT_U32)inArgs[3];
    qlenOld = (GT_U32)inArgs[4];

    /* call cpss api function */
    result = cpssPxPortCnMessageTriggeringStateSet(devNum, portNum, tcQueue, qcnSampleInterval, qlenOld);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssPxPortCnMessageTriggeringStateGet function
* @endinternal
*
* @brief   Get CNM Triggering State Variables
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device number,portNum,tcQueue
* @retval GT_OUT_OF_RANGE          - on wrong qcnSampleInterval, qlenOld
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note qlenOld reset value and QcnSampleInterval init value must be set by the
*       application prior to configuring the queue to be CN aware
*
*/
static CMD_STATUS wrCpssPxPortCnMessageTriggeringStateGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_SW_DEV_NUM devNum;
    GT_PHYSICAL_PORT_NUM    portNum;
    GT_U8                   tcQueue;
    GT_U32                  qcnSampleInterval;
    GT_U32                  qlenOld;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_SW_DEV_NUM)inArgs[0];
    portNum = (GT_PHYSICAL_PORT_NUM)inArgs[1];
    tcQueue = (GT_U8)inArgs[2];

    /* call cpss api function */
    result = cpssPxPortCnMessageTriggeringStateGet(devNum, portNum, tcQueue, &qcnSampleInterval, &qlenOld);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d%d", qcnSampleInterval, qlenOld);

    return CMD_OK;
}

/**
* @internal wrCpssPxPortCnMessageGenerationConfigSet function
* @endinternal
*
* @brief   Set CNM generation configuration.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_OUT_OF_RANGE          - on out of range value
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssPxPortCnMessageGenerationConfigSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_SW_DEV_NUM devNum;
    CPSS_PX_PORT_CNM_GENERATION_CONFIG_STC cnmGenerationCfg;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* init with zeros */
    cmdOsMemSet(&cnmGenerationCfg, 0, sizeof(cnmGenerationCfg));

    /* map input arguments to locals */
    devNum = (GT_SW_DEV_NUM)inArgs[0];

    result = cpssPxPortCnMessageGenerationConfigGet(devNum, &cnmGenerationCfg);
    if (result != GT_OK)
    {
        /* pack output arguments to galtis string */
        galtisOutput(outArgs, result, "");
        return CMD_OK;
    }

    cnmGenerationCfg.packetType                 = (GT_U32) inFields[0];
    cnmGenerationCfg.priority                   = (GT_U32) inFields[1];
    cnmGenerationCfg.dropPrecedence             = (CPSS_DP_LEVEL_ENT)inFields[2];
    cnmGenerationCfg.tc4pfc                     = (GT_U32) inFields[3];
    cnmGenerationCfg.localGeneratedPacketTypeAssignmentEnable = (GT_BOOL) inFields[4];
    cnmGenerationCfg.localGeneratedPacketType   = (GT_U32) inFields[5];

    /* call cpss api function */
    result = cpssPxPortCnMessageGenerationConfigSet(devNum, &cnmGenerationCfg);
    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssPxPortCnMessageGenerationConfigGetFirst function
* @endinternal
*
* @brief   Get CNM generation configuration.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssPxPortCnMessageGenerationConfigGetFirst
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_SW_DEV_NUM devNum;
    CPSS_PX_PORT_CNM_GENERATION_CONFIG_STC cnmGenerationCfg;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_SW_DEV_NUM)inArgs[0];

    /* call cpss api function */
    result = cpssPxPortCnMessageGenerationConfigGet(devNum, &cnmGenerationCfg);

    /* pack and output table fields */
    inFields[0]  = cnmGenerationCfg.packetType;
    inFields[1]  = cnmGenerationCfg.priority;
    inFields[2]  = cnmGenerationCfg.dropPrecedence;
    inFields[3]  = cnmGenerationCfg.tc4pfc;
    inFields[4]  = cnmGenerationCfg.localGeneratedPacketTypeAssignmentEnable;
    inFields[5]  = cnmGenerationCfg.localGeneratedPacketType;

    fieldOutput("%d%d%d%d%d%d%", 
                inFields[0], inFields[1], inFields[2], 
                inFields[3], inFields[4], inFields[5]);

    galtisOutput(outArgs, result, "%f");

    return CMD_OK;
}

/*******************************************************************************/
static CMD_STATUS wrCpssPxPortCnMessageGenerationConfigGetNext
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    galtisOutput(outArgs, GT_OK, "%d", -1);
    return CMD_OK;
}

/**
* @internal wrCpssPxPortCnMessagePortMapEntrySet function
* @endinternal
*
* @brief   Set port map configuration per target port
*         for the triggered QCN incoming descriptor.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device or port number
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
CMD_STATUS wrCpssPxPortCnMessagePortMapEntrySet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                       result;
    GT_SW_DEV_NUM                   devNum;
    GT_PHYSICAL_PORT_NUM            portNum;
    CPSS_PX_PORTS_BMP               portsBmp;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum                          = (GT_SW_DEV_NUM)inArgs[0];

    /* map input fields to locals */
    portNum                         = (GT_PHYSICAL_PORT_NUM)inFields[0];
    portsBmp                        = (CPSS_PX_PORTS_BMP)inFields[1];

    /* Override Device and Port */
    CONVERT_SW_DEV_PHYSICAL_PORT_MAC(devNum, portNum);

    /* call cpss api set function */
    result = cpssPxPortCnMessagePortMapEntrySet(devNum, portNum, portsBmp);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

static GT_U32 gCurrentPort;
/**
* @internal wrCpssPxPortCnMessagePortMapEntryGet function
* @endinternal
*
* @brief   Get port map configuration per target port
*         for the triggered QCN incoming descriptor.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_PARAM             - on wrong device or port number
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
CMD_STATUS wrCpssPxPortCnMessagePortMapEntryGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                       result;
    GT_SW_DEV_NUM                   devNum;
    CPSS_PX_PORTS_BMP               portsBmp;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_SW_DEV_NUM)inArgs[0];

    if (gCurrentPort >= PRV_CPSS_PX_PORTS_NUM_CNS)
    {
        galtisOutput(outArgs, GT_OK, "%d", -1);
        return CMD_OK;
    }

    /* call cpss api set function */
    result = cpssPxPortCnMessagePortMapEntryGet(devNum, gCurrentPort, &portsBmp);
    if (result != GT_OK)
    {
        galtisOutput(outArgs, result, "%d", -1);
        return CMD_OK;
    }

    fieldOutput("%d%d", gCurrentPort, portsBmp);

    /* Increment current index before next read */
    gCurrentPort++;

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, GT_OK, "%f");

    return CMD_OK;
}

/* wrapper for cpssPxPortCnMessagePortMapEntryGet */
CMD_STATUS wrCpssPxPortCnMessagePortMapEntryGetFirst
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

    gCurrentPort = 0;

    return wrCpssPxPortCnMessagePortMapEntryGet(inArgs, inFields, numFields, outArgs); 
}

/* wrapper for cpssPxPortCnMessagePortMapEntryGet */
CMD_STATUS wrCpssPxPortCnMessagePortMapEntryGetNext
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    return wrCpssPxPortCnMessagePortMapEntryGet(inArgs, inFields, numFields, outArgs);
}

/**
* @internal wrCpssPxPortCnTerminationEnableSet function
* @endinternal
*
* @brief   Enable/Disable termination of CNM (Congestion Notification Message) on given port.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssPxPortCnTerminationEnableSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_SW_DEV_NUM devNum;
    GT_PHYSICAL_PORT_NUM portNum;
    GT_BOOL enable;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_SW_DEV_NUM)inArgs[0];
    portNum = (GT_PHYSICAL_PORT_NUM)inArgs[1];
    enable = (GT_BOOL)inArgs[2];

    /* Override Device and Port */
    CONVERT_SW_DEV_PHYSICAL_PORT_MAC(devNum, portNum);

    /* call cpss api function */
    result = cpssPxPortCnTerminationEnableSet(devNum, portNum, enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssPxPortCnTerminationEnableGet function
* @endinternal
*
* @brief   Get the status of CNM termination on given port.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssPxPortCnTerminationEnableGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_SW_DEV_NUM devNum;
    GT_PHYSICAL_PORT_NUM portNum;
    GT_BOOL enable;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_SW_DEV_NUM)inArgs[0];
    portNum = (GT_PHYSICAL_PORT_NUM)inArgs[1];

    /* Override Device and Port */
    CONVERT_SW_DEV_PHYSICAL_PORT_MAC(devNum, portNum);

    /* call cpss api function */
    result = cpssPxPortCnTerminationEnableGet(devNum, portNum, &enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", enable);

    return CMD_OK;
}

/**
* @internal wrCpssPxPortCnFcEnableSet function
* @endinternal
*
* @brief   Enable/Disable generation of flow control packets as a result of CCFC
*         frame termination.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssPxPortCnFcEnableSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_SW_DEV_NUM devNum;
    GT_PHYSICAL_PORT_NUM portNum;
    GT_BOOL enable;


    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_SW_DEV_NUM)inArgs[0];
    portNum = (GT_PHYSICAL_PORT_NUM)inArgs[1];
    enable = (GT_BOOL)inArgs[2];

    /* Override Device and Port */
    CONVERT_SW_DEV_PHYSICAL_PORT_MAC(devNum, portNum);

    /* call cpss api function */
    result = cpssPxPortCnFcEnableSet(devNum, portNum, enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");



    return CMD_OK;
}

/**
* @internal wrCpssPxPortCnFcEnableGet function
* @endinternal
*
* @brief   Get the status of generation of flow control packets as a result of CCFC
*         frame termination on a given port.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssPxPortCnFcEnableGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_SW_DEV_NUM devNum;
    GT_PHYSICAL_PORT_NUM portNum;
    GT_BOOL enable;


    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_SW_DEV_NUM)inArgs[0];
    portNum = (GT_PHYSICAL_PORT_NUM)inArgs[1];

    /* Override Device and Port */
    CONVERT_SW_DEV_PHYSICAL_PORT_MAC(devNum, portNum);

    /* call cpss api function */
    result = cpssPxPortCnFcEnableGet(devNum, portNum, &enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", enable);

    return CMD_OK;
}

/**
* @internal wrCpssPxPortCnFcTimerSet function
* @endinternal
*
* @brief   Set timer value for the IEEE 802.3x pause frame issued as result
*         of CCFC frame termination.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number, device or speed index
* @retval GT_OUT_OF_RANGE          - on out of range pause time value
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note 1. Panic Pause uses Port Speed Index 0 and 7 to indicate XON and XOFF.
*       Port timer configurations 0 and 7 must be configured
*       to 0x0 and 0xFFFF if Panic Pause is used.
*
*/
static CMD_STATUS wrCpssPxPortCnFcTimerSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_SW_DEV_NUM devNum;
    GT_PHYSICAL_PORT_NUM portNum;
    GT_U32 portSpeedIndex;
    GT_U32 timer;


    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_SW_DEV_NUM)inArgs[0];
    portNum = (GT_PHYSICAL_PORT_NUM)inArgs[1];
    portSpeedIndex = (GT_U32)inArgs[2];
    timer = (GT_U32)inArgs[3];

    /* Override Device and Port */
    CONVERT_SW_DEV_PHYSICAL_PORT_MAC(devNum, portNum);

    /* call cpss api function */
    result = cpssPxPortCnFcTimerSet(devNum, portNum, portSpeedIndex, timer);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");



    return CMD_OK;
}

/**
* @internal wrCpssPxPortCnFcTimerGet function
* @endinternal
*
* @brief   Get timer value for the IEEE 802.3x pause frame issued as a result
*         of CCFC frame termination.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_BAD_PARAM             - on wrong port number, device or speed index
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssPxPortCnFcTimerGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_SW_DEV_NUM devNum;
    GT_PHYSICAL_PORT_NUM portNum;
    GT_U32 portSpeedIndex;
    GT_U32 timer;


    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_SW_DEV_NUM)inArgs[0];
    portNum = (GT_PHYSICAL_PORT_NUM)inArgs[1];
    portSpeedIndex = (GT_U32)inArgs[2];

    /* Override Device and Port */
    CONVERT_SW_DEV_PHYSICAL_PORT_MAC(devNum, portNum);

    /* call cpss api function */
    result = cpssPxPortCnFcTimerGet(devNum, portNum, portSpeedIndex, &timer);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", timer);

    return CMD_OK;
}

/**
* @internal wrCpssPxPortCnProfileQueueConfigSet function
* @endinternal
*
* @brief   Sets CN profile configurations for given tc queue.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_PARAM             - on wrong device number, profile set
*                                       or traffic class queue
* @retval GT_OUT_OF_RANGE          - on out of range threshold value
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note CN Profile is part of the Tail-Drop Profile.
*       To bind port to Tail-Drop Profile use cpssPxPortTxTailDropProfileIdSet.
*
*/
static CMD_STATUS wrCpssPxPortCnProfileQueueConfigSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_SW_DEV_NUM devNum;
    CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_SET_ENT profileSet;
    GT_U32 tcQueue;
    CPSS_PX_PORT_CN_PROFILE_CONFIG_STC cnProfileCfg;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_SW_DEV_NUM)inArgs[0];
    profileSet = (CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_SET_ENT)inArgs[1];

    /* init with zeros */
    cmdOsMemSet(&cnProfileCfg, 0, sizeof(cnProfileCfg));

    tcQueue = (GT_U8)inFields[0];
    cnProfileCfg.cnAware = (GT_U32)inFields[1];
    cnProfileCfg.threshold = (GT_U32)inFields[2];
    cnProfileCfg.alpha = (GT_U32)inFields[3];

    /* call cpss api function */
    result = cpssPxPortCnProfileQueueConfigSet(devNum, profileSet, tcQueue, &cnProfileCfg);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/*cpssPxPortCnProfileQueueConfig Table*/
/*index to iterate get func*/
static GT_U32 prvTcQueueIndex = 0;

/*******************************************************************************/
static CMD_STATUS wrCpssPxPortCnProfileQueueConfigGetNext
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8     devNum;
    CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_SET_ENT profileSet;
    CPSS_PX_PORT_CN_PROFILE_CONFIG_STC cnProfileCfg;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_SW_DEV_NUM)inArgs[0];
    profileSet = (CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_SET_ENT)inArgs[1];

    if(prvTcQueueIndex >= CPSS_TC_RANGE_CNS)
    {
        galtisOutput(outArgs, GT_OK, "%d", -1);
        return CMD_OK;
    }

    /* call cpss api function */
    result = cpssPxPortCnProfileQueueConfigGet(devNum, profileSet, prvTcQueueIndex, &cnProfileCfg);

    if (result != GT_OK)
    {
        galtisOutput(outArgs, result, "%d", -1);
        return CMD_OK;
    }

    inFields[0] = prvTcQueueIndex;

    /* pack output arguments to galtis string */
    inFields[1] = cnProfileCfg.cnAware;
    inFields[2] = cnProfileCfg.threshold;
    inFields[3] = cnProfileCfg.alpha;

    /* pack and output table fields */
    fieldOutput("%d%d%d%d", inFields[0], inFields[1], inFields[2], inFields[3]);

    galtisOutput(outArgs, result, "%f");

    prvTcQueueIndex++;
    return CMD_OK;
}

/**
* @internal wrCpssPxPortCnProfileQueueConfigGetFirst function
* @endinternal
*
* @brief   Gets CN profile configurations for given tc queue.
*
* @note   APPLICABLE DEVICES:      Lion and above
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_BAD_PARAM             - on wrong device number, profile set
*                                       or traffic class queue
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssPxPortCnProfileQueueConfigGetFirst
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    /*reset on first*/
    prvTcQueueIndex = 0;

    return wrCpssPxPortCnProfileQueueConfigGetNext(inArgs,inFields,numFields,outArgs);
}

/**
* @internal wrCpssPxPortCnAvailableBuffSet function
* @endinternal
*
* @brief   Set amount of buffers available for dynamic allocation for CN.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @note Valid only if DBA mode enabled.
*
*/
CMD_STATUS wrCpssPxPortCnAvailableBuffSet
(
    IN  GT_UINTPTR  inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR  inFields[CMD_MAX_FIELDS],
    IN  GT_32       numFields,
    OUT GT_8        outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS       result;
    GT_SW_DEV_NUM   devNum;
    GT_U32          availableBuff;

    /* check for valid arguments */
    if ((!inArgs) || (!outArgs))
    {
        return CMD_AGENT_ERROR;
    }

    /* map input arguments to locals */
    devNum        = (GT_SW_DEV_NUM) inArgs[0];
    availableBuff = (GT_U32) inArgs[1];

    /* call cpss api function */
    result = cpssPxPortCnAvailableBuffSet(devNum, availableBuff);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssPxPortCnAvailableBuffGet function
* @endinternal
*
* @brief   Get amount of buffers available for dynamic allocation for CN.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
*/
CMD_STATUS wrCpssPxPortCnAvailableBuffGet
(
    IN  GT_UINTPTR  inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR  inFields[CMD_MAX_FIELDS],
    IN  GT_32       numFields,
    OUT GT_8        outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS       result;
    GT_SW_DEV_NUM   devNum;
    GT_U32          availableBuff;

    /* check for valid arguments */
    if ((!inArgs) || (!outArgs))
    {
        return CMD_AGENT_ERROR;
    }

    /* map input arguments to locals */
    devNum = (GT_SW_DEV_NUM) inArgs[0];

    /* call cpss api function */
    result = cpssPxPortCnAvailableBuffGet(devNum, &availableBuff);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", availableBuff);

    return CMD_OK;
}

/*******************************************************************************
* cpssPxPortCnMessageTypeSet
*
* DESCRIPTION:
*       Sets congestion notification message type - QCN or CCFC
*
* APPLICABLE DEVICES:
*        Pipe.
*
* NOT APPLICABLE DEVICES:
*        None.
*
* INPUTS:
*       devNum      - device number.
*       mType       - CN message type
*
* OUTPUTS:
*       None
*
* RETURNS:
*       GT_OK                    - on success
*       GT_BAD_PARAM             - on wrong device number
*       GT_HW_ERROR              - on hardware error
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*       None.
*
*******************************************************************************/
static CMD_STATUS wrCpssPxPortCnMessageTypeSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8 devNum;
    CPSS_PX_PORT_CN_MESSAGE_TYPE_ENT mType;


    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_SW_DEV_NUM)inArgs[0];
    mType = (CPSS_PX_PORT_CN_MESSAGE_TYPE_ENT)inArgs[1];

    /* call cpss api function */
    result = cpssPxPortCnMessageTypeSet(devNum, mType);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/*******************************************************************************
* cpssPxPortCnMessageTypeSet
*
* DESCRIPTION:
*       Sets congestion notification message type - QCN or CCFC
*
* APPLICABLE DEVICES:
*        Pipe.
*
* NOT APPLICABLE DEVICES:
*        None.
*
* INPUTS:
*       devNum      - device number.
*
* OUTPUTS:
*       mTypePtr    - (pointer to) CN message type
*
* RETURNS:
*       GT_OK                    - on success
*       GT_BAD_PARAM             - on wrong device number
*       GT_HW_ERROR              - on hardware error
*       GT_BAD_PTR               - on NULL pointer
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*       None.
*
*******************************************************************************/
static CMD_STATUS wrCpssPxPortCnMessageTypeGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8 devNum;
    CPSS_PX_PORT_CN_MESSAGE_TYPE_ENT mType;


    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_SW_DEV_NUM)inArgs[0];

    /* call cpss api function */
    result = cpssPxPortCnMessageTypeGet(devNum, &mType);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", mType);

    return CMD_OK;
}

/**** database initialization **************************************/
static CMD_COMMAND dbCommands[] =
{
    /* commandName                                   funcReference                  funcArgs  funcFields */
    {"cpssPxPortCnModeEnableSet",
        &wrCpssPxPortCnModeEnableSet,
        2, 0},
    {"cpssPxPortCnModeEnableGet",
        &wrCpssPxPortCnModeEnableGet,
        1, 0},
    {"cpssPxPortCnPacketLengthSet",
        &wrCpssPxPortCnPacketLengthSet,
        2, 0},
    {"cpssPxPortCnPacketLengthGet",
        &wrCpssPxPortCnPacketLengthGet,
        1, 0},
    {"cpssPxPortCnQueueStatusModeEnableSet",
        &wrCpssPxPortCnQueueStatusModeEnableSet,
        3, 0},
    {"cpssPxPortCnQueueStatusModeEnableGet",
        &wrCpssPxPortCnQueueStatusModeEnableGet,
        1, 0},
    {"cpssPxPortCnSampleEntrySet",
        &wrCpssPxPortCnSampleEntrySet,
        4, 0},
    {"cpssPxPortCnSampleEntryGet",
        &wrCpssPxPortCnSampleEntryGet,
        2, 0},
    {"cpssPxPortCnFbCalcConfigSet",
        &wrCpssPxPortCnFbCalcConfigSet,
        1, 5},
    {"cpssPxPortCnFbCalcConfigGetFirst",
        &wrCpssPxPortCnFbCalcConfigGetFirst,
        1, 0},
    {"cpssPxPortCnFbCalcConfigGetNext",
        &wrCpssPxPortCnFbCalcConfigGetNext,
        1, 0},
    {"cpssPxPortCnMessageTriggeringStateSet",
        &wrCpssPxPortCnMessageTriggeringStateSet,
        5, 0},
    {"cpssPxPortCnMessageTriggeringStateGet",
        &wrCpssPxPortCnMessageTriggeringStateGet,
        3, 0},
    {"cpssPxPortCnMessageGenerationConfigSet",
        &wrCpssPxPortCnMessageGenerationConfigSet,
        1, 6},
    {"cpssPxPortCnMessageGenerationConfigGetFirst",
        &wrCpssPxPortCnMessageGenerationConfigGetFirst,
        1, 0},
    {"cpssPxPortCnMessageGenerationConfigGetNext",
        &wrCpssPxPortCnMessageGenerationConfigGetNext,
        1, 0},
    { "cpssPxPortCnMessagePortMapEntrySet",
        &wrCpssPxPortCnMessagePortMapEntrySet,
        1, 2 },
    { "cpssPxPortCnMessagePortMapEntryGetFirst",
        &wrCpssPxPortCnMessagePortMapEntryGetFirst,
        1, 0 },
    { "cpssPxPortCnMessagePortMapEntryGetNext",
        &wrCpssPxPortCnMessagePortMapEntryGetNext,
        1, 0 },
    {"cpssPxPortCnTerminationEnableSet",
        &wrCpssPxPortCnTerminationEnableSet,
        3, 0},
    {"cpssPxPortCnTerminationEnableGet",
        &wrCpssPxPortCnTerminationEnableGet,
        2, 0},
    {"cpssPxPortCnFcEnableSet",
        &wrCpssPxPortCnFcEnableSet,
        3, 0},
    {"cpssPxPortCnFcEnableGet",
        &wrCpssPxPortCnFcEnableGet,
        2, 0},
    {"cpssPxPortCnFcTimerSet",
        &wrCpssPxPortCnFcTimerSet,
        4, 0},
    {"cpssPxPortCnFcTimerGet",
        &wrCpssPxPortCnFcTimerGet,
        3, 0},
    {"cpssPxPortCnProfileQueueConfigSet",
        &wrCpssPxPortCnProfileQueueConfigSet,
        2, 4},
    {"cpssPxPortCnProfileQueueConfigGetFirst",
        &wrCpssPxPortCnProfileQueueConfigGetFirst,
        2, 0},
    {"cpssPxPortCnProfileQueueConfigGetNext",
        &wrCpssPxPortCnProfileQueueConfigGetNext,
        2, 0},
    {"cpssPxPortCnAvailableBuffSet",
        &wrCpssPxPortCnAvailableBuffSet,
        2, 0},
    {"cpssPxPortCnAvailableBuffGet",
        &wrCpssPxPortCnAvailableBuffGet,
        1, 0},
    {"cpssPxPortCnMessageTypeSet",
         &wrCpssPxPortCnMessageTypeSet,
         2, 0},
    {"cpssPxPortCnMessageTypeGet",
         &wrCpssPxPortCnMessageTypeGet,
         1, 0}
};

#define  numCommands     (sizeof(dbCommands) / sizeof(CMD_COMMAND))

/**
* @internal cmdLibInitCpssPxPortCn function
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
GT_STATUS cmdLibInitCpssPxPortCn
(
    GT_VOID
)
{
    return cmdInitLibrary(dbCommands, numCommands);
}


