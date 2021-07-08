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
* @file osWin32Sem.c
*
* @brief Win32 Operating System Simulation. Semaphore facility.
*
* @version   6
********************************************************************************
*/


#include <windows.h>
#include <process.h>
#include <time.h>

#include <stdio.h>
#include <assert.h>

#include <winbase.h>



#include <gtOs/gtOsSem.h>
#include <gtOs/gtOsMem.h>
#include <gtOs/gtOsIo.h>
#include <gtOs/gtOsSemDefs.h.>



#ifdef OS_WIN32_MTX_AS_CRITICAL_SECTION
static CRITICAL_SECTION userspaceMutexes_mtx;

OS_OBJECT_LIST_STC userspaceMutexes = {
    NULL,                   /* list */
    0,                      /* allocated */
    128,                    /* allocChunk */
    sizeof(GT_WIN32_MUTEX), /* objSize */
    malloc,                 /* allocFuncPtr */
    free                    /* freeFuncPtr */
};


#endif



/************ Internal Typedefs ***********************************************/

#if defined  CPSS_USE_MUTEX_PROFILER   

extern GT_STATUS osMutexLockGlValidation
(
    IN GT_MUTEX mtxid
);


#endif/* CPSS_USE_MUTEX_PROFILER */


/*
 * Typedef: struct GT_WIN_RW_LOCK
 *
 * Description: Defines the Read / Write lock type for Win32 implementation.
 *
 * Fields:
 *      numOfReaders    - Number of readers currently accessing the data-base.
 *      cntSem          - A binary semaphore to protect the access to
 *                        numOfReaders.
 *      writerSem       - A binary semaphore used to prevent further readers
 *                        from getting the lock after the first write arrives.
 *      dbSem           - A semaphore to protect the data-base we are accessing.
 *
 */
typedef struct _winRwLock
{
    GT_U32  numOfReaders;
    GT_SEM  cntSem;
    GT_SEM  writerSem;
    GT_SEM  dbSem;
}GT_WIN_RW_LOCK;



/************ Public Functions ************************************************/

/*******************************************************************************
* osSemBinCreate
*
* DESCRIPTION:
*       Create and initialize a binary semaphore.
*
* INPUTS:
*       name   - semaphore Name
*       init   - init value of semaphore (full or empty)
*
* OUTPUTS:
*       smid - semaphore Id
*
* RETURNS:
*       GT_OK   - on success
*       GT_FAIL - on error
*
* COMMENTS:
*       None
*
*******************************************************************************/
static unsigned long scount = 0;

GT_STATUS osSemBinCreate
(
    IN  const char    *name,
    IN  GT_SEMB_STATE init,
    OUT GT_SEM        *smid
)
{
    char sname[64];

    if (scount == 0)
        scount =  GetCurrentThreadId();

    _snprintf(sname,sizeof(sname),"%s%u",name,scount++);
    sname[sizeof(sname)-1] = 0;
    *smid = (GT_SEM)CreateSemaphore(NULL,       /* no security attributes   */
                          (unsigned long)init,  /* initial count            */
                          (unsigned long)1,     /* maximum count            */
                           sname);              /* semaphore name           */

    if (*smid == 0)
      return GT_FAIL;

    return GT_OK;
}

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
)
{
    if (CloseHandle((HANDLE)smid) == 0)
      return GT_FAIL;

    return GT_OK;
}

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
)
{
    if (timeOut == OS_WAIT_FOREVER)
        timeOut = INFINITE;
    else if(timeOut == OS_NO_WAIT)
        timeOut = 0;

    switch (WaitForSingleObject((HANDLE)smid, timeOut))
    {
        case WAIT_FAILED:
            return GT_FAIL;

        case WAIT_TIMEOUT:
            return GT_TIMEOUT;

        case WAIT_OBJECT_0:
            return GT_OK;
    }

  return GT_FAIL;
}

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
)
{
    ReleaseSemaphore((HANDLE)smid,1,NULL);

    return GT_OK;
}

/**
* @internal osMutexCreate function
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
GT_STATUS osMutexCreate
(
    IN  const char      *name,
    OUT GT_MUTEX        *mtxid
)
{
#ifndef OS_WIN32_MTX_AS_CRITICAL_SECTION
    return osSemBinCreate(name, OS_SEMB_FULL, (GT_SEM*)mtxid);
#else
    int m;
    OS_OBJECT_HEADER_STC *h;
    GT_WIN32_MUTEX *mtx;
    static GT_BOOL facInit  =   GT_FALSE;

    if (facInit == GT_FALSE)
    {
        InitializeCriticalSection(&userspaceMutexes_mtx);
        facInit = GT_TRUE;
    }

    EnterCriticalSection(&userspaceMutexes_mtx);
    m = osObjLibGetNewObject(&userspaceMutexes, 1, name, &h);
    mtx = (GT_WIN32_MUTEX*)h;
    if (m <= 0)
    {
        LeaveCriticalSection(&userspaceMutexes_mtx);
        return GT_FAIL;
    }
    /* initialize recursive mutex */
    InitializeCriticalSection(&(mtx->mtx));

    LeaveCriticalSection(&userspaceMutexes_mtx);

    *mtxid = (GT_MUTEX)m;
    return GT_OK;
#endif
}

/**
* @internal osMutexDelete function
* @endinternal
*
* @brief   Delete mutex.
*
* @param[in] mtxid                    - mutex Id
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
GT_STATUS osMutexDelete
(
    IN GT_MUTEX mtxid
)
{
#ifndef OS_WIN32_MTX_AS_CRITICAL_SECTION
    return osSemDelete((GT_SEM)mtxid);
#else
    CHECK_MTX(mtxid);
    DeleteCriticalSection(&(mtx->mtx));
    mtx->header.type = 0;
#if defined  CPSS_USE_MUTEX_PROFILER      
    mtx->header.glType = GRANULAR_LOCKING_TYPE_NONE;
    mtx->header.devNum = 0;
#endif  
    return GT_OK;
#endif
}

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
)
{
#ifndef OS_WIN32_MTX_AS_CRITICAL_SECTION
    return osSemWait((GT_SEM)mtxid, OS_WAIT_FOREVER);
#else
    CHECK_MTX(mtxid);
    EnterCriticalSection(&(mtx->mtx));
    
#if defined  CPSS_USE_MUTEX_PROFILER   
    /*Perform validation only on first lock*/
    if(mtx->mtx.RecursionCount == 1)
    {
        osMutexLockGlValidation(mtxid);
    }
#endif

#ifdef  OS_USERSPACE_MUTEX_STAT
    mtx->lockcount++;
#endif
    return GT_OK;
#endif
}

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
)
{
#ifndef OS_WIN32_MTX_AS_CRITICAL_SECTION
    return osSemSignal((GT_SEM)mtxid);
#else
    CHECK_MTX(mtxid);
 
    LeaveCriticalSection(&(mtx->mtx));
#ifdef  OS_USERSPACE_MUTEX_STAT
    mtx->unlockcount++;
#endif
    return GT_OK;
#endif
}

#if defined(OS_WIN32_MTX_AS_CRITICAL_SECTION) && defined(OS_USERSPACE_MUTEX_STAT)
/**
* @internal osMutexStat function
* @endinternal
*
* @brief   Prints userspace mutex statistics
*
* @retval GT_OK                    - on success
*/
GT_STATUS osMutexStat(void)
{
    int m;

    for (m = 1; m < userspaceMutexes.allocated; m++)
    {
        GT_WIN32_MUTEX *mtx;
        mtx = (GT_WIN32_MUTEX*)(userspaceMutexes.list[m]);
        if (!mtx || mtx->header.type == 0)
            continue;
        osPrintf("id=%d name=\"%s\" lcount=%d ulcount=%d\n",
                m,
                mtx->header.name,
                mtx->lockcount,
                mtx->unlockcount);
    }
    return GT_OK;
}
#endif /* defined(OS_WIN32_MTX_AS_CRITICAL_SECTION) && defined(OS_USERSPACE_MUTEX_STAT) */

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
)
{
    GT_WIN_RW_LOCK  *rwLock;
    GT_U8           lockName[100];

    rwLock = (GT_WIN_RW_LOCK*)osMalloc(sizeof(GT_WIN_RW_LOCK));
    if(rwLock == NULL)
    {
        return GT_FAIL;
    }

    osSprintf(lockName,"%s-cntSem",name);
    rwLock->numOfReaders = 0;
    if(osSemBinCreate(lockName,OS_SEMB_FULL,&(rwLock->cntSem)) != GT_OK)
    {
        osFree(rwLock);
        return GT_FAIL;
    }

    osSprintf(lockName,"%s-wrSem",name);
    if(osSemBinCreate(lockName,OS_SEMB_FULL,&(rwLock->writerSem)) != GT_OK)
    {
        osSemDelete(rwLock->cntSem);
        osFree(rwLock);
        return GT_FAIL;
    }

    if(osSemBinCreate(lockName,OS_SEMB_FULL,&(rwLock->dbSem)) != GT_OK)
    {
        osSemDelete(rwLock->cntSem);
        osSemDelete(rwLock->writerSem);
        osFree(rwLock);
        return GT_FAIL;
    }

    *lockId = (GT_RW_LOCK)rwLock;

    return GT_OK;
}


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
)
{
    GT_WIN_RW_LOCK  *rwLock;
    GT_STATUS       retVal;

    rwLock = (GT_WIN_RW_LOCK*)lockId;

    if((retVal = osSemDelete(rwLock->cntSem)) != GT_OK)
    {
        return retVal;
    }

    if((retVal = osSemDelete(rwLock->dbSem)) != GT_OK)
    {
        return retVal;
    }

    return osSemDelete(rwLock->writerSem);
}


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
)
{
    GT_WIN_RW_LOCK  *rwLock;
    GT_STATUS       retVal;

    rwLock = (GT_WIN_RW_LOCK*)lockId;

    /* Prevent any new readers / writers from getting in    */
    if((retVal = osSemWait(rwLock->writerSem,timeOut)) != GT_OK)
    {
        return retVal;
    }

    if(lockType == OS_READ_LOCK)
    {
        /* Lock the access to the readers counter               */
        if((retVal = osSemWait(rwLock->cntSem,timeOut)) != GT_OK)
        {
            return retVal;
        }

        rwLock->numOfReaders++;
        if(rwLock->numOfReaders == 1)
        {
            if((retVal = osSemWait(rwLock->dbSem,timeOut)) != GT_OK)
            {
                return retVal;
            }
        }

        osSemSignal(rwLock->cntSem);
        osSemSignal(rwLock->writerSem);
    }
    else
    {
        /* lockType == OS_WRITE_LOCK    */
        if((retVal = osSemWait(rwLock->dbSem,timeOut)) != GT_OK)
        {
            return retVal;
        }
    }

    return GT_OK;
}


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
)
{
    GT_WIN_RW_LOCK  *rwLock;
    GT_STATUS       retVal;

    rwLock = (GT_WIN_RW_LOCK*)lockId;

    if(lockType == OS_READ_LOCK)
    {
        if((retVal = osSemWait(rwLock->cntSem,OS_WAIT_FOREVER)) != GT_OK)
        {
            return retVal;
        }

        rwLock->numOfReaders--;
        if(rwLock->numOfReaders == 0)
        {
            /* Free waiting writers */
            osSemSignal(rwLock->dbSem);
        }

        osSemSignal(rwLock->cntSem);
    }
    else
    {
        /* lockType == OS_WRITE_LOCK    */
        osSemSignal(rwLock->dbSem);
        osSemSignal(rwLock->writerSem);
    }

    return GT_OK;
}






