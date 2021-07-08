/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
* wrapPhyCpssDxCh.c
*
* DESCRIPTION:
*       Wrapper functions for Phy cpss.dxCh functions
*
* FILE REVISION NUMBER:
*       $Revision: 103 $
*
*******************************************************************************/

/* Common galtis includes */
#include <galtisAgent/wrapUtil/cmdCpssPresteraUtils.h>
#include <cmdShell/cmdDb/cmdBase.h>
#include <cmdShell/common/cmdWrapUtils.h>

/* Feature specific includes. */
#include <cpss/dxCh/dxChxGen/port/cpssDxChPortCtrl.h>
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>

/**
* @internal wrCpssDxChPortSerdesManualTxConfigSet function
* @endinternal
*
* @brief   Configure specific parameters of serdes TX in HW.
*
* @note   APPLICABLE DEVICES:      Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2.
*
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - if serdes was not intialized
* @retval GT_OUT_OF_RANGE          - parameter out of range
*
* @note Pay attention: every serdes power up restore CPSS default configurations,
*       so this API should be egaged by application after every serdes power up
*       or use cpssDxChPortSerdesTuningSet.
*
*/
CMD_STATUS wrCpssDxChPortSerdesManualTxConfigSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS               result;
    GT_U8                   devNum;
    GT_U8                   portNum;
    GT_U32                  laneNum;
    CPSS_DXCH_PORT_SERDES_TX_CONFIG_STC  serdesCfg;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum          = (GT_U8)inArgs[0];
    portNum         = (GT_U8)inArgs[1];
    laneNum         = (GT_U32)inArgs[2];

    serdesCfg.txAmp = (GT_U32)inArgs[3];
    if(PRV_CPSS_SIP_5_15_CHECK_MAC(devNum))
    {
        serdesCfg.emph0 = inArgs[4];
        serdesCfg.emph1 = inArgs[5];
        serdesCfg.txAmpAdjEn = GT_FALSE;
        serdesCfg.txAmpShft = GT_FALSE;
    }
    else
    {
        serdesCfg.txAmpAdjEn = (GT_BOOL)inArgs[4];
        serdesCfg.emph0 = (GT_32)inArgs[5];
        serdesCfg.emph1 = (GT_32)inArgs[6];
        serdesCfg.txAmpShft = (GT_BOOL)inArgs[7];
    }
    /* Override Device and Port */
    CONVERT_DEV_PORT_MAC(devNum, portNum);

    /* call cpss api function */
    result = cpssDxChPortSerdesManualTxConfigSet(devNum, portNum, laneNum,
                                                 &serdesCfg);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxChPortSerdesManualRxConfigSet function
* @endinternal
*
* @brief   Configure specific parameters of serdes RX in HW.
*
* @note   APPLICABLE DEVICES:      Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2.
*
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - if serdes was not intialized
* @retval GT_OUT_OF_RANGE          - parameter out of range
*
* @note Pay attention: every serdes power up restore CPSS default configurations,
*       so this API should be egaged by application after every serdes power up
*       or use cpssDxChPortSerdesTuningSet.
*
*/
CMD_STATUS wrCpssDxChPortSerdesManualRxConfigSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS               result;
    GT_U8                   devNum;
    GT_U8                   portNum;
    GT_U32                  laneNum;
    CPSS_DXCH_PORT_SERDES_RX_CONFIG_STC  serdesCfg;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum          = (GT_U8)inArgs[0];
    portNum         = (GT_U8)inArgs[1];
    laneNum         = (GT_U32)inArgs[2];

    serdesCfg.sqlch = (GT_U32)inArgs[3];
    serdesCfg.ffeRes = (GT_U32)inArgs[4];
    serdesCfg.ffeCap = (GT_U32)inArgs[5];
    if(PRV_CPSS_SIP_5_15_CHECK_MAC(devNum))
    {
        serdesCfg.dcGain = (GT_U32)inArgs[6];
        serdesCfg.bandWidth = (GT_U32)inArgs[7];
        serdesCfg.loopBandwidth = (GT_U32)inArgs[8];
    }
    else
    {
        /* inArgs[6] - dfeEn obsolete */
        serdesCfg.align90 = (GT_U32)inArgs[7];
    }
    /* Override Device and Port */
    CONVERT_DEV_PORT_MAC(devNum, portNum);

    /* call cpss api function */
    result = cpssDxChPortSerdesManualRxConfigSet(devNum, portNum, laneNum,
                                                 &serdesCfg);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxChPortSerdesManualRxConfigGet function
* @endinternal
*
* @brief   Gets and shows configured specific parameters of serdes RX in HW.
*
* @note   APPLICABLE DEVICES:      Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Lion2; Bobcat2
*
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - if serdes was not intialized
* @retval GT_OUT_OF_RANGE          - parameter out of range
*/
CMD_STATUS wrCpssDxChPortSerdesManualRxConfigGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS               result;
    GT_U8                   devNum;
    GT_U8                   portNum;
    GT_U32                  laneNum;
    CPSS_DXCH_PORT_SERDES_RX_CONFIG_STC  serdesRxCfg;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum          = (GT_U8)inArgs[0];
    portNum         = (GT_U8)inArgs[1];
    laneNum         = (GT_U32)inArgs[2];

    /* Override Device and Port */
    CONVERT_DEV_PORT_MAC(devNum, portNum);

    result = cpssDxChPortSerdesManualRxConfigGet( devNum, portNum, laneNum, &serdesRxCfg);

    galtisOutput(outArgs, result, "%d%d%d%d%d%d",
        serdesRxCfg.sqlch, serdesRxCfg.ffeRes, serdesRxCfg.ffeCap,serdesRxCfg.dcGain, serdesRxCfg.bandWidth, serdesRxCfg.loopBandwidth);
    return CMD_OK;
}

/**
* @internal wrCpssDxChPortSerdesManualTxConfigGet function
* @endinternal
*
* @brief   Gets and shows configured specific parameters of serdes TX in HW.
*
* @note   APPLICABLE DEVICES:      Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Lion2; Bobcat2
*
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - if serdes was not intialized
* @retval GT_OUT_OF_RANGE          - parameter out of range
*/
CMD_STATUS wrCpssDxChPortSerdesManualTxConfigGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS               result;
    GT_U8                   devNum;
    GT_U8                   portNum;
    GT_U32                  laneNum;
    CPSS_DXCH_PORT_SERDES_TX_CONFIG_STC  serdesTxCfg;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum          = (GT_U8)inArgs[0];
    portNum         = (GT_U8)inArgs[1];
    laneNum         = (GT_U32)inArgs[2];

    /* Override Device and Port */
    CONVERT_DEV_PORT_MAC(devNum, portNum);

    result = cpssDxChPortSerdesManualTxConfigGet( devNum, portNum, laneNum, &serdesTxCfg);

    galtisOutput(outArgs, result, "%d%d%d",serdesTxCfg.txAmp, serdesTxCfg.emph0, serdesTxCfg.emph1);
    return CMD_OK;
}

/**
* @internal wrCpssDxChPortFecModeSet function
* @endinternal
*
* @brief   Configure Forward error correction (FEC) mode of port
*
* @note   APPLICABLE DEVICES:      Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2.
*
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameters
* @retval GT_NOT_INITIALIZED       - if port not configured
* @retval GT_NOT_SUPPORTED         - if FEC not supported on interface mode defined now on port
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
CMD_STATUS wrCpssDxChPortFecModeSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8 devNum;
    GT_U8 portNum;
    CPSS_DXCH_PORT_FEC_MODE_ENT mode;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */

    devNum = (GT_U8)inArgs[0];
    portNum = (GT_U8)inArgs[1];
    mode = (CPSS_DXCH_PORT_FEC_MODE_ENT)inArgs[2];

    /* Override Device and Port */
    CONVERT_DEV_PORT_MAC(devNum, portNum);

    /* call cpss api function */
    result = cpssDxChPortFecModeSet(devNum, portNum, mode);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxChPortFecModeGet function
* @endinternal
*
* @brief   Read current Forward error correction (FEC) mode of port
*
* @note   APPLICABLE DEVICES:      Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2.
*
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameters
* @retval GT_NOT_INITIALIZED       - if port not configured
* @retval GT_BAD_PTR               - modePtr == NULL
* @retval GT_NOT_SUPPORTED         - if FEC not supported on interface mode defined now on port
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
CMD_STATUS wrCpssDxChPortFecModeGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                   result = GT_OK;
    GT_U8                       devNum;
    GT_U8                       portNum;
    CPSS_DXCH_PORT_FEC_MODE_ENT mode = CPSS_DXCH_PORT_FEC_MODE_DISABLED_E;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    portNum = (GT_U8)inArgs[1];

    /* Override Device and Port */
    CONVERT_DEV_PORT_MAC(devNum, portNum);

    /* call cpss api function */
    result = cpssDxChPortFecModeGet(devNum, portNum, &mode);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", mode);
    return CMD_OK;
}

/**
* @internal wrCpssDxChPortPcsLoopbackModeSet function
* @endinternal
*
* @brief   Configure loopback of specific type on PCS.
*
* @note   APPLICABLE DEVICES:      Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2.
*
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameters
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
CMD_STATUS wrCpssDxChPortPcsLoopbackModeSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS   result;
    GT_U8       devNum;
    GT_U8       portNum;
    CPSS_DXCH_PORT_PCS_LOOPBACK_MODE_ENT    mode;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum  = (GT_U8)inArgs[0];
    portNum = (GT_U8)inArgs[1];
    mode = (CPSS_DXCH_PORT_PCS_LOOPBACK_MODE_ENT)inArgs[2];

    /* Override Device and Port */
    CONVERT_DEV_PORT_MAC(devNum, portNum);

    /* call cpss api function */
    result = cpssDxChPortPcsLoopbackModeSet(devNum, portNum, mode);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxChPortPcsLoopbackModeGet function
* @endinternal
*
* @brief   Configure loopback of specific type on PCS.
*
* @note   APPLICABLE DEVICES:      Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2.
*
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameters
* @retval GT_BAD_PTR               - modePtr == NULL
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
CMD_STATUS wrCpssDxChPortPcsLoopbackModeGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS   result;
    GT_U8       devNum;
    GT_U8       portNum;
    CPSS_DXCH_PORT_PCS_LOOPBACK_MODE_ENT mode;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum  = (GT_U8)inArgs[0];
    portNum = (GT_U8)inArgs[1];

    /* Override Device and Port */
    CONVERT_DEV_PORT_MAC(devNum, portNum);

    /* call cpss api function */
    result = cpssDxChPortPcsLoopbackModeGet(devNum, portNum, &mode);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", mode);

    return CMD_OK;
}


/**
* @internal wrCpssDxChPortExtendedModeEnableSet function
* @endinternal
*
* @brief   Define which GE and XG MAC ports 9 and 11 of every mini-GOP will use -
*         from local mini-GOP or extended from other mini-GOP
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note For now if application interested to implement GE and XG modes of ports 9
*       and 11 of every mini-GOP over extended MAC's, it can call this function
*       at init stage once for port 9 and once for port 11 and enough
*
*/
CMD_STATUS wrCpssDxChPortExtendedModeEnableSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS   result;
    GT_U8       devNum;
    GT_U8       portNum;
    GT_BOOL     enable;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum  = (GT_U8)inArgs[0];
    portNum = (GT_U8)inArgs[1];
    enable = (GT_BOOL)inArgs[2];

    /* Override Device and Port */
    CONVERT_DEV_PORT_MAC(devNum, portNum);

    /* call cpss api function */
    result = cpssDxChPortExtendedModeEnableSet(devNum, portNum, enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxChPortExtendedModeEnableGet function
* @endinternal
*
* @brief   Read which GE and XG MAC ports 9 and 11 of every mini-GOP will use -
*         from local mini-GOP or extended from other mini-GOP
*
* @note   APPLICABLE DEVICES:      Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_BAD_PTR               - enablePtr is NULL
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
CMD_STATUS wrCpssDxChPortExtendedModeEnableGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS   result;
    GT_U8       devNum;
    GT_U8       portNum;
    GT_BOOL     enable;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum  = (GT_U8)inArgs[0];
    portNum = (GT_U8)inArgs[1];

    /* Override Device and Port */
    CONVERT_DEV_PORT_MAC(devNum, portNum);

    /* call cpss api function */
    result = cpssDxChPortExtendedModeEnableGet(devNum, portNum, &enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", enable);

    return CMD_OK;
}

/**
* @internal wrCpssDxChPortSerdesLaneTuningSet function
* @endinternal
*
* @brief   SerDes lane fine tuning values set.
*
* @note   APPLICABLE DEVICES:      xCat; Lion; xCat2; xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameters
* @retval GT_OUT_OF_RANGE          - one of tune values is out of range
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_SUPPORTED         - HW does not support the requested operation
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Set tuning parameters for lane in SW DB, then
*       cpssDxChPortSerdesPowerStatusSet or cpssDxChPortModeSpeedSet will write
*       them to HW.
*
*/
CMD_STATUS wrCpssDxChPortSerdesLaneTuningSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS rc;
    GT_U8     devNum;
    GT_U32    portGroupNum;
    GT_U32    laneNum;
    CPSS_DXCH_PORT_SERDES_SPEED_ENT serdesFrequency;
    CPSS_DXCH_PORT_SERDES_TUNE_STC tuneValues = {0};

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum          = (GT_U8)inArgs[0];
    portGroupNum    = (GT_U32)inArgs[1];
    laneNum         = (GT_U32)inArgs[2];
    serdesFrequency = (CPSS_DXCH_PORT_SERDES_SPEED_ENT)inArgs[3];

    if(PRV_CPSS_SIP_5_15_CHECK_MAC(devNum))
    {
        tuneValues.BW        = (GT_U32)inFields[0];
        tuneValues.DC        = (GT_U32)inFields[1];
        tuneValues.HF        = (GT_U32)inFields[2];
        tuneValues.LB        = (GT_U32)inFields[3];
        tuneValues.LF        = (GT_U32)inFields[4];
        tuneValues.sqlch     = (GT_U32)inFields[5];
        tuneValues.txAmp     = (GT_U32)inFields[6];
        tuneValues.txEmph1   = inFields[7];
        tuneValues.txEmphAmp = inFields[8];
    }
    else
    {
        tuneValues.dfe      = (GT_U32)inFields[0];
        tuneValues.ffeC     = (GT_U32)inFields[1];
        tuneValues.ffeR     = (GT_U32)inFields[2];
        tuneValues.ffeS     = (GT_U32)inFields[3];
        tuneValues.sampler  = (GT_U32)inFields[4];
        tuneValues.sqlch    = (GT_U32)inFields[5];
        tuneValues.txAmp    = (GT_U32)inFields[6];
        tuneValues.txAmpAdj = (GT_U32)inFields[7];
        tuneValues.txEmphAmp = (GT_U32)inFields[8];
        tuneValues.txEmphEn = (GT_BOOL)inFields[9];

        if(PRV_CPSS_DXCH_LION2_B0_AND_ABOVE_CHECK_MAC(devNum))
        {
            GT_U32  i;

            tuneValues.txEmph1 = (GT_U32)inFields[10];
            tuneValues.align90 = (GT_U32)inFields[11];
            tuneValues.txEmphEn1 = (GT_BOOL)inFields[12];
            tuneValues.txAmpShft = (GT_BOOL)inFields[13];

            for(i = 0; i < CPSS_DXCH_PORT_DFE_VALUES_ARRAY_SIZE_CNS; i++)
            {
                tuneValues.dfeValsArray[i] = (GT_U32)inFields[14+i];
            }
        }
    }

    rc = cpssDxChPortSerdesLaneTuningSet(devNum,
                                         portGroupNum,
                                         laneNum,
                                         serdesFrequency,
                                         &tuneValues);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, rc, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxChPortSerdesLaneTuningGet function
* @endinternal
*
* @brief   Get SerDes lane fine tuning values.
*
* @note   APPLICABLE DEVICES:      xCat; Lion; xCat2; xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameters
* @retval GT_OUT_OF_RANGE          - one of tune values is out of range
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_SUPPORTED         - HW does not support the requested operation
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Get tuning parameters for lane from SW DB.
*
*/
CMD_STATUS wrCpssDxChPortSerdesLaneTuningGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS rc;
    GT_U8     devNum;
    GT_U32    portGroupNum;
    GT_U32    laneNum;
    CPSS_DXCH_PORT_SERDES_SPEED_ENT serdesFrequency;
    CPSS_DXCH_PORT_SERDES_TUNE_STC tuneValues;
    GT_U32  i;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum          = (GT_U8)inArgs[0];
    portGroupNum    = (GT_U32)inArgs[1];
    laneNum         = (GT_U32)inArgs[2];
    serdesFrequency = (CPSS_DXCH_PORT_SERDES_SPEED_ENT)inArgs[3];

    rc = cpssDxChPortSerdesLaneTuningGet(devNum, portGroupNum, laneNum,
                                         serdesFrequency,
                                         &tuneValues);
    if (rc != GT_OK)
    {
        galtisOutput(outArgs, rc, "cpssDxChPortSerdesTuningGet");
        return CMD_OK;
    }

    if(PRV_CPSS_SIP_5_15_CHECK_MAC(devNum))
    {
        inFields[0] = tuneValues.BW ;
        inFields[1] = tuneValues.DC;
        inFields[2] = tuneValues.HF;
        inFields[3] = tuneValues.LB;
        inFields[4] = tuneValues.LF;
        inFields[5] = tuneValues.sqlch;
        inFields[6] = tuneValues.txAmp;
        inFields[7] = tuneValues.txEmph1;
        inFields[8] = tuneValues.txEmphAmp;
        fieldOutput("%d%d%d%d%d%d%d%d%d", inFields[0], inFields[1], inFields[2],
                        inFields[3], inFields[4], inFields[5], inFields[6], inFields[7], inFields[8]);
    }
    else
    {
        inFields[0] = tuneValues.dfe;
        inFields[1] = tuneValues.ffeC;
        inFields[2] = tuneValues.ffeR;
        inFields[3] = tuneValues.ffeS;
        inFields[4] = tuneValues.sampler;
        inFields[5] = tuneValues.sqlch;
        inFields[6] = tuneValues.txAmp;
        inFields[7] = tuneValues.txAmpAdj;
        inFields[8] = tuneValues.txEmphAmp;
        inFields[9] = tuneValues.txEmphEn;

        if(PRV_CPSS_DXCH_LION2_B0_AND_ABOVE_CHECK_MAC(devNum))
        {
            inFields[10] = tuneValues.txEmph1;
            inFields[11] = tuneValues.align90;
            inFields[12] = tuneValues.txEmphEn1;
            inFields[13] = tuneValues.txAmpShft;

            for(i = 0; i < CPSS_DXCH_PORT_DFE_VALUES_ARRAY_SIZE_CNS; i++)
            {
                inFields[14+i] = tuneValues.dfeValsArray[i];
            }
        }
        else
        {
            for(i = 10; i < 14+CPSS_DXCH_PORT_DFE_VALUES_ARRAY_SIZE_CNS; i++)
            {
                inFields[i] = 0;
            }
        }

        /* pack and output table fields */
        fieldOutput("%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d", inFields[0], inFields[1], inFields[2],
                        inFields[3], inFields[4], inFields[5], inFields[6], inFields[7], inFields[8],
                        inFields[9], inFields[10], inFields[11], inFields[12], inFields[13], inFields[14], inFields[15],
                        inFields[16], inFields[17], inFields[18], inFields[19]);
    }
    galtisOutput(outArgs, rc, "%f");

    return CMD_OK;
}

/**
* @internal wrCpssDxChPortSerdesErrorInject function
* @endinternal
*
* @brief   Injects errors into the RX data, TX data or both
*
* @note   APPLICABLE DEVICES:      Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; Lion2; xCat3; xCat2; Bobcat2.
*
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error
* @retval GT_NOT_SUPPORTED         - HW does not support the requested operation
* @retval GT_INIT_ERROR            - on Hws initialization failre
*
* @note Debug function.
*
*/
CMD_STATUS wrCpssDxChPortSerdesErrorInject
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS   rc;
    GT_U8    devNum;
    GT_U32   portGroup;
    GT_U32   serdesNum;
    GT_U32   numOfBits;
    CPSS_PORT_DIRECTION_ENT  direction;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum  = (GT_U8)inArgs[0];
    portGroup = (GT_U32)inArgs[1];
    serdesNum = (GT_U32)inArgs[2];
    numOfBits = (GT_U32)inArgs[3];
    direction = (CPSS_PORT_DIRECTION_ENT)inArgs[4];

    rc = cpssDxChPortSerdesErrorInject(devNum, portGroup, serdesNum, numOfBits,direction);

    galtisOutput(outArgs, rc, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxChPortSerdesLanePolaritySet function
* @endinternal
*
* @brief   Set the SerDes lane polarity values.
*
* @note   APPLICABLE DEVICES:      Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; xCat3; Lion2; Bobcat2;
*
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameters
* @retval GT_OUT_OF_RANGE          - one of tune values is out of range
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_SUPPORTED         - HW does not support the requested operation
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note - If serdes initialized: Set the Tx/Rx polarity parameters for lane
*       in both HW and SW DB
*       - If serdes was not initialized: Set the Tx/Rx polarity parameters
*       for lane only in SW DB
*
*/
CMD_STATUS wrCpssDxChPortSerdesLanePolaritySet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS   rc;
    GT_U8       devNum;
    GT_U32      portGroupNum;
    GT_U32      laneNum;
    GT_BOOL     invertTx;
    GT_BOOL     invertRx;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum          = (GT_U8)inArgs[0];
    portGroupNum    = (GT_U32)inArgs[1];
    laneNum         = (GT_U32)inArgs[2];
    invertTx        = (GT_BOOL)inArgs[3];
    invertRx        = (GT_BOOL)inArgs[4];

    rc = cpssDxChPortSerdesLanePolaritySet(devNum, portGroupNum, laneNum, invertTx, invertRx);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, rc, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxChPortSerdesLanePolarityGet function
* @endinternal
*
* @brief   Get the SerDes lane polarity values.
*
* @note   APPLICABLE DEVICES:      Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Lion2; xCat3; Bobcat2;
*
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameters
* @retval GT_OUT_OF_RANGE          - one of tune values is out of range
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_SUPPORTED         - HW does not support the requested operation
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Get the Tx/Rx polarity parameters for lane from HW if serdes initialized
*       or from SW DB if serdes was not initialized
*
*/
CMD_STATUS wrCpssDxChPortSerdesLanePolarityGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS   rc;
    GT_U8       devNum;
    GT_U32      portGroupNum;
    GT_U32      laneNum;
    GT_BOOL     invertTx;
    GT_BOOL     invertRx;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum          = (GT_U8)inArgs[0];
    portGroupNum    = (GT_U32)inArgs[1];
    laneNum         = (GT_U32)inArgs[2];

    rc = cpssDxChPortSerdesLanePolarityGet(devNum, portGroupNum, laneNum, &invertTx, &invertRx);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, rc, "%d %d", invertTx, invertRx);

    return CMD_OK;
}

/**
* @internal wrCpssDxChPortPcsResetSet function
* @endinternal
*
* @brief   Set/unset the PCS reset for given mode on port.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Reset only PCS unit used by port for currently configured interface.
*
*/
CMD_STATUS wrCpssDxChPortPcsResetSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS   rc;
    GT_U8                          devNum;
    GT_PHYSICAL_PORT_NUM           portNum;
    CPSS_PORT_PCS_RESET_MODE_ENT   mode;
    GT_BOOL                        state;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum     = (GT_U8)inArgs[0];
    portNum    = (GT_PHYSICAL_PORT_NUM)inArgs[1];
    mode       = (CPSS_PORT_PCS_RESET_MODE_ENT)inArgs[2];
    state      = (GT_BOOL)inArgs[3];

    rc = cpssDxChPortPcsResetSet(devNum, portNum, mode, state);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, rc, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxChPortRefClockSourceOverrideEnableSet function
* @endinternal
*
* @brief   Enables/disables reference clock source override for port create.
*
* @note   APPLICABLE DEVICES:      xCat3.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number, device, refClockSource
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
CMD_STATUS wrCpssDxChPortRefClockSourceOverrideEnableSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                       result;
    GT_U8                           devNum;
    GT_PHYSICAL_PORT_NUM            portNum;
    GT_BOOL                         overrideEnable;
    CPSS_PORT_REF_CLOCK_SOURCE_ENT  refClockSource;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum  = (GT_U8)inArgs[0];
    portNum = (GT_PHYSICAL_PORT_NUM)inArgs[1];
    overrideEnable = (GT_BOOL)inArgs[2];
    refClockSource = (CPSS_PORT_REF_CLOCK_SOURCE_ENT)inArgs[3];

    /* Override Device and Port */
    CONVERT_DEV_PHYSICAL_PORT_MAC(devNum, portNum);

    /* call cpss api function */
    result = cpssDxChPortRefClockSourceOverrideEnableSet(devNum, portNum, overrideEnable, refClockSource);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;

}

/**
* @internal wrCpssDxChPortRefClockSourceOverrideEnableGet function
* @endinternal
*
* @brief   Gets status of reference clock source override for port create.
*
* @note   APPLICABLE DEVICES:      xCat3.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number, device
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
CMD_STATUS wrCpssDxChPortRefClockSourceOverrideEnableGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                       result;
    GT_U8                           devNum;
    GT_PHYSICAL_PORT_NUM            portNum;
    GT_BOOL                         overrideEnable;
    CPSS_PORT_REF_CLOCK_SOURCE_ENT  refClockSource;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum  = (GT_U8)inArgs[0];
    portNum = (GT_PHYSICAL_PORT_NUM)inArgs[1];

    /* Override Device and Port */
    CONVERT_DEV_PHYSICAL_PORT_MAC(devNum, portNum);

    /* call cpss api function */
    result = cpssDxChPortRefClockSourceOverrideEnableGet(devNum, portNum, &overrideEnable, &refClockSource);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d%d", overrideEnable, refClockSource);

    return CMD_OK;

}


/**
* @internal wrCpssDxChGetNext function
* @endinternal
*
* @brief   This function is called for ending table printing.
*
* @note   APPLICABLE DEVICES:      All DxCh devices
*
* @retval GT_OK                    - on success
*/
static CMD_STATUS wrCpssDxChGetNext

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
* @internal wrCpssDxChPortResourceTmBandwidthSet function
* @endinternal
*
* @brief   Define system bandwith through TM port. Following this configuration
*         TM port resources will be allocated.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat3; Aldrin2; Falcon.
*
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameters
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Note:
*       - API should not be called under traffic
*       - zero bandwidth will release resources of TM for non TM ports
*
*/
CMD_STATUS wrCpssDxChPortResourceTmBandwidthSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS   result;
    GT_U8       devNum;
    GT_U32      tmBandwidthMbps;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum  = (GT_U8)inArgs[0];
    tmBandwidthMbps = (GT_U32)inArgs[1];

    /* call cpss api function */
    result = cpssDxChPortResourceTmBandwidthSet(devNum, tmBandwidthMbps);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}


/**
* @internal wrCpssDxChPortResourceTmBandwidthGet function
* @endinternal
*
* @brief   Get system bandwith through TM port. Following this configuration
*         TM port resources will be allocated.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat3; Aldrin2; Falcon.
*
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameters
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
CMD_STATUS wrCpssDxChPortResourceTmBandwidthGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS   result;
    GT_U8       devNum;
    GT_U32      tmBandwidthMbps;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum  = (GT_U8)inArgs[0];
    tmBandwidthMbps = (GT_U32)inArgs[1];

    /* call cpss api function */
    result = cpssDxChPortResourceTmBandwidthGet(devNum, &tmBandwidthMbps);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", tmBandwidthMbps);

    return CMD_OK;

}

/**** database initialization **************************************/

static CMD_COMMAND dbCommands[] =
{
    {"cpssDxChPortSerdesLaneTuningEntrySet",
        &wrCpssDxChPortSerdesLaneTuningSet,
        4, 20},

    {"cpssDxChPortSerdesLaneTuningEntryGetFirst",
        &wrCpssDxChPortSerdesLaneTuningGet,
        4, 0},

    {"cpssDxChPortSerdesLaneTuningEntryGetNext",
        &wrCpssDxChGetNext,
        4, 0},

    {"cpssDxChPortSerdesLaneTuningEntry1Set",
        &wrCpssDxChPortSerdesLaneTuningSet,
        4, 9},

    {"cpssDxChPortSerdesLaneTuningEntry1GetFirst",
        &wrCpssDxChPortSerdesLaneTuningGet,
        4, 0},

    {"cpssDxChPortSerdesLaneTuningEntry1GetNext",
        &wrCpssDxChGetNext,
        4, 0},

    {"cpssDxChPortFecModeSet",
        &wrCpssDxChPortFecModeSet,
        3, 0},

    {"cpssDxChPortFecModeGet",
        &wrCpssDxChPortFecModeGet,
        2, 0},

    {"cpssDxChPortSerdesManualTxConfigSet",
         &wrCpssDxChPortSerdesManualTxConfigSet,
         8, 0},

    {"cpssDxChPortSerdesManualTxConfigSet1",
         &wrCpssDxChPortSerdesManualTxConfigSet,
         6, 0},

    {"cpssDxChPortSerdesManualRxConfigSet",
         &wrCpssDxChPortSerdesManualRxConfigSet,
         8, 0},

    {"cpssDxChPortSerdesManualRxConfigSet1",
         &wrCpssDxChPortSerdesManualRxConfigSet,
         9, 0},

    {"cpssDxChPortSerdesManualRxConfigGet1",
         &wrCpssDxChPortSerdesManualRxConfigGet,
         3, 0},

    {"cpssDxChPortSerdesManualTxConfigGet1",
         &wrCpssDxChPortSerdesManualTxConfigGet,
         3, 0},

    {"cpssDxChPortPcsLoopbackModeSet",
        &wrCpssDxChPortPcsLoopbackModeSet,
        3, 0},

    {"cpssDxChPortPcsLoopbackModeGet",
        &wrCpssDxChPortPcsLoopbackModeGet,
        2, 0},

    {"cpssDxChPortExtendedModeEnableSet",
        &wrCpssDxChPortExtendedModeEnableSet,
        3, 0},

    {"cpssDxChPortExtendedModeEnableGet",
        &wrCpssDxChPortExtendedModeEnableGet,
        2, 0},

    {"cpssDxChPortRefClockSourceOverrideEnableSet",
        &wrCpssDxChPortRefClockSourceOverrideEnableSet,
        4, 0},

    {"cpssDxChPortRefClockSourceOverrideEnableGet",
        &wrCpssDxChPortRefClockSourceOverrideEnableGet,
        2, 0},

    {"cpssDxChPortResourceTmBandwidthSet",
        &wrCpssDxChPortResourceTmBandwidthSet,
        2, 0},

    {"cpssDxChPortResourceTmBandwidthGet",
        &wrCpssDxChPortResourceTmBandwidthGet,
        1, 0},

    {"cpssDxChPortSerdesLanePolaritySet",
        &wrCpssDxChPortSerdesLanePolaritySet,
        5, 0},

    {"cpssDxChPortSerdesLanePolarityGet",
        &wrCpssDxChPortSerdesLanePolarityGet,
        3, 0},
    {"cpssDxChPortSerdesErrorInject",
        &wrCpssDxChPortSerdesErrorInject,
        5, 0},

    {"cpssDxChPortPcsResetSet",
        &wrCpssDxChPortPcsResetSet,
        4, 0},

};

#define numCommands (sizeof(dbCommands) / sizeof(CMD_COMMAND))

/**
* @internal cmdLibInitCpssDxChPortCtrl function
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
GT_STATUS cmdLibInitCpssDxChPortCtrl
(
    GT_VOID
)
{
    return cmdInitLibrary(dbCommands, numCommands);
}

