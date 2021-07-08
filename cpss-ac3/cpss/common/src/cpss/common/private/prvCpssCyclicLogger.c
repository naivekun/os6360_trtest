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
* @file prvCpssCyclicLogger.c
*
* @brief CPSS implementation for cyclic logging.
*
*
* @version   1
********************************************************************************
*/

#include <cpss/common/cpssTypes.h>
#include <cpss/common/private/prvCpssCyclicLogger.h>
#include <cpss/generic/log/prvCpssLog.h>
#include <cpssCommon/cpssPresteraDefs.h>
#include <cpss/extServices/private/prvCpssBindFunc.h>

#define CPSS_CYCLIC_LOGGER_LOG_NO_PORT 0xFFFFFFFF
/* global string buffer for log entry addition */
static char cyclicLoggerGlobalBuffer[CPSS_CYCLIC_LOGGER_LOG_STRING_SIZE];
/* port manager log history records entries buffer */
static CPSS_CYCLIC_LOGGER_LOG_ENTRY cyclicLoggerLogHistory[CPSS_CYCLIC_LOGGER_LOG_SIZE];
/* log entries buffer pointer */
static GT_U32 cyclicLoggerLogPointer;
/* log entries buffer counter */
static GT_U32 cyclicLoggerLogCount;
static GT_U32 cyclicLoggerLogPseudoTimestamp;

static GT_U8 cyclicLoggerDumpPortList[CPSS_MAX_PORTS_NUM_CNS/8+1];
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
)
{
    va_list args;

    va_start(args, format);

    cpssOsVsnprintf(cyclicLoggerGlobalBuffer, sizeof(cyclicLoggerGlobalBuffer), format, args);
    prvCpssCyclicLoggerLogEntryAdd(cyclicLoggerGlobalBuffer, line, CPSS_CYCLIC_LOGGER_LOG_NO_PORT);

    va_end(args);
}
GT_VOID prvCpssCyclicLoggerPortAddRecord
(
    IN GT_U32 line,
    IN GT_U32 port,
    IN const char* format,
    ...
)
{
    if (port > CPSS_MAX_PORTS_NUM_CNS)
    {
        return;
    }
    if ((1<<(port % 8)) & (cyclicLoggerDumpPortList[port/8]))
    {
        va_list args;

        va_start(args, format);

        cpssOsVsnprintf(cyclicLoggerGlobalBuffer, sizeof(cyclicLoggerGlobalBuffer), format, args);
        prvCpssCyclicLoggerLogEntryAdd(cyclicLoggerGlobalBuffer, line, port);

        va_end(args);
    }
}

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
)
{
    CPSS_ZERO_LEVEL_API_LOCK_MAC

    cpssOsMemSet(cyclicLoggerLogHistory, 0, CPSS_CYCLIC_LOGGER_LOG_SIZE * sizeof(CPSS_CYCLIC_LOGGER_LOG_ENTRY));
    cyclicLoggerLogPointer    = 0;
    cyclicLoggerLogCount      = 0;
    cyclicLoggerLogPseudoTimestamp = 0;
    cpssOsMemSet(cyclicLoggerDumpPortList,0xFF,sizeof(cyclicLoggerDumpPortList));
    CPSS_ZERO_LEVEL_API_UNLOCK_MAC

    return GT_OK;
}

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

)
{
    CPSS_CYCLIC_LOGGER_LOG_ENTRY entry;
    GT_U32 timerSeconds, timerNsec;
    GT_STATUS rc;

    /* macro for API that does not use device number and does not have nested CPSS API calls */
    CPSS_ZERO_LEVEL_API_LOCK_MAC;

    /* getting time-stamp */
    rc = cpssOsTimeRT(&timerSeconds,&timerNsec);
    if (rc!=GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "calling cpssOsTimeRT failed=%d", rc);
    }

    /* Building the entry */
    entry.psudoTimeStamp = ++cyclicLoggerLogPseudoTimestamp;
    /* general timing units: second -> mili second -> micro second -> nano second */
    /* seconds  - cyclic count up to 1023 */
    entry.timeStamp[0] = timerSeconds % 1000;
    /* mili-sec */
    entry.timeStamp[1] = ( (timerNsec / 1000000) % (1000) );
    /* micro-sec */
    entry.timeStamp[2] = ( (timerNsec / 1000) % (1000) );

    cpssOsMemCpy(entry.info, msgPtr, CPSS_CYCLIC_LOGGER_LOG_STRING_SIZE);
    entry.line = line;
    entry.port = port;

    /* Adding the message entry */
    cpssOsMemCpy( &(cyclicLoggerLogHistory[cyclicLoggerLogPointer]), &entry, sizeof(CPSS_CYCLIC_LOGGER_LOG_ENTRY));

    /* Cyclic pointer behaviour */
    if (cyclicLoggerLogPointer+1 >= CPSS_CYCLIC_LOGGER_LOG_SIZE)
    {
        cyclicLoggerLogPointer = 0;
    }
    else
    {
        /* Incrementing the pointer */
        cyclicLoggerLogPointer +=1 ;
    }

    /* Incrementing the counter and handling counter overflow.
       We will allow counter with CPSS_CYCLIC_LOGGER_LOG_SIZE+1 value
       for the host to know about overlap */
    if (cyclicLoggerLogCount <= CPSS_CYCLIC_LOGGER_LOG_SIZE)
    {
        cyclicLoggerLogCount += 1;
    }

    CPSS_ZERO_LEVEL_API_UNLOCK_MAC;

    return GT_OK;
}

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
)
{
    GT_U32 iteration=0;
    GT_U32 logPointer=0;
    GT_U32 idx = 0;
    GT_U32 logCount=0;

    /* macro for API that does not use device number and does not have nested CPSS API calls */
    CPSS_ZERO_LEVEL_API_LOCK_MAC;

    logCount = cyclicLoggerLogCount;
    logPointer = cyclicLoggerLogPointer;

    cpssOsPrintf("\n Port Manager Realtime Log (up to %d characters per message)\n"
                "===========================================================\n", CPSS_CYCLIC_LOGGER_LOG_STRING_SIZE);

    /* If there was overlap, start from the current pointer and read whole log (with overlapping) */
    if (logCount >= CPSS_CYCLIC_LOGGER_LOG_SIZE)
    {
        for (iteration=0, idx=logPointer; iteration < CPSS_CYCLIC_LOGGER_LOG_SIZE; idx++, iteration++)
        {
            if (idx == CPSS_CYCLIC_LOGGER_LOG_SIZE )
            {
                idx = 0;
            }
            if ((cyclicLoggerLogHistory[idx].port!= CPSS_CYCLIC_LOGGER_LOG_NO_PORT))
            {
                cpssOsPrintf("[%u][%03u.%03u.%03u] [Port %2d] %s [Line %d]\n",
                             cyclicLoggerLogHistory[idx].psudoTimeStamp,
                             cyclicLoggerLogHistory[idx].timeStamp[0],
                             cyclicLoggerLogHistory[idx].timeStamp[1],
                             cyclicLoggerLogHistory[idx].timeStamp[2],
                             cyclicLoggerLogHistory[idx].port,
                             cyclicLoggerLogHistory[idx].info,
                             cyclicLoggerLogHistory[idx].line);
            }
            else
            {
                cpssOsPrintf("[%u][%03u.%03u.%03u] %s [Line %d]\n",
                             cyclicLoggerLogHistory[idx].psudoTimeStamp,
                             cyclicLoggerLogHistory[idx].timeStamp[0],
                             cyclicLoggerLogHistory[idx].timeStamp[1],
                             cyclicLoggerLogHistory[idx].timeStamp[2],
                             cyclicLoggerLogHistory[idx].info,
                             cyclicLoggerLogHistory[idx].line);
            }
        }
    }
    else
    {
        /* No overlap happend yet */
        for (idx=0; idx < logPointer; idx++)
        {
            if ((cyclicLoggerLogHistory[idx].port!= CPSS_CYCLIC_LOGGER_LOG_NO_PORT))
            {
                cpssOsPrintf("[%u][%03u.%03u.%03u] [Port %d] %s [Line %d]\n",
                             cyclicLoggerLogHistory[idx].psudoTimeStamp,
                             cyclicLoggerLogHistory[idx].timeStamp[0],
                             cyclicLoggerLogHistory[idx].timeStamp[1],
                             cyclicLoggerLogHistory[idx].timeStamp[2],
                             cyclicLoggerLogHistory[idx].port,
                             cyclicLoggerLogHistory[idx].info,
                             cyclicLoggerLogHistory[idx].line);
            }
            else
            {
                cpssOsPrintf("[%u][%03u.%03u.%03u] %s [Line %d]\n",
                             cyclicLoggerLogHistory[idx].psudoTimeStamp,
                             cyclicLoggerLogHistory[idx].timeStamp[0],
                             cyclicLoggerLogHistory[idx].timeStamp[1],
                             cyclicLoggerLogHistory[idx].timeStamp[2],
                             cyclicLoggerLogHistory[idx].info,
                             cyclicLoggerLogHistory[idx].line);
            }
        }
    }

    cpssOsPrintf("\n");

    CPSS_ZERO_LEVEL_API_UNLOCK_MAC;

    return GT_OK;
}

/**
* @internal prvCpssCyclicLoggerPortListClear function
* @endinternal
*
* @brief   Clear dump log port list
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; Lion2; xCat2; Bobcat2; AC3X; Caelum; Aldrin; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on failure
*/
GT_STATUS prvCpssCyclicLoggerPortListClear
(
    IN GT_VOID
)
{
    cpssOsMemSet(cyclicLoggerDumpPortList,0,sizeof(cyclicLoggerDumpPortList));
    return GT_OK;

}

/**
* @internal prvCpssCyclicLoggerPortListAdd function
* @endinternal
*
* @brief   Add port to dump log port list
* @param[in] port                  - port  number
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; Lion2; xCat2; Bobcat2; AC3X; Caelum; Aldrin; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on failure
*/
GT_STATUS prvCpssCyclicLoggerPortListAdd
(
    IN GT_U32   port
)
{
    if (port > CPSS_MAX_PORTS_NUM_CNS)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    cyclicLoggerDumpPortList[port/8] |= 1 << (port%8);
    return GT_OK;
}

/**
* @internal prvCpssCyclicLoggerPortListRmv function
* @endinternal
*
* @brief   Remove port to dump log port list
* @param[in] port                  - port  number
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; Lion2; xCat2; Bobcat2; AC3X; Caelum; Aldrin; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on failure
*/
GT_STATUS prvCpssCyclicLoggerPortListRmv
(
    IN GT_U32   port
)
{
    if (port > CPSS_MAX_PORTS_NUM_CNS)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    cyclicLoggerDumpPortList[port/8] &= ~(1 << (port%8));
    return GT_OK;
}

/**
* @internal prvCpssCyclicLoggerPortListRmv function
* @endinternal
*
* @brief   Add all ports to dump log port list
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; Lion2; xCat2; Bobcat2; AC3X; Caelum; Aldrin; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on failure
*/
GT_STATUS prvCpssCyclicLoggerPortListAll
(
    IN GT_VOID
)
{
    cpssOsMemSet(cyclicLoggerDumpPortList,0xFF, sizeof(cyclicLoggerDumpPortList));
    return GT_OK;

}

