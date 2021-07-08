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
* @file wrapCpssPxPortTxTailDrop.c
*
* @brief Wrapper functions for
* cpss/px/port/cpssPxPortTxTailDrop.h API's
*
* @version   1
********************************************************************************
*/

/* Common galtis includes */
#include <galtisAgent/wrapUtil/cmdCpssPresteraUtils.h>
#include <cmdShell/cmdDb/cmdBase.h>
#include <cmdShell/common/cmdWrapUtils.h>
#include <cpss/px/config/private/prvCpssPxInfo.h>

/* Feature specific includes. */
#include <cpss/px/port/cpssPxPortTxTailDrop.h>


/**
* @internal wrCpssPxPortTxTailDropProfileIdSet function
* @endinternal
*
* @brief   Bind a Physical Port to a specific Drop Profile Set.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
*/
CMD_STATUS wrCpssPxPortTxTailDropProfileIdSet
(
    IN  GT_UINTPTR  inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR  inFields[CMD_MAX_FIELDS],
    IN  GT_32       numFields,
    OUT GT_8        outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS               result;
    GT_SW_DEV_NUM           devNum;
    GT_PHYSICAL_PORT_NUM    portNum;
    CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_SET_ENT   profileSet;

    /* check for valid arguments */
    if ((!inArgs) || (!outArgs))
    {
        return CMD_AGENT_ERROR;
    }

    /* map input arguments to locals */
    devNum      = (GT_SW_DEV_NUM) inArgs[0];
    portNum     = (GT_PHYSICAL_PORT_NUM) inArgs[1];
    profileSet  = (CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_SET_ENT) inArgs[2];

    /* override device and port */
    CONVERT_SW_DEV_PHYSICAL_PORT_MAC(devNum, portNum);

    /* call cpss api function */
    result = cpssPxPortTxTailDropProfileIdSet(devNum, portNum, profileSet);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssPxPortTxTailDropProfileIdGet function
* @endinternal
*
* @brief   Get Drop Profile Set according to a Physical Port.
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
CMD_STATUS wrCpssPxPortTxTailDropProfileIdGet
(
    IN  GT_UINTPTR  inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR  inFields[CMD_MAX_FIELDS],
    IN  GT_32       numFields,
    OUT GT_8        outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS               result;
    GT_SW_DEV_NUM           devNum;
    GT_PHYSICAL_PORT_NUM    portNum;
    CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_SET_ENT   profileSet;

    /* check for valid arguments */
    if ((!inArgs) || (!outArgs))
    {
        return CMD_AGENT_ERROR;
    }

    /* map input arguments to locals */
    devNum      = (GT_SW_DEV_NUM) inArgs[0];
    portNum     = (GT_PHYSICAL_PORT_NUM) inArgs[1];

    /* override device and port */
    CONVERT_SW_DEV_PHYSICAL_PORT_MAC(devNum, portNum);

    /* call cpss api function */
    result = cpssPxPortTxTailDropProfileIdGet(devNum, portNum, &profileSet);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", profileSet);

    return CMD_OK;
}

/**
* @internal wrCpssPxPortTxTailDropUcEnableSet function
* @endinternal
*
* @brief   Enable/Disable tail-dropping for all packets based on the profile limits.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
CMD_STATUS wrCpssPxPortTxTailDropUcEnableSet
(
    IN  GT_UINTPTR  inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR  inFields[CMD_MAX_FIELDS],
    IN  GT_32       numFields,
    OUT GT_8        outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS       result;
    GT_SW_DEV_NUM   devNum;
    GT_BOOL         enable;

    /* check for valid arguments */
    if ((!inArgs) || (!outArgs))
    {
        return CMD_AGENT_ERROR;
    }

    /* map input arguments to locals */
    devNum  = (GT_SW_DEV_NUM) inArgs[0];
    enable  = (GT_BOOL) inArgs[1];

    /* call cpss api function */
    result = cpssPxPortTxTailDropUcEnableSet(devNum, enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssPxPortTxTailDropUcEnableGet function
* @endinternal
*
* @brief   Get enable/disable tail-dropping for all packets based on the profile
*         limits.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
CMD_STATUS wrCpssPxPortTxTailDropUcEnableGet
(
    IN  GT_UINTPTR  inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR  inFields[CMD_MAX_FIELDS],
    IN  GT_32       numFields,
    OUT GT_8        outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS       result;
    GT_SW_DEV_NUM   devNum;
    GT_BOOL         enable;

    /* check for valid arguments */
    if ((!inArgs) || (!outArgs))
    {
        return CMD_AGENT_ERROR;
    }

    /* map input arguments to locals */
    devNum  = (GT_SW_DEV_NUM) inArgs[0];

    /* call cpss api function */
    result = cpssPxPortTxTailDropUcEnableGet(devNum, &enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", enable);

    return CMD_OK;
}

/**
* @internal wrCpssPxPortTxTailDropProfileSet function
* @endinternal
*
* @brief   Enables/Disables sharing of buffers and descriptors for all queues of an
*         egress port and set maximal port's limits of buffers and descriptors.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device number, profile set or
*                                       portAlpha
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_OUT_OF_RANGE          - on out of range value
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
CMD_STATUS wrCpssPxPortTxTailDropProfileSet
(
    IN  GT_UINTPTR  inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR  inFields[CMD_MAX_FIELDS],
    IN  GT_32       numFields,
    OUT GT_8        outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS       result;
    GT_SW_DEV_NUM   devNum;
    GT_U32          portMaxBuffLimit;
    GT_U32          portMaxDescLimit;
    CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_SET_ENT   profileSet;
    CPSS_PX_PORT_TX_TAIL_DROP_DBA_ALPHA_ENT     portAlpha;

    /* check for valid arguments */
    if ((!inArgs) || (!outArgs))
    {
        return CMD_AGENT_ERROR;
    }

    /* map input arguments to locals */
    devNum           = (GT_SW_DEV_NUM) inArgs[0];
    profileSet       = (CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_SET_ENT) inArgs[1];
    portMaxBuffLimit = (GT_U32) inArgs[2];
    portMaxDescLimit = (GT_U32) inArgs[3];
    portAlpha        = (CPSS_PX_PORT_TX_TAIL_DROP_DBA_ALPHA_ENT) inArgs[4];

    /* call cpss api function */
    result = cpssPxPortTxTailDropProfileSet(devNum, profileSet, portMaxBuffLimit,
                                            portMaxDescLimit, portAlpha);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssPxPortTxTailDropProfileGet function
* @endinternal
*
* @brief   Get sharing status of buffers and descriptors for all queues of an
*         egress port and get maximal port's limits of buffers and descriptors.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device number or profile set
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
CMD_STATUS wrCpssPxPortTxTailDropProfileGet
(
    IN  GT_UINTPTR  inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR  inFields[CMD_MAX_FIELDS],
    IN  GT_32       numFields,
    OUT GT_8        outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS       result;
    GT_SW_DEV_NUM   devNum;
    GT_U32          portMaxBuffLimit;
    GT_U32          portMaxDescLimit;
    CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_SET_ENT   profileSet;
    CPSS_PX_PORT_TX_TAIL_DROP_DBA_ALPHA_ENT     portAlpha;

    /* check for valid arguments */
    if ((!inArgs) || (!outArgs))
    {
        return CMD_AGENT_ERROR;
    }

    /* map input arguments to locals */
    devNum      = (GT_SW_DEV_NUM) inArgs[0];
    profileSet  = (CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_SET_ENT) inArgs[1];

    /* call cpss api function */
    result = cpssPxPortTxTailDropProfileGet(devNum, profileSet, &portMaxBuffLimit,
                                            &portMaxDescLimit, &portAlpha);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d%d%d", portMaxBuffLimit,
                                            portMaxDescLimit,
                                            portAlpha);

    return CMD_OK;
}

/**
* @internal wrCpssPxPortTxTailDropPacketModeLengthSet function
* @endinternal
*
* @brief   Configure the packet length used for Tail Drop in packet mode.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_OUT_OF_RANGE          - on length out of range
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The function should be called only when traffic disabled
*       Mode configuration for Pipe performed per profile.
*
*/
CMD_STATUS wrCpssPxPortTxTailDropPacketModeLengthSet
(
    IN  GT_UINTPTR  inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR  inFields[CMD_MAX_FIELDS],
    IN  GT_32       numFields,
    OUT GT_8        outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS       result;
    GT_SW_DEV_NUM   devNum;
    GT_U32          length;

    /* check for valid arguments */
    if ((!inArgs) || (!outArgs))
    {
        return CMD_AGENT_ERROR;
    }

    /* map input arguments to locals */
    devNum  = (GT_SW_DEV_NUM) inArgs[0];
    length  = (GT_U32) inArgs[1];

    /* call cpss api function */
    result = cpssPxPortTxTailDropPacketModeLengthSet(devNum, length);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssPxPortTxTailDropPacketModeLengthGet function
* @endinternal
*
* @brief   Get the packet length used for Tail Drop in packet mode.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
CMD_STATUS wrCpssPxPortTxTailDropPacketModeLengthGet
(
    IN  GT_UINTPTR  inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR  inFields[CMD_MAX_FIELDS],
    IN  GT_32       numFields,
    OUT GT_8        outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS       result;
    GT_SW_DEV_NUM   devNum;
    GT_U32          length;

    /* check for valid arguments */
    if ((!inArgs) || (!outArgs))
    {
        return CMD_AGENT_ERROR;
    }

    /* map input arguments to locals */
    devNum  = (GT_SW_DEV_NUM) inArgs[0];

    /* call cpss api function */
    result = cpssPxPortTxTailDropPacketModeLengthGet(devNum, &length);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", length);

    return CMD_OK;
}

/**
* @internal wrCpssPxPortTxTailDropProfileBufferConsumptionModeSet function
* @endinternal
*
* @brief   Sets the packet buffer consumption mode per Tail Drop profile.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device, profile or wrong mode
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
CMD_STATUS wrCpssPxPortTxTailDropProfileBufferConsumptionModeSet
(
    IN  GT_UINTPTR  inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR  inFields[CMD_MAX_FIELDS],
    IN  GT_32       numFields,
    OUT GT_8        outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS       result;
    GT_SW_DEV_NUM   devNum;
    CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_SET_ENT               profile;
    CPSS_PX_PORT_TX_TAIL_DROP_BUFFER_CONSUMPTION_MODE_ENT   mode;

    /* check for valid arguments */
    if ((!inArgs) || (!outArgs))
    {
        return CMD_AGENT_ERROR;
    }

    /* map input arguments to locals */
    devNum  = (GT_SW_DEV_NUM) inArgs[0];
    profile = (CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_SET_ENT) inArgs[1];
    mode    = (CPSS_PX_PORT_TX_TAIL_DROP_BUFFER_CONSUMPTION_MODE_ENT) inArgs[2];

    /* call cpss api function */
    result = cpssPxPortTxTailDropProfileBufferConsumptionModeSet(devNum, profile,
                                                                 mode);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssPxPortTxTailDropProfileBufferConsumptionModeGet function
* @endinternal
*
* @brief   Gets the packet buffer consumption mode per Tail Drop profile.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device, profile
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - one of the parameters in NULL pointer
*/
CMD_STATUS wrCpssPxPortTxTailDropProfileBufferConsumptionModeGet
(
    IN  GT_UINTPTR  inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR  inFields[CMD_MAX_FIELDS],
    IN  GT_32       numFields,
    OUT GT_8        outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS       result;
    GT_SW_DEV_NUM   devNum;
    CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_SET_ENT               profile;
    CPSS_PX_PORT_TX_TAIL_DROP_BUFFER_CONSUMPTION_MODE_ENT   mode;

    /* check for valid arguments */
    if ((!inArgs) || (!outArgs))
    {
        return CMD_AGENT_ERROR;
    }

    /* map input arguments to locals */
    devNum  = (GT_SW_DEV_NUM) inArgs[0];
    profile = (CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_SET_ENT) inArgs[1];

    /* call cpss api function */
    result = cpssPxPortTxTailDropProfileBufferConsumptionModeGet(devNum, profile,
                                                                 &mode);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", mode);

    return CMD_OK;
}

/**
* @internal wrCpssPxPortTxTailDropMaskSharedBuffEnableSet function
* @endinternal
*
* @brief   Enable/Disable shared buffer indication masking. When enabled, the tail
*         drop mechanism will ignore the buffer fill level indication.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
CMD_STATUS wrCpssPxPortTxTailDropMaskSharedBuffEnableSet
(
    IN  GT_UINTPTR  inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR  inFields[CMD_MAX_FIELDS],
    IN  GT_32       numFields,
    OUT GT_8        outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS       result;
    GT_SW_DEV_NUM   devNum;
    GT_BOOL         enable;

    /* check for valid arguments */
    if ((!inArgs) || (!outArgs))
    {
        return CMD_AGENT_ERROR;
    }

    /* map input arguments to locals */
    devNum  = (GT_SW_DEV_NUM) inArgs[0];
    enable  = (GT_BOOL) inArgs[1];

    /* call cpss api function */
    result = cpssPxPortTxTailDropMaskSharedBuffEnableSet(devNum, enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssPxPortTxTailDropMaskSharedBuffEnableGet function
* @endinternal
*
* @brief   Gets status of shared buffer indication masking. When enabled, the tail
*         drop mechanism will ignore the buffer fill level indication.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - one of the parameters in NULL pointer
*/
CMD_STATUS wrCpssPxPortTxTailDropMaskSharedBuffEnableGet
(
    IN  GT_UINTPTR  inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR  inFields[CMD_MAX_FIELDS],
    IN  GT_32       numFields,
    OUT GT_8        outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS       result;
    GT_SW_DEV_NUM   devNum;
    GT_BOOL         enable;

    /* check for valid arguments */
    if ((!inArgs) || (!outArgs))
    {
        return CMD_AGENT_ERROR;
    }

    /* map input arguments to locals */
    devNum  = (GT_SW_DEV_NUM) inArgs[0];

    /* call cpss api function */
    result = cpssPxPortTxTailDropMaskSharedBuffEnableGet(devNum, &enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", enable);

    return CMD_OK;
}

/**
* @internal wrCpssPxPortTxTailDropSharedBuffMaxLimitSet function
* @endinternal
*
* @brief   Set max shared buffer limit value.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_OUT_OF_RANGE          - on illegal maxSharedBufferLimit
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
CMD_STATUS wrCpssPxPortTxTailDropSharedBuffMaxLimitSet
(
    IN  GT_UINTPTR  inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR  inFields[CMD_MAX_FIELDS],
    IN  GT_32       numFields,
    OUT GT_8        outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS       result;
    GT_SW_DEV_NUM   devNum;
    GT_U32          maxSharedBufferLimit;

    /* check for valid arguments */
    if ((!inArgs) || (!outArgs))
    {
        return CMD_AGENT_ERROR;
    }

    /* map input arguments to locals */
    devNum               = (GT_SW_DEV_NUM) inArgs[0];
    maxSharedBufferLimit = (GT_U32) inArgs[1];

    /* call cpss api function */
    result = cpssPxPortTxTailDropSharedBuffMaxLimitSet(devNum, maxSharedBufferLimit);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssPxPortTxTailDropSharedBuffMaxLimitGet function
* @endinternal
*
* @brief   Get max shared buffer limit value.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - one of the parameters in NULL pointer
*/
CMD_STATUS wrCpssPxPortTxTailDropSharedBuffMaxLimitGet
(
    IN  GT_UINTPTR  inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR  inFields[CMD_MAX_FIELDS],
    IN  GT_32       numFields,
    OUT GT_8        outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS       result;
    GT_SW_DEV_NUM   devNum;
    GT_U32          maxSharedBufferLimit;

    /* check for valid arguments */
    if ((!inArgs) || (!outArgs))
    {
        return CMD_AGENT_ERROR;
    }

    /* map input arguments to locals */
    devNum = (GT_SW_DEV_NUM) inArgs[0];

    /* call cpss api function */
    result = cpssPxPortTxTailDropSharedBuffMaxLimitGet(devNum, &maxSharedBufferLimit);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", maxSharedBufferLimit);

    return CMD_OK;
}

/**
* @internal wrCpssPxPortTxTailDropProfileWeightedRandomTailDropEnableSet function
* @endinternal
*
* @brief   Enable/disable Weighted Random Tail Drop Threshold to overcome
*         synchronization.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on invalid input paramters value
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
CMD_STATUS wrCpssPxPortTxTailDropProfileWeightedRandomTailDropEnableSet
(
    IN  GT_UINTPTR  inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR  inFields[CMD_MAX_FIELDS],
    IN  GT_32       numFields,
    OUT GT_8        outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS       result;
    GT_SW_DEV_NUM   devNum;
    GT_U32          dp;
    GT_U32          tc;
    CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_SET_ENT   profile;
    CPSS_PX_PORT_TX_TAIL_DROP_WRTD_ENABLER_STC  enabler;

    /* check for valid arguments */
    if ((!inArgs) || (!outArgs))
    {
        return CMD_AGENT_ERROR;
    }

    /* map input arguments to locals */
    devNum  = (GT_SW_DEV_NUM) inArgs[0];
    profile = (CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_SET_ENT) inArgs[1];
    dp      = (GT_U32) inArgs[2];
    tc      = (GT_U32) inArgs[3];
    enabler.tcDpLimit         = (GT_BOOL) inArgs[4];
    enabler.portLimit         = (GT_BOOL) inArgs[5];
    enabler.tcLimit           = (GT_BOOL) inArgs[6];
    enabler.sharedPoolLimit   = (GT_BOOL) inArgs[7];

    /* call cpss api function */
    result = cpssPxPortTxTailDropProfileWeightedRandomTailDropEnableSet(devNum,
            profile, dp, tc, &enabler);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssPxPortTxTailDropProfileWeightedRandomTailDropEnableGet function
* @endinternal
*
* @brief   Get Random Tail drop Threshold status.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on invalid input paramters value
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
CMD_STATUS wrCpssPxPortTxTailDropProfileWeightedRandomTailDropEnableGet
(
    IN  GT_UINTPTR  inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR  inFields[CMD_MAX_FIELDS],
    IN  GT_32       numFields,
    OUT GT_8        outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS       result;
    GT_SW_DEV_NUM   devNum;
    GT_U32          dp;
    GT_U32          tc;
    CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_SET_ENT   profile;
    CPSS_PX_PORT_TX_TAIL_DROP_WRTD_ENABLER_STC  enabler;

    /* check for valid arguments */
    if ((!inArgs) || (!outArgs))
    {
        return CMD_AGENT_ERROR;
    }

    /* map input arguments to locals */
    devNum  = (GT_SW_DEV_NUM) inArgs[0];
    profile = (CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_SET_ENT) inArgs[1];
    dp      = (GT_U32) inArgs[2];
    tc      = (GT_U32) inArgs[3];

    /* call cpss api function */
    result = cpssPxPortTxTailDropProfileWeightedRandomTailDropEnableGet(devNum,
            profile, dp, tc, &enabler);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d%d%d%d", enabler.tcDpLimit,
                 enabler.portLimit, enabler.tcLimit, enabler.sharedPoolLimit);

    return CMD_OK;
}

/**
* @internal wrCpssPxPortTxTailDropWrtdMasksSet function
* @endinternal
*
* @brief   Sets Weighted Random Tail Drop (WRTD) masks.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_OUT_OF_RANGE          - on out of range value
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
CMD_STATUS wrCpssPxPortTxTailDropWrtdMasksSet
(
    IN  GT_UINTPTR  inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR  inFields[CMD_MAX_FIELDS],
    IN  GT_32       numFields,
    OUT GT_8        outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS       result;
    GT_SW_DEV_NUM   devNum;
    CPSS_PX_PORT_TX_TAIL_DROP_WRTD_MASK_LSB_STC maskLsb;

    /* check for valid arguments */
    if ((!inArgs) || (!outArgs))
    {
        return CMD_AGENT_ERROR;
    }

    /* map input arguments to locals */
    devNum       = (GT_SW_DEV_NUM) inArgs[0];
    maskLsb.tcDp = (GT_U32) inArgs[1];
    maskLsb.port = (GT_U32) inArgs[2];
    maskLsb.tc   = (GT_U32) inArgs[3];
    maskLsb.pool = (GT_U32) inArgs[4];

    /* call cpss api function */
    result = cpssPxPortTxTailDropWrtdMasksSet(devNum, &maskLsb);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssPxPortTxTailDropWrtdMasksGet function
* @endinternal
*
* @brief   Gets Weighted Random Tail Drop (WRTD) masks.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
CMD_STATUS wrCpssPxPortTxTailDropWrtdMasksGet
(
    IN  GT_UINTPTR  inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR  inFields[CMD_MAX_FIELDS],
    IN  GT_32       numFields,
    OUT GT_8        outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS       result;
    GT_SW_DEV_NUM   devNum;
    CPSS_PX_PORT_TX_TAIL_DROP_WRTD_MASK_LSB_STC maskLsb;

    /* check for valid arguments */
    if ((!inArgs) || (!outArgs))
    {
        return CMD_AGENT_ERROR;
    }

    /* map input arguments to locals */
    devNum = (GT_SW_DEV_NUM) inArgs[0];

    /* call cpss api function */
    result = cpssPxPortTxTailDropWrtdMasksGet(devNum, &maskLsb);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d%d%d%d", maskLsb.tcDp, maskLsb.port,
                 maskLsb.tc, maskLsb.pool);

    return CMD_OK;
}

/**
* @internal wrCpssPxPortTxTailDropProfileTcSharingSet function
* @endinternal
*
* @brief   Enable/Disable usage of the shared descriptors / buffer pool for
*         packets with the traffic class (tc) that are transmited via a port that
*         is associated with the Profile (profile). Sets the shared pool
*         associated for traffic class and profile.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device number, profile
* @retval GT_OUT_OF_RANGE          - on out of range value
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
CMD_STATUS wrCpssPxPortTxTailDropProfileTcSharingSet
(
    IN  GT_UINTPTR  inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR  inFields[CMD_MAX_FIELDS],
    IN  GT_32       numFields,
    OUT GT_8        outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS       result;
    GT_SW_DEV_NUM   devNum;
    GT_U32          tc;
    GT_U32          poolNum;
    CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_SET_ENT       profile;
    CPSS_PX_PORT_TX_TAIL_DROP_SHARED_DP_MODE_ENT    mode;

    /* check for valid arguments */
    if ((!inArgs) || (!outArgs))
    {
        return CMD_AGENT_ERROR;
    }

    /* map input arguments to locals */
    devNum  = (GT_SW_DEV_NUM) inArgs[0];
    profile = (CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_SET_ENT) inArgs[1];
    tc      = (GT_U32) inArgs[2];
    mode    = (CPSS_PX_PORT_TX_TAIL_DROP_SHARED_DP_MODE_ENT) inArgs[3];
    poolNum = (GT_U32) inArgs[4];

    /* call cpss api function */
    result = cpssPxPortTxTailDropProfileTcSharingSet(devNum, profile, tc,
                                                     mode, poolNum);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssPxPortTxTailDropProfileTcSharingGet function
* @endinternal
*
* @brief   Get usage of the shared descriptors / buffer pool status for packets
*         with the traffic class (tc) that are transmited via a port that is
*         associated with the Profile (profile). Get the shared pool associated
*         for traffic class and Profile.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device number, pfSet
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_OUT_OF_RANGE          - on out of range value
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
CMD_STATUS wrCpssPxPortTxTailDropProfileTcSharingGet
(
    IN  GT_UINTPTR  inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR  inFields[CMD_MAX_FIELDS],
    IN  GT_32       numFields,
    OUT GT_8        outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS       result;
    GT_SW_DEV_NUM   devNum;
    GT_U32          tc;
    GT_U32          poolNum;
    CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_SET_ENT       profile;
    CPSS_PX_PORT_TX_TAIL_DROP_SHARED_DP_MODE_ENT    mode;

    /* check for valid arguments */
    if ((!inArgs) || (!outArgs))
    {
        return CMD_AGENT_ERROR;
    }

    /* map input arguments to locals */
    devNum  = (GT_SW_DEV_NUM) inArgs[0];
    profile = (CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_SET_ENT) inArgs[1];
    tc      = (GT_U32) inArgs[2];

    /* call cpss api function */
    result = cpssPxPortTxTailDropProfileTcSharingGet(devNum, profile, tc,
                                                     &mode, &poolNum);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d%d", mode, poolNum);

    return CMD_OK;
}

/**
* @internal wrCpssPxPortTxTailDropProfileTcSet function
* @endinternal
*
* @brief   Set tail drop profiles limits for particular TC.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device number, profile set or
*                                       traffic class
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_OUT_OF_RANGE          - on out of range value
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
CMD_STATUS wrCpssPxPortTxTailDropProfileTcSet
(
    IN  GT_UINTPTR  inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR  inFields[CMD_MAX_FIELDS],
    IN  GT_32       numFields,
    OUT GT_8        outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS       result;
    GT_SW_DEV_NUM   devNum;
    GT_U32          tc;
    CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_SET_ENT   profile;
    CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_TC_STC    params;

    /* check for valid arguments */
    if ((!inArgs) || (!outArgs))
    {
        return CMD_AGENT_ERROR;
    }

    /* map input arguments to locals */
    devNum  = (GT_SW_DEV_NUM) inArgs[0];
    profile = (CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_SET_ENT) inArgs[1];
    tc      = (GT_U32) inArgs[2];
    params.dp0MaxBuffNum = (GT_U32) inArgs[3];
    params.dp1MaxBuffNum = (GT_U32) inArgs[4];
    params.dp2MaxBuffNum = (GT_U32) inArgs[5];
    params.dp0MaxDescNum = (GT_U32) inArgs[6];
    params.dp1MaxDescNum = (GT_U32) inArgs[7];
    params.dp2MaxDescNum = (GT_U32) inArgs[8];
    params.tcMaxBuffNum  = (GT_U32) inArgs[9];
    params.tcMaxDescNum  = (GT_U32) inArgs[10];
    params.dp0QueueAlpha = (CPSS_PX_PORT_TX_TAIL_DROP_DBA_ALPHA_ENT) inArgs[11];
    params.dp1QueueAlpha = (CPSS_PX_PORT_TX_TAIL_DROP_DBA_ALPHA_ENT) inArgs[12];
    params.dp2QueueAlpha = (CPSS_PX_PORT_TX_TAIL_DROP_DBA_ALPHA_ENT) inArgs[13];

    /* call cpss api function */
    result = cpssPxPortTxTailDropProfileTcSet(devNum, profile, tc, &params);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssPxPortTxTailDropProfileTcGet function
* @endinternal
*
* @brief   Get tail drop profiles limits for particular TC.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device number, profile set or
*                                       traffic class
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
CMD_STATUS wrCpssPxPortTxTailDropProfileTcGet
(
    IN  GT_UINTPTR  inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR  inFields[CMD_MAX_FIELDS],
    IN  GT_32       numFields,
    OUT GT_8        outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS       result;
    GT_SW_DEV_NUM   devNum;
    GT_U32          tc;
    CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_SET_ENT   profile;
    CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_TC_STC    params;

    /* check for valid arguments */
    if ((!inArgs) || (!outArgs))
    {
        return CMD_AGENT_ERROR;
    }

    /* map input arguments to locals */
    devNum  = (GT_SW_DEV_NUM) inArgs[0];
    profile = (CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_SET_ENT) inArgs[1];
    tc      = (GT_U32) inArgs[2];

    /* call cpss api function */
    result = cpssPxPortTxTailDropProfileTcGet(devNum, profile, tc, &params);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d%d%d%d%d%d%d%d%d%d%d",
                 params.dp0MaxBuffNum,
                 params.dp1MaxBuffNum,
                 params.dp2MaxBuffNum,
                 params.dp0MaxDescNum,
                 params.dp1MaxDescNum,
                 params.dp2MaxDescNum,
                 params.tcMaxBuffNum,
                 params.tcMaxDescNum,
                 params.dp0QueueAlpha,
                 params.dp1QueueAlpha,
                 params.dp2QueueAlpha);

    return CMD_OK;
}

/**
* @internal wrCpssPxPortTxTailDropTcBuffNumberGet function
* @endinternal
*
* @brief   Gets the current number of buffers allocated on specified port
*         for specified Traffic Class queues.
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
CMD_STATUS wrCpssPxPortTxTailDropTcBuffNumberGet
(
    IN  GT_UINTPTR  inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR  inFields[CMD_MAX_FIELDS],
    IN  GT_32       numFields,
    OUT GT_8        outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS       result;
    GT_SW_DEV_NUM   devNum;
    GT_U32          tc;
    GT_U32          numberOfBuffers;
    GT_PHYSICAL_PORT_NUM    portNum;

    /* check for valid arguments */
    if ((!inArgs) || (!outArgs))
    {
        return CMD_AGENT_ERROR;
    }

    /* map input arguments to locals */
    devNum  = (GT_SW_DEV_NUM) inArgs[0];
    portNum = (GT_PHYSICAL_PORT_NUM) inArgs[1];
    tc      = (GT_U32) inArgs[2];

    /* override device and port */
    CONVERT_SW_DEV_PHYSICAL_PORT_MAC(devNum, portNum);

    /* call cpss api function */
    result = cpssPxPortTxTailDropTcBuffNumberGet(devNum, portNum, tc, &numberOfBuffers);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", numberOfBuffers);

    return CMD_OK;
}

/**
* @internal wrCpssPxPortTxTailDropMcastPcktDescLimitSet function
* @endinternal
*
* @brief   Set maximal descriptors limits for multicast packets.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
CMD_STATUS wrCpssPxPortTxTailDropMcastPcktDescLimitSet
(
    IN  GT_UINTPTR  inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR  inFields[CMD_MAX_FIELDS],
    IN  GT_32       numFields,
    OUT GT_8        outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS       result;
    GT_SW_DEV_NUM   devNum;
    GT_U32          mcastMaxDescNum;

    /* check for valid arguments */
    if ((!inArgs) || (!outArgs))
    {
        return CMD_AGENT_ERROR;
    }

    /* map input arguments to locals */
    devNum          = (GT_SW_DEV_NUM) inArgs[0];
    mcastMaxDescNum = (GT_U32) inArgs[1];

    /* call cpss api function */
    result = cpssPxPortTxTailDropMcastPcktDescLimitSet(devNum, mcastMaxDescNum);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssPxPortTxTailDropMcastPcktDescLimitGet function
* @endinternal
*
* @brief   Get maximal descriptors limits for multicast packets.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
CMD_STATUS wrCpssPxPortTxTailDropMcastPcktDescLimitGet
(
    IN  GT_UINTPTR  inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR  inFields[CMD_MAX_FIELDS],
    IN  GT_32       numFields,
    OUT GT_8        outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS       result;
    GT_SW_DEV_NUM   devNum;
    GT_U32          mcastMaxDescNum;

    /* check for valid arguments */
    if ((!inArgs) || (!outArgs))
    {
        return CMD_AGENT_ERROR;
    }

    /* map input arguments to locals */
    devNum = (GT_SW_DEV_NUM) inArgs[0];

    /* call cpss api function */
    result = cpssPxPortTxTailDropMcastPcktDescLimitGet(devNum, &mcastMaxDescNum);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", mcastMaxDescNum);

    return CMD_OK;
}

/**
* @internal wrCpssPxPortTxTailDropMcastBuffersLimitSet function
* @endinternal
*
* @brief   Set maximal buffers limits for multicast packets.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_OUT_OF_RANGE          - on out of range mcastMaxBuffNum
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
CMD_STATUS wrCpssPxPortTxTailDropMcastBuffersLimitSet
(
    IN  GT_UINTPTR  inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR  inFields[CMD_MAX_FIELDS],
    IN  GT_32       numFields,
    OUT GT_8        outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS       result;
    GT_SW_DEV_NUM   devNum;
    GT_U32          mcastMaxBuffNum;

    /* check for valid arguments */
    if ((!inArgs) || (!outArgs))
    {
        return CMD_AGENT_ERROR;
    }

    /* map input arguments to locals */
    devNum = (GT_SW_DEV_NUM) inArgs[0];
    mcastMaxBuffNum = (GT_U32) inArgs[1];

    /* call cpss api function */
    result = cpssPxPortTxTailDropMcastBuffersLimitSet(devNum, mcastMaxBuffNum);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssPxPortTxTailDropMcastBuffersLimitGet function
* @endinternal
*
* @brief   Get maximal buffers limits for multicast packets.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
CMD_STATUS wrCpssPxPortTxTailDropMcastBuffersLimitGet
(
    IN  GT_UINTPTR  inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR  inFields[CMD_MAX_FIELDS],
    IN  GT_32       numFields,
    OUT GT_8        outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS       result;
    GT_SW_DEV_NUM   devNum;
    GT_U32          mcastMaxBuffNum;

    /* check for valid arguments */
    if ((!inArgs) || (!outArgs))
    {
        return CMD_AGENT_ERROR;
    }

    /* map input arguments to locals */
    devNum = (GT_SW_DEV_NUM) inArgs[0];

    /* call cpss api function */
    result = cpssPxPortTxTailDropMcastBuffersLimitGet(devNum, &mcastMaxBuffNum);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", mcastMaxBuffNum);

    return CMD_OK;
}

/**
* @internal wrCpssPxPortTxTailDropMcastDescNumberGet function
* @endinternal
*
* @brief   Gets total number of multi-target descriptors allocated.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Multi-target descriptors are descriptors that are duplicated to
*       multiple egress targets, e.g. Multicast, Broadcast, or unknown Unicast
*       flooded packets.
*
*/
CMD_STATUS wrCpssPxPortTxTailDropMcastDescNumberGet
(
    IN  GT_UINTPTR  inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR  inFields[CMD_MAX_FIELDS],
    IN  GT_32       numFields,
    OUT GT_8        outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS       result;
    GT_SW_DEV_NUM   devNum;
    GT_U32          number;

    /* check for valid arguments */
    if ((!inArgs) || (!outArgs))
    {
        return CMD_AGENT_ERROR;
    }

    /* map input arguments to locals */
    devNum = (GT_SW_DEV_NUM) inArgs[0];

    /* call cpss api function */
    result = cpssPxPortTxTailDropMcastDescNumberGet(devNum, &number);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", number);

    return CMD_OK;
}

/**
* @internal wrCpssPxPortTxTailDropMcastBuffNumberGet function
* @endinternal
*
* @brief   Gets total number of multi-target buffers allocated (virtual buffers).
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The buffers associated with enqueued multi-target descriptors are
*       counted multiple times, once for each multi-target descriptor instance.
*       Multi-target descriptors are descriptors that are duplicated to
*       multiple egress targets, e.g. Multicast, Broadcast, or unknown Unicast
*       flooded packets.
*
*/
CMD_STATUS wrCpssPxPortTxTailDropMcastBuffNumberGet
(
    IN  GT_UINTPTR  inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR  inFields[CMD_MAX_FIELDS],
    IN  GT_32       numFields,
    OUT GT_8        outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS       result;
    GT_SW_DEV_NUM   devNum;
    GT_U32          number;

    /* check for valid arguments */
    if ((!inArgs) || (!outArgs))
    {
        return CMD_AGENT_ERROR;
    }

    /* map input arguments to locals */
    devNum = (GT_SW_DEV_NUM) inArgs[0];

    /* call cpss api function */
    result = cpssPxPortTxTailDropMcastBuffNumberGet(devNum, &number);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", number);

    return CMD_OK;
}

/**
* @internal wrCpssPxPortTxTailDropSharedPolicySet function
* @endinternal
*
* @brief   Sets shared pool allocation policy for enqueuing of packets.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device or wrong policy
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
CMD_STATUS wrCpssPxPortTxTailDropSharedPolicySet
(
    IN  GT_UINTPTR  inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR  inFields[CMD_MAX_FIELDS],
    IN  GT_32       numFields,
    OUT GT_8        outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS       result;
    GT_SW_DEV_NUM   devNum;
    CPSS_PX_PORT_TX_TAIL_DROP_SHARED_POLICY_ENT policy;

    /* check for valid arguments */
    if ((!inArgs) || (!outArgs))
    {
        return CMD_AGENT_ERROR;
    }

    /* map input arguments to locals */
    devNum = (GT_SW_DEV_NUM) inArgs[0];
    policy = (CPSS_PX_PORT_TX_TAIL_DROP_SHARED_POLICY_ENT) inArgs[1];

    /* call cpss api function */
    result = cpssPxPortTxTailDropSharedPolicySet(devNum, policy);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssPxPortTxTailDropSharedPolicyGet function
* @endinternal
*
* @brief   Gets shared pool allocation policy for enqueuing of packets.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_BAD_PARAM             - on wrong device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
CMD_STATUS wrCpssPxPortTxTailDropSharedPolicyGet
(
    IN  GT_UINTPTR  inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR  inFields[CMD_MAX_FIELDS],
    IN  GT_32       numFields,
    OUT GT_8        outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS       result;
    GT_SW_DEV_NUM   devNum;
    CPSS_PX_PORT_TX_TAIL_DROP_SHARED_POLICY_ENT policy;

    /* check for valid arguments */
    if ((!inArgs) || (!outArgs))
    {
        return CMD_AGENT_ERROR;
    }

    /* map input arguments to locals */
    devNum = (GT_SW_DEV_NUM) inArgs[0];

    /* call cpss api function */
    result = cpssPxPortTxTailDropSharedPolicyGet(devNum, &policy);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", policy);

    return CMD_OK;
}

/**
* @internal wrCpssPxPortTxTailDropSharedPoolLimitsSet function
* @endinternal
*
* @brief   Set maximal descriptors and buffers limits for shared pool.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_OUT_OF_RANGE          - on out of range value
*/
CMD_STATUS wrCpssPxPortTxTailDropSharedPoolLimitsSet
(
    IN  GT_UINTPTR  inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR  inFields[CMD_MAX_FIELDS],
    IN  GT_32       numFields,
    OUT GT_8        outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS       result;
    GT_SW_DEV_NUM   devNum;
    GT_U32          poolNum;
    GT_U32          maxBuffNum;
    GT_U32          maxDescNum;

    /* check for valid arguments */
    if ((!inArgs) || (!outArgs))
    {
        return CMD_AGENT_ERROR;
    }

    /* map input arguments to locals */
    devNum      = (GT_SW_DEV_NUM) inArgs[0];
    poolNum     = (GT_U32) inArgs[1];
    maxBuffNum  = (GT_U32) inArgs[2];
    maxDescNum  = (GT_U32) inArgs[3];

    /* call cpss api function */
    result = cpssPxPortTxTailDropSharedPoolLimitsSet(devNum, poolNum,
                                                     maxBuffNum, maxDescNum);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssPxPortTxTailDropSharedPoolLimitsGet function
* @endinternal
*
* @brief   Get maximal descriptors and buffers limits for shared pool.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
CMD_STATUS wrCpssPxPortTxTailDropSharedPoolLimitsGet
(
    IN  GT_UINTPTR  inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR  inFields[CMD_MAX_FIELDS],
    IN  GT_32       numFields,
    OUT GT_8        outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS       result;
    GT_SW_DEV_NUM   devNum;
    GT_U32          poolNum;
    GT_U32          maxBuffNum;
    GT_U32          maxDescNum;

    /* check for valid arguments */
    if ((!inArgs) || (!outArgs))
    {
        return CMD_AGENT_ERROR;
    }

    /* map input arguments to locals */
    devNum = (GT_SW_DEV_NUM) inArgs[0];
    poolNum = (GT_U32) inArgs[1];

    /* call cpss api function */
    result = cpssPxPortTxTailDropSharedPoolLimitsGet(devNum, poolNum,
                                                     &maxBuffNum, &maxDescNum);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d%d", maxBuffNum, maxDescNum);

    return CMD_OK;
}

/**
* @internal wrCpssPxPortTxTailDropSharedResourceDescNumberGet function
* @endinternal
*
* @brief   Gets total number of descriptors allocated from the shared descriptors
*         pool.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong poolNum or devNum
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
CMD_STATUS wrCpssPxPortTxTailDropSharedResourceDescNumberGet
(
    IN  GT_UINTPTR  inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR  inFields[CMD_MAX_FIELDS],
    IN  GT_32       numFields,
    OUT GT_8        outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS       result;
    GT_SW_DEV_NUM   devNum;
    GT_U32          tcQueue;
    GT_U32          number;

    /* check for valid arguments */
    if ((!inArgs) || (!outArgs))
    {
        return CMD_AGENT_ERROR;
    }

    /* map input arguments to locals */
    devNum  = (GT_SW_DEV_NUM) inArgs[0];
    tcQueue = (GT_U32) inArgs[1];

    /* call cpss api function */
    result = cpssPxPortTxTailDropSharedResourceDescNumberGet(devNum, tcQueue,
                                                             &number);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", number);

    return CMD_OK;
}

/**
* @internal wrCpssPxPortTxTailDropSharedResourceBuffNumberGet function
* @endinternal
*
* @brief   Gets the number of virtual buffers enqueued from the shared buffer pool.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong poolNum or devNum
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
CMD_STATUS wrCpssPxPortTxTailDropSharedResourceBuffNumberGet
(
    IN  GT_UINTPTR  inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR  inFields[CMD_MAX_FIELDS],
    IN  GT_32       numFields,
    OUT GT_8        outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS       result;
    GT_SW_DEV_NUM   devNum;
    GT_U32          tcQueue;
    GT_U32          number;

    /* check for valid arguments */
    if ((!inArgs) || (!outArgs))
    {
        return CMD_AGENT_ERROR;
    }

    /* map input arguments to locals */
    devNum = (GT_SW_DEV_NUM) inArgs[0];
    tcQueue = (GT_U32) inArgs[1];

    /* call cpss api function */
    result = cpssPxPortTxTailDropSharedResourceBuffNumberGet(devNum, tcQueue,
                                                             &number);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", number);

    return CMD_OK;
}

/**
* @internal wrCpssPxPortTxTailDropGlobalDescNumberGet function
* @endinternal
*
* @brief   Gets total number of descriptors allocated.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
CMD_STATUS wrCpssPxPortTxTailDropGlobalDescNumberGet
(
    IN  GT_UINTPTR  inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR  inFields[CMD_MAX_FIELDS],
    IN  GT_32       numFields,
    OUT GT_8        outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS       result;
    GT_SW_DEV_NUM   devNum;
    GT_U32          number;

    /* check for valid arguments */
    if ((!inArgs) || (!outArgs))
    {
        return CMD_AGENT_ERROR;
    }

    /* map input arguments to locals */
    devNum = (GT_SW_DEV_NUM) inArgs[0];

    /* call cpss api function */
    result = cpssPxPortTxTailDropGlobalDescNumberGet(devNum, &number);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", number);

    return CMD_OK;
}

/**
* @internal wrCpssPxPortTxTailDropGlobalBuffNumberGet function
* @endinternal
*
* @brief   Gets total number of virtual buffers enqueued.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The buffers associated with enqueued multi-target descriptors are
*       counted multiple times, once for each multi-target descriptor instance.
*
*/
CMD_STATUS wrCpssPxPortTxTailDropGlobalBuffNumberGet
(
    IN  GT_UINTPTR  inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR  inFields[CMD_MAX_FIELDS],
    IN  GT_32       numFields,
    OUT GT_8        outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS       result;
    GT_SW_DEV_NUM   devNum;
    GT_U32          number;

    /* check for valid arguments */
    if ((!inArgs) || (!outArgs))
    {
        return CMD_AGENT_ERROR;
    }

    /* map input arguments to locals */
    devNum = (GT_SW_DEV_NUM) inArgs[0];

    /* call cpss api function */
    result = cpssPxPortTxTailDropGlobalBuffNumberGet(devNum, &number);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", number);

    return CMD_OK;
}

/**
* @internal wrCpssPxPortTxTailDropDescNumberGet function
* @endinternal
*
* @brief   Gets the current number of descriptors allocated per specified port.
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
CMD_STATUS wrCpssPxPortTxTailDropDescNumberGet
(
    IN  GT_UINTPTR  inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR  inFields[CMD_MAX_FIELDS],
    IN  GT_32       numFields,
    OUT GT_8        outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS               result;
    GT_SW_DEV_NUM           devNum;
    GT_PHYSICAL_PORT_NUM    portNum;
    GT_U32                  number;

    /* check for valid arguments */
    if ((!inArgs) || (!outArgs))
    {
        return CMD_AGENT_ERROR;
    }

    /* map input arguments to locals */
    devNum  = (GT_SW_DEV_NUM) inArgs[0];
    portNum = (GT_PHYSICAL_PORT_NUM) inArgs[1];

    /* override device and port */
    CONVERT_SW_DEV_PHYSICAL_PORT_MAC(devNum, portNum);

    /* call cpss api function */
    result = cpssPxPortTxTailDropDescNumberGet(devNum, portNum, &number);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", number);

    return CMD_OK;
}

/**
* @internal wrCpssPxPortTxTailDropBuffNumberGet function
* @endinternal
*
* @brief   Gets the current number of buffers allocated per specified port.
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
CMD_STATUS wrCpssPxPortTxTailDropBuffNumberGet
(
    IN  GT_UINTPTR  inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR  inFields[CMD_MAX_FIELDS],
    IN  GT_32       numFields,
    OUT GT_8        outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS               result;
    GT_SW_DEV_NUM           devNum;
    GT_PHYSICAL_PORT_NUM    portNum;
    GT_U32                  number;

    /* check for valid arguments */
    if ((!inArgs) || (!outArgs))
    {
        return CMD_AGENT_ERROR;
    }

    /* map input arguments to locals */
    devNum  = (GT_SW_DEV_NUM) inArgs[0];
    portNum = (GT_PHYSICAL_PORT_NUM) inArgs[1];

    /* override device and port */
    CONVERT_SW_DEV_PHYSICAL_PORT_MAC(devNum, portNum);

    /* call cpss api function */
    result = cpssPxPortTxTailDropBuffNumberGet(devNum, portNum, &number);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", number);

    return CMD_OK;
}

/**
* @internal wrCpssPxPortTxTailDropDbaModeEnableSet function
* @endinternal
*
* @brief   Enable/disable Dynamic Buffers Allocation (DBA) mode of resource
*         management.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
*/
CMD_STATUS wrCpssPxPortTxTailDropDbaModeEnableSet
(
    IN  GT_UINTPTR  inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR  inFields[CMD_MAX_FIELDS],
    IN  GT_32       numFields,
    OUT GT_8        outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS       result;
    GT_SW_DEV_NUM   devNum;
    GT_BOOL         enable;

    /* check for valid arguments */
    if ((!inArgs) || (!outArgs))
    {
        return CMD_AGENT_ERROR;
    }

    /* map input arguments to locals */
    devNum = (GT_SW_DEV_NUM) inArgs[0];
    enable = (GT_BOOL) inArgs[1];

    /* call cpss api function */
    result = cpssPxPortTxTailDropDbaModeEnableSet(devNum, enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssPxPortTxTailDropDbaModeEnableGet function
* @endinternal
*
* @brief   Get state of Dynamic Buffers Allocation (DBA) mode of resource
*         management.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
*/
CMD_STATUS wrCpssPxPortTxTailDropDbaModeEnableGet
(
    IN  GT_UINTPTR  inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR  inFields[CMD_MAX_FIELDS],
    IN  GT_32       numFields,
    OUT GT_8        outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS       result;
    GT_SW_DEV_NUM   devNum;
    GT_BOOL         enable;

    /* check for valid arguments */
    if ((!inArgs) || (!outArgs))
    {
        return CMD_AGENT_ERROR;
    }

    /* map input arguments to locals */
    devNum = (GT_SW_DEV_NUM) inArgs[0];

    /* call cpss api function */
    result = cpssPxPortTxTailDropDbaModeEnableGet(devNum, &enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", enable);

    return CMD_OK;
}

/**
* @internal wrCpssPxPortTxTailDropDbaAvailableBuffSet function
* @endinternal
*
* @brief   Set amount of buffers available for dynamic allocation.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @note Valid only if DBA mode enabled.
*
*/
CMD_STATUS wrCpssPxPortTxTailDropDbaAvailableBuffSet
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
    result = cpssPxPortTxTailDropDbaAvailableBuffSet(devNum, availableBuff);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssPxPortTxTailDropDbaAvailableBuffGet function
* @endinternal
*
* @brief   Get amount of buffers available for dynamic allocation.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
*/
CMD_STATUS wrCpssPxPortTxTailDropDbaAvailableBuffGet
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
    result = cpssPxPortTxTailDropDbaAvailableBuffGet(devNum, &availableBuff);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", availableBuff);

    return CMD_OK;
}


/**** database initialization **************************************/
static CMD_COMMAND dbCommands[] =
{
    /*
        commandName,
        funcReference,
        funcArgs, funcFields
    */
    {
        "cpssPxPortTxTailDropProfileIdSet",
        &wrCpssPxPortTxTailDropProfileIdSet,
        3, 0
    },
    {
        "cpssPxPortTxTailDropProfileIdGet",
        &wrCpssPxPortTxTailDropProfileIdGet,
        2, 0
    },
    {
        "cpssPxPortTxTailDropUcEnableSet",
        &wrCpssPxPortTxTailDropUcEnableSet,
        2, 0
    },
    {
        "cpssPxPortTxTailDropUcEnableGet",
        &wrCpssPxPortTxTailDropUcEnableGet,
        1, 0
    },
    {
        "cpssPxPortTxTailDropProfileSet",
        &wrCpssPxPortTxTailDropProfileSet,
        5, 0
    },
    {
        "cpssPxPortTxTailDropProfileGet",
        &wrCpssPxPortTxTailDropProfileGet,
        2, 0
    },
    {
        "cpssPxPortTxTailDropPacketModeLengthSet",
        &wrCpssPxPortTxTailDropPacketModeLengthSet,
        2, 0
    },
    {
        "cpssPxPortTxTailDropPacketModeLengthGet",
        &wrCpssPxPortTxTailDropPacketModeLengthGet,
        1, 0
    },
    {
        "cpssPxPortTxTailDropProfileBufferConsumpModeSet",
        &wrCpssPxPortTxTailDropProfileBufferConsumptionModeSet,
        3, 0
    },
    {
        "cpssPxPortTxTailDropProfileBufferConsumpModeGet",
        &wrCpssPxPortTxTailDropProfileBufferConsumptionModeGet,
        2, 0
    },
    {
        "cpssPxPortTxTailDropMaskSharedBuffEnableSet",
        &wrCpssPxPortTxTailDropMaskSharedBuffEnableSet,
        2, 0
    },
    {
        "cpssPxPortTxTailDropMaskSharedBuffEnableGet",
        &wrCpssPxPortTxTailDropMaskSharedBuffEnableGet,
        1, 0
    },
    {
        "cpssPxPortTxTailDropSharedBuffMaxLimitSet",
        &wrCpssPxPortTxTailDropSharedBuffMaxLimitSet,
        2, 0
    },
    {
        "cpssPxPortTxTailDropSharedBuffMaxLimitGet",
        &wrCpssPxPortTxTailDropSharedBuffMaxLimitGet,
        1, 0
    },
    {
        "cpssPxPortTxTailDropProfileWrtdEnableSet",
        &wrCpssPxPortTxTailDropProfileWeightedRandomTailDropEnableSet,
        8, 0
    },
    {
        "cpssPxPortTxTailDropProfileWrtdEnableGet",
        &wrCpssPxPortTxTailDropProfileWeightedRandomTailDropEnableGet,
        4, 0
    },
    {
        "cpssPxPortTxTailDropWrtdMasksSet",
        &wrCpssPxPortTxTailDropWrtdMasksSet,
        5, 0
    },
    {
        "cpssPxPortTxTailDropWrtdMasksGet",
        &wrCpssPxPortTxTailDropWrtdMasksGet,
        1, 0
    },
    {
        "cpssPxPortTxTailDropProfileTcSharingSet",
        &wrCpssPxPortTxTailDropProfileTcSharingSet,
        5, 0
    },
    {
        "cpssPxPortTxTailDropProfileTcSharingGet",
        &wrCpssPxPortTxTailDropProfileTcSharingGet,
        3, 0
    },
    {
        "cpssPxPortTxTailDropProfileTcSet",
        &wrCpssPxPortTxTailDropProfileTcSet,
        14, 0
    },
    {
        "cpssPxPortTxTailDropProfileTcGet",
        &wrCpssPxPortTxTailDropProfileTcGet,
        3, 0
    },
    {
        "cpssPxPortTxTailDropTcBuffNumberGet",
        &wrCpssPxPortTxTailDropTcBuffNumberGet,
        3, 0
    },
    {
        "cpssPxPortTxTailDropMcastPcktDescLimitSet",
        &wrCpssPxPortTxTailDropMcastPcktDescLimitSet,
        2, 0
    },
    {
        "cpssPxPortTxTailDropMcastPcktDescLimitGet",
        &wrCpssPxPortTxTailDropMcastPcktDescLimitGet,
        1, 0
    },
    {
        "cpssPxPortTxTailDropMcastBuffersLimitSet",
        &wrCpssPxPortTxTailDropMcastBuffersLimitSet,
        2, 0
    },
    {
        "cpssPxPortTxTailDropMcastBuffersLimitGet",
        &wrCpssPxPortTxTailDropMcastBuffersLimitGet,
        1, 0
    },
    {
        "cpssPxPortTxTailDropMcastDescNumberGet",
        &wrCpssPxPortTxTailDropMcastDescNumberGet,
        1, 0
    },
    {
        "cpssPxPortTxTailDropMcastBuffNumberGet",
        &wrCpssPxPortTxTailDropMcastBuffNumberGet,
        1, 0
    },
    {
        "cpssPxPortTxTailDropSharedPolicySet",
        &wrCpssPxPortTxTailDropSharedPolicySet,
        2, 0
    },
    {
        "cpssPxPortTxTailDropSharedPolicyGet",
        &wrCpssPxPortTxTailDropSharedPolicyGet,
        1, 0
    },
    {
        "cpssPxPortTxTailDropSharedPoolLimitsSet",
        &wrCpssPxPortTxTailDropSharedPoolLimitsSet,
        4, 0
    },
    {
        "cpssPxPortTxTailDropSharedPoolLimitsGet",
        &wrCpssPxPortTxTailDropSharedPoolLimitsGet,
        2, 0
    },
    {
        "cpssPxPortTxTailDropSharedResourceDescNumberGet",
        &wrCpssPxPortTxTailDropSharedResourceDescNumberGet,
        2, 0
    },
    {
        "cpssPxPortTxTailDropSharedResourceBuffNumberGet",
        &wrCpssPxPortTxTailDropSharedResourceBuffNumberGet,
        2, 0
    },
    {
        "cpssPxPortTxTailDropGlobalDescNumberGet",
        &wrCpssPxPortTxTailDropGlobalDescNumberGet,
        1, 0
    },
    {
        "cpssPxPortTxTailDropGlobalBuffNumberGet",
        &wrCpssPxPortTxTailDropGlobalBuffNumberGet,
        1, 0
    },
    {
        "cpssPxPortTxTailDropDescNumberGet",
        &wrCpssPxPortTxTailDropDescNumberGet,
        2, 0
    },
    {
        "cpssPxPortTxTailDropBuffNumberGet",
        &wrCpssPxPortTxTailDropBuffNumberGet,
        2, 0
    },
    {
        "cpssPxPortTxTailDropDbaModeEnableSet",
        &wrCpssPxPortTxTailDropDbaModeEnableSet,
        2, 0
    },
    {
        "cpssPxPortTxTailDropDbaModeEnableGet",
        &wrCpssPxPortTxTailDropDbaModeEnableGet,
        1, 0
    },
    {
        "cpssPxPortTxTailDropDbaAvailableBuffSet",
        &wrCpssPxPortTxTailDropDbaAvailableBuffSet,
        2, 0
    },
    {
        "cpssPxPortTxTailDropDbaAvailableBuffGet",
        &wrCpssPxPortTxTailDropDbaAvailableBuffGet,
        1, 0
    }
};


#define  numCommands     (sizeof(dbCommands) / sizeof(CMD_COMMAND))


/**
* @internal cmdLibInitCpssPxPortTxTailDrop function
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
GT_STATUS cmdLibInitCpssPxPortTxTailDrop
(
    GT_VOID
)
{
    return cmdInitLibrary(dbCommands, numCommands);
}

