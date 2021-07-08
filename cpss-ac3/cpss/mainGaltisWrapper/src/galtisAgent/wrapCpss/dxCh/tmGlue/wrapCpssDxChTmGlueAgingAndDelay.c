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
* @file wrapCpssDxChTmGlueAgingAndDelay.c
*
* @brief Wrapper functions for TM Aging and Delay Measurments cpss
*
* @version   3
********************************************************************************
*/

/* Common galtis includes */
#include <galtisAgent/wrapUtil/cmdCpssPresteraUtils.h>
#include <cmdShell/cmdDb/cmdBase.h>
#include <cmdShell/common/cmdWrapUtils.h>


/* Feature specific includes. */
#include <cpss/common/cpssTypes.h>
#include <cpss/dxCh/dxChxGen/tmGlue/cpssDxChTmGlueAgingAndDelay.h>

/**
* @internal wrCpssDxChTmGlueAgingAndDelayDropAgedPacketEnableSet function
* @endinternal
*
* @brief   Enable dropping of aged packets.
*         Sets if packets that are stored longer than highest aging threhold should
*         be dropped or queue Drop Profile shuld be changed to highest threshold DP.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Aldrin; AC3X.
*
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChTmGlueAgingAndDelayDropAgedPacketEnableSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS       result;

    GT_U8           devNum;
    GT_BOOL         enable;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    enable = (GT_BOOL)inArgs[1];

    /* call cpss api function */
    result = cpssDxChTmGlueAgingAndDelayDropAgedPacketEnableSet(devNum, enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}

/**
* @internal wrCpssDxChTmGlueAgingAndDelayDropAgePacketEnableGet function
* @endinternal
*
* @brief   Get dropping aged packets status
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Aldrin; AC3X.
*
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChTmGlueAgingAndDelayDropAgePacketEnableGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS   result;

    GT_U8       devNum;
    GT_BOOL     enable;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];

    /* call cpss api function */
    result = cpssDxChTmGlueAgingAndDelayDropAgedPacketEnableGet(devNum, &enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", enable);
    return CMD_OK;
}

/**
* @internal wrCpssDxChTmGlueAgingAndDelayAgedPacketsCountingEnableSet function
* @endinternal
*
* @brief   Enable statistics on packets dropped due to aging in
*         TM queue aging and delay counter set
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Aldrin; AC3X.
*
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChTmGlueAgingAndDelayAgedPacketsCountingEnableSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS       result;

    GT_U8           devNum;
    GT_BOOL         enable;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    enable = (GT_BOOL)inArgs[1];

    /* call cpss api function */
    result = cpssDxChTmGlueAgingAndDelayAgedPacketsCountingEnableSet(devNum, enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}

/**
* @internal wrCpssDxChTmGlueAgingAndDelayAgedPacketsCountingEnableGet function
* @endinternal
*
* @brief   Get status of enable statistics on packets dropped due to aging in
*         TM queue aging and delay counter set
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Aldrin; AC3X.
*
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChTmGlueAgingAndDelayAgedPacketsCountingEnableGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS   result;

    GT_U8       devNum;
    GT_BOOL     enable;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];

    /* call cpss api function */
    result = cpssDxChTmGlueAgingAndDelayAgedPacketsCountingEnableGet(devNum, &enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", enable);
    return CMD_OK;
}

/**
* @internal wrCpssDxChTmGlueGeneralPerPortDropOutgoingPacketEnableSet function
* @endinternal
*
* @brief   Enable Drop of packets transmitted through TM-Port
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Aldrin; AC3X.
*
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChTmGlueGeneralPerPortDropOutgoingPacketEnableSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS       result;

    GT_U8           devNum;
    GT_U32          tmPort;
    GT_BOOL         enable;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    tmPort = (GT_U32)inArgs[1];
    enable = (GT_BOOL)inArgs[2];

    /* call cpss api function */
    result = cpssDxChTmGlueGeneralPerPortDropOutgoingPacketEnableSet(devNum, tmPort, enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}

/**
* @internal wrCpssDxChTmGlueGeneralPerPortDropOutgoingPacketEnableGet function
* @endinternal
*
* @brief   Return drop status of packets transmitted through TM-Port
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Aldrin; AC3X.
*
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChTmGlueGeneralPerPortDropOutgoingPacketEnableGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS   result;

    GT_U8       devNum;
    GT_U32      tmPort;
    GT_BOOL     enable;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    tmPort = (GT_U32)inArgs[1];

    /* call cpss api function */
    result = cpssDxChTmGlueGeneralPerPortDropOutgoingPacketEnableGet(devNum, tmPort, &enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", enable);
    return CMD_OK;
}

/**
* @internal wrCpssDxChTmGlueAgingAndDelayAgedPacketCounterConfigSet function
* @endinternal
*
* @brief   Set Configuration for Dropped Aged Packet Counter
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Aldrin; AC3X.
*
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The counter is increment if the following logic apply:
*       (agedPacketCouterQueueMask & PacketQueue) = agedPacketCouterQueue
*
*/
static CMD_STATUS wrCpssDxChTmGlueAgingAndDelayAgedPacketCounterConfigSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS       result;

    GT_U8           devNum;
    GT_U32          dropAgedPacketCouterQueueMask;
    GT_U32          dropAgedPacketCouterQueue;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    dropAgedPacketCouterQueueMask = (GT_U32)inArgs[1];
    dropAgedPacketCouterQueue     = (GT_U32)inArgs[2];

    /* call cpss api function */
    result = cpssDxChTmGlueAgingAndDelayAgedPacketCounterConfigSet(devNum,
                                                                   dropAgedPacketCouterQueueMask,
                                                                   dropAgedPacketCouterQueue);
    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}


/**
* @internal wrCpssDxChTmGlueAgingAndDelayAgedPacketCounterConfigGet function
* @endinternal
*
* @brief   Get Configuration for Dropped Aged Packet Counter
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Aldrin; AC3X.
*
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The counter is increment if the following logic apply:
*       (dropAgedPacketCouterQueueMask & PacketQueue) = dropAgedPacketCouterQueue
*
*/
static CMD_STATUS wrCpssDxChTmGlueAgingAndDelayAgedPacketCounterConfigGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS   result;

    GT_U8       devNum;
    GT_U32      dropAgedPacketCouterQueueMask;
    GT_U32      dropAgedPacketCouterQueue;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];

    /* call cpss api function */
    result = cpssDxChTmGlueAgingAndDelayAgedPacketCounterConfigGet(devNum,
                                                                   &dropAgedPacketCouterQueueMask,
                                                                   &dropAgedPacketCouterQueue);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d%d", dropAgedPacketCouterQueueMask,dropAgedPacketCouterQueue);
    return CMD_OK;
}

/**
* @internal wrCpssDxChTmGlueAgingAndDelayAgedPacketCounterGet function
* @endinternal
*
* @brief   Read the counter that counts the number of aged packets that were dropped
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Aldrin; AC3X.
*
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Clear On Read Counter
*       The counter is increment if the following logic apply:
*       (agedPacketCouterQueueMask & PacketQueue) = agedPacketCouterQueue
*
*/
static CMD_STATUS wrCpssDxChTmGlueAgingAndDelayAgedPacketCounterGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS   result;

    GT_U8       devNum;
    GT_U32      agedPacketCounter;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];

    /* call cpss api function */
    result = cpssDxChTmGlueAgingAndDelayAgedPacketCounterGet(devNum,
                                                             &agedPacketCounter);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", agedPacketCounter);
    return CMD_OK;
}

/**
* @internal wrCpssDxChTmGlueAgingAndDelayTimerResolutionSet function
* @endinternal
*
* @brief   Set aging's timer units resolution.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Aldrin; AC3X.
*
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChTmGlueAgingAndDelayTimerResolutionSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS       result;

    GT_U8           devNum;
    GT_U32          timerResolution;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    timerResolution = (GT_U32)inArgs[1];

    /* call cpss api function */
    result = cpssDxChTmGlueAgingAndDelayTimerResolutionSet(devNum,&timerResolution);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", timerResolution);
    return CMD_OK;
}

/**
* @internal wrCpssDxChTmGlueAgingAndDelayTimerResolutionGet function
* @endinternal
*
* @brief   Get aging's timer units resolution.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Aldrin; AC3X.
*
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChTmGlueAgingAndDelayTimerResolutionGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS   result;

    GT_U8       devNum;
    GT_U32      timerResolution;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];

    /* call cpss api function */
    result = cpssDxChTmGlueAgingAndDelayTimerResolutionGet(devNum,
                                                                &timerResolution);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", timerResolution);
    return CMD_OK;
}

/**
* @internal wrCpssDxChTmGlueAgingAndDelayTimerGet function
* @endinternal
*
* @brief   The aging and delay timer is the reference clock used for calculating
*         the time each packet was stored.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Aldrin; AC3X.
*
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The timer is incremented by 1 each time the free running counter
*       reach "0".
*       see cpssDxChTmGlueAgingAndDelayTimerResolutionSet
*
*/
static CMD_STATUS wrCpssDxChTmGlueAgingAndDelayTimerGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS   result;

    GT_U8       devNum;
    GT_U32      timerPtr;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];

    /* call cpss api function */
    result = cpssDxChTmGlueAgingAndDelayTimerGet(devNum,
                                                 &timerPtr);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", timerPtr);
    return CMD_OK;
}

/**
* @internal wrCpssDxChTmGlueAgingAndDelayAgingProfileThresholdsSet function
* @endinternal
*
* @brief   Configure the Aging Profile Thresholds Table
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Aldrin; AC3X.
*
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The thresholds units are set in cpssDxChTmGlueAgingAndDelayTimerResolutionSet
*
*/
static CMD_STATUS wrCpssDxChTmGlueAgingAndDelayAgingProfileThresholdsSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS       result;

    GT_U8           devNum;
    GT_U32          profileId;
    CPSS_DXCH_TM_AGING_AND_DELAY_PROFILE_THRESHOLDS_STC  thresholds;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    profileId = (GT_U32)inArgs[1];
    thresholds.threshold0 = (GT_U32)inArgs[2];
    thresholds.threshold1 = (GT_U32)inArgs[3];
    thresholds.threshold2 = (GT_U32)inArgs[4];

    /* call cpss api function */
    result = cpssDxChTmGlueAgingAndDelayAgingProfileThresholdsSet(devNum,
                                                                profileId,
                                                                &thresholds);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}

/**
* @internal wrCpssDxChTmGlueAgingAndDelayAgingProfileThresholdsGet function
* @endinternal
*
* @brief   Get the Aging Profile Thresholds Table configuration
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Aldrin; AC3X.
*
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The thresholds units are set in cpssDxChTmGlueAgingAndDelayTimerResolutionSet
*
*/
static CMD_STATUS wrCpssDxChTmGlueAgingAndDelayAgingProfileThresholdsGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS   result;

    GT_U8       devNum;
    GT_U32      profileId;
    CPSS_DXCH_TM_AGING_AND_DELAY_PROFILE_THRESHOLDS_STC  thresholds;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    profileId = (GT_U32)inArgs[1];

    /* call cpss api function */
    result = cpssDxChTmGlueAgingAndDelayAgingProfileThresholdsGet(devNum,
                                                                  profileId,
                                                                  &thresholds);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d%d%d", thresholds.threshold0, thresholds.threshold1, thresholds.threshold2);
    return CMD_OK;
}

/**
* @internal wrCpssDxChTmGlueAgingAndDelayQueueAgingProfileIdSet function
* @endinternal
*
* @brief   Configure Queue Aging Profile Table that Maps a Traffic Manager Queue ID
*         to an aging profile
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Aldrin; AC3X.
*
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChTmGlueAgingAndDelayQueueAgingProfileIdSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS       result;

    GT_U8           devNum;
    GT_U32          queueId;
    GT_U32          profileId;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    queueId = (GT_U32)inArgs[1];
    profileId = (GT_U32)inArgs[2];

    /* call cpss api function */
    result = cpssDxChTmGlueAgingAndDelayQueueAgingProfileIdSet(devNum,
                                                               queueId,
                                                               profileId);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}

/**
* @internal wrCpssDxChTmGlueAgingAndDelayQueueAgingProfileIdGet function
* @endinternal
*
* @brief   Get Queue Aging Profile Table configuration that Maps a Traffic Manager
*         Queue ID to an aging profile
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Aldrin; AC3X.
*
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChTmGlueAgingAndDelayQueueAgingProfileIdGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS   result;

    GT_U8       devNum;
    GT_U32      queueId;
    GT_U32      profileId;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    queueId = (GT_U32)inArgs[1];

    /* call cpss api function */
    result = cpssDxChTmGlueAgingAndDelayQueueAgingProfileIdGet(devNum,
                                                               queueId,
                                                               &profileId);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", profileId);
    return CMD_OK;
}

/**
* @internal wrCpssDxChTmGlueAgingAndDelayCounterQueueIdSet function
* @endinternal
*
* @brief   Map one of aging and delay counter sets to a queuedId number.
*         The counters in the set perform delay and aging statistics on the packets
*         queued to the configured Queue-ID.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Aldrin; AC3X.
*
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChTmGlueAgingAndDelayCounterQueueIdSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS       result;

    GT_U8           devNum;
    GT_U32          counterSetIndex;
    GT_U32          queueId;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    counterSetIndex = (GT_U32)inArgs[1];
    queueId = (GT_U32)inArgs[2];

    /* call cpss api function */
    result = cpssDxChTmGlueAgingAndDelayCounterQueueIdSet(devNum,
                                                       counterSetIndex,
                                                       queueId);
    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}


/**
* @internal wrCpssDxChTmGlueAgingAndDelayCounterQueueIdGet function
* @endinternal
*
* @brief   Get Mapping of aging and delay counter set to a queuedId number.
*         The counters in the set perform delay and aging statistics on the packets
*         queued to the configured Queue-ID.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Aldrin; AC3X.
*
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChTmGlueAgingAndDelayCounterQueueIdGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS   result;

    GT_U8       devNum;
    GT_U32      counterSetIndex;
    GT_U32      queueId;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    counterSetIndex = (GT_U32)inArgs[1];

    /* call cpss api function */
    result = cpssDxChTmGlueAgingAndDelayCounterQueueIdGet(devNum,
                                                           counterSetIndex,
                                                           &queueId);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", queueId);
    return CMD_OK;
}

/**
* @internal wrCpssDxChTmGlueAgingAndDelayCountersGet function
* @endinternal
*
* @brief   Read all the counters of aging and delay counter set
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Aldrin; AC3X.
*
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Clear On Read Counter
*
*/
static CMD_STATUS wrCpssDxChTmGlueAgingAndDelayCountersGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS       result;

    GT_U8                                       devNum;
    GT_U32                                      counterSetIndex;
    CPSS_DXCH_TM_AGING_AND_DELAY_STATISTICS_STC agingCounters;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    counterSetIndex = (GT_U32)inArgs[1];

    /* call cpss api function */
    result = cpssDxChTmGlueAgingAndDelayCountersGet(devNum,
                                                   counterSetIndex,
                                                   &agingCounters);

     /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d%d%d%d%d%d%d%d",
                 agingCounters.packetCounter,
                 agingCounters.octetCounter.l[0],
                 agingCounters.octetCounter.l[1],
                 agingCounters.latencyTimeCounter.l[0],
                 agingCounters.latencyTimeCounter.l[1],
                 agingCounters.agedPacketCounter,
                 agingCounters.maxLatencyTime,
                 agingCounters.minLatencyTime);
    return CMD_OK;
}

/**
* @internal wrCpssDxChTmGlueAgingAndDelayPerPortAgedPacketCounterGet function
* @endinternal
*
* @brief   Read the counter that counts the number of aged out packets
*         that should have been transmiited from tm-port
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Aldrin; AC3X.
*
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Clear On Read Counter
*
*/
static CMD_STATUS wrCpssDxChTmGlueAgingAndDelayPerPortAgedPacketCounterGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS   result;

    GT_U8       devNum;
    GT_U32      tmPort;
    GT_U32      agedOutPacketCounter;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    tmPort = (GT_U32)inArgs[1];

    /* call cpss api function */
    result = cpssDxChTmGlueAgingAndDelayPerPortAgedPacketCounterGet(devNum,
                                                                       tmPort,
                                                                       &agedOutPacketCounter);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", agedOutPacketCounter);
    return CMD_OK;
}
/**** database initialization **************************************/

static CMD_COMMAND dbCommands[] =
{

    {"cpssDxChTmGlueAgingAndDelayDropAgePacketEnableSet",
        &wrCpssDxChTmGlueAgingAndDelayDropAgedPacketEnableSet,
        2, 0},
    {"cpssDxChTmGlueAgingAndDelayDropAgePacketEnableGet",
        &wrCpssDxChTmGlueAgingAndDelayDropAgePacketEnableGet,
        1, 0},

    {"cpssDxChTmGlueAgingAndDelayDroppedPckCountingEnSet",
        &wrCpssDxChTmGlueAgingAndDelayAgedPacketsCountingEnableSet,
        2, 0},
    {"cpssDxChTmGlueAgingAndDelayDroppedPckCountingEnGet",
        &wrCpssDxChTmGlueAgingAndDelayAgedPacketsCountingEnableGet,
        1, 0},

    {"cpssDxChTmGlueGeneralPortDropOutPckEnSet",
        &wrCpssDxChTmGlueGeneralPerPortDropOutgoingPacketEnableSet,
        3, 0},
    {"cpssDxChTmGlueGeneralPortDropOutPckEnGet",
        &wrCpssDxChTmGlueGeneralPerPortDropOutgoingPacketEnableGet,
        2, 0},

    {"cpssDxChTmGlueAgingAndDelayDropAgedPckCntrCfgSet",
        &wrCpssDxChTmGlueAgingAndDelayAgedPacketCounterConfigSet,
        3, 0},
    {"cpssDxChTmGlueAgingAndDelayDropAgedPckCntrCfgGet",
        &wrCpssDxChTmGlueAgingAndDelayAgedPacketCounterConfigGet,
        1, 0},

    {"cpssDxChTmGlueAgingAndDelayDropAgedPckCounterGet",
        &wrCpssDxChTmGlueAgingAndDelayAgedPacketCounterGet,
        1, 0},

    {"cpssDxChTmGlueAgingAndDelayQueueTimerResolutionSet",
        &wrCpssDxChTmGlueAgingAndDelayTimerResolutionSet,
        2, 0},
    {"cpssDxChTmGlueAgingAndDelayQueueTimerResolutionGet",
        &wrCpssDxChTmGlueAgingAndDelayTimerResolutionGet,
        1, 0},

    {"cpssDxChTmGlueAgingAndDelayQueueTimerGet",
        &wrCpssDxChTmGlueAgingAndDelayTimerGet,
        1, 0},

    {"cpssDxChTmGlueAgingAndDelayProfileThresholdsSet",
        &wrCpssDxChTmGlueAgingAndDelayAgingProfileThresholdsSet,
        5, 0},
    {"cpssDxChTmGlueAgingAndDelayProfileThresholdsGet",
        &wrCpssDxChTmGlueAgingAndDelayAgingProfileThresholdsGet,
        2, 0},

    {"cpssDxChTmGlueAgingAndDelayAgingQueueProfileIdSet",
        &wrCpssDxChTmGlueAgingAndDelayQueueAgingProfileIdSet,
        3, 0},
    {"cpssDxChTmGlueAgingAndDelayAgingQueueProfileIdGet",
        &wrCpssDxChTmGlueAgingAndDelayQueueAgingProfileIdGet,
        2, 0},

    {"cpssDxChTmGlueAgingAndDelayStatisticAgeQueueIdSet",
        &wrCpssDxChTmGlueAgingAndDelayCounterQueueIdSet,
        3, 0},
    {"cpssDxChTmGlueAgingAndDelayStatisticAgeQueueIdGet",
        &wrCpssDxChTmGlueAgingAndDelayCounterQueueIdGet,
        2, 0},

    {"cpssDxChTmGlueAgingAndDelayStatisticAgePckCntrGet",
        &wrCpssDxChTmGlueAgingAndDelayCountersGet,
        2, 0},

    {"cpssDxChTmGlueAgingAndDelayPortAgedOutPckCntrGet",
        &wrCpssDxChTmGlueAgingAndDelayPerPortAgedPacketCounterGet,
        2, 0}
};

#define numCommands (sizeof(dbCommands) / sizeof(CMD_COMMAND))

/**
* @internal cmdLibInitCpssDxChTmGlueAgingAndDelay function
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
GT_STATUS cmdLibInitCpssDxChTmGlueAgingAndDelay
(
GT_VOID
)
{
    return cmdInitLibrary(dbCommands, numCommands);
}

