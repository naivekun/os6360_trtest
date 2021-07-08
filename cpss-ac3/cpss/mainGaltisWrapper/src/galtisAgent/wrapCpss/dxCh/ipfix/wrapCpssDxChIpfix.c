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
* @file wrapCpssDxChIpfix.c
*
* @brief Wrapper functions for IPFIX cpss.dxCh functions.
*
* @version   6
********************************************************************************
*/

/* Common galtis includes */
#include <galtisAgent/wrapUtil/cmdCpssPresteraUtils.h>
#include <cmdShell/cmdDb/cmdBase.h>
#include <cmdShell/common/cmdWrapUtils.h>

/* Feature specific includes */
#include <cpss/dxCh/dxChxGen/ipfix/cpssDxChIpfix.h>

/**
* @internal ipfixMultiPortGroupsBmpGet function
* @endinternal
*
* @brief   Get the portGroupsBmp for multi port groups device.
*         when 'enabled' --> wrappers will use the APIs
*         with portGroupsBmp parameter
* @param[in] devNum                   - device number
*
* @param[out] enablePtr                - (pointer to)enable / disable the use of APIs with portGroupsBmp parameter.
* @param[out] portGroupsBmpPtr         - (pointer to)port groups bmp , relevant only when enable = GT_TRUE
*                                       NONE
*/
static void ipfixMultiPortGroupsBmpGet
(
    IN   GT_U8               devNum,
    OUT  GT_BOOL             *enablePtr,
    OUT  GT_PORT_GROUPS_BMP  *portGroupsBmpPtr
)
{
    /* default */
    *enablePtr  = GT_FALSE;
    *portGroupsBmpPtr = CPSS_PORT_GROUP_UNAWARE_MODE_CNS;

    if (0 == PRV_CPSS_IS_DEV_EXISTS_MAC(devNum))
    {
        return;
    }

    utilMultiPortGroupsBmpGet(devNum, enablePtr, portGroupsBmpPtr);
}

/**
* @internal wrCpssDxChIpfixAgingEnableSet function
* @endinternal
*
* @brief   Enables or disabled the activation of aging for IPfix.
*
* @note   APPLICABLE DEVICES:      DxChXcat and above.
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum or stage.
* @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
*/
static CMD_STATUS wrCpssDxChIpfixAgingEnableSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8 devNum;
    CPSS_DXCH_POLICER_STAGE_TYPE_ENT stage;
    GT_BOOL enable;


    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */

    devNum = (GT_U8)inArgs[0];
    stage  = (CPSS_DXCH_POLICER_STAGE_TYPE_ENT)inArgs[1];
    enable = (GT_BOOL)inArgs[2];

    /* call cpss api function */
    result = cpssDxChIpfixAgingEnableSet(devNum, stage, enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxChIpfixAgingEnableGet function
* @endinternal
*
* @brief   Gets enabling status of aging for IPfix.
*
* @note   APPLICABLE DEVICES:      DxChXcat and above.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum or stage.
* @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
*/
static CMD_STATUS wrCpssDxChIpfixAgingEnableGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8 devNum;
    CPSS_DXCH_POLICER_STAGE_TYPE_ENT stage;
    GT_BOOL enable;


    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */

    devNum = (GT_U8)inArgs[0];
    stage  = (CPSS_DXCH_POLICER_STAGE_TYPE_ENT)inArgs[1];

    /* call cpss api function */
    result = cpssDxChIpfixAgingEnableGet(devNum, stage, &enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", enable);

    return CMD_OK;
}

/**
* @internal wrCpssDxChIpfixCpuCodeSet function
* @endinternal
*
* @brief   Sets the 6 most significant bits of the CPU Code in case of mirroring.
*         The two least significant bits are taken from the IPfix entry.
*
* @note   APPLICABLE DEVICES:      DxChXcat and above.
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum, stage or cpu code.
* @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
*/
static CMD_STATUS wrCpssDxChIpfixCpuCodeSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8 devNum;
    CPSS_DXCH_POLICER_STAGE_TYPE_ENT stage;
    CPSS_NET_RX_CPU_CODE_ENT cpuCode;


    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */

    devNum = (GT_U8)inArgs[0];
    stage  = (CPSS_DXCH_POLICER_STAGE_TYPE_ENT)inArgs[1];
    cpuCode = (CPSS_NET_RX_CPU_CODE_ENT)inArgs[2];

    /* call cpss api function */
    result = cpssDxChIpfixCpuCodeSet(devNum, stage, cpuCode);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxChIpfixCpuCodeGet function
* @endinternal
*
* @brief   Gets the 6 most significant bits of the CPU Code in case of mirroring.
*         The two least significant bits are taken from the IPfix entry.
*
* @note   APPLICABLE DEVICES:      DxChXcat and above.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum or stage.
* @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
*/
static CMD_STATUS wrCpssDxChIpfixCpuCodeGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8 devNum;
    CPSS_DXCH_POLICER_STAGE_TYPE_ENT stage;
    CPSS_NET_RX_CPU_CODE_ENT cpuCode;


    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */

    devNum = (GT_U8)inArgs[0];
    stage  = (CPSS_DXCH_POLICER_STAGE_TYPE_ENT)inArgs[1];

    /* call cpss api function */
    result = cpssDxChIpfixCpuCodeGet(devNum, stage, &cpuCode);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", cpuCode);

    return CMD_OK;
}

/**
* @internal wrCpssDxChIpfixDropCountModeSet function
* @endinternal
*
* @brief   Configures IPFIX drop counting mode.
*
* @note   APPLICABLE DEVICES:      DxChXcat and above.
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum, stage or mode.
* @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
*/
static CMD_STATUS wrCpssDxChIpfixDropCountModeSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8 devNum;
    CPSS_DXCH_POLICER_STAGE_TYPE_ENT stage;
    CPSS_DXCH_IPFIX_DROP_COUNT_MODE_ENT mode;


    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */

    devNum = (GT_U8)inArgs[0];
    stage  = (CPSS_DXCH_POLICER_STAGE_TYPE_ENT)inArgs[1];
    mode = (CPSS_DXCH_IPFIX_DROP_COUNT_MODE_ENT)inArgs[2];

    /* call cpss api function */
    result = cpssDxChIpfixDropCountModeSet(devNum, stage, mode);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxChIpfixDropCountModeGet function
* @endinternal
*
* @brief   Gets IPFIX drop counting mode.
*
* @note   APPLICABLE DEVICES:      DxChXcat and above.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum or stage.
* @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
*/
static CMD_STATUS wrCpssDxChIpfixDropCountModeGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8 devNum;
    CPSS_DXCH_POLICER_STAGE_TYPE_ENT stage;
    CPSS_DXCH_IPFIX_DROP_COUNT_MODE_ENT mode;


    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */

    devNum = (GT_U8)inArgs[0];
    stage  = (CPSS_DXCH_POLICER_STAGE_TYPE_ENT)inArgs[1];

    /* call cpss api function */
    result = cpssDxChIpfixDropCountModeGet(devNum, stage, &mode);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", mode);

    return CMD_OK;
}

/**
* @internal wrCpssDxChIpfixTimerGet function
* @endinternal
*
* @brief   Gets IPFIX timer
*
* @note   APPLICABLE DEVICES:      DxChXcat and above.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum or stage.
* @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
*/
static CMD_STATUS wrCpssDxChIpfixTimerGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8 devNum;
    CPSS_DXCH_POLICER_STAGE_TYPE_ENT stage;
    CPSS_DXCH_IPFIX_TIMER_STC timer;


    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */

    devNum = (GT_U8)inArgs[0];
    stage  = (CPSS_DXCH_POLICER_STAGE_TYPE_ENT)inArgs[1];

    /* call cpss api function */
    result = cpssDxChIpfixTimerGet(devNum, stage, &timer);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d%d%d", timer.nanoSecondTimer,
                                            timer.secondTimer.l[0],
                                            timer.secondTimer.l[1]);

    return CMD_OK;
}

/**
* @internal wrCpssDxChIpfixTimestampUploadSet function
* @endinternal
*
* @brief   Configures IPFIX timestamp upload per stages.
*
* @note   APPLICABLE DEVICES:      DxChXcat and above.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum, stage or upload mode.
* @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
*/
static CMD_STATUS wrCpssDxChIpfixTimestampUploadSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8 devNum;
    CPSS_DXCH_POLICER_STAGE_TYPE_ENT stage;
    CPSS_DXCH_IPFIX_TIMESTAMP_UPLOAD_CONF_STC upload;


    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */

    devNum = (GT_U8)inArgs[0];
    stage  = (CPSS_DXCH_POLICER_STAGE_TYPE_ENT)inArgs[1];
    upload.uploadMode = (CPSS_DXCH_IPFIX_TIMESTAMP_UPLOAD_MODE_ENT)inArgs[2];
    upload.timer.nanoSecondTimer = (GT_U32)inArgs[3];
    upload.timer.secondTimer.l[0] = (GT_U32)inArgs[4];
    upload.timer.secondTimer.l[1] = (GT_U32)inArgs[5];

    /* call cpss api function */
    result = cpssDxChIpfixTimestampUploadSet(devNum, stage, &upload);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxChIpfixTimestampUploadGet function
* @endinternal
*
* @brief   Gets IPFIX timestamp upload configuration for a stage.
*
* @note   APPLICABLE DEVICES:      DxChXcat and above.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum or stage.
* @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
*/
static CMD_STATUS wrCpssDxChIpfixTimestampUploadGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8 devNum;
    CPSS_DXCH_POLICER_STAGE_TYPE_ENT stage;
    CPSS_DXCH_IPFIX_TIMESTAMP_UPLOAD_CONF_STC upload;


    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */

    devNum = (GT_U8)inArgs[0];
    stage  = (CPSS_DXCH_POLICER_STAGE_TYPE_ENT)inArgs[1];

    /* call cpss api function */
    result = cpssDxChIpfixTimestampUploadGet(devNum, stage, &upload);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d%d%d%d", upload.uploadMode,
                                              upload.timer.nanoSecondTimer,
                                              upload.timer.secondTimer.l[0],
                                              upload.timer.secondTimer.l[1]);

    return CMD_OK;
}

/**
* @internal wrCpssDxChIpfixTimestampUploadTrigger function
* @endinternal
*
* @brief   Triggers the IPFIX timestamp upload operation.
*
* @note   APPLICABLE DEVICES:      DxChXcat and above.
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum.
* @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
*
* @note IPLR0 triggering controls IPLR1 & EPLR as well, all uploads are done
*       simultenously.
*
*/
static CMD_STATUS wrCpssDxChIpfixTimestampUploadTrigger
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8 devNum;


    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */

    devNum = (GT_U8)inArgs[0];

    /* call cpss api function */
    result = cpssDxChIpfixTimestampUploadTrigger(devNum);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxChIpfixTimestampUploadStatusGet function
* @endinternal
*
* @brief   Gets IPFIX timestamp upload operation status.
*
* @note   APPLICABLE DEVICES:      DxChXcat and above.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum.
* @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
*/
static CMD_STATUS wrCpssDxChIpfixTimestampUploadStatusGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8 devNum;
    GT_BOOL uploadStatus;


    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */

    devNum = (GT_U8)inArgs[0];

    /* call cpss api function */
    result = cpssDxChIpfixTimestampUploadStatusGet(devNum, &uploadStatus);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", uploadStatus);

    return CMD_OK;
}

/**
* @internal wrCpssDxChIpfixWraparoundConfSet function
* @endinternal
*
* @brief   Configures IPFIX wraparound parameters.
*
* @note   APPLICABLE DEVICES:      DxChXcat and above.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum, stage, action or threshold.
* @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
*/
static CMD_STATUS wrCpssDxChIpfixWraparoundConfSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8 devNum;
    CPSS_DXCH_POLICER_STAGE_TYPE_ENT stage;
    CPSS_DXCH_IPFIX_WRAPAROUND_CFG_STC conf;


    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */

    devNum = (GT_U8)inArgs[0];
    stage  = (CPSS_DXCH_POLICER_STAGE_TYPE_ENT)inArgs[1];
    conf.action= (CPSS_DXCH_IPFIX_WRAPAROUND_ACTION_ENT)inArgs[2];
    conf.dropThreshold = (GT_U32)inArgs[3];
    conf.packetThreshold = (GT_U32)inArgs[4];
    conf.byteThreshold.l[0] = (GT_U32)inArgs[5];
    conf.byteThreshold.l[1] = (GT_U32)inArgs[6];

    /* call cpss api function */
    result = cpssDxChIpfixWraparoundConfSet(devNum, stage, &conf);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxChIpfixWraparoundConfGet function
* @endinternal
*
* @brief   Get IPFIX wraparound configuration.
*
* @note   APPLICABLE DEVICES:      DxChXcat and above.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum or stage.
* @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
*/
static CMD_STATUS wrCpssDxChIpfixWraparoundConfGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8 devNum;
    CPSS_DXCH_POLICER_STAGE_TYPE_ENT stage;
    CPSS_DXCH_IPFIX_WRAPAROUND_CFG_STC conf;


    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */

    devNum = (GT_U8)inArgs[0];
    stage  = (CPSS_DXCH_POLICER_STAGE_TYPE_ENT)inArgs[1];

    /* call cpss api function */
    result = cpssDxChIpfixWraparoundConfGet(devNum, stage, &conf);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d%d%d%d%d", conf.action,
                                                conf.dropThreshold,
                                                conf.packetThreshold,
                                                conf.byteThreshold.l[0],
                                                conf.byteThreshold.l[1]);

    return CMD_OK;
}

/**
* @internal wrCpssDxChIpfixAlarmEventsGet function
* @endinternal
*
* @brief   Retrieves up to 16 IPFIX entries indexes where alarm events (due to
*         sampling) occured.
*
* @note   APPLICABLE DEVICES:      DxChXcat and above.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum or stage.
* @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
*/
static CMD_STATUS wrCpssDxChIpfixAlarmEventsGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8 devNum;
    CPSS_DXCH_POLICER_STAGE_TYPE_ENT stage;
    GT_U32  eventsArr[CPSS_DXCH_IPFIX_ALARM_EVENTS_ARRAY_LENGTH_CNS];
    GT_U32  eventsNum;
    GT_BOOL             pgEnable; /* multi port group  enable */
    GT_PORT_GROUPS_BMP  pgBmp;    /* port group BMP           */

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */

    devNum = (GT_U8)inArgs[0];
    stage  = (CPSS_DXCH_POLICER_STAGE_TYPE_ENT)inArgs[1];

    ipfixMultiPortGroupsBmpGet(devNum, &pgEnable, &pgBmp);

    /* call cpss api function */
    if (pgEnable == GT_FALSE)
    {
        result = cpssDxChIpfixAlarmEventsGet(devNum, stage, eventsArr, &eventsNum);
    }
    else
    {
        result = cpssDxChIpfixPortGroupAlarmEventsGet(devNum, pgBmp, stage, eventsArr, &eventsNum);
    }

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d",
                 eventsArr[0], eventsArr[1], eventsArr[2], eventsArr[3],
                 eventsArr[4], eventsArr[5], eventsArr[6], eventsArr[7],
                 eventsArr[8], eventsArr[9], eventsArr[10], eventsArr[11],
                 eventsArr[12], eventsArr[13], eventsArr[14], eventsArr[15], eventsNum);

    return CMD_OK;
}


static GT_U32 bmp[64];
static GT_U32 currentIndex;
static GT_U32 startIndex;
static GT_U32 endIndex;

static CMD_STATUS wrCpssDxChIpfixBmpGetBit
(
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    if( currentIndex > endIndex )
    {
        galtisOutput(outArgs, CMD_OK, "%d", -1);
        return CMD_OK;
    }

    inFields[0] = currentIndex;
    inFields[1] = ( ((bmp[(currentIndex-startIndex)/32]) & (1 << ((currentIndex-startIndex)%32))) == 0 ) ? 0 : 1 ;

    fieldOutput("%d%d", inFields[0],inFields[1]);
    /* pack output arguments to galtis string */
    galtisOutput(outArgs, GT_OK, "%f");

    return CMD_OK;
}

/**
* @internal wrCpssDxChIpfixAgingGetFirst function
* @endinternal
*
* @brief   Retrieves a bitmap of aged entries indexes.
*
* @note   APPLICABLE DEVICES:      DxChXcat and above.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum or stage.
* @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
*
* @note It is application responsibility to allocate the buffer size for the
*       bitmap(bmpPtr). Since the buffer is made from GT_U32 elements, each
*       element contains 32 indications.
*
*/
static CMD_STATUS wrCpssDxChIpfixAgingGetFirst
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8 devNum;
    CPSS_DXCH_POLICER_STAGE_TYPE_ENT stage;
    GT_BOOL reset;
    GT_BOOL             pgEnable; /* multi port group  enable */
    GT_PORT_GROUPS_BMP  pgBmp;    /* port group BMP           */


    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */

    devNum = (GT_U8)inArgs[0];
    stage  = (CPSS_DXCH_POLICER_STAGE_TYPE_ENT)inArgs[1];
    currentIndex = startIndex = (GT_U32)inArgs[2];
    endIndex = (GT_U32)inArgs[3];
    reset = (GT_BOOL)inArgs[4];

    ipfixMultiPortGroupsBmpGet(devNum, &pgEnable, &pgBmp);

    /* call cpss api function */
    if (pgEnable == GT_FALSE)
    {
        result = cpssDxChIpfixAgingStatusGet(devNum, stage, startIndex, endIndex,
                                             reset, bmp);
    }
    else
    {
        result = cpssDxChIpfixPortGroupAgingStatusGet(devNum, pgBmp, stage, startIndex, endIndex,
                                                      reset, bmp);
    }

    if (result != GT_OK)
    {
        galtisOutput(outArgs, result, "%d", -1);
        return CMD_OK;
    }

    return wrCpssDxChIpfixBmpGetBit(inFields, outArgs);
}

/******************************************************************************/

static CMD_STATUS wrCpssDxChIpfixAgingGetNext
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    currentIndex++;

    return wrCpssDxChIpfixBmpGetBit(inFields, outArgs);
}

/**
* @internal wrCpssDxChIpfixWraparoundGetFirst function
* @endinternal
*
* @brief   Retrieves a bitmap of wraparound entries indexes.
*
* @note   APPLICABLE DEVICES:      DxChXcat and above.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum or stage.
* @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
*
* @note It is application responsibility to allocate the buffer size for the
*       bitmap(bmpPtr). Since the buffer is made from GT_U32 elements, each
*       element contains 32 indications.
*
*/
static CMD_STATUS wrCpssDxChIpfixWraparoundGetFirst
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8 devNum;
    CPSS_DXCH_POLICER_STAGE_TYPE_ENT stage;
    GT_BOOL reset;
    GT_BOOL             pgEnable; /* multi port group  enable */
    GT_PORT_GROUPS_BMP  pgBmp;    /* port group BMP           */


    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */

    devNum = (GT_U8)inArgs[0];
    stage  = (CPSS_DXCH_POLICER_STAGE_TYPE_ENT)inArgs[1];
    currentIndex = startIndex = (GT_U32)inArgs[2];
    endIndex = (GT_U32)inArgs[3];
    reset = (GT_BOOL)inArgs[4];

    ipfixMultiPortGroupsBmpGet(devNum, &pgEnable, &pgBmp);

    /* call cpss api function */
    if (pgEnable == GT_FALSE)
    {
        result = cpssDxChIpfixWraparoundStatusGet(devNum, stage, startIndex, endIndex,
                                                  reset, bmp);
    }
    else
    {
        result = cpssDxChIpfixPortGroupWraparoundStatusGet(devNum, pgBmp, stage, startIndex, endIndex,
                                                           reset, bmp);
    }

    if (result != GT_OK)
    {
        galtisOutput(outArgs, result, "%d", -1);
        return CMD_OK;
    }

    return wrCpssDxChIpfixBmpGetBit(inFields, outArgs);
}

/******************************************************************************/

static CMD_STATUS wrCpssDxChIpfixWraparoundGetNext
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    currentIndex++;

    return wrCpssDxChIpfixBmpGetBit(inFields, outArgs);
}

/**
* @internal wrCpssDxChIpfixEntrySet function
* @endinternal
*
* @brief   Sets IPFIX entry.
*
* @note   APPLICABLE DEVICES:      DxChXcat and above.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_TIMEOUT               - on time out upon counter reset by
*                                       indirect access.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum, stage or entryIndex.
* @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
*
* @note In order to set IPFIX entry under traffic perform the following
*       algorithm(none-zero setting of IPFIX Counters while enabled\still
*       under traffic may cause data coherency problems):
*       - disconnect entry from traffic
*       - call cpssDxChPolicerCountingWriteBackCacheFlush
*       - set new values
*       - connect entry back to traffic
*       This API address the same memory area (counters place) as can be set
*       by "cpssDxCh3PolicerBillingEntrySet".
*
*/
static CMD_STATUS wrCpssDxChIpfixEntrySet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8 devNum;
    CPSS_DXCH_POLICER_STAGE_TYPE_ENT stage;
    GT_U32 entryIndex;
    CPSS_DXCH_IPFIX_ENTRY_STC ipfixEntry;
    GT_BOOL             pgEnable; /* multi port group  enable */
    GT_PORT_GROUPS_BMP  pgBmp;    /* port group BMP           */


    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */

    devNum = (GT_U8)inArgs[0];
    stage  = (CPSS_DXCH_POLICER_STAGE_TYPE_ENT)inArgs[1];
    entryIndex = (GT_U32)inFields[0];
    ipfixEntry.timeStamp = (GT_U32)inFields[1];
    ipfixEntry.packetCount = (GT_U32)inFields[2];
    ipfixEntry.byteCount.l[0] = (GT_U32)inFields[3];
    ipfixEntry.byteCount.l[1] = (GT_U32)inFields[4];
    ipfixEntry.dropCounter = (GT_U32)inFields[5];
    ipfixEntry.randomOffset = (GT_U32)inFields[6];
    ipfixEntry.lastSampledValue.l[0] = (GT_U32)inFields[7];
    ipfixEntry.lastSampledValue.l[1] = (GT_U32)inFields[8];
    ipfixEntry.samplingWindow.l[0] = (GT_U32)inFields[9];
    ipfixEntry.samplingWindow.l[1] = (GT_U32)inFields[10];
    ipfixEntry.samplingAction = (CPSS_DXCH_IPFIX_SAMPLING_ACTION_ENT)inFields[11];
    ipfixEntry.logSamplingRange = (GT_U32)inFields[12];
    ipfixEntry.randomFlag = (CPSS_DXCH_IPFIX_SAMPLING_DIST_ENT)inFields[13];
    ipfixEntry.samplingMode = (CPSS_DXCH_IPFIX_SAMPLING_MODE_ENT)inFields[14];
    ipfixEntry.cpuSubCode = (GT_U32)inFields[15];

    /* call cpss api function */
    ipfixMultiPortGroupsBmpGet(devNum, &pgEnable, &pgBmp);

    /* call cpss api function */
    if (pgEnable == GT_FALSE)
    {
        result = cpssDxChIpfixEntrySet(devNum, stage, entryIndex, &ipfixEntry);
    }
    else
    {
        result = cpssDxChIpfixPortGroupEntrySet(devNum, pgBmp, stage, entryIndex, &ipfixEntry);
    }

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxChIpfixEntryGet function
* @endinternal
*
* @brief   Gets IPFIX entry.
*
* @note   APPLICABLE DEVICES:      DxChXcat and above.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_TIMEOUT               - on time out of IPLR Table indirect access.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum, stage or entryIndex.
* @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
*
* @note When counters are read in clear on read mode (reset == GT_TRUE), The following
*       actions are done on the different IPFIX entry fields:
*       - The <Packet Counter>, <Byte Counter>, and <Dropped Packet Counter> fields are cleared.
*       - The <Timestamp> field is updated to the current time.
*       - The <Last Sampled Value> is cleared.
*
*/
static CMD_STATUS wrCpssDxChIpfixEntryGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8 devNum;
    CPSS_DXCH_POLICER_STAGE_TYPE_ENT stage;
    GT_BOOL reset;
    CPSS_DXCH_IPFIX_ENTRY_STC ipfixEntry;
    GT_BOOL             pgEnable; /* multi port group  enable */
    GT_PORT_GROUPS_BMP  pgBmp;    /* port group BMP           */


    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    if( currentIndex > endIndex )
    {
        galtisOutput(outArgs, CMD_OK, "%d", -1);
        return CMD_OK;
    }

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    stage = (CPSS_DXCH_POLICER_STAGE_TYPE_ENT)inArgs[1];
    reset = (GT_BOOL)inArgs[4];

    /* call cpss api function */
    ipfixMultiPortGroupsBmpGet(devNum, &pgEnable, &pgBmp);

    /* call cpss api function */
    if (pgEnable == GT_FALSE)
    {
        result = cpssDxChIpfixEntryGet(devNum, stage, currentIndex, reset, &ipfixEntry);
    }
    else
    {
        result = cpssDxChIpfixPortGroupEntryGet(devNum, pgBmp, stage, currentIndex, reset, &ipfixEntry);
    }

    if (result != GT_OK)
    {
        galtisOutput(outArgs, result, "%d", -1);
        return CMD_OK;
    }

    inFields[0] = currentIndex;
    inFields[1] = ipfixEntry.timeStamp;
    inFields[2] = ipfixEntry.packetCount;
    inFields[3] = ipfixEntry.byteCount.l[0];
    inFields[4] = ipfixEntry.byteCount.l[1];
    inFields[5] = ipfixEntry.dropCounter;
    inFields[6] = ipfixEntry.randomOffset;
    inFields[7] = ipfixEntry.lastSampledValue.l[0];
    inFields[8] = ipfixEntry.lastSampledValue.l[1];
    inFields[9] = ipfixEntry.samplingWindow.l[0];
    inFields[10] = ipfixEntry.samplingWindow.l[1];
    inFields[11] = ipfixEntry.samplingAction;
    inFields[12] = ipfixEntry.logSamplingRange;
    inFields[13] = ipfixEntry.randomFlag;
    inFields[14] = ipfixEntry.samplingMode;
    inFields[15] = ipfixEntry.cpuSubCode;

    fieldOutput("%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d", inFields[0],inFields[1],inFields[2],inFields[3],
                                                   inFields[4],inFields[5],inFields[6],inFields[7],
                                                   inFields[8],inFields[9],inFields[10],inFields[11],
                                                  inFields[12],inFields[13],inFields[14],inFields[15]);


    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%f");

    return CMD_OK;
}

/************************************************************************************/

static CMD_STATUS wrCpssDxChIpfixEntryGetFirst
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

    /* map input arguments to locals */

    currentIndex = startIndex = (GT_U32)inArgs[2];
    endIndex = (GT_U32)inArgs[3];

    return wrCpssDxChIpfixEntryGet(inArgs,inFields,numFields,outArgs);
}

/************************************************************************************/

static CMD_STATUS wrCpssDxChIpfixEntryGetNext
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    currentIndex++;

    return wrCpssDxChIpfixEntryGet(inArgs,inFields,numFields,outArgs);
}

/**
* @internal wrCpssDxChIpfixTimestampToCpuEnableSet function
* @endinternal
*
* @brief   Configures all TO_CPU DSA tags to include a Timestamp.
*
* @note   APPLICABLE DEVICES:      Lion and above.
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum.
* @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
*/
static CMD_STATUS wrCpssDxChIpfixTimestampToCpuEnableSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8 devNum;
    GT_BOOL enable;


    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */

    devNum = (GT_U8)inArgs[0];
    enable = (GT_BOOL)inArgs[1];

    /* call cpss api function */
    result = cpssDxChIpfixTimestampToCpuEnableSet(devNum, enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxChIpfixTimestampToCpuEnableGet function
* @endinternal
*
* @brief   Gets enabling status of TO_CPU DSA tags including a Timestamp.
*
* @note   APPLICABLE DEVICES:      Lion and above.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum.
* @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
*/
static CMD_STATUS wrCpssDxChIpfixTimestampToCpuEnableGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8 devNum;
    GT_BOOL enable;


    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */

    devNum = (GT_U8)inArgs[0];

    /* call cpss api function */
    result = cpssDxChIpfixTimestampToCpuEnableGet(devNum, &enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", enable);

    return CMD_OK;
}
/**** database initialization **************************************/

static CMD_COMMAND dbCommands[] =
{
   {"cpssDxChIpfixAgingEnableSet",
         &wrCpssDxChIpfixAgingEnableSet,
         3, 0},

   {"cpssDxChIpfixAgingEnableGet",
         &wrCpssDxChIpfixAgingEnableGet,
         2, 0},

   {"cpssDxChIpfixCpuCodeSet",
         &wrCpssDxChIpfixCpuCodeSet,
         3, 0},

   {"cpssDxChIpfixCpuCodeGet",
         &wrCpssDxChIpfixCpuCodeGet,
         2, 0},

   {"cpssDxChIpfixDropCountModeSet",
         &wrCpssDxChIpfixDropCountModeSet,
         3, 0},

   {"cpssDxChIpfixDropCountModeGet",
         &wrCpssDxChIpfixDropCountModeGet,
         2, 0},

   {"cpssDxChIpfixTimerGet",
         &wrCpssDxChIpfixTimerGet,
         2, 0},

   {"cpssDxChIpfixTimestampUploadSet",
         &wrCpssDxChIpfixTimestampUploadSet,
         6, 0},

   {"cpssDxChIpfixTimestampUploadGet",
         &wrCpssDxChIpfixTimestampUploadGet,
         2, 0},

   {"cpssDxChIpfixTimestampUploadTrigger",
         &wrCpssDxChIpfixTimestampUploadTrigger,
         1, 0},

   {"cpssDxChIpfixTimestampUploadStatusGet",
         &wrCpssDxChIpfixTimestampUploadStatusGet,
         1, 0},

   {"cpssDxChIpfixWraparoundConfSet",
         &wrCpssDxChIpfixWraparoundConfSet,
         7, 0},

   {"cpssDxChIpfixWraparoundConfGet",
         &wrCpssDxChIpfixWraparoundConfGet,
         2, 0},

   {"cpssDxChIpfixAlarmEventsGet",
         &wrCpssDxChIpfixAlarmEventsGet,
         2, 0},


   {"cpssDxChIpfixAgingGetFirst",
         &wrCpssDxChIpfixAgingGetFirst,
         5, 0},

   {"cpssDxChIpfixAgingGetNext",
         &wrCpssDxChIpfixAgingGetNext,
         5, 0},

   {"cpssDxChIpfixWraparoundGetFirst",
         &wrCpssDxChIpfixWraparoundGetFirst,
         5, 0},

   {"cpssDxChIpfixWraparoundGetNext",
         &wrCpssDxChIpfixWraparoundGetNext,
         5, 0},

   {"cpssDxChIpfixSet",
         &wrCpssDxChIpfixEntrySet,
         2, 16},

   {"cpssDxChIpfixGetFirst",
         &wrCpssDxChIpfixEntryGetFirst,
         5, 0},

   {"cpssDxChIpfixGetNext",
         &wrCpssDxChIpfixEntryGetNext,
         5, 0},

   {"cpssDxChIpfixTimestampToCpuEnableSet",
         &wrCpssDxChIpfixTimestampToCpuEnableSet,
         2, 0},

   {"cpssDxChIpfixTimestampToCpuEnableGet",
         &wrCpssDxChIpfixTimestampToCpuEnableGet,
         1, 0},

};

#define numCommands (sizeof(dbCommands) / sizeof(CMD_COMMAND))

/**
* @internal cmdLibInitCpssDxChIpfix function
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
GT_STATUS cmdLibInitCpssDxChIpfix
(
    GT_VOID
)
{
    return cmdInitLibrary(dbCommands, numCommands);
}



