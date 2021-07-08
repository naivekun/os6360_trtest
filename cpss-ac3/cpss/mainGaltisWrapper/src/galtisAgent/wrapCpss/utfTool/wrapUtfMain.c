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
* @file wrapUtfMain.c
*
* @brief Wrapper functions for UTF functions
*
* @version   9
********************************************************************************
*/

/* Common galtis includes */
#include <galtisAgent/wrapUtil/cmdCpssPresteraUtils.h>
#include <cmdShell/cmdDb/cmdBase.h>
#include <cmdShell/common/cmdWrapUtils.h>

/* Feature specific includes. */
#ifdef INCLUDE_UTF
#include <utf/utfMain.h>
#endif /*INCLUDE_UTF*/

/**
* @internal wrUtfLogOutputSelect function
* @endinternal
*
* @brief   Switches logger output between serial interface and file.
*
* @retval GT_OK                    -  Output log interface was selected successfully.
* @retval GT_BAD_PARAM             -  Invalid output interface id was passed.
* @retval GT_FAIL                  -  General failure error. Should never happen.
*
* @note By default output is done into serial interface.
*       Use this function if you want to have a log file.
*
*/
static CMD_STATUS wrUtfLogOutputSelect

(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;

    UTF_LOG_OUTPUT_ENT  eLogOutput;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    switch (inArgs[0])
    {
        case 0:
            eLogOutput = UTF_LOG_OUTPUT_SERIAL_ALL_E;
            break;
        case 1:
            eLogOutput = UTF_LOG_OUTPUT_FILE_E;
            break;
        case 2:
            eLogOutput = UTF_LOG_OUTPUT_SERIAL_FINAL_WITH_DEBUG_E;
            break;
        default:
            eLogOutput = UTF_LOG_OUTPUT_SERIAL_FINAL_E;
            break;
    }

    /* call cpss api function */
    result = utfLogOutputSelect(eLogOutput);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}

/**
* @internal wrUtfTestsRun function
* @endinternal
*
* @brief   Runs all suits in case of NULL input path (testPathPtr),
*         or suit if suite name is specified as testPathPtr,
*         or specific test in the suit.
*
* @retval GT_OK                    -  Running of tests was successful.
* @retval GT_NOT_FOUND             -  Test path wasn't found.
* @retval GT_BAD_PARAM             -  Bad number of cycles was passed.
* @retval GT_UTF_TEST_FAILED       -  Some test fails.
* @retval GT_FAIL                  -  General failure error. Should never happen.
*/
static CMD_STATUS wrUtfTestsRun

(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS    result;
    IN GT_CHAR   *testPathPtr;
    IN GT_U32    numOfRuns;
    IN GT_BOOL   fContinue;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    testPathPtr = (GT_CHAR *)inArgs[0];
    numOfRuns = (GT_U32)inArgs[1];
    fContinue = (GT_BOOL)inArgs[2];
    
    if (testPathPtr != NULL)
    {
       if(*testPathPtr == '0')
            testPathPtr=NULL;
    }

    /* call cpss api function */
    result = utfTestsRun(testPathPtr,numOfRuns,fContinue);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}

/**
* @internal wrUtfTestsStartRunFromPtr function
* @endinternal
*
* @brief   Runs suits or specific tests from the start point.
*
* @retval GT_OK                    -  Running of tests was successful.
* @retval GT_NOT_FOUND             -  Test path wasn't found.
* @retval GT_BAD_PARAM             -  Bad number of cycles was passed.
* @retval GT_UTF_TEST_FAILED       -  Some test fails.
* @retval GT_FAIL                  -  General failure error. Should never happen.
*/
static CMD_STATUS wrUtfTestsStartRunFromPtr

(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    IN GT_CHAR     *testPathPtr;
    IN GT_U32    numOfRuns;
    IN GT_BOOL   fContinue;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    testPathPtr = (GT_CHAR *)inArgs[0];
    numOfRuns = (GT_U32)inArgs[1];
    fContinue = (GT_BOOL)inArgs[2];


    if(*testPathPtr == '0')
        testPathPtr=NULL;
    /* call cpss api function */
    result = utfTestsStartRun(testPathPtr,numOfRuns,fContinue);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}


/**** database initialization **************************************/

static CMD_COMMAND dbCommands[] =
{

    {"utfLogOutputSelect",
        &wrUtfLogOutputSelect,
        1, 0},
    {"utfTestsRun",
        &wrUtfTestsRun,
        3, 0},
    {"utfTestsStartRunFromPtr",
        &wrUtfTestsStartRunFromPtr,
        3, 0}

};

#define numCommands (sizeof(dbCommands) / sizeof(CMD_COMMAND))

/**
* @internal cmdLibInitUtfMain function
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
GT_STATUS cmdLibInitUtfMain
(
    GT_VOID
)
{
    return cmdInitLibrary(dbCommands, numCommands);
}



