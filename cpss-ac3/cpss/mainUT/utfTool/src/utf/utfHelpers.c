/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
* prvUtfHelpers.c
*
* DESCRIPTION:
*       API for logger, time management, memory allocation.
*       It is used only in the utfMain.c
*
* FILE REVISION NUMBER:
*       $Revision: 9 $
*
*******************************************************************************/
#if (defined CHX_FAMILY)
    #define DXCH_CODE
#endif /* (defined CHX_FAMILY) */
#if (defined PX_FAMILY)
    #define PX_CODE
#endif /* (defined PX_FAMILY) */

#include <cpss/extServices/os/gtOs/gtGenTypes.h>
#include <cpss/extServices/private/prvCpssBindFunc.h>
#include <gtOs/gtOsStr.h>
#include <ctype.h>
#include <stdio.h>
#include <cpssCommon/cpssPresteraDefs.h>
#ifdef DXCH_CODE
#include <cpss/dxCh/dxChxGen/networkIf/cpssDxChNetIfTypes.h>
#endif /*DXCH_CODE*/

#include <utf/utfMain.h>
#include <utf/private/prvUtfHelpers.h>

/* Control the printing */
static UTF_LOG_OUTPUT_ENT logOutputPrint = UTF_LOG_OUTPUT_SERIAL_ALL_E;

static FILE* fp = NULL;
static FILE* fResultFilePtr = NULL;

/* flag to indicate if logger file is opened */
static GT_BOOL prvUtfIsFileOpened = GT_FALSE;

/* Definitions for printouts intercepting */
static int (*saved_cpssOsPrintf)(const char *format, ...);
static int prvUtfLogPrintDummy(const char *format, ...);

#ifdef DXCH_CODE
/* Definitions for packets send intercepting */
static GT_STATUS (*saved_cpssDxChNetIfSdmaSyncTxPacketSend)
(
    IN GT_U8                                    devNum,
    IN CPSS_DXCH_NET_TX_PARAMS_STC              *pcktParamsPtr,
    IN GT_U8                                    *buffList[],
    IN GT_U32                                   buffLenList[],
    IN GT_U32                                   numOfBufs
);
static GT_STATUS (*saved_cpssDxChNetIfMiiSyncTxPacketSend)
(
    IN GT_U8                                    devNum,
    IN CPSS_DXCH_NET_TX_PARAMS_STC              *pcktParamsPtr,
    IN GT_U8                                    *buffList[],
    IN GT_U32                                   buffLenList[],
    IN GT_U32                                   numOfBufs
);
static GT_STATUS prvUtfPacketSendDummy
(
    IN GT_U8                                    devNum,
    IN CPSS_DXCH_NET_TX_PARAMS_STC              *pcktParamsPtr,
    IN GT_U8                                    *buffList[],
    IN GT_U32                                   buffLenList[],
    IN GT_U32                                   numOfBufs
);
extern GT_STATUS (*cpssDxChNetIfSdmaSyncTxPacketSendPtr)
(
    IN GT_U8                                    devNum,
    IN CPSS_DXCH_NET_TX_PARAMS_STC              *pcktParamsPtr,
    IN GT_U8                                    *buffList[],
    IN GT_U32                                   buffLenList[],
    IN GT_U32                                   numOfBufs
);
extern GT_STATUS (*cpssDxChNetIfMiiSyncTxPacketSendPtr)
(
    IN GT_U8                                    devNum,
    IN CPSS_DXCH_NET_TX_PARAMS_STC              *pcktParamsPtr,
    IN GT_U8                                    *buffList[],
    IN GT_U32                                   buffLenList[],
    IN GT_U32                                   numOfBufs
);
#endif /*DXCH_CODE*/


#ifndef ASIC_SIMULATION
#define cpss_fprintf(_log)
#else
#define cpss_fprintf(_log)  fprintf _log
#endif
/**
* @internal prvUtfLogOpen function
* @endinternal
*
* @brief   Opens logger with defined output interface.
*
* @param[in] fileName                 -   log output file name
*
* @retval GT_OK                    -  logger has been successfully opened.
* @retval GT_BAD_PARAM             -  Invalid output interface id was passed.
* @retval GT_FAIL                  -  General failure error. Should never happen.
*/
GT_STATUS prvUtfLogOpen
(
    IN const GT_CHAR* fileName
)
{
    if (NULL == fileName)
    {
        return GT_BAD_PTR;
    }

    if (NULL != fp)
    {
        return GT_ALREADY_EXIST;
    }

    fp = fopen(fileName, "wt");
    if (NULL == fp)
    {
        return GT_BAD_PTR;
    }

    /* set flag */
    prvUtfIsFileOpened = GT_TRUE;

    return GT_OK;
}

/**
* @internal prvUtfLogReadParam function
* @endinternal
*
* @brief   Read parameters from file to array.
*
* @param[in] fileName                 - log output file name to check
* @param[in] numberParams             - number of parameters to read
*
* @param[out] arrayOfParamPtr          - pointer to array of params from result file
*
* @retval GT_OK                    - logger has been successfully opened.
* @retval GT_BAD_PARAM             - Invalid output interface id was passed.
* @retval GT_BAD_SIZE              - Invalid input file size.
* @retval GT_NO_RESOURCE           - Memory error.
* @retval GT_FAIL                  - General failure error. Should never happen.
*
* @note this functions used by utfLogResultRun
*
*/
GT_STATUS prvUtfLogReadParam
(
    IN  const GT_CHAR         *fileName,
    IN  GT_8                  numberParams,
    OUT UTF_RESULT_PARAM_STC *arrayOfParamPtr
)
{
    GT_U32      lSize;
    GT_U8*      buffer;
    GT_8        valueAnsi[UTF_MAX_VALUE_LEN_CNS];
    GT_U32      result;
    GT_U8       count;
    GT_U8       paramCharCounter;
    GT_U32      bufferCounter;
    GT_BOOL     setItValue;

    CPSS_NULL_PTR_CHECK_MAC(fileName);
    CPSS_NULL_PTR_CHECK_MAC(arrayOfParamPtr);

    if (NULL != fResultFilePtr)
    {
        return GT_ALREADY_EXIST;
    }
    fResultFilePtr = fopen(fileName, "rt");

    if (NULL == fResultFilePtr)
    {
        return GT_FAIL;
    }
    else
    {
        /* obtain file size:*/
        fseek (fResultFilePtr, 0, SEEK_END);
        lSize = ftell (fResultFilePtr);
        rewind (fResultFilePtr);

        if (lSize > UTF_MAX_LOG_SIZE_CNS)
        {
            return GT_BAD_SIZE;
        }

        /* allocate memory to contain the whole file: */
        buffer = (GT_VOID*) cpssOsMalloc(lSize);
        if (NULL == buffer)
        {
            return GT_NO_RESOURCE;
        }

        /* copy the file into the buffer: */
        result = (GT_U32)fread(buffer, 1, lSize, fResultFilePtr);

        /*  result value depend on type of text files (\r\n or \n), for windows it will be less then lSize*/
        /*  if (result != lSize) {fputs ("Reading error",stderr); exit (3);} */

        /* the whole file is now loaded in the memory buffer. */

        bufferCounter = 0;

        cpssOsBzero((GT_VOID*) arrayOfParamPtr, sizeof(*arrayOfParamPtr) * numberParams);

        for(count = 0; count < numberParams; count++)
        {
            setItValue = 0;
            paramCharCounter = 0;
            cpssOsBzero((GT_VOID*) valueAnsi, sizeof(valueAnsi));

            while (*(buffer + bufferCounter) != '\n' && bufferCounter < result)
            {
               if (*(buffer + bufferCounter) == '\t')
               {
                    setItValue = 1;
                    paramCharCounter = 0;
                    bufferCounter++;
               }
               if (setItValue == 0 && paramCharCounter < UTF_MAX_PARAM_NAME_LEN_CNS)
               {
                    arrayOfParamPtr[count].paramName[paramCharCounter] = *(buffer + bufferCounter);
                    paramCharCounter++;
               }
               if (setItValue == 1 && paramCharCounter < UTF_MAX_VALUE_LEN_CNS && isdigit(*(buffer + bufferCounter)))
               {
                    valueAnsi[paramCharCounter] = *(buffer + bufferCounter);
                    paramCharCounter++;
               }
               bufferCounter++;
            }
            if (*(buffer + bufferCounter) == '\n')
            {
                bufferCounter++;
                arrayOfParamPtr[count].paramValue = osStrTo32((GT_CHAR*)valueAnsi);
            }
        }
        cpssOsFree(buffer);
        fclose(fResultFilePtr);
        fResultFilePtr = NULL;
    }
    return GT_OK;
}

/**
* @internal prvUtfLogMessage function
* @endinternal
*
* @brief   This routine writes message to the output interface.
*         This function handles all issues regarding the logging to specific
*         interfaces.
* @param[in] formatStringPtr          -   (pointer to) format string.
* @param[in] argvPtr[]                -   (pointer to) vector of output string arguments.
* @param[in] argc                     -   number of arguments in output string.
*
* @retval GT_OK                    -  logging has been done successfully.
* @retval GT_FAIL                  -  General failure error. Should never happen.
*
* @note It uses OS abstraction layer to output.
*
*/
GT_VOID prvUtfLogMessage
(
    IN const GT_CHAR    *formatStringPtr,
    IN const GT_UINTPTR argvPtr[],
    IN GT_U32           argc
)
{
    if ((logOutputPrint == UTF_LOG_OUTPUT_SERIAL_FINAL_E) ||
        (logOutputPrint == UTF_LOG_OUTPUT_SERIAL_FINAL_WITH_DEBUG_E) ||
        ((logOutputPrint == UTF_LOG_OUTPUT_FILE_E) && (!prvUtfIsFileOpened)))
    {
        return;
    }

    switch (argc)
    {
        case 0:
            if (logOutputPrint == UTF_LOG_OUTPUT_FILE_E)
            {
                cpss_fprintf((fp, formatStringPtr));
            }
            cpssOsPrintf(formatStringPtr);
            break;
        case 1:
            if (logOutputPrint == UTF_LOG_OUTPUT_FILE_E)
            {
                cpss_fprintf((fp, formatStringPtr, argvPtr[0]));
            }
            cpssOsPrintf(formatStringPtr, argvPtr[0]);
            break;
        case 2:
            if (logOutputPrint == UTF_LOG_OUTPUT_FILE_E)
            {
                cpss_fprintf((fp, formatStringPtr, argvPtr[0], argvPtr[1]));
            }
            cpssOsPrintf(formatStringPtr, argvPtr[0], argvPtr[1]);
            break;
        case 3:
            if (logOutputPrint == UTF_LOG_OUTPUT_FILE_E)
            {
                cpss_fprintf((fp, formatStringPtr, argvPtr[0], argvPtr[1], argvPtr[2]));
            }
            cpssOsPrintf(formatStringPtr, argvPtr[0], argvPtr[1], argvPtr[2]);
            break;
        case 4:
            if (logOutputPrint == UTF_LOG_OUTPUT_FILE_E)
            {
                cpss_fprintf((fp, formatStringPtr, argvPtr[0], argvPtr[1], argvPtr[2], argvPtr[3]));
            }
            cpssOsPrintf(formatStringPtr, argvPtr[0], argvPtr[1], argvPtr[2], argvPtr[3]);
            break;
        case 5:
            if (logOutputPrint == UTF_LOG_OUTPUT_FILE_E)
            {
                cpss_fprintf((fp, formatStringPtr, argvPtr[0], argvPtr[1], argvPtr[2], argvPtr[3], argvPtr[4]));
            }
            cpssOsPrintf(formatStringPtr, argvPtr[0], argvPtr[1], argvPtr[2], argvPtr[3], argvPtr[4]);
            break;
        case 6:
            if (logOutputPrint == UTF_LOG_OUTPUT_FILE_E)
            {
                cpss_fprintf((fp, formatStringPtr, argvPtr[0], argvPtr[1], argvPtr[2], argvPtr[3], argvPtr[4], argvPtr[5]));
            }
            cpssOsPrintf(formatStringPtr, argvPtr[0], argvPtr[1], argvPtr[2], argvPtr[3], argvPtr[4], argvPtr[5]);
            break;
        case 7:
            if (logOutputPrint == UTF_LOG_OUTPUT_FILE_E)
            {
                cpss_fprintf((fp, formatStringPtr, argvPtr[0], argvPtr[1], argvPtr[2], argvPtr[3], argvPtr[4], argvPtr[5], argvPtr[6]));
            }
            cpssOsPrintf(formatStringPtr, argvPtr[0], argvPtr[1], argvPtr[2], argvPtr[3], argvPtr[4], argvPtr[5], argvPtr[6]);
            break;
        case 8:
            if (logOutputPrint == UTF_LOG_OUTPUT_FILE_E)
            {
                cpss_fprintf((fp, formatStringPtr, argvPtr[0], argvPtr[1], argvPtr[2], argvPtr[3], argvPtr[4], argvPtr[5], argvPtr[6],argvPtr[7]));
            }
            cpssOsPrintf(formatStringPtr, argvPtr[0], argvPtr[1], argvPtr[2], argvPtr[3], argvPtr[4], argvPtr[5], argvPtr[6],argvPtr[7]);
            break;
        case 9:
            if (logOutputPrint == UTF_LOG_OUTPUT_FILE_E)
            {
                cpss_fprintf((fp, formatStringPtr, argvPtr[0], argvPtr[1], argvPtr[2], argvPtr[3], argvPtr[4], argvPtr[5], argvPtr[6],argvPtr[7],argvPtr[8]));
            }
            cpssOsPrintf(formatStringPtr, argvPtr[0], argvPtr[1], argvPtr[2], argvPtr[3], argvPtr[4], argvPtr[5], argvPtr[6],argvPtr[7],argvPtr[8]);
            break;
        case 10:
            if (logOutputPrint == UTF_LOG_OUTPUT_FILE_E)
            {
                cpss_fprintf((fp, formatStringPtr, argvPtr[0], argvPtr[1], argvPtr[2], argvPtr[3], argvPtr[4], argvPtr[5], argvPtr[6],argvPtr[7],argvPtr[8],argvPtr[9]));
            }
            cpssOsPrintf(formatStringPtr, argvPtr[0], argvPtr[1], argvPtr[2], argvPtr[3], argvPtr[4], argvPtr[5], argvPtr[6],argvPtr[7],argvPtr[8],argvPtr[9]);
            break;
        case 11:
            if (logOutputPrint == UTF_LOG_OUTPUT_FILE_E)
            {
                cpss_fprintf((fp, formatStringPtr, argvPtr[0], argvPtr[1], argvPtr[2], argvPtr[3], argvPtr[4], argvPtr[5], argvPtr[6],argvPtr[7],argvPtr[8],argvPtr[9],argvPtr[10]));
            }
            cpssOsPrintf(formatStringPtr, argvPtr[0], argvPtr[1], argvPtr[2], argvPtr[3], argvPtr[4], argvPtr[5], argvPtr[6],argvPtr[7],argvPtr[8],argvPtr[9],argvPtr[10]);
            break;
        case 12:
            if (logOutputPrint == UTF_LOG_OUTPUT_FILE_E)
            {
                cpss_fprintf((fp, formatStringPtr, argvPtr[0], argvPtr[1], argvPtr[2], argvPtr[3], argvPtr[4], argvPtr[5], argvPtr[6],argvPtr[7],argvPtr[8],argvPtr[9],argvPtr[10],argvPtr[11]));
            }
            cpssOsPrintf(formatStringPtr, argvPtr[0], argvPtr[1], argvPtr[2], argvPtr[3], argvPtr[4], argvPtr[5], argvPtr[6],argvPtr[7],argvPtr[8],argvPtr[9],argvPtr[10],argvPtr[11]);
            break;
        default:
            if (logOutputPrint == UTF_LOG_OUTPUT_FILE_E)
            {
                cpss_fprintf((fp, "[UTF]: prvUtfLogMessage: too many arguments [%d]\n", (int)argc));
            }
            cpssOsPrintf("[UTF]: prvUtfLogMessage: too many arguments [%d]\n", argc);
            break;
    }
}

/**
* @internal prvUtfLogSyncMessage function
* @endinternal
*
* @brief   This routine writes sync message to the output interface.
*         This function handles all issues regarding the logging to specific
*         interfaces.
* @param[in] formatStringPtr          -   (pointer to) format string.
* @param[in] argvPtr[]                -   (pointer to) vector of output string arguments.
* @param[in] argc                     -   number of arguments in output string.
*                                       None.
*
* @note It uses OS abstraction layer to output.
*
*/
GT_VOID prvUtfLogSyncMessage
(
    IN const GT_CHAR    *formatStringPtr,
    IN const GT_UINTPTR argvPtr[],
    IN GT_U32           argc
)
{
    if ((logOutputPrint == UTF_LOG_OUTPUT_SERIAL_FINAL_E) ||
        (logOutputPrint == UTF_LOG_OUTPUT_SERIAL_FINAL_WITH_DEBUG_E) ||
        ((logOutputPrint == UTF_LOG_OUTPUT_FILE_E) && (!prvUtfIsFileOpened)))
    {
        return;
    }

    switch (argc)
    {
        case 0:
            if (logOutputPrint == UTF_LOG_OUTPUT_FILE_E)
            {
                cpss_fprintf((fp, formatStringPtr));
            }
            cpssOsPrintSync(formatStringPtr);
            break;
        case 1:
            if (logOutputPrint == UTF_LOG_OUTPUT_FILE_E)
            {
                cpss_fprintf((fp, formatStringPtr, argvPtr[0]));
            }
            cpssOsPrintSync(formatStringPtr, argvPtr[0]);
            break;
        case 2:
            if (logOutputPrint == UTF_LOG_OUTPUT_FILE_E)
            {
                cpss_fprintf((fp, formatStringPtr, argvPtr[0], argvPtr[1]));
            }
            cpssOsPrintSync(formatStringPtr, argvPtr[0], argvPtr[1]);
            break;
        case 3:
            if (logOutputPrint == UTF_LOG_OUTPUT_FILE_E)
            {
                cpss_fprintf((fp, formatStringPtr, argvPtr[0], argvPtr[1], argvPtr[2]));
            }
            cpssOsPrintSync(formatStringPtr, argvPtr[0], argvPtr[1], argvPtr[2]);
            break;
        default:
            if (logOutputPrint == UTF_LOG_OUTPUT_FILE_E)
            {
                cpss_fprintf((fp, "[UTF]: prvUtfLogSyncMessage: too many arguments [%d]\n", (int)argc));
            }
            cpssOsPrintSync("[UTF]: prvUtfLogSyncMessage: too many arguments [%d]\n", argc);
            break;
    }
}

/**
* @internal prvUtfDebugLogMessage function
* @endinternal
*
* @brief   This routine writes debug message to the serial interface.
*
* @param[in] formatStringPtr          -   (pointer to) format string.
* @param[in] argvPtr[]                -   (pointer to) vector of output string arguments.
* @param[in] argc                     -   number of arguments in output string.
*                                       None.
*
* @note It uses OS abstraction layer to output.
*
*/
GT_VOID prvUtfDebugLogMessage
(
    IN const GT_CHAR    *formatStringPtr,
    IN const GT_UINTPTR argvPtr[],
    IN GT_U32           argc
)
{
    if (logOutputPrint != UTF_LOG_OUTPUT_SERIAL_FINAL_WITH_DEBUG_E)
    {
        return;
    }

    switch (argc)
    {
        case 0:
            cpssOsPrintf(formatStringPtr);
            break;
        case 1:
            cpssOsPrintf(formatStringPtr, argvPtr[0]);
            break;
        case 2:
            cpssOsPrintf(formatStringPtr, argvPtr[0], argvPtr[1]);
            break;
        case 3:
            cpssOsPrintf(formatStringPtr, argvPtr[0], argvPtr[1], argvPtr[2]);
            break;
        default:
            cpssOsPrintf("[UTF]: prvUtfDebugLogMessage: too many arguments [%d]\n", argc);
            break;
    }
}

/**
* @internal prvUtfLogClose function
* @endinternal
*
* @brief   This routine closes logger.
*/
GT_STATUS prvUtfLogClose
(
    GT_VOID
)
{
    if (NULL == fp)
    {
        return GT_BAD_PTR;
    }

    fclose(fp);
    fp = NULL;

    /* set flag */
    prvUtfIsFileOpened = GT_FALSE;

    return GT_OK;
}

/**
* @internal prvUtfLogPrintEnable function
* @endinternal
*
* @brief   This routine set printing mode.
*
* @param[in] utfLogOutputSelectMode   - printing mode.
*                                       the previous settings
*/
UTF_LOG_OUTPUT_ENT prvUtfLogPrintEnable
(
    UTF_LOG_OUTPUT_ENT utfLogOutputSelectMode
)
{
    UTF_LOG_OUTPUT_ENT previous = logOutputPrint;

    /* set only valid mode */
    switch (utfLogOutputSelectMode)
    {
        case UTF_LOG_OUTPUT_SERIAL_ALL_E:
        case UTF_LOG_OUTPUT_FILE_E:
        case UTF_LOG_OUTPUT_SERIAL_FINAL_E:
        case UTF_LOG_OUTPUT_SERIAL_FINAL_WITH_DEBUG_E:
            logOutputPrint = utfLogOutputSelectMode;
            break;
        default:
            logOutputPrint = UTF_LOG_OUTPUT_SERIAL_FINAL_WITH_DEBUG_E;
            break;
    }

    return previous;
}

/**
* @internal prvUtfLogPrintModeGet function
* @endinternal
*
* @brief   This routine gets printing mode.
*/
UTF_LOG_OUTPUT_ENT prvUtfLogPrintModeGet
(
    GT_VOID
)
{
    return logOutputPrint;
}

/**
* @internal prvUtfLogPrintEnableGet function
* @endinternal
*
* @brief   This routine gets printing mode.
*/
GT_BOOL prvUtfLogPrintEnableGet
(
    GT_VOID
)
{
    GT_BOOL            printEnable;

    printEnable = GT_TRUE;
    if ((logOutputPrint == UTF_LOG_OUTPUT_SERIAL_FINAL_E) ||
        (logOutputPrint == UTF_LOG_OUTPUT_SERIAL_FINAL_WITH_DEBUG_E) ||
        (logOutputPrint == UTF_LOG_OUTPUT_FILE_E))
    {
        printEnable = GT_FALSE;
    }

    return printEnable;
}

/**
* @internal prvUtfLogPrintDisableOutput function
* @endinternal
*
* @brief   This routine disables printing by intercepting cpssOsPrintf
*/
GT_VOID prvUtfLogPrintDisableOutput
(
    GT_VOID
)
{
    saved_cpssOsPrintf = cpssOsPrintf;
    cpssOsPrintf = prvUtfLogPrintDummy;
    return;
}

/**
* @internal prvUtfLogPrintEnableOutput function
* @endinternal
*
* @brief   This routine enables printing by restoring saved cpssOsPrintf
*/
GT_VOID prvUtfLogPrintEnableOutput
(
    GT_VOID
)
{
    cpssOsPrintf = saved_cpssOsPrintf;
    return;
}

/**
* @internal prvUtfLogPrintDummy function
* @endinternal
*
* @brief   Dummy printing function.
*
* @note Not for direct use.
*
*/
int prvUtfLogPrintDummy
(
    IN  const char* format,
    IN  ...
)
{
    /* avoid compilation warning */
    (GT_VOID)format;

    return 0;
}

#ifdef DXCH_CODE
/**
* @internal prvUtfPacketSendDummy function
* @endinternal
*
* @brief   Dummy packet send function.
*
* @note Not for direct use.
*
*/
GT_STATUS prvUtfPacketSendDummy
(
    IN GT_U8                                    devNum,
    IN CPSS_DXCH_NET_TX_PARAMS_STC              *pcktParamsPtr,
    IN GT_U8                                    *buffList[],
    IN GT_U32                                   buffLenList[],
    IN GT_U32                                   numOfBufs
)
{
    /* avoid compilation warnings */
    (GT_VOID)devNum;
    (GT_VOID)pcktParamsPtr;
    (GT_VOID)buffList;
    (GT_VOID)buffLenList;
    (GT_VOID)numOfBufs;

    return GT_OK;
}

/**
* @internal prvUtfTrafficSendDisable function
* @endinternal
*
* @brief   This routine disables packets sending in traffic generator by intercepting
*         packet send functions
*/
GT_VOID prvUtfTrafficSendDisable
(
    GT_VOID
)
{
    saved_cpssDxChNetIfSdmaSyncTxPacketSend = cpssDxChNetIfSdmaSyncTxPacketSendPtr;
    cpssDxChNetIfSdmaSyncTxPacketSendPtr = prvUtfPacketSendDummy;

    saved_cpssDxChNetIfMiiSyncTxPacketSend = cpssDxChNetIfMiiSyncTxPacketSendPtr;
    cpssDxChNetIfMiiSyncTxPacketSendPtr = prvUtfPacketSendDummy;
    return;
}

/**
* @internal prvUtfTrafficSendEnable function
* @endinternal
*
* @brief   This routine enables packets sending in traffic generator by restoring
*         saved packet send functions
*/
GT_VOID prvUtfTrafficSendEnable
(
    GT_VOID
)
{
    cpssDxChNetIfSdmaSyncTxPacketSendPtr = saved_cpssDxChNetIfSdmaSyncTxPacketSend;

    cpssDxChNetIfMiiSyncTxPacketSendPtr = saved_cpssDxChNetIfMiiSyncTxPacketSend;
    return;
}
#endif/*DXCH_CODE*/


