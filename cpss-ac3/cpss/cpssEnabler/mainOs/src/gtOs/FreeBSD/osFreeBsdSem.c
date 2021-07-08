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
* @file osFreeBsdSem.c
*
* @brief FreeBsd User Mode Operating System wrapper. Semaphore facility.
*
* @version   13
********************************************************************************
*/

#include <gtOs/gtOsMem.h>
#include <gtOs/gtOsSem.h>
#include <gtOs/gtOsIo.h>


/* enable statistic collection for userspace mutexes */
#define OS_USERSPACE_MUTEX_STAT

/************ Internal Typedefs ***********************************************/
/*
 * Typedef: struct GT_LINUX_RW_LOCK
 *
 * Description: Defines the Read / Write lock type for Linux implementation.
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
typedef struct _linuxRwLock
{
    GT_U32  numOfReaders;
    GT_SEM  cntSem;
    GT_SEM  writerSem;
    GT_SEM  dbSem;
}GT_LINUX_RW_LOCK;

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
    GT_LINUX_RW_LOCK    *rwLock;
    GT_U8               lockName[100];

    rwLock = (GT_LINUX_RW_LOCK*)osMalloc(sizeof(GT_LINUX_RW_LOCK));
    if(rwLock == NULL)
    {
        return GT_FAIL;
    }

    osSprintf((char*)lockName,"%s-cntSem",name);
    rwLock->numOfReaders = 0;
    if(osSemBinCreate((char*)lockName,OS_SEMB_FULL,&(rwLock->cntSem)) != GT_OK)
    {
        osFree(rwLock);
        return GT_FAIL;
    }

    osSprintf((char*)lockName,"%s-wrSem",name);
    if(osSemBinCreate((char*)lockName,OS_SEMB_FULL,&(rwLock->writerSem)) != GT_OK)
    {
        osSemDelete(rwLock->cntSem);
        osFree(rwLock);
        return GT_FAIL;
    }

    if(osSemBinCreate((char*)lockName,OS_SEMB_FULL,&(rwLock->dbSem)) != GT_OK)
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
    GT_LINUX_RW_LOCK    *rwLock;
    GT_STATUS           retVal;

    rwLock = (GT_LINUX_RW_LOCK*)lockId;

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
    GT_LINUX_RW_LOCK    *rwLock;
    GT_STATUS           retVal;

    rwLock = (GT_LINUX_RW_LOCK*)lockId;

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
    GT_LINUX_RW_LOCK    *rwLock;
    GT_STATUS           retVal;

    rwLock = (GT_LINUX_RW_LOCK*)lockId;

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




