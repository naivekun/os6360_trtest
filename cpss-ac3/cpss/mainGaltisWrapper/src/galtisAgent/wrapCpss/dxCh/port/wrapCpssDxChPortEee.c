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
* @file wrapCpssDxChPortEee.c
*
* @brief Wrapper functions for CPSS DXCH Port EEE API
* CPSS DxCh Port's IEEE 802.3az Energy Efficient Ethernet (EEE) Support.
*
* @note 1. The device support relevant when the PHY works in EEE slave mode.
* When the PHY works in EEE master mode, the device is not involved in
* EEE processing.
* 2. The feature not relevant when port is not 'tri-speed'
* 3. LPI is short for 'Low Power Idle'
*
*
* @version   1
********************************************************************************
*/

/* Common galtis includes */
#include <galtisAgent/wrapUtil/cmdCpssPresteraUtils.h>
#include <cmdShell/cmdDb/cmdBase.h>
#include <cmdShell/common/cmdWrapUtils.h>


/* Feature specific includes. */
#include <cpss/dxCh/dxChxGen/port/cpssDxChPortEee.h>
#include <cpss/generic/config/private/prvCpssConfigTypes.h>
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>

/**
* @internal wrCpssDxChPortEeeLpiRequestEnableSet function
* @endinternal
*
* @brief   Set port enable/disable transmitting of LPI signaling to the PHY and
*         identification of LPI on reception.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or devNum
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChPortEeeLpiRequestEnableSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_U32   ii = 0;
    GT_STATUS                       result;
    GT_U8                           devNum;
    GT_PHYSICAL_PORT_NUM            portNum;
    GT_BOOL                         enable;

    /* check for valid arguments */
    if (!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[ii++];
    portNum  = (GT_PHYSICAL_PORT_NUM)inArgs[ii++];
    enable  = (GT_BOOL)inArgs[ii++];

    CONVERT_DEV_PHYSICAL_PORT_MAC(devNum, portNum);

    /* call cpss api function */
    result = cpssDxChPortEeeLpiRequestEnableSet(devNum, portNum, enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}

/**
* @internal wrCpssDxChPortEeeLpiRequestEnableGet function
* @endinternal
*
* @brief   Get port's enable/disable transmitting of LPI signaling to the PHY and
*         identification of LPI on reception.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or devNum
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChPortEeeLpiRequestEnableGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_U32   ii = 0;
    GT_STATUS  result;
    GT_U8      devNum;
    GT_PHYSICAL_PORT_NUM            portNum;
    GT_BOOL    enable;

    /* check for valid arguments */
    if (!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[ii++];
    portNum  = (GT_PHYSICAL_PORT_NUM)inArgs[ii++];

    CONVERT_DEV_PHYSICAL_PORT_MAC(devNum, portNum);

    /* call cpss api function */
    result = cpssDxChPortEeeLpiRequestEnableGet(devNum, portNum, &enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", enable);
    return CMD_OK;
}

/**
* @internal wrCpssDxChPortEeeLpiManualModeSet function
* @endinternal
*
* @brief   Set port manual mode (enabled/disabled/forced) for LPI signaling.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or devNum
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChPortEeeLpiManualModeSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_U32   ii = 0;
    GT_STATUS                       result;
    GT_U8                           devNum;
    GT_PHYSICAL_PORT_NUM            portNum;
    CPSS_DXCH_PORT_EEE_LPI_MANUAL_MODE_ENT mode;

    /* check for valid arguments */
    if (!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[ii++];
    portNum  = (GT_PHYSICAL_PORT_NUM)inArgs[ii++];
    mode  = (CPSS_DXCH_PORT_EEE_LPI_MANUAL_MODE_ENT)inArgs[ii++];

    CONVERT_DEV_PHYSICAL_PORT_MAC(devNum, portNum);

    /* call cpss api function */
    result = cpssDxChPortEeeLpiManualModeSet(devNum, portNum, mode);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}

/**
* @internal wrCpssDxChPortEeeLpiManualModeGet function
* @endinternal
*
* @brief   Get port manual mode (enabled/disabled/forced) for LPI signaling.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or devNum
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChPortEeeLpiManualModeGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_U32   ii = 0;
    GT_STATUS  result;
    GT_U8      devNum;
    GT_PHYSICAL_PORT_NUM            portNum;
    CPSS_DXCH_PORT_EEE_LPI_MANUAL_MODE_ENT mode;

    /* check for valid arguments */
    if (!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[ii++];
    portNum  = (GT_PHYSICAL_PORT_NUM)inArgs[ii++];

    CONVERT_DEV_PHYSICAL_PORT_MAC(devNum, portNum);

    /* call cpss api function */
    result = cpssDxChPortEeeLpiManualModeGet(devNum, portNum, &mode);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", mode);
    return CMD_OK;
}

/**
* @internal wrCpssDxChPortEeeLpiTimeLimitsSet function
* @endinternal
*
* @brief   Set port LPI timers.
*         NOTE: The 'time' granularity is 1 uSec for 1000 Mbps and 10 uSec for 100 Mbps.
*         so the function set value 'as is'.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or devNum
* @retval GT_OUT_OF_RANGE          - on out of range : twLimit , liLimit , tsLimit
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChPortEeeLpiTimeLimitsSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_U32   ii = 0;
    GT_STATUS                       result;
    GT_U8                           devNum;
    GT_PHYSICAL_PORT_NUM            portNum;
    GT_U32                  twLimit;
    GT_U32                  liLimit;
    GT_U32                  tsLimit;

    /* check for valid arguments */
    if (!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[ii++];
    portNum  = (GT_PHYSICAL_PORT_NUM)inArgs[ii++];
    twLimit  = (GT_U32)inArgs[ii++];
    liLimit  = (GT_U32)inArgs[ii++];
    tsLimit  = (GT_U32)inArgs[ii++];

    CONVERT_DEV_PHYSICAL_PORT_MAC(devNum, portNum);

    /* call cpss api function */
    result = cpssDxChPortEeeLpiTimeLimitsSet(devNum, portNum, twLimit, liLimit, tsLimit);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}

/**
* @internal wrCpssDxChPortEeeLpiTimeLimitsGet function
* @endinternal
*
* @brief   Get port LPI timers.
*         NOTE: The 'time' granularity is 1 uSec for 1000 Mbps and 10 uSec for 100 Mbps.
*         so the function get value 'as is'.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or devNum
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChPortEeeLpiTimeLimitsGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_U32   ii = 0;
    GT_STATUS  result;
    GT_U8      devNum;
    GT_PHYSICAL_PORT_NUM            portNum;
    GT_U32                  twLimit;
    GT_U32                  liLimit;
    GT_U32                  tsLimit;

    /* check for valid arguments */
    if (!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[ii++];
    portNum  = (GT_PHYSICAL_PORT_NUM)inArgs[ii++];

    CONVERT_DEV_PHYSICAL_PORT_MAC(devNum, portNum);

    /* call cpss api function */
    result = cpssDxChPortEeeLpiTimeLimitsGet(devNum, portNum, &twLimit, &liLimit , &tsLimit);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d%d%d", twLimit, liLimit, tsLimit);
    return CMD_OK;
}



/* EEE per port Table */
static GT_PHYSICAL_PORT_NUM   portIndex;
/* look for valid portIndex that hold 'Mac' */
static GT_STATUS getPortWithMac(IN GT_U8    devNum)
{
    GT_U32  portMacNum;
    GT_STATUS rc;

    do{
        rc = prvCpssDxChPortPhysicalPortMapCheckAndConvert(devNum, portIndex, PRV_CPSS_DXCH_PORT_TYPE_MAC_E, &portMacNum);
        if(rc != GT_OK)
        {
            portIndex++;
            continue;
        }

        return GT_OK;
    }while(portIndex < 256);

    return GT_NO_MORE;
}

/**
* @internal wrCpssDxChPortEeeLpiStatusGetNext function
* @endinternal
*
* @brief   Get port LPI status.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or devNum
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChPortEeeLpiStatusGetNext
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_U32   ii = 0;
    GT_STATUS                       result;
    GT_U8                           devNum;
    GT_PHYSICAL_PORT_NUM            portNum;
    CPSS_DXCH_PORT_EEE_LPI_STATUS_STC statusInfo;

    /* check for valid arguments */
    if (!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[ii++];

    if (GT_OK != getPortWithMac(devNum))
    {
        /* no more ports with MAC */
        galtisOutput(outArgs, GT_OK, "%d", -1);
        return CMD_OK;
    }

    /* call cpss api function */
    result = cpssDxChPortEeeLpiStatusGet(devNum, portIndex, &statusInfo);

    portNum = portIndex;
    CONVERT_BACK_DEV_PHYSICAL_PORT_MAC(devNum, portNum);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d%d%d%d%d%d",portNum,
        statusInfo.macTxPathLpi,
        statusInfo.macTxPathLpWait,
        statusInfo.macRxPathLpi,
        statusInfo.pcsTxPathLpi,
        statusInfo.pcsRxPathLpi
    );

    portIndex++;

    return CMD_OK;
}

/* get first */
static CMD_STATUS wrCpssDxChPortEeeLpiStatusGetFirst
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    portIndex = 0;
    return wrCpssDxChPortEeeLpiStatusGetNext(inArgs,inFields,numFields,outArgs);
}


/**** database initialization **************************************/

static CMD_COMMAND dbCommands[] =
{
    {"cpssDxChPortEeeLpiRequestEnableSet",
        &wrCpssDxChPortEeeLpiRequestEnableSet,
        3, 0},
    {"cpssDxChPortEeeLpiRequestEnableGet",
        &wrCpssDxChPortEeeLpiRequestEnableGet,
        2, 0},

    {"cpssDxChPortEeeLpiManualModeSet",
        &wrCpssDxChPortEeeLpiManualModeSet,
        3, 0},
    {"cpssDxChPortEeeLpiManualModeGet",
        &wrCpssDxChPortEeeLpiManualModeGet,
        2, 0},

    {"cpssDxChPortEeeLpiTimeLimitsSet",
        &wrCpssDxChPortEeeLpiTimeLimitsSet,
        5, 0},
    {"cpssDxChPortEeeLpiTimeLimitsGet",
        &wrCpssDxChPortEeeLpiTimeLimitsGet,
        2, 0},

    /**********/
    /* tables */
    /**********/
    /* table : cpssDxChPortEeeLpiStatus */
            {"cpssDxChPortEeeLpiStatusGetFirst",
                &wrCpssDxChPortEeeLpiStatusGetFirst,
                1, /*dev*/
                0},/*portNum,macTxPathLpi,macTxPathLpWait,macRxPathLpi,pcsTxPathLpi,pcsRxPathLpi*/

            {"cpssDxChPortEeeLpiStatusGetNext",
                &wrCpssDxChPortEeeLpiStatusGetNext,
                1, /*dev*/
                0}/*portNum,macTxPathLpi,macTxPathLpWait,macRxPathLpi,pcsTxPathLpi,pcsRxPathLpi*/

};

#define numCommands (sizeof(dbCommands) / sizeof(CMD_COMMAND))

/**
* @internal cmdLibInitCpssDxChPortEee function
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
GT_STATUS cmdLibInitCpssDxChPortEee
(
    GT_VOID
)
{
    return cmdInitLibrary(dbCommands, numCommands);
}


