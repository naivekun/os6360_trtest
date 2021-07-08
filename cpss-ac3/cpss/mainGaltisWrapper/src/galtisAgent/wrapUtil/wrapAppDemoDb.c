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
* @file wrapAppDemoDb.c
*
* @brief wrappers for apAppDemoDb.c
*
* @version   5
********************************************************************************
*/

/* Common galtis includes */
#include <galtisAgent/wrapUtil/cmdCpssPresteraUtils.h>
#include <cmdShell/cmdDb/cmdBase.h>
#include <cmdShell/common/cmdWrapUtils.h>

/* Feature specific includes. */

/**
* @internal wrAppDemoDbEntryAdd function
* @endinternal
*
* @brief   Set AppDemo DataBase value.This value will be considered during system
*         initialization process.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - if name is too long
* @retval GT_BAD_PTR               - if NULL pointer
* @retval GT_NO_RESOURCE           - if Database is full
*/
static CMD_STATUS wrAppDemoDbEntryAdd
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U32 value;


    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */

    value = (GT_U32)inArgs[1];

    /* call cpss api function */
    result = cmdAppDbEntryAdd((GT_CHAR*)inArgs[0], value);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrAppDemoDbEntryGet function
* @endinternal
*
* @brief   Get parameter value from AppDemo DataBase.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - if name is too long
* @retval GT_BAD_PTR               - if NULL pointer
* @retval GT_NO_SUCH               - there is no such parameter in Database
*/
static CMD_STATUS wrAppDemoDbEntryGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U32 value;


    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;


    /* call cpss api function */
    result = cmdAppDbEntryGet((GT_CHAR*)inArgs[0], &value);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", value);

    return CMD_OK;
}

/**
* @internal wrAppDemoDbDump function
* @endinternal
*
* @brief   Dumps entries set in AppDemo database to console.
*
* @retval GT_OK                    - on success
*/
static CMD_STATUS wrAppDemoDbDump
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;


    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */


    /* call cpss api function */
    result = cmdAppDbDump();

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**** database initialization **************************************/



static CMD_COMMAND dbCommands[] =
{
        {"appDemoDbEntryAdd",
         &wrAppDemoDbEntryAdd,
         2, 0},
        {"appDemoDbEntryGet",
         &wrAppDemoDbEntryGet,
         1, 0},
        {"appDemoDbDump",
         &wrAppDemoDbDump,
         0, 0},
};

#define numCommands (sizeof(dbCommands) / sizeof(CMD_COMMAND))

/**
* @internal cmdLibInitAppDemoDb function
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
GT_STATUS cmdLibInitAppDemoDb
(
    GT_VOID
)
{
    return cmdInitLibrary(dbCommands, numCommands);
}


