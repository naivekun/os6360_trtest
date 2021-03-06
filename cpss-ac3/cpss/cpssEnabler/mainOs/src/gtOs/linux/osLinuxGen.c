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
* @file osLinuxGen.c
*
* @brief Operating System wrapper general APIs implementation
*
* @version   5
********************************************************************************
*/


/************* Includes *******************************************************/
#define _GNU_SOURCE
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <gtOs/gtOsGen.h>
#include <gtOs/gtOsSharedPp.h>
#include <gtOs/gtOsSharedData.h>

#include <gtUtil/gtBmPool.h>

#ifndef SHARED_MEMORY
/* Required for shrMemSharedPpInit */
#   include <gtOs/gtOsSharedPp.h>
#endif


/*************** Globals ******************************************************/
static GT_BOOL gtOsWrapperOpen __SHARED_DATA_MAINOS = GT_FALSE;


/**
* @internal osWrapperOpen function
* @endinternal
*
* @brief   Initialize OS wrapper facility.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_ALREADY_EXIST         - wrapper was already open
*/
GT_STATUS osWrapperOpen(void * osSpec)
{
    (void)osSpec;
    if (gtOsWrapperOpen == GT_TRUE)
    {
        return GT_OK;
    }
    else
    {
#if (!defined SHARED_MEMORY && !defined LINUX_SIM)
        /*  Open the packet processor device driver. 
            NOTE - share DMA buffer here only wihtout Shared Memory approach!
            Otherwise it should be initialized
            into shrMemInitSharedLibrary[_FirstClient] */
        if (shrMemSharedPpInit(GT_TRUE) != GT_OK)
        {
            return GT_FAIL;
        }

#endif
        /* Must be set to GT_TRUE before use any of OS functions. */
        gtOsWrapperOpen = GT_TRUE;

        gtPoolInit( 500 );
    }

    return GT_OK;
}

/**
* @internal osWrapperClose function
* @endinternal
*
* @brief   Close OS wrapper facility and free all used resources.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
GT_STATUS osWrapperClose(void)
{
    IS_WRAPPER_OPEN_STATUS;

    return GT_OK;
}

/*******************************************************************************
* osWrapperIsOpen
*
* DESCRIPTION:
*       Returns GT_TRUE if OS wrapper facility was initialized .
*
* INPUTS:
*       None
*
* OUTPUTS:
*       None
*
* RETURNS:
*       GT_TRUE          - on success
*       GT_FALSE         - on error
*
* COMMENTS:
*       None
*
*******************************************************************************/
GT_INLINE GT_BOOL osWrapperIsOpen(void)
{
    return gtOsWrapperOpen;
}




