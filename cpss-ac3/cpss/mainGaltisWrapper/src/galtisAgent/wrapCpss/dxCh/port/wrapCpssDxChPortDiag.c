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
* @file wrapCpssDxChPortDiag.c
*
* @brief Wrapper functions for
* cpss/DxCh/DxChxGen/Port/cpssDxChPortDiag.h API's
*
* @version   1
********************************************************************************
*/

/* Common galtis includes */
#include <galtisAgent/wrapUtil/cmdCpssPresteraUtils.h>
#include <cmdShell/cmdDb/cmdBase.h>
#include <cmdShell/common/cmdWrapUtils.h>

/* Feature specific includes. */
#include <cpss/common/cpssTypes.h>
#include <cpss/dxCh/dxChxGen/port/cpssDxChPortDiag.h>


/**
* @internal wrCpssDxChPortDiagVoltageGet function
* @endinternal
*
* @brief   Get the voltage (in mV) of given port.
*
* @note   APPLICABLE DEVICES:      Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, portNum
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
*/
CMD_STATUS wrCpssDxChPortDiagVoltageGet
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
    GT_U32                  voltage;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum      = (GT_U8)inArgs[0];
    portNum     = (GT_PHYSICAL_PORT_NUM)inArgs[1];
   
    /* Override Device and Port */
    CONVERT_DEV_PHYSICAL_PORT_MAC(devNum, portNum);

    /* call cpss api function */
    result = cpssDxChPortDiagVoltageGet(devNum, portNum, &voltage);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", voltage);

    return CMD_OK;
}


/**
* @internal wrCpssDxChPortDiagTemperatureGet function
* @endinternal
*
* @brief   Get the Temperature (in C) of given port.
*
* @note   APPLICABLE DEVICES:      Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, portNum
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
*/
CMD_STATUS wrCpssDxChPortDiagTemperatureGet
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
    GT_32                   temp;
    GT_BOOL                 isNegative;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum      = (GT_U8)inArgs[0];
    portNum     = (GT_PHYSICAL_PORT_NUM)inArgs[1];
   
    /* Override Device and Port */
    CONVERT_DEV_PHYSICAL_PORT_MAC(devNum, portNum);

    /* call cpss api function */
    result = cpssDxChPortDiagTemperatureGet(devNum, portNum, &temp);

    isNegative = (temp < 0) ? GT_TRUE : GT_FALSE;

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d%d", temp, isNegative);

    return CMD_OK;
}



/**** database initialization **************************************/

static CMD_COMMAND dbCommands[] =
{
    /* commandName,                                    funcReference                        funcArgs funcFields */
    { "cpssDxChPortDiagVoltageGet",             &wrCpssDxChPortDiagVoltageGet,              2,      0},
    { "cpssDxChPortDiagTemperatureGet",         &wrCpssDxChPortDiagTemperatureGet,          2,      0}
};

#define numCommands (sizeof(dbCommands) / sizeof(CMD_COMMAND))

/**
* @internal cmdLibInitCpssDxChPortDiag function
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
GT_STATUS cmdLibInitCpssDxChPortDiag
(
    GT_VOID
)
{
    return cmdInitLibrary(dbCommands, numCommands);
}


