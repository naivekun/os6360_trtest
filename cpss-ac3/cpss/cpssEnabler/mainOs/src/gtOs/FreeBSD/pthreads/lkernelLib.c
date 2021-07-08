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
* @file lkernelLib.c
*
* @brief Pthread implementation of mainOs tasks and semaphores
* Initialization and common functions
*
* @version   4
********************************************************************************
*/
#define _GNU_SOURCE

#include <gtOs/gtOsInit.h>

#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#if 0
#include <sys/mman.h>
#endif

#ifdef SHARED_MEMORY
#include <gtOs/gtOsSharedMalloc.h>
#endif
#include "mainOsPthread.h"

#ifndef PTHREAD_ADAPTIVE_MUTEX_INITIALIZER_NP
#define PTHREAD_ADAPTIVE_MUTEX_INITIALIZER_NP PTHREAD_MUTEX_INITIALIZER
#endif

/************* Static variables ***********************************************/
static enum {
    E_STATE_UNINITIALIZED,
    E_STATE_INITIALIZED,
    E_STATE_WAITING,
    E_STATE_STOP
} programState = E_STATE_UNINITIALIZED;
static pthread_t rootTask;

/************* Static Function declarations ***********************************/
static void sigStopHandler(int);

/************ Public Functions ************************************************/

/**
* @internal osStartEngine function
* @endinternal
*
* @brief   Initialize mainOs engine, then start entryPoint routine
*         Need to be inplemented for Linux/FreeBSD
* @param[in] argC                     - main() parameter
* @param[in] argV[]                   - main() parameter
* @param[in] name                     - task name
* @param[in] entryPoint               - entry point function
*                                       GT_OK on success
*/
GT_STATUS osStartEngine
(
    IN int argC,
    IN const char* argV[],
    IN const char* name,
    IN GT_VOIDFUNCPTR entryPoint
)
{
    setbuf(stdout, NULL);


#if 0
    /*
    **  Lock all memory pages associated with this process to prevent delays
    **  due to process (or thread) memory being swapped out to disk and back.
    */
    mlockall( (MCL_CURRENT) );
#endif

    V2L_ltaskInit(name);
    V2L_lsemInit();
    V2L_lmsgQInit();

    rootTask = pthread_self();
    programState = E_STATE_INITIALIZED;
    signal(SIGINT, sigStopHandler);

    if (entryPoint)
    {
        printf( "\r\nStarting System Root Task" );
        entryPoint();
    }

    programState = E_STATE_WAITING;
    while (programState != E_STATE_STOP)
    {
        pause();
    }
    printf( "\r\nExiting...\n" );

    return GT_OK;
}

/**
* @internal osStopEngine function
* @endinternal
*
* @brief   Stop engine
*         Need to be inplemented for Linux/FreeBSD
*/
GT_STATUS osStopEngine(void)
{
    if (programState == E_STATE_UNINITIALIZED)
    {
        /* already signalled or not started yet */
        return GT_FAIL;
    }
    pthread_kill(rootTask, SIGINT);
    return GT_OK;
}



/************* Static functions ***********************************************/
static void sigStopHandler(int sig)
{
    if (programState == E_STATE_WAITING)
    {
        programState = E_STATE_STOP;
    }
    else
    {
        exit(0);
    }
}


/************ Private Functions ***********************************************/

/*******************************************************************************
* V2L_ts_malloc
*
* DESCRIPTION:
*       thread-safe malloc
*
* INPUTS:
*       blksize    - number of bytes to allocate
*
* OUTPUTS:
*       None
*
* RETURNS:
*       Pointer to allocated buffer or NULL
*
* COMMENTS:
*       None
*
*******************************************************************************/
void* V2L_ts_malloc
(
    IN  size_t blksize
)
{
    void *blkaddr;
    static pthread_mutex_t
        malloc_lock = PTHREAD_ADAPTIVE_MUTEX_INITIALIZER_NP;

    pthread_cleanup_push( (void(*)(void *))pthread_mutex_unlock,
                          (void *)&malloc_lock );
    pthread_mutex_lock( &malloc_lock );

#ifdef SHARED_MEMORY
    blkaddr = SHARED_MALLOC_MAC( blksize );
#else
    blkaddr = malloc( blksize );
#endif

    pthread_cleanup_pop( 1 );

    return( blkaddr );
}
    
/**
* @internal V2L_ts_free function
* @endinternal
*
* @brief   thread-safe free
*
* @param[in] blkaddr                  - address previosly allocated buffer
*                                       None
*/
void V2L_ts_free
(
    IN  void *blkaddr
)
{
    static pthread_mutex_t
        free_lock = PTHREAD_ADAPTIVE_MUTEX_INITIALIZER_NP;

    pthread_cleanup_push( (void(*)(void *))pthread_mutex_unlock,
                          (void *)&free_lock );
    pthread_mutex_lock( &free_lock );

#ifdef SHARED_MEMORY
    SHARED_FREE_MAC( blkaddr );
#else
    free( blkaddr );
#endif

    pthread_cleanup_pop( 1 );
}



