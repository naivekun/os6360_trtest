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
* @file gtOsSelect.h
*
* @brief extended operating system wrapper library implements select()
* functions
*
* @version   3
********************************************************************************
*/

#ifndef __gtOsSelect_h
#define __gtOsSelect_h

#ifdef __cplusplus
extern "C" {
#endif

#include <gtOs/gtGenTypes.h>
#include <gtStack/gtStackTypes.h>

#define GT_INFINITE 0xffffffff

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
GT_VOID * osSelectCreateSet
(
    void
);


/**
* @internal osSelectEraseSet function
* @endinternal
*
* @brief   Erase (delete) a set of file descriptors
*
* @param[in] set                      - Pointer to the set.
*                                       None
*/
GT_VOID osSelectEraseSet
(
    GT_VOID *  set
);


/**
* @internal osSelectZeroSet function
* @endinternal
*
* @brief   Zeros a set of file descriptors
*
* @param[in] set                      - Pointer to the set.
*                                       None
*/
GT_VOID osSelectZeroSet
(
    GT_VOID *  set
);


/**
* @internal osSelectAddFdToSet function
* @endinternal
*
* @brief   Add a file descriptor to a specific set
*
* @param[in] set                      - Pointer to the set
* @param[in] fd                       - A file descriptor
*                                       None
*/
GT_VOID osSelectAddFdToSet
(
    GT_VOID *  set ,
    GT_FD      fd
);


/**
* @internal osSelectClearFdFromSet function
* @endinternal
*
* @brief   Remove (clear) a file descriptor from a specific set
*
* @param[in] set                      - Pointer to the set
* @param[in] fd                       - A file descriptor
*                                       None
*/
GT_VOID osSelectClearFdFromSet
(
    GT_VOID *  set ,
    GT_FD      fd
);


/**
* @internal osSelectIsFdSet function
* @endinternal
*
* @brief   Test if a specific file descriptor is set in a set
*
* @param[in] set                      - Pointer to the set
* @param[in] fd                       - A file descriptor
*                                       GT_TRUE (non zero) if set , returned as unsigned int
*                                       GT_FALSE (zero) if not set , returned as unsigned int
*/
GT_BOOL osSelectIsFdSet
(
    GT_VOID *  set ,
    GT_FD      fd
);


/**
* @internal osSelectCopySet function
* @endinternal
*
* @brief   Duplicate sets (require 2 pointers for sets)
*
* @param[in] srcSet                   - Pointer to source set
* @param[in] dstSet                   - Pointer to destination set
*                                       Mone
*/
GT_VOID osSelectCopySet
(
    GT_VOID *  srcSet ,
    GT_VOID *  dstSet 
);


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
);

/**
* @internal osSocketGetSocketFdSetSize function
* @endinternal
*
* @brief   Returns the size of sockaddr_in.
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
);

#ifdef __cplusplus
}
#endif

#endif  /* ifndef __gtOsSelect_h */
/* Do Not Add Anything Below This Line */



