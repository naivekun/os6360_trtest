#include <stdio.h>

#if defined(WIN32)

#include <windows.h>

#define OS_USERSPACE_MUTEX_STAT
#define OS_WIN32_MTX_AS_CRITICAL_SECTION


#ifdef OS_WIN32_MTX_AS_CRITICAL_SECTION
#include <gtOs/osObjIdLib.h>
#endif


#elif  defined(LINUX) /* defined(WIN32) */
#include <pthread.h>
#include <gtOs/osObjIdLib.h> 
#include <string.h>
#endif /*elif (defined(LINUX))*/


#if defined(WIN32)


#ifdef OS_WIN32_MTX_AS_CRITICAL_SECTION
typedef struct _osMutex
{
    OS_OBJECT_HEADER_STC    header;
    CRITICAL_SECTION        mtx;
#ifdef  OS_USERSPACE_MUTEX_STAT
    int                     lockcount;
    int                     unlockcount;
#endif
}GT_WIN32_MUTEX;



#define CHECK_MTX(mtxId) \
    GT_WIN32_MUTEX *mtx; \
    if ((int)mtxId < 1 || (int)mtxId >= userspaceMutexes.allocated) \
        return GT_FAIL; \
    mtx = (GT_WIN32_MUTEX*)(userspaceMutexes.list[(int)mtxId]); \
    if (!mtx || !mtx->header.type) \
        return GT_FAIL;


#endif /* defined(OS_WIN32_MTX_AS_CRITICAL_SECTION) */

#elif  defined(LINUX)/* defined(WIN32) */

#define SEM_STAT

/************* Internal types *************************************************/
typedef enum {
    SEMTYPE_NONE = 0,
    SEMTYPE_MUTEX,
    SEMTYPE_COUNTING,
    SEMTYPE_BINARY
} semType;

typedef struct _V2L_semSTC
{
    OS_OBJECT_HEADER_STC    header;
    int                     count;
    pthread_mutex_t         mtx;
    pthread_cond_t          cond;
    pthread_t                       mtxOwner;
#ifdef  SEM_STAT
    int                     sig_count;
    int                     wait_count;
    int                     waiting;
    int                     timeout_count;
#endif
} _V2L_semSTC;



#define CHECK_MTX(semId) \
    _V2L_semSTC *mtx; \
    if ((int)semId < 1 || (int)semId >= semaphores.allocated) \
        return GT_FAIL; \
    mtx = (_V2L_semSTC*)(semaphores.list[(int)semId]); \
    if (!mtx || !mtx->header.type) \
        return GT_FAIL;

#endif /*elif (defined(LINUX)&&defined (V2L_PTHREADS))*/




