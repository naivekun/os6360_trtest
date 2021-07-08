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
* @file prvUtfHelpers.h
*
* @brief Internal header which defines API for logger, time management,
* memory allocation.
* It is used only in the utfMain.c
*
* @version   10
********************************************************************************
*/
#ifndef __prvUtfHelpersh
#define __prvUtfHelpersh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* defines */
/*
 * Description: Defines routines to be done to log message.
 *      It is used when there is no any argument in output string.
 *
 * Parameters:
 *      x - format string of log message
 */
#define PRV_UTF_LOG0_MAC(x)                                                    \
    do {                                                                       \
        prvUtfLogMessage(x, NULL, 0);                                          \
    } while (0)

/*
 * Description: Defines routines to be done to log message.
 *      It is used when there is only one argument in output string.
 *
 * Parameters:
 *      x       -   format string of log message
 *      arg1    -   argument for output string
 */
#define PRV_UTF_LOG1_MAC(x, arg1)                                              \
    do {                                                                       \
        UTF_ARGVP_ARR_1_MAC(argvl,arg1);                        \
        prvUtfLogMessage(x, argvl, 1);                                         \
    } while (0)

/*
 * Description: Defines routines to be done to log message.
 *      It is used when there are two arguments in output string.
 *
 * Parameters:
 *      x           -   format string of log message
 *      arg1, arg2  -   arguments for output string
 */
#define PRV_UTF_LOG2_MAC(x, arg1, arg2)                                        \
    do {                                                                       \
        UTF_ARGVP_ARR_2_MAC(argvl,arg1,arg2);                        \
        prvUtfLogMessage(x, argvl, 2);                                         \
    } while (0)

/*
 * Description: Defines routines to be done to log message.
 *      It is used when there are three arguments in output string.
 *
 * Parameters:
 *      x                   -   format string of log message
 *      arg1, arg2, arg3    -   arguments for output string
 */
#define PRV_UTF_LOG3_MAC(x, arg1, arg2, arg3)                                  \
    do {                                                                       \
        UTF_ARGVP_ARR_3_MAC(argvl,arg1,arg2,arg3);                        \
        prvUtfLogMessage(x, argvl, 3);                                         \
    } while (0)

/*
 * Description: Defines routines to be done to log message.
 *      It is used when there are four arguments in output string.
 *
 * Parameters:
 *      x                           -   format string of log message
 *      arg1, arg2, arg3, arg4      -   arguments for output string
 */
#define PRV_UTF_LOG4_MAC(x, arg1, arg2, arg3, arg4)                            \
    do {                                                                       \
        UTF_ARGVP_ARR_4_MAC(argvl,arg1,arg2,arg3,arg4);                        \
        prvUtfLogMessage(x, argvl, 4);                                         \
    } while (0)


/*
 * Description: Defines routines to be done to log message.
 *      It is used when there are five arguments in output string.
 *
 * Parameters:
 *      x                               -   format string of log message
 *      arg1, arg2, arg3, arg4, arg5    -   arguments for output string
 */
#define PRV_UTF_LOG5_MAC(x, arg1, arg2, arg3, arg4, arg5)                      \
    do {                                                                       \
        UTF_ARGVP_ARR_5_MAC(argvl,arg1,arg2,arg3,arg4,arg5);                        \
        prvUtfLogMessage(x, argvl, 5);                                         \
    } while (0)

/*
 * Description: Defines routines to be done to log message.
 *      It is used when there are six arguments in output string.
 *
 * Parameters:
 *      x                                   -   format string of log message
 *      arg1, arg2, arg3, arg4, arg5, arg6  -   arguments for output string
 */
#define PRV_UTF_LOG6_MAC(x, arg1, arg2, arg3, arg4, arg5, arg6)                \
    do {                                                                       \
        UTF_ARGVP_ARR_6_MAC(argvl,arg1,arg2,arg3,arg4,arg5,arg6);              \
        prvUtfLogMessage(x, argvl, 6);                                         \
    } while (0)

/*
 * Description: Defines routines to be done to log message.
 *      It is used when there are seven arguments in output string.
 *
 * Parameters:
 *      x                                   -   format string of log message
 *      arg1, arg2, arg3, arg4, arg5, arg6, arg7  -   arguments for output string
 */
#define PRV_UTF_LOG7_MAC(x, arg1, arg2, arg3, arg4, arg5, arg6, arg7)          \
    do {                                                                       \
        UTF_ARGVP_ARR_7_MAC(argvl,arg1,arg2,arg3,arg4,arg5,arg6,arg7);         \
        prvUtfLogMessage(x, argvl, 7);                                         \
    } while (0)

/*
 * Description: Defines routines to be done to log message.
 *      It is used when there are seven arguments in output string.
 *
 * Parameters:
 *      x                                       - format string of log message
 *      arg1,arg2,arg3,arg4,arg5,arg6,arg7,arg8 - arguments for output string
 */
#define PRV_UTF_LOG8_MAC(x, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8)    \
    do {                                                                       \
        UTF_ARGVP_ARR_8_MAC(argvl,arg1,arg2,arg3,arg4,arg5,arg6,arg7,arg8);    \
        prvUtfLogMessage(x, argvl, 8);                                         \
    } while (0)

/*
 * Description: Defines routines to be done to log message.
 *      It is used when there are seven arguments in output string.
 *
 * Parameters:
 *      x                                            - format string of log message
 *      arg1,arg2,arg3,arg4,arg5,arg6,arg7,arg8,arg9 - arguments for output string
 */
#define PRV_UTF_LOG9_MAC(x,arg1,arg2,arg3,arg4,arg5,arg6,arg7,arg8,arg9)       \
    do {                                                                       \
        UTF_ARGVP_ARR_9_MAC(argvl,arg1,arg2,arg3,arg4,arg5,arg6,arg7,arg8,arg9);\
        prvUtfLogMessage(x, argvl, 9);                                         \
    } while (0)

/*
 * Description: Defines routines to be done to log message.
 *      It is used when there are seven arguments in output string.
 *
 * Parameters:
 *      x                                            - format string of log message
 *      arg1,arg2,arg3,arg4,arg5,arg6,arg7,arg8,arg9,arg10 - arguments for output string
 */
#define PRV_UTF_LOG10_MAC(x,arg1,arg2,arg3,arg4,arg5,arg6,arg7,arg8,arg9,arg10)       \
    do {                                                                       \
        UTF_ARGVP_ARR_10_MAC(argvl,arg1,arg2,arg3,arg4,arg5,arg6,arg7,arg8,arg9,arg10);\
        prvUtfLogMessage(x, argvl, 10);                                         \
    } while (0)

/*
 * Description: Defines routines to be done to log message.
 *      It is used when there are seven arguments in output string.
 *
 * Parameters:
 *      x                                            - format string of log message
 *      arg1,arg2,arg3,arg4,arg5,arg6,arg7,arg8,arg9,arg10,arg11 - arguments for output string
 */
#define PRV_UTF_LOG11_MAC(x,arg1,arg2,arg3,arg4,arg5,arg6,arg7,arg8,arg9,arg10,arg11)       \
    do {                                                                       \
        UTF_ARGVP_ARR_11_MAC(argvl,arg1,arg2,arg3,arg4,arg5,arg6,arg7,arg8,arg9,arg10,arg11);\
        prvUtfLogMessage(x, argvl, 11);                                         \
    } while (0)

/*
 * Description: Defines routines to be done to log message.
 *      It is used when there are seven arguments in output string.
 *
 * Parameters:
 *      x                                            - format string of log message
 *      arg1,arg2,arg3,arg4,arg5,arg6,arg7,arg8,arg9,arg10,arg11,arg12 - arguments for output string
 */
#define PRV_UTF_LOG12_MAC(x,arg1,arg2,arg3,arg4,arg5,arg6,arg7,arg8,arg9,arg10,arg11,arg12)       \
    do {                                                                       \
        UTF_ARGVP_ARR_12_MAC(argvl,arg1,arg2,arg3,arg4,arg5,arg6,arg7,arg8,arg9,arg10,arg11,arg12);\
        prvUtfLogMessage(x, argvl, 12);                                         \
    } while (0)

/*
 * Description: Defines routines to be done to log sync message.
 *      It is used when there is no any argument in output string.
 *
 * Parameters:
 *      x - format string of log message
 */
#define PRV_UTF_LOG0_SYNC_MAC(x)                                               \
    do {                                                                       \
        prvUtfLogSyncMessage(x, NULL, 0);                                      \
    } while (0)

/*
 * Description: Defines routines to be done to log sync message.
 *      It is used when there is only one argument in output string.
 *
 * Parameters:
 *      x       -   format string of log message
 *      arg1    -   argument for output string
 */
#define PRV_UTF_LOG1_SYNC_MAC(x, arg1)                                         \
    do {                                                                       \
        UTF_ARGVP_ARR_1_MAC(argvl,arg1);                                       \
        prvUtfLogSyncMessage(x, argvl, 1);                                     \
    } while (0)

/*
 * Description: Defines routines to be done to log sync message.
 *      It is used when there are two arguments in output string.
 *
 * Parameters:
 *      x           -   format string of log message
 *      arg1, arg2  -   arguments for output string
 */
#define PRV_UTF_LOG2_SYNC_MAC(x, arg1, arg2)                                   \
    do {                                                                       \
        UTF_ARGVP_ARR_2_MAC(argvl,arg1,arg2);                                  \
        prvUtfLogSyncMessage(x, argvl, 2);                                     \
    } while (0)

/*
 * Description: Defines routines to be done to log sync message.
 *      It is used when there are two arguments in output string.
 *
 * Parameters:
 *      x                 -   format string of log message
 *      arg1, arg2, arg3  -   arguments for output string
 */
#define PRV_UTF_LOG3_SYNC_MAC(x, arg1, arg2, arg3)                             \
    do {                                                                       \
        UTF_ARGVP_ARR_3_MAC(argvl,arg1,arg2,arg3);                             \
        prvUtfLogSyncMessage(x, argvl, 3);                                     \
    } while (0)

/*
 * Description: Defines routines to be done to log debug message.
 *      It is used when there is no any argument in output string.
 *
 * Parameters:
 *      x - format string of log message
 */
#define PRV_UTF_LOG0_DEBUG_MAC(x)                                              \
    do {                                                                       \
        prvUtfDebugLogMessage(x, NULL, 0);                                     \
    } while (0)

/*
 * Description: Defines routines to be done to log debug message.
 *      It is used when there is only one argument in output string.
 *
 * Parameters:
 *      x       -   format string of log message
 *      arg1    -   argument for output string
 */
#define PRV_UTF_LOG1_DEBUG_MAC(x, arg1)                                        \
    do {                                                                       \
        UTF_ARGVP_ARR_1_MAC(argvl,arg1);                                       \
        prvUtfDebugLogMessage(x, argvl, 1);                                    \
    } while (0)

/*
 * Description: Defines routines to be done to log debug message.
 *      It is used when there are two arguments in output string.
 *
 * Parameters:
 *      x           -   format string of log message
 *      arg1, arg2  -   arguments for output string
 */
#define PRV_UTF_LOG2_DEBUG_MAC(x, arg1, arg2)                                  \
    do {                                                                       \
        UTF_ARGVP_ARR_2_MAC(argvl,arg1,arg2);                                  \
        prvUtfDebugLogMessage(x, argvl, 2);                                    \
    } while (0)

/*
 * Description: Defines routines to be done to log debug message.
 *      It is used when there are two arguments in output string.
 *
 * Parameters:
 *      x                 -   format string of log message
 *      arg1, arg2, arg3  -   arguments for output string
 */
#define PRV_UTF_LOG3_DEBUG_MAC(x, arg1, arg2, arg3)                            \
    do {                                                                       \
        UTF_ARGVP_ARR_3_MAC(argvl,arg1,arg2,arg3);                             \
        prvUtfDebugLogMessage(x, argvl, 3);                                    \
    } while (0)

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
);

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
);

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
*
* @retval GT_OK                    -  logging has been done successfully.
* @retval GT_FAIL                  -  General failure error. Should never happen.
*
* @note It uses OS abstraction layer to output.
*
*/
GT_VOID prvUtfLogSyncMessage
(
    IN const GT_CHAR    *formatStringPtr,
    IN const GT_UINTPTR argvPtr[],
    IN GT_U32           argc
);

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
);

/**
* @internal prvUtfLogClose function
* @endinternal
*
* @brief   This routine closes logger.
*/
GT_STATUS prvUtfLogClose
(
    GT_VOID
);


/* TODO: add time management and memory allocation functions here */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvUtfHelpersh */


