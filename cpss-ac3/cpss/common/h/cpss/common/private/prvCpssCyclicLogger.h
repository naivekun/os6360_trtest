/********************************************************************************
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
* @file prvCpssCyclicLogger.h
*
* @brief CPSS implementation for cyclic logging.
*
*
* @version   1
********************************************************************************
*/

#ifndef __cpssCyclicLogger__
#define __cpssCyclicLogger__

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpss/common/cpssTypes.h>

/*
    The cyclic logger is needed for any state machine code.
    Currently, the cyclic logger is used by Port Manager.
*/

#define CPSS_CYCLIC_LOGGER_PORT_ADD_RECORD(...)                       \
    do                                                          \
    {                                                           \
        /*osPrintf(__VA_ARGS__);*/                              \
        prvCpssCyclicLoggerPortAddRecord(__VA_ARGS__);              \
    } while (0)

/* Add log message to the log record */
#define CPSS_CYCLIC_LOGGER_ADD_RECORD(...)                       \
    do                                                          \
    {                                                           \
        /*osPrintf(__VA_ARGS__);*/                              \
        prvCpssCyclicLoggerAddRecord(__VA_ARGS__);              \
    } while (0)

/* Add to log record then perform CPSS log */
#define CPSS_CYCLIC_LOGGER_LOG_ERROR_AND_RETURN_MAC(_rc, ...)  \
    do {                                                      \
        /*osPrintf(__VA_ARGS__);*/                            \
        CPSS_CYCLIC_LOGGER_ADD_RECORD(__LINE__,__VA_ARGS__);            \
        CPSS_LOG_ERROR_AND_RETURN_MAC(_rc, __VA_ARGS__);      \
    } while (0)

/* Add to log record then perform CPSS log */
#define CPSS_CYCLIC_LOGGER_LOG_INFORMATION_MAC(...)            \
    do {                                                      \
        CPSS_CYCLIC_LOGGER_ADD_RECORD(__LINE__,__VA_ARGS__);            \
        CPSS_LOG_INFORMATION_MAC(__VA_ARGS__);                \
    } while (0)


#define CPSS_CYCLIC_LOGGER_LOG_PORT_INFORMATION_MAC(_port,...)            \
    do {                                                      \
        CPSS_CYCLIC_LOGGER_PORT_ADD_RECORD(__LINE__,_port,__VA_ARGS__);            \
        CPSS_LOG_INFORMATION_MAC(__VA_ARGS__);                \
    } while (0)

/* Add to log record then perform CPSS log */
#define CPSS_CYCLIC_LOGGER_DBG_LOG_INFORMATION_MAC(...)        \
    do {                                                      \
        /*osPrintf(__VA_ARGS__);*/                            \
        CPSS_CYCLIC_LOGGER_ADD_RECORD(__LINE__,__VA_ARGS__);            \
        CPSS_LOG_INFORMATION_MAC(__VA_ARGS__);                \
    } while (0)

#define CPSS_CYCLIC_LOGGER_LOG_STRING_SIZE   256
#define CPSS_CYCLIC_LOGGER_LOG_SIZE          1000

/**
* @struct CPSS_CYCLIC_LOGGER_LOG_ENTRY
 *
 * @brief This structure contains stability results for each port low level unit
*/
typedef struct{

    /** log entry counter */
    GT_U32 psudoTimeStamp;

    GT_U32 timeStamp[3];

    GT_U8 info[CPSS_CYCLIC_LOGGER_LOG_STRING_SIZE];

    /** @brief log line number
     *  Comments:
     *  None.
     */
    GT_U32 line;
    GT_U32 port;

} CPSS_CYCLIC_LOGGER_LOG_ENTRY;

/**
* @internal prvCpssCyclicLoggerAddRecord function
* @endinternal
*
* @brief   Add log entry record from give format and arguments.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; Lion2; xCat2; Bobcat2; AC3X; Caelum; Aldrin; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] line                     - log  number
* @param[in] format                   - log entry  starting.
*                                      ...          - optional parameters according to formatPtr
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on failure
*/
GT_VOID prvCpssCyclicLoggerAddRecord
(
    IN GT_U32 line,
    IN const char* format,
    ...
);

GT_VOID prvCpssCyclicLoggerPortAddRecord
(
    IN GT_U32 line,
    IN GT_U32 port,
    IN const char* format,
    ...
);

/**
* @internal prvCpssCyclicLoggerInit function
* @endinternal
*
* @brief   Init cyclic log entried and counters.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; Lion2; xCat2; Bobcat2; AC3X; Caelum; Aldrin; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on failure
*/
GT_STATUS prvCpssCyclicLoggerInit
(
    IN GT_VOID
);

/**
* @internal prvCpssCyclicLoggerLogEntryAdd function
* @endinternal
*
* @brief   Add log entry to the cyclic log entried db.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; Lion2; xCat2; Bobcat2; AC3X; Caelum; Aldrin; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on failure
*/
GT_STATUS prvCpssCyclicLoggerLogEntryAdd
(
    IN char *msgPtr,
    IN GT_U32 line,
    IN GT_U32 port
);

/**
* @internal prvCpssCyclicLoggerLogEntriesDump function
* @endinternal
*
* @brief   Dump log history.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; Lion2; xCat2; Bobcat2; AC3X; Caelum; Aldrin; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on failure
*/
GT_STATUS prvCpssCyclicLoggerLogEntriesDump
(
    IN GT_VOID
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __cpssCyclicLogger__ */

