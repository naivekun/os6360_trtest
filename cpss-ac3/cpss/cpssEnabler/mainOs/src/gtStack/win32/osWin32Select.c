/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE IS A REFERENCE CODE FOR MARVELL SWITCH PRODUCTS.  IT IS PROVIDED   *
* "AS IS" WITH NO WARRANTIES, EXPRESSED, IMPLIED OR OTHERWISE, REGARDING ITS   *
* ACCURACY, COMPLETENESS OR PERFORMANCE.                                       *
* CUSTOMERS ARE FREE TO MODIFY IT AND USE IT ONLY IN THEIR PRODUCTION          *
* SOFTWARE RELEASES WITH MARVELL SWITCH CHIPSETS.                              *
*******************************************************************************/

#include <winsock2.h>

#include <gtOs/gtOsMem.h>
#include <gtStack/gtStackTypes.h>
#include <gtStack/gtOsSelect.h>
#include <gtUtil/gtBmPool.h>


#ifndef __CALL
#define __CALL __stdcall
#endif

#ifndef __FAR
#define __FAR
#endif


/***** Public Functions ************************************************/
#if 0
extern int __CALL __FAR __WSAFDIsSet(unsigned int , fd_set __FAR *);
#endif


/*******************************************************************************
* osSelectCreateSet()
*
* DESCRIPTION:
*       Create a set of file descriptors for the select function
*
* INPUTS:
*       none
*
* OUTPUTS:
*       none
*
* RETURNS:
*       Pointer to the set. If unable to create, returns null. Note that the
*       pointer is from void type.
*
* COMMENTS:
*
*******************************************************************************/
GT_VOID * osSelectCreateSet()
{
    fd_set * set ;

    set = (fd_set*) osMalloc(sizeof(fd_set));
    if (set != NULL)
    {
        FD_ZERO(set) ;
    }

    return (GT_VOID*) set ;
}


/**
* @internal osSelectEraseSet function
* @endinternal
*
* @brief   Erase (delete) a set of file descriptors
*/
GT_VOID osSelectEraseSet(GT_VOID *  set)
{
     osFree(set);
}

/**
* @internal osSelectZeroSet function
* @endinternal
*
* @brief   Zeros a set of file descriptors
*/
GT_VOID osSelectZeroSet(GT_VOID *  set)
{
    FD_ZERO((struct fd_set *)set) ;
}


/**
* @internal osSelectAddFdToSet function
* @endinternal
*
* @brief   Add a file descriptor to a specific set
*/
GT_VOID osSelectAddFdToSet(GT_VOID *  set ,GT_FD  fd)
{
    FD_SET((SOCKET)fd, (struct fd_set * )set) ;
}


/**
* @internal osSelectClearFdFromSet function
* @endinternal
*
* @brief   Remove (clear) a file descriptor from a specific set
*/
GT_VOID osSelectClearFdFromSet(GT_VOID *  set ,GT_FD  fd)
{
    FD_CLR((SOCKET)fd, (struct fd_set*)set) ;
}


/**
* @internal osSelectIsFdSet function
* @endinternal
*
* @brief   Test if a specific file descriptor is set in a set
*/
GT_BOOL osSelectIsFdSet(GT_VOID *  set ,GT_FD  fd)
{
    if ( (FD_ISSET((SOCKET)fd, (struct fd_set*)set)) != 0)
    {
        return GT_TRUE;
    }
    else
    {
        return GT_FALSE;
    }
}


/**
* @internal osSelectCopySet function
* @endinternal
*
* @brief   Duplicate sets (require 2 pointers for sets)
*/
GT_VOID osSelectCopySet(GT_VOID *  srcSet, GT_VOID *  dstSet)
{
    *((struct fd_set*)dstSet) = *((struct fd_set*)srcSet) ;
}


/**
* @internal osSelect function
* @endinternal
*
* @brief   OS Dependent select function
*
* @param[in] width                    - The highest-numbered descriptor in any of the next three
*                                      sets + 1 (if zero, The default length will be taken)
* @param[in] readSet                  - Pointer to a read operation  descriptor set
* @param[in] writeSet                 - Pointer to a write operation descriptor set
* @param[in] exceptionSet             Pointer to an exception descriptor set (not supported in
*                                      all OS, such as VxWorks)
* @param[in] timeOut                  - Maximum time to wait on in milliseconds. Sending a
*                                      GT_INFINITE value will block indefinitely. Zero value cause
*                                      no block.
*                                       On success, returns the number of descriptors contained in the
*                                       descriptor sets, which may be zero if the timeout expires before
*
* @retval anything interesting happens. On  error, -1 is returned, and errno
*                                       is set appropriately; the sets and timeout become
*                                       undefined, so do not rely on their contents after an error.
*/
GT_SSIZE_T osSelect
(
    GT_FD      width ,
    GT_VOID *  readSet ,
    GT_VOID *  writeSet ,
    GT_VOID *  exceptionSet ,
    GT_U32     timeOut
)
{
    struct timeval tv ;
    int lwidth ,rc,ec;

    /* Convert to seconds and calculate the reminder in microseconds */
    tv.tv_sec = timeOut/1000 ;
    tv.tv_usec = (timeOut - (tv.tv_sec*1000)) * 1000;

    if (width == 0)
        lwidth = FD_SETSIZE ;
    else
        lwidth = (int)width ;

    if (timeOut == GT_INFINITE)

        return (GT_SSIZE_T)(select(lwidth ,
                         (struct fd_set*)readSet ,
                         (struct fd_set*)writeSet ,
                         (struct fd_set*)exceptionSet ,
                         NULL)) ;
    else
        {
                WORD wVersionRequested;
                WSADATA wsaData;
                int err;

                wVersionRequested = MAKEWORD( 2, 2 );

                err = WSAStartup( wVersionRequested, &wsaData );
                if ( err != 0 )
                {
                        /* Tell the user that we could not find a usable */
                        /* WinSock DLL.                                  */
                        return 0;
                }


        rc =  (GT_SSIZE_T)(select(lwidth ,
                         (struct fd_set*)readSet ,
                         (struct fd_set*)writeSet ,
                         (struct fd_set*)exceptionSet ,
                         &tv)) ;
                if(rc == SOCKET_ERROR)
                {
                        ec = WSAGetLastError();
                        Sleep(timeOut);
                }

                return (GT_SSIZE_T)rc;
        }
    /*
    ** Note that now FD_ISSET on the file descriptor with its
    ** corresponded set will be true if the file descriptor is
    ** ready
    */
}

/**
* @internal osSocketGetSocketFdSetSize function
* @endinternal
*
* @brief   Returns the size of fd_set.
*
* @param[out] sockFdSize               - to be filled with fd_set size.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on fail
*
* @note USERS THAT IMPLEMENTS THEIR OWN OS LAYER CAN RETURN SIZE = 0.
*
*/
GT_STATUS osSocketGetSocketFdSetSize
(
    OUT GT_U32*    sockFdSize
)
{
    *sockFdSize = sizeof(struct fd_set);

    return GT_OK ;
}


