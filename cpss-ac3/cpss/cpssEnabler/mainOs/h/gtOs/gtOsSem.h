/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE IS A REFERENCE CODE FOR MARVELL SWITCH PRODUCTS.  IT IS PROVIDED   *
* "AS IS" WITH NO WARRANTIES, EXPRESSED, IMPLIED OR OTHERWISE, REGARDING ITS   *
* ACCURACY, COMPLETENESS OR PERFORMANCE.                                       *
* CUSTOMERS ARE FREE TO MODIFY IT AND USE IT ONLY IN THEIR PRODUCTION          *
* SOFTWARE RELEASES WITH MARVELL SWITCH CHIPSETS.                              *
*******************************************************************************/
/**
********************************************************************************
* @file gtOsSem.h
*
* @brief Operating System wrapper. Semaphore facility.
*
* @version   11
********************************************************************************
*/

#ifndef __gtOsSemh
#define __gtOsSemh

#ifdef __cplusplus
extern "C" {
#endif

/************* Includes *******************************************************/

#include <gtOs/gtGenTypes.h>

/************* Defines ********************************************************/

#define OS_WAIT_FOREVER             0
/* try to take semaphore, return immediately even if semaphore still busy */
#define OS_NO_WAIT                  0xFFFFFFFF


/************* Typedefs *******************************************************/
#if (!defined __cmdExtServices_h_) || (defined PSS_PRODUCT)
typedef GT_UINTPTR GT_SEM;
typedef GT_UINTPTR GT_MUTEX;
#endif

typedef enum
{
    OS_SEMB_EMPTY = 0,
    OS_SEMB_FULL
}GT_SEMB_STATE;

/* typedefs for Read / Write locks  */
typedef void * GT_RW_LOCK;

typedef enum
{
    OS_READ_LOCK = 0,
    OS_WRITE_LOCK
}GT_RW_LOCK_TYPE;




/************* Functions ******************************************************/

/**
* @internal osSemBinCreate function
* @endinternal
*
* @brief   Create and initialize a binary semaphore.
*
* @param[in] name                     - semaphore Name
* @param[in] init                     -  value of semaphore (full or empty)
*
* @param[out] smid                     - semaphore Id
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
GT_STATUS osSemBinCreate
(
    IN  const char    *name,
    IN  GT_SEMB_STATE init,
    OUT GT_SEM        *smid
);

/**
* @internal osSemMCreate function
* @endinternal
*
* @brief   Create and initialize a mutext semaphore.
*
* @param[in] name                     - semaphore Name
*
* @param[out] smid                     - semaphore Id
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*
* @note This is not API. Should be used as development tool only.
*       Will be removed.
*
*/
GT_STATUS osSemMCreate
(
    IN  const char    *name,
    OUT GT_SEM        *smid
);

/**
* @internal osSemCCreate function
* @endinternal
*
* @brief   Create counting semaphore.
*
* @param[in] name                     - semaphore Name
* @param[in] init                     -  value of semaphore
*
* @param[out] smid                     - semaphore Id
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
GT_STATUS osSemCCreate
(
    IN  const char    *name,
    IN  GT_U32        init,
    OUT GT_SEM        *smid
);

/**
* @internal osSemDelete function
* @endinternal
*
* @brief   Delete semaphore.
*
* @param[in] smid                     - semaphore Id
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
GT_STATUS osSemDelete
(
    IN GT_SEM smid
);

/**
* @internal osSemWait function
* @endinternal
*
* @brief   Wait on semaphore.
*
* @param[in] smid                     - semaphore Id
* @param[in] timeOut                  - time out in miliseconds or OS_WAIT_FOREVER to wait forever
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval OS_TIMEOUT               - on time out
*/
GT_STATUS osSemWait
(
    IN GT_SEM smid,
    IN GT_U32 timeOut
);

/**
* @internal osSemSignal function
* @endinternal
*
* @brief   Signal a semaphore.
*
* @param[in] smid                     - semaphore Id
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
GT_STATUS osSemSignal
(
    IN GT_SEM smid
);


/**
* @internal CPSS_osMutexCreate function
* @endinternal
*
* @brief   Create and initialize a Mutex object.
*
* @param[in] name                     - mutex Name
*
* @param[out] mtxid                    - mutex Id
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
#define osMutexCreate CPSS_osMutexCreate
GT_STATUS CPSS_osMutexCreate
(
    IN  const char      *name,
    OUT GT_MUTEX        *mtxid
);

/**
* @internal CPSS_osMutexDelete function
* @endinternal
*
* @brief   Delete mutex.
*
* @param[in] mtxid                    - mutex Id
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
#define osMutexDelete CPSS_osMutexDelete
GT_STATUS CPSS_osMutexDelete
(
    IN GT_MUTEX mtxid
);

/**
* @internal osMutexLock function
* @endinternal
*
* @brief   Lock a mutex.
*
* @param[in] mtxid                    - mutex Id
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
GT_STATUS osMutexLock
(
    IN GT_MUTEX mtxid
);

/**
* @internal osMutexUnlock function
* @endinternal
*
* @brief   Unlock a mutex.
*
* @param[in] mtxid                    - mutex Id
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
GT_STATUS osMutexUnlock
(
    IN GT_MUTEX mtxid
);


/**
* @internal osRwLockCreate function
* @endinternal
*
* @brief   Create and initialize a Read / Write lock.
*
* @param[in] name                     - Lock Name.
*
* @param[out] lockId                   - semaphore Id
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*
* @note 1. The lock is created in empty state.
*
*/
GT_STATUS osRwLockCreate
(
    IN  const char  *name,
    OUT GT_RW_LOCK  *lockId
);

/**
* @internal osRwLockDelete function
* @endinternal
*
* @brief   Delete a read / write lock.
*
* @param[in] lockId                   - Lock Id
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
GT_STATUS osRwLockDelete
(
    IN GT_RW_LOCK lockId
);


/**
* @internal osRwLockWait function
* @endinternal
*
* @brief   Wait on a read / write lock.
*
* @param[in] lockId                   - Lock Id.
* @param[in] lockType                 - The lock type.
* @param[in] timeOut                  - time out in miliseconds or 0 to wait forever.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval OS_TIMEOUT               - on time out
*/
GT_STATUS osRwLockWait
(
    IN GT_RW_LOCK       lockId,
    IN GT_RW_LOCK_TYPE  lockType,
    IN GT_U32           timeOut
);


/**
* @internal osRwLockSignal function
* @endinternal
*
* @brief   Signal a Read / Write lock.
*
* @param[in] lockId                   - Lock Id.
* @param[in] lockType                 - The lock type this lock was taken for, read / write.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
GT_STATUS osRwLockSignal
(
    IN GT_RW_LOCK       lockId,
    IN GT_RW_LOCK_TYPE  lockType
);

#if defined  CPSS_USE_MUTEX_PROFILER

/**
* @internal osMutexSetGlAttributes function
* @endinternal
*
* @brief   Set granular locking attributes to mutex. Such as type of the mutex related to the granular locking
*         and device number.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
GT_STATUS osMutexSetGlAttributes
( 
    IN GT_MUTEX                           mtxId,
    IN GT_BOOL                            isZeroLevel,
    IN GT_BOOL                            isDeviceOriented,
    IN GT_BOOL                            isRxTxFunctionality,
    IN GT_U8                              devNum
);
/**
* @internal osCheckUnprotectedPerDeviceDbAccess function
* @endinternal
*
* @brief   This function checks that device related mutex is locked when accessing device
*         related software database
* @param[in] devNum                   - device mumber
* @param[in] functionName             - name of the function that is being checked
*                                       NONE
*/

GT_VOID osCheckUnprotectedPerDeviceDbAccess
( 
    IN GT_U32                                   devNum,
    IN const char *                           functionName
 );

#endif

#ifdef __cplusplus
}
#endif /*CPSS_USE_MUTEX_PROFILER*/

#endif  /* __gtOsSemh */
/* Do Not Add Anything Below This Line */



