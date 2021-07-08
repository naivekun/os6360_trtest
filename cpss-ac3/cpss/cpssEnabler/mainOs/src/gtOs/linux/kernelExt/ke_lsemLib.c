/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE IS A REFERENCE CODE FOR MARVELL SWITCH PRODUCTS.  IT IS PROVIDED   *
* "AS IS" WITH NO WARRANTIES, EXPRESSED, IMPLIED OR OTHERWISE, REGARDING ITS   *
* ACCURACY, COMPLETENESS OR PERFORMANCE.                                       *
* CUSTOMERS ARE FREE TO MODIFY IT AND USE IT ONLY IN THEIR PRODUCTION          *
* SOFTWARE RELEASES WITH MARVELL SWITCH CHIPSETS.                              *
*******************************************************************************/

#define _GNU_SOURCE
#include "kernelExt.h"
#include <gtOs/gtOsSem.h>
#include <gtOs/gtOsIo.h>

#include <string.h>
#include <pthread.h>
#include <stdlib.h>
#include <errno.h>
#include <gtOs/osObjIdLib.h>
#include <gtOs/gtOsSharedMalloc.h>


#define OS_USERSPACE_MUTEX_STAT
#if !defined(SHARED_MEMORY) && !defined(OS_USERSPACE_MUTEXES)
#  define OS_USERSPACE_MUTEXES
#endif

#if __GLIBC_PREREQ(2,13)
# ifdef __USE_XOPEN2K
#  define HAVE_ROBUST_MUTEXES 1
# endif
#endif

#if defined(OS_USERSPACE_MUTEXES)
#ifdef SHARED_MEMORY
#include <gtOs/gtOsSharedData.h>
#endif

typedef struct _osMutex
{
    OS_OBJECT_HEADER_STC    header;
    pthread_mutex_t         mtx;
#ifdef  OS_USERSPACE_MUTEX_STAT
    int                     lockcount;
    int                     unlockcount;
#endif
}GT_LINUX_MUTEX;
#define LMUTEX_FLAG 0x8000000
#define IS_LOCAL_MUTEX(m) ((int)m & LMUTEX_FLAG)
#define SEM_TO_MTX(m) ((GT_MUTEX)((int)m & ~LMUTEX_FLAG))
#define MTX_TO_SEM(m) ((GT_SEM)((int)m | LMUTEX_FLAG))

static OS_OBJECT_LIST_STC userspaceMutexes = {
    NULL,                   /* list */
    0,                      /* allocated */
    128,                    /* allocChunk */
    sizeof(GT_LINUX_MUTEX), /* objSize */
    SHARED_MALLOC_MAC,      /* allocFuncPtr */
    SHARED_FREE_MAC         /* freeFuncPtr */
};

#define CHECK_MTX(mtxId) \
    GT_LINUX_MUTEX *mtx; \
    if ((int)mtxId < 1 || (int)mtxId >= userspaceMutexes.allocated) \
        return GT_FAIL; \
    mtx = (GT_LINUX_MUTEX*)(userspaceMutexes.list[(int)mtxId]); \
    if (!mtx || !mtx->header.type) \
        return GT_FAIL;

#ifndef SHARED_MEMORY
static pthread_mutex_t userspaceMutexes_mtx = PTHREAD_MUTEX_INITIALIZER;
#else
static int initialized __SHARED_DATA_MAINOS = 0;
static pthread_mutex_t userspaceMutexes_mtx __SHARED_DATA_MAINOS;
#endif

#endif /* defined(OS_USERSPACE_MUTEXES) */


/************ Public Functions ************************************************/

/**
* @internal V2L_lsemInit function
* @endinternal
*
* @brief   Initialize semaphores
*
* @retval OK                       - on success
* @retval ERROR                    - on error
*
* @note This function called from osStartEngine()
*
*/
int V2L_lsemInit(void)
{
#if defined(SHARED_MEMORY) && defined(OS_USERSPACE_MUTEXES)
    if (!initialized)
    {
        pthread_mutexattr_t mattr;
        pthread_mutexattr_init(&mattr);
        pthread_mutexattr_setpshared(&mattr, PTHREAD_PROCESS_SHARED);
#ifdef HAVE_ROBUST_MUTEXES
        pthread_mutexattr_setrobust(&mattr, PTHREAD_MUTEX_ROBUST);
#endif
        pthread_mutex_init(&userspaceMutexes_mtx, &mattr);
        initialized = 1;
    }
#endif
    return 0;
}


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
)
{
    int ret;
    mv_sem_create_stc rparam;

    rparam.flags = (init == OS_SEMB_FULL) ? 1 : 0;
    rparam.flags |= MV_SEMAPTHORE_F_BINARY;
    if (name)
        strncpy(rparam.name, name, sizeof(rparam.name));
    else
        rparam.name[0] = 0;

    ret = mv_ctrl(MVKERNELEXT_IOC_SEMCREATE, &rparam);
    if (ret < 0)
    {
        *smid = 0;
        return GT_FAIL;
    }

    *smid = (GT_SEM)ret;

    return GT_OK;
}

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
)
{
#if defined(OS_USERSPACE_MUTEXES)
    GT_STATUS   rc;
    GT_MUTEX    mtx;

    /* fix 'const' warning */
    char        tname[32];
    strncpy(tname, name?name:"", sizeof(tname));
    tname[sizeof(tname)-1] = 0;

    rc = osMutexCreate(tname, &mtx);
    if (rc == GT_OK)
        *smid = MTX_TO_SEM(mtx);
    else
        *smid = 0;
    return rc;
#else
    int ret;
    mv_sem_create_stc rparam;

    rparam.flags = MV_SEMAPTHORE_F_MTX;
    if (name)
        strncpy(rparam.name, name, sizeof(rparam.name));
    else
        rparam.name[0] = 0;

    ret = mv_ctrl(MVKERNELEXT_IOC_SEMCREATE, &rparam);
    if (ret < 0)
    {
        *smid = 0;
        return GT_FAIL;
    }

    *smid = (GT_SEM)ret;

    return GT_OK;
#endif
}

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
)
{
    int ret;
    mv_sem_create_stc rparam;

    rparam.flags = MV_SEMAPTHORE_F_COUNT | (init & MV_SEMAPTHORE_F_COUNT_MASK);
    if (name)
        strncpy(rparam.name, name, sizeof(rparam.name));
    else
        rparam.name[0] = 0;

    ret = mv_ctrl(MVKERNELEXT_IOC_SEMCREATE, &rparam);
    if (ret < 0)
    {
        *smid = 0;
        return GT_FAIL;
    }

    *smid = (GT_SEM)ret;

    return GT_OK;
}

/**
* @internal osSemMOpenNamed function
* @endinternal
*
* @brief   Create or open an existing named mutex semaphore.
*
* @param[in] name                     - semaphore Name
*
* @param[out] smid                     - semaphore Id
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*
* @note Works only with kernel module semaphores
*
*/
GT_STATUS osSemMOpenNamed
(
    IN  const char    *name,
    OUT GT_SEM        *smid
)
{
    int ret;
    mv_sem_create_stc rparam;

    rparam.flags = MV_SEMAPTHORE_F_MTX | MV_SEMAPTHORE_F_OPENEXIST;
    if (name)
        strncpy(rparam.name, name, sizeof(rparam.name));
    else
        rparam.name[0] = 0;

    ret = mv_ctrl(MVKERNELEXT_IOC_SEMCREATE, &rparam);
    if (ret < 0)
    {
        *smid = 0;
        return GT_FAIL;
    }

    *smid = (GT_SEM)ret;

    return GT_OK;
}

/**
* @internal osSemBinOpenNamed function
* @endinternal
*
* @brief   Create or open existing named binary semaphore.
*
* @param[in] name                     - semaphore Name
* @param[in] init                     -  value of semaphore (full or empty)
*
* @param[out] smid                     - semaphore Id
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*
* @note Works only with kernel module semaphores
*
*/
GT_STATUS osSemBinOpenNamed
(
    IN  const char    *name,
    IN  GT_SEMB_STATE init,
    OUT GT_SEM        *smid
)
{
    int ret;
    mv_sem_create_stc rparam;

    rparam.flags = (init == OS_SEMB_FULL) ? 1 : 0;
    rparam.flags |= MV_SEMAPTHORE_F_MTX | MV_SEMAPTHORE_F_OPENEXIST;
    if (name)
        strncpy(rparam.name, name, sizeof(rparam.name));
    else
        rparam.name[0] = 0;

    ret = mv_ctrl(MVKERNELEXT_IOC_SEMCREATE, &rparam);
    if (ret < 0)
    {
        *smid = 0;
        return GT_FAIL;
    }

    *smid = (GT_SEM)ret;

    return GT_OK;
}

/**
* @internal osSemCOpenNamed function
* @endinternal
*
* @brief   Create or open existing named counting semaphore.
*
* @param[in] name                     - semaphore Name
* @param[in] init                     -  value of semaphore
*
* @param[out] smid                     - semaphore Id
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*
* @note Works only with kernel module semaphores
*
*/
GT_STATUS osSemCOpenNamed
(
    IN  const char    *name,
    IN  GT_U32        init,
    OUT GT_SEM        *smid
)
{
    int ret;
    mv_sem_create_stc rparam;

    rparam.flags = (init & MV_SEMAPTHORE_F_COUNT_MASK);
    rparam.flags |= MV_SEMAPTHORE_F_COUNT | MV_SEMAPTHORE_F_OPENEXIST;
    if (name)
        strncpy(rparam.name, name, sizeof(rparam.name));
    else
        rparam.name[0] = 0;

    ret = mv_ctrl(MVKERNELEXT_IOC_SEMCREATE, &rparam);
    if (ret < 0)
    {
        *smid = 0;
        return GT_FAIL;
    }

    *smid = (GT_SEM)ret;

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
    int rc;

/* Supported for osSemMCreate . Should be removed */
#if defined(OS_USERSPACE_MUTEXES)
    if (IS_LOCAL_MUTEX((int)smid))
    {
        return osMutexDelete(SEM_TO_MTX(smid));
    }
#endif
    rc = mv_ctrl(MVKERNELEXT_IOC_SEMDELETE, smid);

    if (rc != 0)
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
    int ret;

#if defined(OS_USERSPACE_MUTEXES)
    if (IS_LOCAL_MUTEX(smid))
    {
        return osMutexLock(SEM_TO_MTX(smid));
    }
#endif
    if (timeOut == OS_WAIT_FOREVER)
    {
        do {
            ret = mv_sem_wait((int)smid);
        } while (ret < 0 && errno == MVKERNELEXT_EINTR);
        return (ret == 0) ? GT_OK : GT_FAIL;
    }
    if (timeOut == OS_NO_WAIT)
    {
        return mv_sem_trywait((int)smid);
    }

    /* wait with timeout */
    do {
        mv_sem_timedwait_stc params;
        params.semid = (int)smid;
        params.timeout = timeOut;
        do {
            ret = mv_ctrl(MVKERNELEXT_IOC_SEMWAITTMO, &params);
        } while (ret < 0 && errno == MVKERNELEXT_EINTR);
    } while (ret < 0 && errno == MVKERNELEXT_EINTR);

    if (ret < 0 && errno == MVKERNELEXT_ETIMEOUT)
        return GT_TIMEOUT;

    return (ret == 0) ? GT_OK : GT_FAIL;
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
    int ret;

#if defined(OS_USERSPACE_MUTEXES)
    if (IS_LOCAL_MUTEX(smid))
    {
        return osMutexUnlock(SEM_TO_MTX(smid));
    }
#endif
    ret = mv_sem_signal((int)smid);

    if (ret != 0)
        return GT_FAIL;

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
/* OS_USERSPACE_MUTEXES not defined in shared-lib mode */
#ifndef OS_USERSPACE_MUTEXES
    return osSemMCreate(name, (GT_SEM*)mtxid);
#else
    int m;
    pthread_mutexattr_t mattr;
    OS_OBJECT_HEADER_STC *h;
    GT_LINUX_MUTEX *mtx;

    pthread_mutex_lock(&userspaceMutexes_mtx);
    m = osObjLibGetNewObject(&userspaceMutexes, 1, name, &h);
    mtx = (GT_LINUX_MUTEX*)h;
    if (m <= 0)
    {
        pthread_mutex_unlock(&userspaceMutexes_mtx);
        return GT_FAIL;
    }
    /* initialize recursive mutex */
    pthread_mutexattr_init(&mattr);
#ifdef SHARED_MEMORY
    pthread_mutexattr_setpshared(&mattr, PTHREAD_PROCESS_SHARED);
#endif
    pthread_mutexattr_settype(&mattr, PTHREAD_MUTEX_RECURSIVE);
    pthread_mutex_init(&(mtx->mtx), &mattr);
    pthread_mutexattr_destroy(&mattr);

    pthread_mutex_unlock(&userspaceMutexes_mtx);

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
#ifndef OS_USERSPACE_MUTEXES
    return osSemDelete((GT_SEM)mtxid);
#else
    CHECK_MTX(mtxid);
    pthread_mutex_destroy(&(mtx->mtx));
    mtx->header.type = 0;
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
#ifndef OS_USERSPACE_MUTEXES
    return osSemWait((GT_SEM)mtxid, OS_WAIT_FOREVER);
#else
    CHECK_MTX(mtxid);
#ifdef HAVE_ROBUST_MUTEXES
    if (pthread_mutex_lock(&(mtx->mtx)) == EOWNERDEAD)
        pthread_mutex_consistent(&(mtx->mtx));
#else
    pthread_mutex_lock(&(mtx->mtx));
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
#ifndef OS_USERSPACE_MUTEXES
    return osSemSignal((GT_SEM)mtxid);
#else
    CHECK_MTX(mtxid);
    pthread_mutex_unlock(&(mtx->mtx));
#ifdef  OS_USERSPACE_MUTEX_STAT
    mtx->unlockcount++;
#endif
    return GT_OK;
#endif
}

#if defined(OS_USERSPACE_MUTEXES) && defined(OS_USERSPACE_MUTEX_STAT)
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
        GT_LINUX_MUTEX *mtx;
        mtx = (GT_LINUX_MUTEX*)(userspaceMutexes.list[m]);
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
#endif /* defined(OS_USERSPACE_MUTEXES) && defined(OS_USERSPACE_MUTEX_STAT) */


