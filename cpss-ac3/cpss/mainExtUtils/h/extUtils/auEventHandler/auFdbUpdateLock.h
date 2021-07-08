/*******************************************************************************
*              (c), Copyright 2015, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
********************************************************************************
*/
/**
********************************************************************************
* @file auFdbUpdateLock.h
*
* @brief Address update lock for Fdb update actions
*
* @version   1
********************************************************************************
*/

#ifndef __auFdbUpdateLockh
#define __auFdbUpdateLockh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/*******************************************************************************
* A mutual exclusion required with the following CPSS APIs:
*   - cpssDxChBrgFdbMacEntrySet
*   - cpssDxChBrgFdbQaSend
*   - cpssDxChBrgFdbMacEntryDelete
*   - cpssDxChBrgFdbMacEntryWrite
*   - cpssDxChBrgFdbMacEntryRead
*   - cpssDxChBrgFdbMacEntryInvalidate
*   - cpssDxChBrgFdbFromCpuAuMsgStatusGet
*   - cpssDxChBrgFdbAuMsgBlockGet
*   - cpssDxChBrgFdbFuMsgBlockGet
*   - cpssDxChBrgFdbAuqFuqMessagesNumberGet
*
*   - cpssDxChBrgFdbActionsEnableSet
*   - cpssDxChBrgFdbTrigActionStatusGet
*   - cpssDxChBrgFdbMacTriggerModeSet
*   - cpssDxChBrgFdbTrigActionStart
*   - cpssDxChBrgFdbMacTriggerToggle
*
* Pay attention that pairs of calls must be treated as one call from mutual
* exclusion point of view:
*   LOCK
*   cpssDxChBrgFdbMacEntrySet
*   while(..)
*       cpssDxChBrgFdbFromCpuAuMsgStatusGet;
*   UNLOCK
*
*   LOCK
*   cpssDxChBrgFdbMacEntryDelete
*   while(..)
*       cpssDxChBrgFdbFromCpuAuMsgStatusGet;
*   UNLOCK
*
*   LOCK
*   cpssDxChBrgFdbQaSend
*   while(..)
*       cpssDxChBrgFdbFromCpuAuMsgStatusGet;
*   UNLOCK
*******************************************************************************/
#include <cpss/extServices/private/prvCpssBindFunc.h>
#ifndef INCLUDE_UTF
#  define AU_FDB_UPDATE_LOCK()
#  define AU_FDB_UPDATE_UNLOCK()
#else
   extern CPSS_OS_MUTEX auFdbUpdateLockMtx;
#  define AU_FDB_UPDATE_LOCK() cpssOsMutexLock(auFdbUpdateLockMtx)
#  define AU_FDB_UPDATE_UNLOCK() cpssOsMutexUnlock(auFdbUpdateLockMtx)
#endif

/**
* @internal auFdbUpdateLockInit function
* @endinternal
*
* @brief   Initialize lock
*/
GT_STATUS auFdbUpdateLockInit(void);

/**
* @internal auFdbUpdateLockReset function
* @endinternal
*
* @brief   Uninit lock
*/
GT_STATUS auFdbUpdateLockReset(void);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __auFdbUpdateLockh */

