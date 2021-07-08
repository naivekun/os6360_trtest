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
* @file wrapCpssDxChHwInit.c
* @version   8
********************************************************************************
*/

/* Common galtis includes */
#include <galtisAgent/wrapUtil/cmdCpssPresteraUtils.h>
#include <cmdShell/cmdDb/cmdBase.h>
#include <cmdShell/common/cmdWrapUtils.h>

/* Feature specific includes. */
#include <cpss/common/cpssTypes.h>
#include <cpss/dxCh/dxChxGen/cpssHwInit/cpssDxChHwInit.h>
#include <cpss/dxCh/dxChxGen/cpssHwInit/private/prvCpssDxChHwInitSoftReset.h>

/**
* @internal wrCpssDxChHwPpSoftResetTrigger function
* @endinternal
*
* @brief   This routine issue soft reset for a specific pp.
*
* @note   APPLICABLE DEVICES:      All DxCh Devices
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum
*
* @note The soft reset is the
*
*/
static CMD_STATUS wrCpssDxChHwPpSoftResetTrigger
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
    result = cpssDxChHwPpSoftResetTrigger(devNum);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}


/**
* @internal wrCpssDxChHwPpSoftResetSkipParamSet function
* @endinternal
*
* @brief   This routine configure if to skip parameters related to soft reset.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum, or skipType
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChHwPpSoftResetSkipParamSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8 devNum;
    CPSS_HW_PP_RESET_SKIP_TYPE_ENT skipType;
    GT_BOOL skipEnable;


    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */

    devNum = (GT_U8)inArgs[0];
    skipType = (CPSS_HW_PP_RESET_SKIP_TYPE_ENT)inArgs[1];
    skipEnable = (GT_BOOL)inArgs[2];

    /* call cpss api function */
    result = cpssDxChHwPpSoftResetSkipParamSet(devNum,skipType, skipEnable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxChHwPpSoftResetSkipParamGet function
* @endinternal
*
* @brief   This routine return configuration of skip parameters related to soft reset.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum, or skipType
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
*/
static CMD_STATUS wrCpssDxChHwPpSoftResetSkipParamGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                       result;
    GT_U8                           devNum;
    CPSS_HW_PP_RESET_SKIP_TYPE_ENT  skipType;
    GT_BOOL                         skipEnable;


    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */

    devNum = (GT_U8)inArgs[0];
    skipType = (CPSS_HW_PP_RESET_SKIP_TYPE_ENT)inArgs[1];

    /* call cpss api function */
    result = cpssDxChHwPpSoftResetSkipParamGet(devNum,skipType, &skipEnable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", skipEnable);


    return CMD_OK;
}

/**
* @internal wrCpssDxChHwPpInitStageGet function
* @endinternal
*
* @brief   Indicates the initialization stage of the device.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum, or skipType
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
*/
static CMD_STATUS wrCpssDxChHwPpInitStageGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                       result;
    GT_U8                           devNum;
    CPSS_HW_PP_INIT_STAGE_ENT       initStage;


    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */

    devNum = (GT_U8)inArgs[0];

    /* call cpss api function */
    result = cpssDxChHwPpInitStageGet(devNum,&initStage);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", initStage);

    return CMD_OK;
}

/**
* @internal wrCpssDxChHwInterruptCoalescingSet function
* @endinternal
*
* @brief   Configures the interrupt coalescing parameters and enable\disable the
*         functionality.
*
* @note   APPLICABLE DEVICES:      All DxCh devices
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_OUT_OF_RANGE          - parameter out of range
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChHwInterruptCoalescingSet
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
    GT_U32      period;
    GT_BOOL     linkChangeOverride;


    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    enable = (GT_BOOL)inArgs[1];
    period = (GT_U32)inArgs[2];
    linkChangeOverride = (GT_BOOL)inArgs[3];

    /* call cpss api function */
    result = cpssDxChHwInterruptCoalescingSet(devNum, enable, period, linkChangeOverride);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}


/**
* @internal wrCpssDxChHwInterruptCoalescingGet function
* @endinternal
*
* @brief   Gets the interrupt coalescing configuration parameters.
*
* @note   APPLICABLE DEVICES:      All DxCh devices
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_OUT_OF_RANGE          - parameter out of range
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The HW resolusion for the interrupt coalescing period is 0.32uSec.
*
*/
static CMD_STATUS wrCpssDxChHwInterruptCoalescingGet
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
    GT_U32      period;
    GT_BOOL     linkChangeOverride;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];

    /* call cpss api function */
    result = cpssDxChHwInterruptCoalescingGet(devNum, &enable, &period,
                                                           &linkChangeOverride);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d%d%d", enable, period,
                                                linkChangeOverride);

    return CMD_OK;
}

/**
* @internal wrCpssDxChHwAuDescSizeGet function
* @endinternal
*
* @brief   This function returns the size in bytes of a single Address Update
*         descriptor, for a given device.
*
* @note   APPLICABLE DEVICES:      All DxCh Devices
*
* @retval GT_OK                    - on success,
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChHwAuDescSizeGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    CPSS_PP_DEVICE_TYPE devType;
    GT_U32 descSize;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devType = (CPSS_PP_DEVICE_TYPE)inArgs[0];

    /* call cpss api function */
    result = cpssDxChHwAuDescSizeGet(devType, &descSize);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxChHwCoreClockGet function
* @endinternal
*
* @brief   This function returns the core clock value from cpss DB and from HW.
*
* @note   APPLICABLE DEVICES:      All DxCh Devices
*
* @retval GT_OK                    - on success,
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChHwCoreClockGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8 devNum;
    GT_U32 coreClkDb;
    GT_U32 coreClkHw;


    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];

    /* call cpss api function */
    result = cpssDxChHwCoreClockGet(devNum, &coreClkDb, &coreClkHw);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d%d", coreClkDb, coreClkHw);

    return CMD_OK;
}

/**
* @internal wrCpssDxChHwRxBufAlignmentGet function
* @endinternal
*
* @brief   This function returns the required alignment in bytes of a RX buffer,
*         for a given device.
*
* @note   APPLICABLE DEVICES:      All DxCh Devices
*
* @retval GT_OK                    - on success,
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChHwRxBufAlignmentGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    CPSS_PP_DEVICE_TYPE devType;
    GT_U32 byteAlignment;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devType = (CPSS_PP_DEVICE_TYPE)inArgs[0];

    /* call cpss api function */
    result = cpssDxChHwRxBufAlignmentGet(devType, &byteAlignment);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxChHwRxDescSizeGet function
* @endinternal
*
* @brief   This function returns the size in bytes of a single Rx descriptor,
*         for a given device.
*
* @note   APPLICABLE DEVICES:      All DxCh Devices
*
* @retval GT_OK                    - on success,
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChHwRxDescSizeGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    CPSS_PP_DEVICE_TYPE devType;
    GT_U32 descSize;


    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devType = (CPSS_PP_DEVICE_TYPE)inArgs[0];

    /* call cpss api function */
    result = cpssDxChHwRxDescSizeGet(devType, &descSize);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxChHwTxDescSizeGet function
* @endinternal
*
* @brief   This function returns the size in bytes of a single Tx descriptor,
*         for a given device.
*
* @note   APPLICABLE DEVICES:      All DxCh Devices
*
* @retval GT_OK                    - on success,
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChHwTxDescSizeGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    CPSS_PP_DEVICE_TYPE devType;
    GT_U32 descSize;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devType = (CPSS_PP_DEVICE_TYPE)inArgs[0];

    /* call cpss api function */
    result = cpssDxChHwTxDescSizeGet(devType, &descSize);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxChCfgHwDevNumSet function
* @endinternal
*
* @brief   write HW device number
*
* @note   APPLICABLE DEVICES:      All DxCh Devices
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on bad device
* @retval GT_OUT_OF_RANGE          - on hwDevNum > 31
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChCfgHwDevNumSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;

    GT_U8           devNum;
    GT_HW_DEV_NUM   hwDevNum;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    hwDevNum = (GT_HW_DEV_NUM)inArgs[1];

    /* call cpss api function */
    result = cpssDxChCfgHwDevNumSet(devNum, hwDevNum);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}

/**
* @internal wrCpssDxChCfgHwDevNumGet function
* @endinternal
*
* @brief   Read HW device number
*
* @note   APPLICABLE DEVICES:      All DxCh Devices
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on bad device
* @retval GT_BAD_PTR               - one of the parameters in NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChCfgHwDevNumGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS       result;
    GT_U8           devNum;
    GT_HW_DEV_NUM   hwDevNum;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];

    /* call cpss api function */
    result = cpssDxChCfgHwDevNumGet(devNum, &hwDevNum);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", hwDevNum);

    return CMD_OK;

}

/**
* @internal wrCpssDxChGpioPhyConfigSet function
* @endinternal
*
* @brief   Set per-PHY GPIO configuration.
*
* @note   APPLICABLE DEVICES:      AC3X.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; xCat3; Lion2; Bobcat2; Caelum; Aldrin; Bobcat3; Aldrin2; Falcon.
*
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - one of the parameters value is wrong
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_FAIL                  - otherwise
*
* @note Applicable only to remote ports on AC3X.
*
*/
static CMD_STATUS wrCpssDxChGpioPhyConfigSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS   result;
    GT_U8       devNum;
    GT_PHYSICAL_PORT_NUM    portNum;
    GT_U32      directionBitmap;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    portNum = (GT_PHYSICAL_PORT_NUM)inArgs[1];
    directionBitmap = (GT_BOOL)inArgs[2];

    /* Override Device and Port */
    CONVERT_DEV_PHYSICAL_PORT_MAC(devNum, portNum);
    /* call cpss api function */
    result = cpssDxChGpioPhyConfigSet(devNum, portNum, directionBitmap);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxChGpioPhyConfigGet function
* @endinternal
*
* @brief   Get per-PHY GPIO configuration.
*
* @note   APPLICABLE DEVICES:      AC3X.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; xCat3; Lion2; Bobcat2; Caelum; Aldrin; Bobcat3; Aldrin2; Falcon.
*
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - one of the parameters value is wrong
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_FAIL                  - otherwise
*
* @note Applicable only to remote ports on AC3X.
*
*/
static CMD_STATUS wrCpssDxChGpioPhyConfigGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS   result;
    GT_U8       devNum;
    GT_PHYSICAL_PORT_NUM    portNum;
    GT_U32      modeBitmap;
    GT_U32      directionBitmap;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    portNum = (GT_PHYSICAL_PORT_NUM)inArgs[1];

    /* Override Device and Port */
    CONVERT_DEV_PHYSICAL_PORT_MAC(devNum, portNum);
    /* call cpss api function */
    result = cpssDxChGpioPhyConfigGet(devNum, portNum, &modeBitmap, &directionBitmap);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d%d",
                    modeBitmap,
                    directionBitmap);

    return CMD_OK;
}

/**
* @internal wrCpssDxChGpioPhyDataRead function
* @endinternal
*
* @brief   Read per-PHY input GPIO data.
*
* @note   APPLICABLE DEVICES:      AC3X.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; xCat3; Lion2; Bobcat2; Caelum; Aldrin; Bobcat3; Aldrin2; Falcon.
*
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - one of the parameters value is wrong
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_FAIL                  - otherwise
*
* @note Applicable only to remote ports on AC3X.
*
*/
static CMD_STATUS wrCpssDxChGpioPhyDataRead
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS   result;
    GT_U8       devNum;
    GT_PHYSICAL_PORT_NUM    portNum;
    GT_U32      dataBitmap;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    portNum = (GT_PHYSICAL_PORT_NUM)inArgs[1];

    /* Override Device and Port */
    CONVERT_DEV_PHYSICAL_PORT_MAC(devNum, portNum);
    /* call cpss api function */
    result = cpssDxChGpioPhyDataRead(devNum, portNum, &dataBitmap);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d",
                    dataBitmap);

    return CMD_OK;
}

/**
* @internal wrCpssDxChGpioPhyDataWrite function
* @endinternal
*
* @brief   Write per-PHY output GPIO data.
*
* @note   APPLICABLE DEVICES:      AC3X.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; xCat3; Lion2; Bobcat2; Caelum; Aldrin; Bobcat3; Aldrin2; Falcon.
*
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - one of the parameters value is wrong
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_FAIL                  - otherwise
*
* @note Applicable only to remote ports on AC3X.
*
*/
static CMD_STATUS wrCpssDxChGpioPhyDataWrite
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS   result;
    GT_U8       devNum;
    GT_PHYSICAL_PORT_NUM    portNum;
    GT_U32      dataBitmap;
    GT_U32      dataBitmapMask;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    portNum = (GT_PHYSICAL_PORT_NUM)inArgs[1];
    dataBitmap = (GT_U32)inArgs[2];
    dataBitmapMask = (GT_U32)inArgs[3];

    /* Override Device and Port */
    CONVERT_DEV_PHYSICAL_PORT_MAC(devNum, portNum);
    /* call cpss api function */
    result = cpssDxChGpioPhyDataWrite(devNum, portNum, dataBitmap, dataBitmapMask);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**/

/**
* @internal prvCpssDxChHwInitSoftResetSip6SkipInitMatrixWriteBitMask function
* @endinternal
*
* @brief   Write bit mask to given Skip Init Matrix register instance
*          in given Falcon Tile or in given Raven.
*          .
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                - device number
* @param[in] dfxType               - dfx Type - core or GOP
* @param[in] tileIndex             - index of tile 0..3
* @param[in] gopIndex              - index of GOP 0..3 - for GOP connected to the given tile.
*                                    relevant only when dfx Type is GOP
* @param[in] skipType              - the type of Skip Initialization Marix
*                                    see comments in CPSS_HW_PP_RESET_SKIP_TYPE_ENT.
* @param[in] mask                  - Mask for selecting the written bits.
* @param[in] value                 - Data to be written to register.
*
* @retval GT_OK                    - on success,
* @retval GT_BAD_PARAM             - wrong devNum, or tileIndex
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_FAIL                  - on hardware error
* @retval GT_NOT_INITIALIZED       - if the driver was not initialized
* @retval GT_HW_ERROR              - on hardware error
*/
static CMD_STATUS wrPrvCpssDxChHwInitSoftResetSip6SkipInitMatrixWriteBitMask
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                                      result;
    GT_U8                                          devNum;
    PRV_CPSS_DXCH_SOFT_RESET_DFX_TYPE_ENT          dfxType;
    GT_U32                                         tileIndex;
    GT_U32                                         gopIndex;
    PRV_CPSS_DXCH_SOFT_RESET_SKIP_INIT_MATRIX_ENT  skipType;
    GT_U32                                         mask;
    GT_U32                                         data;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */

    devNum    = (GT_U8                                        )inArgs[0];
    dfxType   = (PRV_CPSS_DXCH_SOFT_RESET_DFX_TYPE_ENT        )inArgs[1];
    tileIndex = (GT_U32                                       )inArgs[2];
    gopIndex  = (GT_U32                                       )inArgs[3];
    skipType  = (PRV_CPSS_DXCH_SOFT_RESET_SKIP_INIT_MATRIX_ENT)inArgs[4];
    mask      = (GT_U32                                       )inArgs[5];
    data      = (GT_U32                                       )inArgs[6];

    /* call cpss api function */
    result = prvCpssDxChHwInitSoftResetSip6SkipInitMatrixWriteBitMask(
        devNum, dfxType, tileIndex, gopIndex, skipType, mask, data);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal prvCpssDxChHwInitSoftResetSip6SkipInitMatrixRead function
* @endinternal
*
* @brief   Read given Skip Init Matrix register instance
*          in given Falcon Tile or in given Raven.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                - device number
* @param[in] dfxType               - dfx Type - core or GOP
* @param[in] tileIndex             - index of tile 0..3
* @param[in] gopIndex              - index of GOP 0..3 - for GOP connected to the given tile.
*                                    relevant only when dfx Type is GOP
* @param[in] skipType              - the type of Skip Initialization Marix
*                                    see comments in CPSS_HW_PP_RESET_SKIP_TYPE_ENT.
* @param[out] dataPtr              - pointer to data read from register
*
* @retval GT_OK                    - on success,
* @retval GT_BAD_PARAM             - wrong devNum, or tileIndex
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_FAIL                  - on hardware error
* @retval GT_NOT_INITIALIZED       - if the driver was not initialized
* @retval GT_HW_ERROR              - on hardware error
*/
static CMD_STATUS wrPrvCpssDxChHwInitSoftResetSip6SkipInitMatrixRead
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                                      result;
    GT_U8                                          devNum;
    PRV_CPSS_DXCH_SOFT_RESET_DFX_TYPE_ENT          dfxType;
    GT_U32                                         tileIndex;
    GT_U32                                         gopIndex;
    PRV_CPSS_DXCH_SOFT_RESET_SKIP_INIT_MATRIX_ENT  skipType;
    GT_U32                                         data;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */

    devNum    = (GT_U8                                        )inArgs[0];
    dfxType   = (PRV_CPSS_DXCH_SOFT_RESET_DFX_TYPE_ENT        )inArgs[1];
    tileIndex = (GT_U32                                       )inArgs[2];
    gopIndex  = (GT_U32                                       )inArgs[3];
    skipType  = (PRV_CPSS_DXCH_SOFT_RESET_SKIP_INIT_MATRIX_ENT)inArgs[4];

    /* call cpss api function */
    result = prvCpssDxChHwInitSoftResetSip6SkipInitMatrixRead(
        devNum, dfxType, tileIndex, gopIndex, skipType, &data);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", data);

    return CMD_OK;
}

/**
* @internal prvCpssDxChHwInitSoftResetSip6ResetTriggerRegWriteBitMask function
* @endinternal
*
* @brief   Write bit mask to reset trigger register instance
*          in given Falcon Tile or in given Raven.
*          .
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                - device number
* @param[in] dfxType               - dfx Type - core or GOP
* @param[in] tileIndex             - index of tile 0..3
* @param[in] gopIndex              - index of GOP 0..3 - for GOP connected to the given tile.
*                                    relevant only when dfx Type is GOP
* @param[in] mask                  - Mask for selecting the written bits.
* @param[in] value                 - Data to be written to register.
*
* @retval GT_OK                    - on success,
* @retval GT_BAD_PARAM             - wrong devNum, or tileIndex
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_FAIL                  - on hardware error
* @retval GT_NOT_INITIALIZED       - if the driver was not initialized
* @retval GT_HW_ERROR              - on hardware error
*/
static CMD_STATUS wrPrvCpssDxChHwInitSoftResetSip6ResetTriggerRegWriteBitMask
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                                      result;
    GT_U8                                          devNum;
    PRV_CPSS_DXCH_SOFT_RESET_DFX_TYPE_ENT          dfxType;
    GT_U32                                         tileIndex;
    GT_U32                                         gopIndex;
    GT_U32                                         mask;
    GT_U32                                         data;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */

    devNum    = (GT_U8                                )inArgs[0];
    dfxType   = (PRV_CPSS_DXCH_SOFT_RESET_DFX_TYPE_ENT)inArgs[1];
    tileIndex = (GT_U32                               )inArgs[2];
    gopIndex  = (GT_U32                               )inArgs[3];
    mask      = (GT_U32                               )inArgs[4];
    data      = (GT_U32                               )inArgs[5];

    /* call cpss api function */
    result = prvCpssDxChHwInitSoftResetSip6ResetTriggerRegWriteBitMask(
        devNum, dfxType, tileIndex, gopIndex, mask, data);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal prvCpssDxChHwInitSoftResetSip6CnmSystemResetTrigger function
* @endinternal
*
* @brief   Trigger Falcon CNM System Soff Reset.
*          .
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                - device number
*
* @retval GT_OK                    - on success,
* @retval GT_BAD_PARAM             - wrong devNum, or tileIndex
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_FAIL                  - on hardware error
* @retval GT_NOT_INITIALIZED       - if the driver was not initialized
* @retval GT_HW_ERROR              - on hardware error
*/
static CMD_STATUS wrPrvCpssDxChHwInitSoftResetSip6CnmSystemResetTrigger
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                                      result;
    GT_U8                                          devNum;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */

    devNum    = (GT_U8                                )inArgs[0];

    /* call cpss api function */
    result = prvCpssDxChHwInitSoftResetSip6CnmSystemResetTrigger(devNum);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/*********************************************/

/**
* @internal cpssDxChHwPpSoftResetChipletsTrigger function
* @endinternal
*
* @brief   This routine issue soft reset for a set of chiplets.
* @brief   Does not reset Main Dies.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - device number.
* @param[in] chipletsBmp              - bitmap of chiplet indexes to reset.
*                                       CPSS_CHIPLETS_UNAWARE_MODE_CNS for all chiplets of the device
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Soft reset is the process in which selected entities (as configured by
*       using the cpssDxChHwPpSoftResetChipletsSkipParamSet API) are reset to their
*       default values.
*
*/
static CMD_STATUS wrCpssDxChHwPpSoftResetChipletsTrigger
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS               result;
    GT_U8                   devNum;
    GT_CHIPLETS_BMP         chipletsBmp;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum      = (GT_U8)inArgs[0];
    chipletsBmp = (GT_CHIPLETS_BMP)inArgs[1];

    /* call cpss api function */
    result = cpssDxChHwPpSoftResetChipletsTrigger(
        devNum, chipletsBmp);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}

/**
* @internal cpssDxChHwPpSoftResetChipletsSkipParamSet function
* @endinternal
*
* @brief   This routine configure skip parameters related to soft reset of set of chiplets.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - device number
* @param[in] chipletsBmp              - bitmap of chiplet indexes to reset.
*                                       CPSS_CHIPLETS_UNAWARE_MODE_CNS for all chiplets of the device
* @param[in] skipType                 - the skip parameter to set
*                                      see comments in CPSS_HW_PP_RESET_SKIP_TYPE_ENT.
* @param[in] skipEnable               - GT_FALSE: Do Not Skip
*                                       GT_TRUE:  Skip
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum, or skipType
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChHwPpSoftResetChipletsSkipParamSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                       result;
    GT_U8                           devNum;
    GT_CHIPLETS_BMP                 chipletsBmp;
    CPSS_HW_PP_RESET_SKIP_TYPE_ENT  skipType;
    GT_BOOL                         skipEnable;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum      = (GT_U8)inArgs[0];
    chipletsBmp = (GT_CHIPLETS_BMP)inArgs[1];
    skipType    = (CPSS_HW_PP_RESET_SKIP_TYPE_ENT)inArgs[2];
    skipEnable  = (GT_BOOL)inArgs[3];

    /* call cpss api function */
    result = cpssDxChHwPpSoftResetChipletsSkipParamSet(
        devNum, chipletsBmp, skipType, skipEnable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}

/**
* @internal cpssDxChHwPpSoftResetChipletsSkipParamGet function
* @endinternal
*
* @brief   This routine get skip parameters related to soft reset of chiplet.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - device number
* @param[in] chipletsBmp              - bitmap of chiplet indexes to set skip reset state,
*                                       CPSS_CHIPLETS_UNAWARE_MODE_CNS for all chiplets of the device.
* @param[in] skipType                 - the skip parameter to get.
*                                      see comments in CPSS_HW_PP_RESET_SKIP_TYPE_ENT.
* @param[out] skipEnablePtr           - pointer to GT_FALSE: Do Not Skip
*                                                  GT_TRUE:  Skip
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum, or skipType
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
static CMD_STATUS wrCpssDxChHwPpSoftResetChipletsSkipParamGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                       result;
    GT_U8                           devNum;
    GT_CHIPLETS_BMP                 chipletsBmp;
    CPSS_HW_PP_RESET_SKIP_TYPE_ENT  skipType;
    GT_BOOL                         skipEnable;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum      = (GT_U8)inArgs[0];
    chipletsBmp = (GT_CHIPLETS_BMP)inArgs[1];
    skipType    = (CPSS_HW_PP_RESET_SKIP_TYPE_ENT)inArgs[2];

    /* call cpss api function */
    result = cpssDxChHwPpSoftResetChipletsSkipParamGet(
        devNum, chipletsBmp, skipType, &skipEnable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", skipEnable);
    return CMD_OK;
}

/**** database initialization **************************************/

static CMD_COMMAND dbCommands[] =
{
    {"cpssDxChHwPpSoftResetTrigger",
         &wrCpssDxChHwPpSoftResetTrigger,
         1, 0},
    {"cpssDxChHwPpSoftResetSkipParamSet",
         &wrCpssDxChHwPpSoftResetSkipParamSet,
         3, 0},
    {"cpssDxChHwPpSoftResetSkipParamGet",
         &wrCpssDxChHwPpSoftResetSkipParamGet,
         2, 0},
    {"cpssDxChHwPpInitStageGet",
         &wrCpssDxChHwPpInitStageGet,
         1, 0},
    {"cpssDxChHwInterruptCoalescingSet",
         &wrCpssDxChHwInterruptCoalescingSet,
         4, 0},
    {"cpssDxChHwInterruptCoalescingGet",
         &wrCpssDxChHwInterruptCoalescingGet,
         1, 0},
    {"cpssDxChHwAuDescSizeGet",
         &wrCpssDxChHwAuDescSizeGet,
         1, 0},
    {"cpssDxChHwCoreClockGet",
         &wrCpssDxChHwCoreClockGet,
         1, 0},
    {"cpssDxChHwRxBufAlignmentGet",
         &wrCpssDxChHwRxBufAlignmentGet,
         1, 0},
    {"cpssDxChHwRxDescSizeGet",
         &wrCpssDxChHwRxDescSizeGet,
         1, 0},
    {"cpssDxChHwTxDescSizeGet",
         &wrCpssDxChHwTxDescSizeGet,
         1, 0},
    {"cpssDxChCfgHwDevNumSet",
        &wrCpssDxChCfgHwDevNumSet,
        2, 0},
    {"cpssDxChCfgHwDevNumGet",
        &wrCpssDxChCfgHwDevNumGet,
        1, 0},
    {"cpssDxChGpioPhyConfigSet",
        &wrCpssDxChGpioPhyConfigSet,
        3, 0},
    {"cpssDxChGpioPhyConfigGet",
        &wrCpssDxChGpioPhyConfigGet,
        2, 0},
    {"cpssDxChGpioPhyDataRead",
        &wrCpssDxChGpioPhyDataRead,
        2, 0},
    {"cpssDxChGpioPhyDataWrite",
        &wrCpssDxChGpioPhyDataWrite,
        4, 0},
    {"prvDxChHwPpSoftResetSip6SkipInitMatrixWrBitMask",
        &wrPrvCpssDxChHwInitSoftResetSip6SkipInitMatrixWriteBitMask,
        7, 0},
    {"prvDxChHwPpSoftResetSip6SkipInitMatrixRead",
        &wrPrvCpssDxChHwInitSoftResetSip6SkipInitMatrixRead,
        5, 0},
    {"prvDxChHwPpSoftResetSip6ResetTriggerRegWrBitMask",
        &wrPrvCpssDxChHwInitSoftResetSip6ResetTriggerRegWriteBitMask,
        6, 0},
    {"prvDxChHwPpSoftResetSip6CnmSystemResetTrigger",
        &wrPrvCpssDxChHwInitSoftResetSip6CnmSystemResetTrigger,
        1, 0},
    {"cpssDxChHwPpSoftResetChipletsTrigger",
         &wrCpssDxChHwPpSoftResetChipletsTrigger,
         2, 0},
    {"cpssDxChHwPpSoftResetChipletsSkipParamSet",
         &wrCpssDxChHwPpSoftResetChipletsSkipParamSet,
         4, 0},
    {"cpssDxChHwPpSoftResetChipletsSkipParamGet",
         &wrCpssDxChHwPpSoftResetChipletsSkipParamGet,
         3, 0}
};

#define numCommands (sizeof(dbCommands) / sizeof(CMD_COMMAND))

/**
* @internal cmdLibInitCpssDxChHwInit function
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
GT_STATUS cmdLibInitCpssDxChHwInit
(
    GT_VOID
)
{
    return cmdInitLibrary(dbCommands, numCommands);
}


