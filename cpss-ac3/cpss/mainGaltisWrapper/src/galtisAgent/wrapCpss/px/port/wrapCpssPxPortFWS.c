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
* @file wrapCpssPxPortFWS.c
*
* @brief Wrapper functions for
* Port wire speed tests.
*
* @version   1
********************************************************************************
*/

/* Common galtis includes */
#include <galtisAgent/wrapUtil/cmdCpssPresteraUtils.h>
#include <cmdShell/cmdDb/cmdBase.h>
#include <cmdShell/common/cmdWrapUtils.h>
#include <cpss/px/config/private/prvCpssPxInfo.h>
#include <cpss/px/port/cpssPxPortMapping.h>
/* include the external services */
#include <cmdShell/common/cmdExtServices.h>

/* Feature specific includes. */
/* port used in Mac Counters tables */

static GT_PHYSICAL_PORT_NUM portForFWSRatesGet = 0xFF;
static CMD_PORT_FWS_RATE_STC rateDbArr[CPSS_MAX_PORTS_NUM_CNS];


/**
* @internal wrCpssPxPortFWSRatesTableGetNext function
* @endinternal
*
* @brief   Gets port TX/RX packet rate.
*
* @note   APPLICABLE DEVICES:      All Px devices
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
*/
static CMD_STATUS  wrCpssPxPortFWSRatesTableGetNext
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;  /* return code */
    GT_U8   devNum;
    static CPSS_PX_DETAILED_PORT_MAP_STC portMapShadow;
    CPSS_PX_DETAILED_PORT_MAP_STC *portMapShadowPtr = &portMapShadow;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];

    while(1)
    {
        if(portForFWSRatesGet >= CPSS_MAX_PORTS_NUM_CNS)
        {
            galtisOutput(outArgs, GT_OK, "%d", -1);
            return CMD_OK;
        }

        result = cpssPxPortPhysicalPortDetailedMapGet(
            devNum, portForFWSRatesGet, /*OUT*/portMapShadowPtr);
        if(result != GT_OK)
        {
            /* end of all ports */
            galtisOutput(outArgs, GT_OK, "%d", -1);
            return CMD_OK;
        }

        if(portMapShadowPtr->valid != GT_TRUE)
        {
            portForFWSRatesGet++;
            continue;
        }
        else
        {
            break;
        }
    }

    inFields[0] = portForFWSRatesGet;
    inFields[1] = rateDbArr[portForFWSRatesGet].rxPacketRate;
    inFields[2] = rateDbArr[portForFWSRatesGet].txPacketRate;

    /* pack and output table fields */
    fieldOutput("%d%d%d", inFields[0], inFields[1], inFields[2]);

    galtisOutput(outArgs, GT_OK, "%f");

    portForFWSRatesGet++;

    return CMD_OK;
}


/**
* @internal wrCpssPxPortFWSRatesTableGetFirst function
* @endinternal
*
* @brief   Gets port TX/RX packet rate.
*
* @note   APPLICABLE DEVICES:      All Px devices
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
*/
static CMD_STATUS  wrCpssPxPortFWSRatesTableGetFirst
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;  /* return code */
    GT_U8   devNum;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];

    portForFWSRatesGet = 0;

    /* call cpss api function */
    result = cmdPortRateTableGet(devNum, CPSS_MAX_PORTS_NUM_CNS, rateDbArr);

    if (result != GT_OK)
    {
        galtisOutput(outArgs, result, "");
        return CMD_OK;
    }

    return wrCpssPxPortFWSRatesTableGetNext(inArgs,inFields,numFields,outArgs);
}



/**** database initialization **************************************/
static CMD_COMMAND dbCommands[] =
{
    /* commandName                         funcReference                         funcArgs  funcFields */
    { "cpssPxPortFWSRatesTableGetNext",    &wrCpssPxPortFWSRatesTableGetNext,    1, 0},
    { "cpssPxPortFWSRatesTableGetFirst",   &wrCpssPxPortFWSRatesTableGetFirst,   1, 0}
};


#define  numCommands     (sizeof(dbCommands) / sizeof(CMD_COMMAND))


/**
* @internal cmdLibInitCpssPxPortFWS function
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
GT_STATUS cmdLibInitCpssPxPortFWS
(
    GT_VOID
)
{
    return cmdInitLibrary(dbCommands, numCommands);
}

