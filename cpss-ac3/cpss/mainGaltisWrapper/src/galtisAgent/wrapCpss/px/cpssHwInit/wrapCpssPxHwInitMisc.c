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
* @file wrapCpssPxHwInitMisc.c
* @version   1
********************************************************************************
*/

/* Common galtis includes */
#include <galtisAgent/wrapUtil/cmdCpssPresteraUtils.h>
#include <cmdShell/cmdDb/cmdBase.h>
#include <cmdShell/common/cmdWrapUtils.h>
#include <cpss/px/cpssHwInit/cpssPxHwInit.h>

/*******************************************************************************
* cpssPxHwCoreClockGet
*
* DESCRIPTION:
*       This function returns the core clock value from HW.
*
* APPLICABLE DEVICES:
*        Pipe.
*
* NOT APPLICABLE DEVICES:
*        None.
*
* INPUTS:
*       devNum         - The Pp's device number.
*
* OUTPUTS:
*       coreClkPtr    - Pp's core clock from HW and aligned with CPSS DB (MHz)
*
* RETURNS:
*       GT_OK                    - on success,
*       GT_BAD_PTR               - one of the parameters is NULL pointer
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*       None.
*
*******************************************************************************/
#include <cpss/common/cpssTypes.h>

static CMD_STATUS wrCpssPxHwCoreClockGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS     result;
    GT_SW_DEV_NUM devNum;
    GT_U32        coreClk;


    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_SW_DEV_NUM)inArgs[0];

    /* call cpss api function */
    result = cpssPxHwCoreClockGet(devNum, &coreClk);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", coreClk);

    return CMD_OK;
}

static CPSS_PX_CFG_CNTR_ENT    counterType;
/**
* @internal wrCpssPxCfgCntrGet function
* @endinternal
*
* @brief   Gets the MCFC counters for specified device.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on bad parameters
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssPxCfgCntrGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS     result;
    GT_SW_DEV_NUM devNum;
    GT_U32        counterVal;

    /* map input arguments to locals */
    devNum = (GT_SW_DEV_NUM)inArgs[0];

    if (counterType >= CPSS_PX_CFG_CNTR_LAST_E)
    {
        galtisOutput(outArgs, GT_OK, "%d", -1);
        return CMD_OK;
    }

    /* call cpss api function */
    result = cpssPxCfgCntrGet(devNum, counterType, &counterVal);
    if (result != GT_OK)
    {
        galtisOutput(outArgs, result, ""); 
        return CMD_OK;
    }

    inFields[0] = counterType++;
    inFields[1] = counterVal;

    /* pack and output table fields */
    fieldOutput("%d%d", inFields[0], inFields[1]);

    galtisOutput(outArgs, GT_OK, "%f");

    return CMD_OK;
}

/* wrCpssPxCfgCntrGet */
CMD_STATUS wrCpssPxCfgCntrGetFirst
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

    /* map input fields to global */
    counterType = CPSS_PX_CFG_CNTR_RECEIVED_PFC_E;

    return wrCpssPxCfgCntrGet(inArgs,inFields,numFields,outArgs);
}

/* wrCpssPxCfgCntrGet */
CMD_STATUS wrCpssPxCfgCntrGetNext
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{

    return wrCpssPxCfgCntrGet(inArgs,inFields,numFields,outArgs);
}

/**** database initialization **************************************/

static CMD_COMMAND dbCommands[] =
{
    {"cpssPxHwCoreClockGet",
         &wrCpssPxHwCoreClockGet,
         1, 0},
    {"cpssPxCfgCntrGetFirst",
         &wrCpssPxCfgCntrGetFirst,
         1, 0},
    {"cpssPxCfgCntrGetNext",
         &wrCpssPxCfgCntrGetNext,
         1, 0}
};

#define numCommands (sizeof(dbCommands) / sizeof(CMD_COMMAND))

/**
* @internal cmdLibInitCpssPxHwInitMisc function
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
GT_STATUS cmdLibInitCpssPxHwInitMisc
(
    GT_VOID
)
{
    return cmdInitLibrary(dbCommands, numCommands);
}

