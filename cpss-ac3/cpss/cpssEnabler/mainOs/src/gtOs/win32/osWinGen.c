/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE IS A REFERENCE CODE FOR MARVELL SWITCH PRODUCTS.  IT IS PROVIDED   *
* "AS IS" WITH NO WARRANTIES, EXPRESSED, IMPLIED OR OTHERWISE, REGARDING ITS   *
* ACCURACY, COMPLETENESS OR PERFORMANCE.                                       *
* CUSTOMERS ARE FREE TO MODIFY IT AND USE IT ONLY IN THEIR PRODUCTION          *
* SOFTWARE RELEASES WITH MARVELL SWITCH CHIPSETS.                              *
*******************************************************************************/


/************* Includes *******************************************************/

#include <winsock2.h>
#include <gtOs/gtGenTypes.h>
#include <gtUtil/gtBmPool.h>

/*************** Globals ******************************************************/
static GT_BOOL gtOsWrapperOpen = GT_FALSE;

/************* Functions ******************************************************/



/**
* @internal osWrapperOpen function
* @endinternal
*
* @brief   Initialize OS wrapper facility.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
GT_STATUS osWrapperOpen(void * osSpec)
{
    if (gtOsWrapperOpen == GT_TRUE)
    {
        return GT_OK;
    }
    else
    {
        /* Must be set to GT_TRUE before use any of OS functions. */
        gtOsWrapperOpen = GT_TRUE;

        gtPoolInit(500 ); /* Number of pools to support */
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



