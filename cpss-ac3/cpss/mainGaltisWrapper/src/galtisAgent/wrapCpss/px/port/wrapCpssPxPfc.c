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
* @file wrapCpssPxPfc.c
*
* @brief Wrapper functions for
* cpss/px/port/cpssPxPfc.h API's
*
* @version   1
********************************************************************************
*/

/* Common galtis includes */
#include <galtisAgent/wrapUtil/cmdCpssPresteraUtils.h>
#include <cmdShell/cmdDb/cmdBase.h>
#include <cmdShell/common/cmdWrapUtils.h>

/* Feature specific includes. */
#include <cpss/px/port/cpssPxPortCtrl.h>
#include <cpss/px/port/cpssPxPortMapping.h>
#include <cpss/px/cpssHwInit/private/prvCpssPxRegsVer1.h>
#include <cpss/px/config/private/prvCpssPxInfo.h>
#include <cpss/px/port/cpssPxPortTxTailDrop.h>
#include <cpss/px/port/cpssPxPortTxScheduler.h>
#include <cpss/px/cos/private/prvCpssPxCoS.h>


#include <cpss/px/port/cpssPxPortPfc.h>
/**
* @internal wrCpssPxPfcEnableSet function
* @endinternal
*
* @brief   Enable/Disable PFC (Priority Flow Control) response functionality.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device number or PFC enable option
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note If PFC response is enabled, the shaper’s baseline must be
*       at least 0x3FFFC0, see:
*       cpssPxPortTxShaperBaselineSet.
*       Note: Triggering cannot be disabled by this API.
*
*/
CMD_STATUS wrCpssPxPfcEnableSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                       result;
    GT_SW_DEV_NUM                   devNum;
    CPSS_PX_PORT_PFC_ENABLE_ENT     pfcEnable;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum                                  = (GT_SW_DEV_NUM)inArgs[0];
    pfcEnable                               = (CPSS_PX_PORT_PFC_ENABLE_ENT)inArgs[1];

    /* call cpss api function */
    result = cpssPxPortPfcEnableSet(devNum, pfcEnable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssPxPfcEnableGet function
* @endinternal
*
* @brief   Get the status of PFC response functionality.
*
* @note   APPLICABLE DEVICES:      Pipe
* @note   NOT APPLICABLE DEVICES:  None
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Triggering is enabled by default.
*
*/
CMD_STATUS wrCpssPxPfcEnableGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                       result;
    GT_SW_DEV_NUM                   devNum;
    CPSS_PX_PORT_PFC_ENABLE_ENT     pfcEnable;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum                                  = (GT_SW_DEV_NUM)inArgs[0];

    /* call cpss api function */
    result = cpssPxPortPfcEnableGet(devNum, &pfcEnable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", pfcEnable);

    return CMD_OK;
}
/**
* @internal wrCpssPxPortPfcProfileIndexSet function
* @endinternal
*
* @brief   Binds a source port to a PFC profile.
*
* @note   APPLICABLE DEVICES:      Pipe
* @note   NOT APPLICABLE DEVICES:  None
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_OUT_OF_RANGE          - on out of range profile index
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
CMD_STATUS wrCpssPxPortPfcProfileIndexSet
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
    GT_U32                          profileIndex;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum                                  = (GT_SW_DEV_NUM)inArgs[0];
    portNum                                 = (GT_PHYSICAL_PORT_NUM)inArgs[1];
    profileIndex                            = (GT_U32)inArgs[2];

    /* call cpss api function */
    result = cpssPxPortPfcProfileIndexSet(devNum, portNum, profileIndex);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}
/**
* @internal wrCpssPxPortPfcProfileIndexGet function
* @endinternal
*
* @brief   Gets the port's PFC profile.
*
* @note   APPLICABLE DEVICES:      Pipe
* @note   NOT APPLICABLE DEVICES:  None
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
CMD_STATUS wrCpssPxPortPfcProfileIndexGet
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
    GT_U32                          profileIndex;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum                                  = (GT_SW_DEV_NUM)inArgs[0];
    portNum                                 = (GT_PHYSICAL_PORT_NUM)inArgs[1];

    /* call cpss api function */
    result = cpssPxPortPfcProfileIndexGet(devNum, portNum, &profileIndex);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", profileIndex);

   return CMD_OK;
}

/**
* @internal wrCpssPxPortPfcDbaBuffersSet function
* @endinternal
*
* @brief   Define the number of buffers that are Available for the PFC Dynamic threshold calculations.
*
* @note   APPLICABLE DEVICES:      Pipe
* @note   NOT APPLICABLE DEVICES:  None
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_OUT_OF_RANGE          - on out of range buffers number
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
CMD_STATUS wrCpssPxPortPfcDbaBuffersSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                       result;
    GT_SW_DEV_NUM                   devNum;
    GT_U32                          buffNum;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum                                  = (GT_SW_DEV_NUM)inArgs[0];
    buffNum                                 = (GT_U32)inArgs[1];

    /* call cpss api function */
    result = cpssPxPortPfcDbaAvailableBuffersSet(devNum, buffNum);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}
/**
* @internal wrCpssPxPortPfcDbaBuffersGet function
* @endinternal
*
* @brief   Get the number of buffers that are Available for the PFC Dynamic threshold calculations.
*
* @note   APPLICABLE DEVICES:      Pipe
* @note   NOT APPLICABLE DEVICES:  None
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_OUT_OF_RANGE          - on out of range buffers number
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
CMD_STATUS wrCpssPxPortPfcDbaBuffersGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                       result;
    GT_SW_DEV_NUM                   devNum;
    GT_U32                          buffsNum;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum                                  = (GT_SW_DEV_NUM)inArgs[0];

    /* call cpss api function */
    result = cpssPxPortPfcDbaAvailableBuffersGet(devNum, &buffsNum);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", buffsNum);

   return CMD_OK;
}

/**
* @internal wrCpssPxPortPfcProfileQueueConfigSet function
* @endinternal
*
* @brief   Sets PFC profile configurations for given tc queue.
*
* @note   APPLICABLE DEVICES:      Pipe
* @note   NOT APPLICABLE DEVICES:  None
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_PARAM             - on wrong device number, profile index
*                                       or traffic class queue
* @retval GT_OUT_OF_RANGE          - on out of range threshold value
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note All thresholds are set in buffers or packets.
*       See cpssPxPortPfcCountingModeSet.
*
*/
CMD_STATUS wrCpssPxPortPfcProfileQueueConfigSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS   result;

    GT_SW_DEV_NUM       devNum;
    GT_U32              profileIndex;
    GT_U32              tcQueue;
    CPSS_PX_PORT_PFC_PROFILE_CONFIG_STC pfcProfileCfg;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_SW_DEV_NUM)inArgs[0];
    profileIndex = (GT_U8)inArgs[1];
    tcQueue = (GT_U32)inArgs[2];

    /* map input arguments to locals */
    pfcProfileCfg.xonThreshold  = (GT_U32)inFields[0];
    pfcProfileCfg.xoffThreshold = (GT_U32)inFields[1];
    pfcProfileCfg.xonAlpha      = (GT_U32)inFields[2];
    pfcProfileCfg.xoffAlpha     = (GT_U32)inFields[3];

    /* call cpss api function */
    result = cpssPxPortPfcProfileQueueConfigSet(devNum, profileIndex,
                                                  tcQueue, &pfcProfileCfg);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}

/**
* @internal wrCpssPxPortPfcProfileQueueConfigGet function
* @endinternal
*
* @brief   Gets PFC profile configurations for given tc queue.
*
* @note   APPLICABLE DEVICES:      Pipe
* @note   NOT APPLICABLE DEVICES:  None
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_PARAM             - on wrong device number, profile index
*                                       or traffic class queue
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note All thresholds are set in buffers or packets.
*       See cpssPxPortPfcCountingModeSet.
*
*/
CMD_STATUS wrCpssPxPortPfcProfileQueueConfigGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                               result;
    GT_SW_DEV_NUM                           devNum;
    GT_U32                                  profileIndex;
    GT_U32                                  tcQueue;
    CPSS_PX_PORT_PFC_PROFILE_CONFIG_STC     pfcProfileCfg;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum                            = (GT_SW_DEV_NUM)inArgs[0];
    profileIndex                      = (GT_U32)inArgs[2];
    tcQueue                           = (GT_U32)inArgs[1];

    /* call cpss api function */
    result = cpssPxPortPfcProfileQueueConfigGet(devNum, tcQueue, profileIndex, &pfcProfileCfg);

    /* pack output arguments to galtis string */
    inFields[0] = pfcProfileCfg.xonThreshold;
    inFields[1] = pfcProfileCfg.xoffThreshold;
    inFields[2] = pfcProfileCfg.xonAlpha;
    inFields[3] = pfcProfileCfg.xoffAlpha;

    /* pack and output table fields */
    galtisOutput(outArgs, result, "%d%d%d%d", inFields[0], inFields[1], inFields[2], inFields[3]);


    return CMD_OK;
}


static CMD_STATUS  wrCpssPxPortPfcProfileQueueConfigGetFirst
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{

    return wrCpssPxPortPfcProfileQueueConfigGet(inArgs, inFields, numFields, outArgs);
}

static CMD_STATUS  wrCpssPxPortPfcProfileQueueConfigGetNext
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
* @internal wrCpssPxPortPfcCountingModeSet function
* @endinternal
*
* @brief   Sets PFC counting mode.
*
* @note   APPLICABLE DEVICES:      Pipe
* @note   NOT APPLICABLE DEVICES:  None
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
CMD_STATUS wrCpssPxPortPfcCountingModeSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                       result;
    GT_SW_DEV_NUM                   devNum;
    CPSS_PX_PORT_PFC_COUNT_MODE_ENT  pfcCountMode;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum                                  = (GT_SW_DEV_NUM)inArgs[0];
    pfcCountMode                            = (CPSS_PX_PORT_PFC_COUNT_MODE_ENT)inArgs[1];

    /* call cpss api function */
    result = cpssPxPortPfcCountingModeSet(devNum, pfcCountMode);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}
/**
* @internal wrCpssPxPortPfcCountingModeGet function
* @endinternal
*
* @brief   Gets PFC counting mode.
*
* @note   APPLICABLE DEVICES:      Pipe
* @note   NOT APPLICABLE DEVICES:  None
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
CMD_STATUS wrCpssPxPortPfcCountingModeGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                       result;
    GT_SW_DEV_NUM                   devNum;
    CPSS_PX_PORT_PFC_COUNT_MODE_ENT pfcCountMode;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum                                  = (GT_SW_DEV_NUM)inArgs[0];

    /* call cpss api function */
    result = cpssPxPortPfcCountingModeGet(devNum, &pfcCountMode);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", pfcCountMode);

    return CMD_OK;
}

/**
* @internal wrCpssPxPortPfcGlobalDropEnableSet function
* @endinternal
*
* @brief   Enable/Disable PFC global drop.
*
* @note   APPLICABLE DEVICES:      Pipe
* @note   NOT APPLICABLE DEVICES:  None
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note To configure drop threshold use cpssPxPortPfcGlobalQueueConfigSet.
*
*/
CMD_STATUS wrCpssPxPortPfcGlobalDropEnableSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                       result;
    GT_SW_DEV_NUM                   devNum;
    GT_BOOL                         enable;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum                                  = (GT_SW_DEV_NUM)inArgs[0];
    enable                                  = (GT_BOOL)inArgs[1];

    /* call cpss api function */
    result = cpssPxPortPfcGlobalDropEnableSet(devNum, enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}
/**
* @internal wrCpssPxPortPfcGlobalDropEnableGet function
* @endinternal
*
* @brief   Gets the current status of PFC global drop.
*
* @note   APPLICABLE DEVICES:      Pipe
* @note   NOT APPLICABLE DEVICES:  None
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
CMD_STATUS wrCpssPxPortPfcGlobalDropEnableGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                       result;
    GT_SW_DEV_NUM                   devNum;
    GT_BOOL                         enable;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum                                  = (GT_SW_DEV_NUM)inArgs[0];

    /* call cpss api function */
    result = cpssPxPortPfcGlobalDropEnableGet(devNum, &enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", enable);

    return CMD_OK;
}

/**
* @internal wrCpssPxPortPfcGlobalQueueConfigSet function
* @endinternal
*
* @brief   Sets PFC profile configurations for given tc queue.
*         APPLICABLE DEVICES:
*         Pipe
*         NOT APPLICABLE DEVICES:
*         None
*         INPUTS:
*         devNum    - device number
*         tcQueue    - traffic class queue (APPLICABLE RANGES: 0..7)
*         xoffThreshold - XOFF threshold
*         APPLICABLE RANGES: 0..0x1FFFF
*         dropThreshold - Drop threshold. When a global counter with given tcQueue
*         crosses up the dropThreshold the packets are dropped
*         APPLICABLE RANGES: 0..0x1FFFF
*         xonThreshold - XON threshold
*
* @note   APPLICABLE DEVICES:      Pipe
* @note   NOT APPLICABLE DEVICES:  None
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device number, profile index
*                                       or traffic class queue
* @retval GT_OUT_OF_RANGE          - on out of range threshold value
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note 1. dropThreshold is used when PFC global drop is enabled.
*       See cpssPxPortPfcGlobalDropEnableSet.
*       2. All thresholds are set in buffers or packets.
*       See cpssPxPortPfcCountingModeSet.
*
*/
CMD_STATUS wrCpssPxPortPfcGlobalQueueConfigSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS   result;

    GT_SW_DEV_NUM       devNum;
    GT_U32              tcQueue;
    GT_U32              xoffThreshold;
    GT_U32              dropThreshold;
    GT_U32              xonThreshold;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_SW_DEV_NUM)inArgs[0];
    tcQueue = (GT_U32)inArgs[1];
    xoffThreshold = (GT_U32)inArgs[2];
    dropThreshold = (GT_U32)inArgs[3];
    xonThreshold = (GT_U32)inArgs[4];


    /* call cpss api function */
    result = cpssPxPortPfcGlobalQueueConfigSet(devNum, tcQueue,
                                        xoffThreshold, dropThreshold, xonThreshold);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssPxPortPfcGlobalQueueConfigGet function
* @endinternal
*
* @brief   Gets PFC profile configurations for given tc queue.
*
* @note   APPLICABLE DEVICES:      Pipe
* @note   NOT APPLICABLE DEVICES:  None
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_BAD_PARAM             - on wrong device number, profile index
*                                       or traffic class queue
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note All thresholds are set in buffers or packets.
*       See cpssPxPortPfcCountingModeSet.
*
*/
static CMD_STATUS wrCpssPxPortPfcGlobalQueueConfigGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_SW_DEV_NUM devNum;
    GT_U32 tcQueue;
    GT_U32 xoffThreshold;
    GT_U32 dropThreshold;
    GT_U32 xonThreshold;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_SW_DEV_NUM)inArgs[0];
    tcQueue = (GT_U32)inArgs[1];

    /* call cpss api function */
    result = cpssPxPortPfcGlobalQueueConfigGet(devNum, tcQueue,
                                        &xoffThreshold, &dropThreshold, &xonThreshold);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d%d%d", xoffThreshold, dropThreshold, xonThreshold);

    return CMD_OK;
}
/**
* @internal wrCpssPxPortPfcSourcePortToPfcCounterSet function
* @endinternal
*
* @brief   Set PFC mapping of a physical port to a PFC counter.
*
* @note   APPLICABLE DEVICES:      Pipe
* @note   NOT APPLICABLE DEVICES:  None
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device
* @retval GT_OUT_OF_RANGE          - on wrong pfcCounterNum
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
CMD_STATUS wrCpssPxPortPfcSourcePortToPfcCounterSet
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
    GT_U32                          pfcCounterNum;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum                                  = (GT_SW_DEV_NUM)inArgs[0];
    portNum                                 = (GT_PHYSICAL_PORT_NUM)inArgs[1];
    pfcCounterNum                           = (GT_U32)inArgs[2];

    /* call cpss api function */
    result = cpssPxPortPfcSourcePortToPfcCounterSet(devNum, portNum, pfcCounterNum);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}
/**
* @internal wrCpssPxPortPfcSourcePortToPfcCounterGet function
* @endinternal
*
* @brief   Get PFC mapping of a physical port to a PFC counter.
*
* @note   APPLICABLE DEVICES:      Pipe
* @note   NOT APPLICABLE DEVICES:  None
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
CMD_STATUS wrCpssPxPortPfcSourcePortToPfcCounterGet
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
    GT_U32                          pfcCounterNum;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum                                  = (GT_SW_DEV_NUM)inArgs[0];
    portNum                                 = (GT_PHYSICAL_PORT_NUM)inArgs[1];

    /* call cpss api function */
    result = cpssPxPortPfcSourcePortToPfcCounterGet(devNum, portNum, &pfcCounterNum);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", pfcCounterNum);

   return CMD_OK;
}

/**
* @internal wrCpssPxPortPfcQueueCounterGet function
* @endinternal
*
* @brief   Get the number buffers/packets per proirity queue.
*
* @note   APPLICABLE DEVICES:      Pipe
* @note   NOT APPLICABLE DEVICES:  None
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong devNum, tcQueue
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - on NULL pointer
*
* @note Buffer or packet counting mode is set by cpssPxPortPfcCountingModeSet.
*
*/
CMD_STATUS wrCpssPxPortPfcQueueCounterGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                       result;
    GT_SW_DEV_NUM                   devNum;
    GT_U32                          tcQueue;
    GT_U32                          pfcCounterNum;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum                                  = (GT_SW_DEV_NUM)inArgs[0];
    tcQueue                                 = (GT_U32)inArgs[1];

    /* call cpss api function */
    result = cpssPxPortPfcQueueCounterGet(devNum, tcQueue, &pfcCounterNum);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", pfcCounterNum);

   return CMD_OK;
}

/**
* @internal wrCpssPxPortPfcCounterGet function
* @endinternal
*
* @brief   Get PFC counter value per PFC counter and traffic class.
*
* @note   APPLICABLE DEVICES:      Pipe
* @note   NOT APPLICABLE DEVICES:  None
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device
* @retval GT_OUT_OF_RANGE          - on wrong pfcCounterNum
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - on NULL pointer
*/
CMD_STATUS wrCpssPxPortPfcCounterGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                       result;
    GT_SW_DEV_NUM                   devNum;
    GT_U32                          tcQueue;
    GT_U32                          pfcCounterNum;
    GT_U32                          pfcCounterValue;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum                                  = (GT_SW_DEV_NUM)inArgs[0];
    tcQueue                                 = (GT_U32)inArgs[1];
    pfcCounterNum                           = (GT_U32)inArgs[2];

    /* call cpss api function */
    result = cpssPxPortPfcCounterGet(devNum, tcQueue, pfcCounterNum, &pfcCounterValue);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", pfcCounterValue);

   return CMD_OK;
}


/**
* @internal wrCpssPxPortPfcTimerMapEnableSet function
* @endinternal
*
* @brief   Enables mapping of PFC timer to priority queue for given scheduler profile.
*
* @note   APPLICABLE DEVICES:      Pipe
* @note   NOT APPLICABLE DEVICES:  None
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device number or profile set
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note To bind port to scheduler profile use:
*       cpssPxPortTxBindPortToSchedulerProfileSet.
*
*/
CMD_STATUS wrCpssPxPortPfcTimerMapEnableSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                                   result;
    GT_SW_DEV_NUM                               devNum;
    CPSS_PX_PORT_TX_SCHEDULER_PROFILE_SET_ENT   profileSet;
    GT_BOOL                                     enable;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum                                   = (GT_SW_DEV_NUM)inArgs[0];
    profileSet                               = (CPSS_PX_PORT_TX_SCHEDULER_PROFILE_SET_ENT)inArgs[1];
    enable                                   = (GT_BOOL)inArgs[2];

    /* call cpss api function */
    result = cpssPxPortPfcTimerMapEnableSet(devNum, profileSet, enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssPxPortPfcTimerMapEnableGet function
* @endinternal
*
* @brief   Get the status of PFS timer to priority queue mapping for given
*         scheduler profile.
*
* @note   APPLICABLE DEVICES:      Pipe
* @note   NOT APPLICABLE DEVICES:  None
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_PARAM             - on wrong device number or profile set
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
CMD_STATUS wrCpssPxPortPfcTimerMapEnableGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                                   result;
    GT_SW_DEV_NUM                               devNum;
    CPSS_PX_PORT_TX_SCHEDULER_PROFILE_SET_ENT   profileSet;
    GT_BOOL                                     enable;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum                                  = (GT_SW_DEV_NUM)inArgs[0];
    profileSet                              = (CPSS_PX_PORT_TX_SCHEDULER_PROFILE_SET_ENT)inArgs[1];

    /* call cpss api function */
    result = cpssPxPortPfcTimerMapEnableGet(devNum, profileSet, &enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", enable);

   return CMD_OK;
}
/**
* @internal wrCpssPxPortPfcTimerToQueueMapSet function
* @endinternal
*
* @brief   Sets PFC timer to priority queue map.
*
* @note   APPLICABLE DEVICES:      Pipe
* @note   NOT APPLICABLE DEVICES:  None
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device number or PFC timer
* @retval GT_OUT_OF_RANGE          - on out of traffic class queue
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
CMD_STATUS wrCpssPxPortPfcTimerToQueueMapSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                                   result;
    GT_SW_DEV_NUM                               devNum;
    GT_U32                                      pfcTimer;
    GT_U32                                      tcQueue;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum                                   = (GT_SW_DEV_NUM)inArgs[0];
    pfcTimer                                 = (GT_U32)inArgs[1];
    tcQueue                                  = (GT_U32)inArgs[2];

    /* call cpss api function */
    result = cpssPxPortPfcTimerToQueueMapSet(devNum, pfcTimer, tcQueue);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssPxPortPfcTimerToQueueMapGet function
* @endinternal
*
* @brief   Gets PFC timer to priority queue map.
*
* @note   APPLICABLE DEVICES:      Pipe
* @note   NOT APPLICABLE DEVICES:  None
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device number or PFC timer
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
CMD_STATUS wrCpssPxPortPfcTimerToQueueMapGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                                   result;
    GT_SW_DEV_NUM                               devNum;
    GT_U32                                      pfcTimer;
    GT_U32                                      tcQueue;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum                                  = (GT_SW_DEV_NUM)inArgs[0];
    pfcTimer                                = (GT_U32)inArgs[1];

    /* call cpss api function */
    result = cpssPxPortPfcTimerToQueueMapGet(devNum, pfcTimer, &tcQueue);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", tcQueue);

   return CMD_OK;
}


/**
* @internal wrCpssPxPortPfcShaperToPortRateRatioSet function
* @endinternal
*
* @brief   Sets shaper rate to port speed ratio on given scheduler profile
*         and traffic class queue.
*
* @note   APPLICABLE DEVICES:      Pipe
* @note   NOT APPLICABLE DEVICES:  None
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device number ,profile set
*                                       or traffic class queue
* @retval GT_OUT_OF_RANGE          - on out of range shaper rate to port speed ratio
* @retval GT_HW_ERROR              - on hardware error
*
* @note To bind port to scheduler profile use:
*       cpssPxPortTxBindPortToSchedulerProfileSet.
*
*/
CMD_STATUS wrCpssPxPortPfcShaperToPortRateRatioSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                                   result;
    GT_SW_DEV_NUM                               devNum;
    CPSS_PX_PORT_TX_SCHEDULER_PROFILE_SET_ENT   profileSet;
    GT_U32                                      tcQueue;
    GT_U32                                      shaperToPortRateRatio;
    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum                                   = (GT_SW_DEV_NUM)inArgs[0];
    profileSet                               = (CPSS_PX_PORT_TX_SCHEDULER_PROFILE_SET_ENT)inArgs[1];
    tcQueue                                  = (GT_U32)inArgs[2];
    shaperToPortRateRatio                    = (GT_U32)inArgs[3];

    /* call cpss api function */
    result = cpssPxPortPfcShaperToPortRateRatioSet(devNum, profileSet, tcQueue, shaperToPortRateRatio);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}


/**
* @internal wrCpssPxPortPfcShaperToPortRateRatioGet function
* @endinternal
*
* @brief   Gets shaper rate to port speed ratio on given scheduler profile
*         and traffic class queue.
*
* @note   APPLICABLE DEVICES:      Pipe
* @note   NOT APPLICABLE DEVICES:  None
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device number ,profile set
*                                       or traffic class queue
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - on NULL pointer
*/
CMD_STATUS wrCpssPxPortPfcShaperToPortRateRatioGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                                   result;
    GT_SW_DEV_NUM                               devNum;
    CPSS_PX_PORT_TX_SCHEDULER_PROFILE_SET_ENT   profileSet;
    GT_U32                                      tcQueue;
    GT_U32                                      shaperToPortRateRatio;
    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum                                   = (GT_SW_DEV_NUM)inArgs[0];
    profileSet                               = (CPSS_PX_PORT_TX_SCHEDULER_PROFILE_SET_ENT)inArgs[1];
    tcQueue                                  = (GT_U32)inArgs[2];

    /* call cpss api function */
    result = cpssPxPortPfcShaperToPortRateRatioGet(devNum, profileSet, tcQueue, &shaperToPortRateRatio);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", shaperToPortRateRatio);

    return CMD_OK;
}
/**
* @internal wrCpssPxPortPfcForwardEnableSet function
* @endinternal
*
* @brief   Enable/disable forwarding of PFC frames to the ingress
*         pipeline of a specified port.
*
* @note   APPLICABLE DEVICES:      Pipe
* @note   NOT APPLICABLE DEVICES:  None
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device number or profile set
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note A packet is considered as a PFC frame if all of the following are true:
*       - Packet’s Length/EtherType field is 88-08
*       - Packet’s OpCode field is 01-01
*       - Packet’s MAC DA is 01-80-C2-00-00-01 or the port’s configured MAC Address
*
*/
CMD_STATUS wrCpssPxPortPfcForwardEnableSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                                   result;
    GT_SW_DEV_NUM                               devNum;
    GT_PHYSICAL_PORT_NUM                        portNum;
    GT_BOOL                                     enable;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum                                   = (GT_SW_DEV_NUM)inArgs[0];
    portNum                                  = (GT_PHYSICAL_PORT_NUM)inArgs[1];
    enable                                   = (GT_BOOL)inArgs[2];

    /* call cpss api function */
    result = cpssPxPortPfcForwardEnableSet(devNum, portNum, enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}



/**
* @internal wrCpssPxPortPfcForwardEnableGet function
* @endinternal
*
* @brief   Get status of PFC frames forwarding
*
* @note   APPLICABLE DEVICES:      Pipe
* @note   NOT APPLICABLE DEVICES:  None
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device number or profile set
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note A packet is considered as a PFC frame if all of the following are true:
*       - Packet’s Length/EtherType field is 88-08
*       - Packet’s OpCode field is 01-01
*       - Packet’s MAC DA is 01-80-C2-00-00-01 or the port’s configured MAC Address
*
*/
CMD_STATUS wrCpssPxPortPfcForwardEnableGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                                   result;
    GT_SW_DEV_NUM                               devNum;
    GT_PHYSICAL_PORT_NUM                        portNum;
    GT_BOOL                                     enable;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum                                  = (GT_SW_DEV_NUM)inArgs[0];
    portNum                                 = (GT_PHYSICAL_PORT_NUM)inArgs[1];

    /* call cpss api function */
    result = cpssPxPortPfcForwardEnableGet(devNum, portNum, &enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", enable);

   return CMD_OK;
}
/**
* @internal wrCpssPxPortPfcPacketClassificationEnableSet function
* @endinternal
*
* @brief   Enable/Disable the ingress control pipe to parse and terminate received
*         flow control frames (PRC/802.3x)
*
* @note   APPLICABLE DEVICES:      Pipe
* @note   NOT APPLICABLE DEVICES:  None
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - on NULL pointer
*
* @note If the configuration is disabled, flow control frames are processed by
*       the control pipe as regular data frames (that is they are subject to
*       learning, forwarding, filtering and mirroring) but they are not
*       terminated and the PFC timers are not extracted.
*
*/
CMD_STATUS wrCpssPxPortPfcPacketClassificationEnableSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                                   result;
    GT_SW_DEV_NUM                               devNum;
    CPSS_PX_PACKET_TYPE                         packetType;
    GT_U32                                       udbpIndex;
    GT_BOOL                                     enable;
    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum                                   = (GT_SW_DEV_NUM)inArgs[0];
    packetType                               = (CPSS_PX_PACKET_TYPE)inArgs[1];
    udbpIndex                                = (GT_U32)inArgs[2];
    enable                                   = (GT_BOOL)inArgs[3];

    /* call cpss api function */
    result = cpssPxPortPfcPacketClassificationEnableSet(devNum, packetType, udbpIndex, enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssPxPortPfcXonMessageFilterEnableSet function
* @endinternal
*
* @brief   Enable/Disable PFC (Priority Flow Control) filtering of
*         continuous flow control XON messages to priority shaper.
*
* @note   APPLICABLE DEVICES:      Pipe
* @note   NOT APPLICABLE DEVICES:  None
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device number or enable option
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Enable this feature in a system with periodic flow control XON messages.
*
*/
CMD_STATUS wrCpssPxPortPfcXonMessageFilterEnableSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                       result;
    GT_SW_DEV_NUM                   devNum;
    GT_BOOL                         enable;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum                                  = (GT_SW_DEV_NUM)inArgs[0];
    enable                                  = (GT_BOOL)inArgs[1];

    /* call cpss api function */
    result = cpssPxPortPfcXonMessageFilterEnableSet(devNum, enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}


/**
* @internal wrCpssPxPortPfcXonMessageFilterEnableGet function
* @endinternal
*
* @brief   Gets current status of the filtering of
*         continuous flow control XON messages to priority shaper.
*
* @note   APPLICABLE DEVICES:      Pipe
* @note   NOT APPLICABLE DEVICES:  None
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
CMD_STATUS wrCpssPxPortPfcXonMessageFilterEnableGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                       result;
    GT_SW_DEV_NUM                   devNum;
    GT_BOOL                         enable;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum                                  = (GT_SW_DEV_NUM)inArgs[0];

    /* call cpss api function */
    result = cpssPxPortPfcXonMessageFilterEnableGet(devNum, &enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", enable);

    return CMD_OK;
}




/**** database initialization **************************************/

static CMD_COMMAND dbCommands[] =
{
    /* commandName, funcReference, funcArgs, funcFields */
    { "cpssPxPortPfcEnableSet ",
        &wrCpssPxPfcEnableSet,
        2, 0 }
   ,{ "cpssPxPortPfcEnableGet",
        &wrCpssPxPfcEnableGet,
        1, 0 }
   ,{ "cpssPxPortPfcProfileIndexSet",
        &wrCpssPxPortPfcProfileIndexSet,
        3,  0 }
   ,{ "cpssPxPortPfcProfileIndexGet",
        &wrCpssPxPortPfcProfileIndexGet,
        2,  0 }
   ,{ "cpssPxPortPfcDbaBuffersSet ",
        &wrCpssPxPortPfcDbaBuffersSet,
        2, 0 }
   ,{ "cpssPxPortPfcDbaBuffersGet",
        &wrCpssPxPortPfcDbaBuffersGet,
        1, 0 }
   ,{ "cpssPxPortPfcProfileQueueConfigSet",
        &wrCpssPxPortPfcProfileQueueConfigSet,
        3, 4}
   ,{ "cpssPxPortPfcProfileQueueConfigGetFirst",
        &wrCpssPxPortPfcProfileQueueConfigGetFirst,
        3, 0}
   ,{ "cpssPxPortPfcProfileQueueConfigGetNext",
        &wrCpssPxPortPfcProfileQueueConfigGetNext,
        3, 0}
   ,{ "cpssPxPortPfcCountingModeSet",
        &wrCpssPxPortPfcCountingModeSet,
        2, 0}
   ,{ "cpssPxPortPfcCountingModeGet",
        &wrCpssPxPortPfcCountingModeGet,
        1, 0}
   ,{ "cpssPxPortPfcGlobalDropEnableSet",
        &wrCpssPxPortPfcGlobalDropEnableSet,
        2, 0}
   ,{ "cpssPxPortPfcGlobalDropEnableGet",
        &wrCpssPxPortPfcGlobalDropEnableGet,
        1, 0}
   ,{ "cpssPxPortPfcTimerMapEnableSet",
        &wrCpssPxPortPfcTimerMapEnableSet,
        3, 0}
   ,{ "cpssPxPortPfcTimerMapEnableGet",
        &wrCpssPxPortPfcTimerMapEnableGet,
        2, 0}
   ,{ "cpssPxPortPfcTimerToQueueMapSet",
        &wrCpssPxPortPfcTimerToQueueMapSet,
        3, 0}
   ,{ "cpssPxPortPfcTimerToQueueMapGet",
        &wrCpssPxPortPfcTimerToQueueMapGet,
        2, 0}
   ,{ "cpssPxPortPfcSourcePortToPfcCounterSet",
        &wrCpssPxPortPfcSourcePortToPfcCounterSet,
        3,  0 }
   ,{ "cpssPxPortPfcSourcePortToPfcCounterGet",
        &wrCpssPxPortPfcSourcePortToPfcCounterGet,
        2,  0 }
   ,{ "cpssPxPortPfcQueueCounterGet",
        &wrCpssPxPortPfcQueueCounterGet,
        2,  0 }
   ,{ "cpssPxPortPfcCounterGet",
        &wrCpssPxPortPfcCounterGet,
        3,  0 }
   ,{ "cpssPxPortPfcGlobalQueueConfigSet",
        &wrCpssPxPortPfcGlobalQueueConfigSet,
        5, 0}
   ,{ "cpssPxPortPfcGlobalQueueConfigGet",
        &wrCpssPxPortPfcGlobalQueueConfigGet,
        2, 0}
   ,{ "cpssPxPortPfcShaperToPortRateRatioSet",
        &wrCpssPxPortPfcShaperToPortRateRatioSet,
        4, 0}
   ,{ "cpssPxPortPfcShaperToPortRateRatioGet",
        &wrCpssPxPortPfcShaperToPortRateRatioGet,
        3, 0}
   ,{ "cpssPxPortPfcForwardEnableSet",
        &wrCpssPxPortPfcForwardEnableSet,
        3, 0}
   ,{ "cpssPxPortPfcForwardEnableGet",
        &wrCpssPxPortPfcForwardEnableGet,
        2, 0}
   ,{ "cpssPxPortPfcPacketClassificationEnableSet",
        &wrCpssPxPortPfcPacketClassificationEnableSet,
        4, 0}
   ,{ "cpssPxPortPfcXonMessageFilterEnableSet ",
        &wrCpssPxPortPfcXonMessageFilterEnableSet,
        2, 0 }
   ,{ "cpssPxPortPfcXonMessageFilterEnableGet",
        &wrCpssPxPortPfcXonMessageFilterEnableGet,
        1, 0 }

};

#define numCommands (sizeof(dbCommands) / sizeof(CMD_COMMAND))

/**
* @internal cmdLibInitCpssPxPfc function
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
GT_STATUS cmdLibInitCpssPxPfc
(
    GT_VOID
)
{
    return cmdInitLibrary(dbCommands, numCommands);
}






