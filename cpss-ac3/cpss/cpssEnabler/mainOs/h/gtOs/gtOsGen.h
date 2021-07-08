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
* @file gtOsGen.h
*
* @brief Operating System wrapper general APIs
*
* @version   10
********************************************************************************
*/

#ifndef __gtOsGenh
#define __gtOsGenh

#ifdef __cplusplus
extern "C" {
#endif

/************* Includes *******************************************************/

#include <gtOs/gtGenTypes.h>

#define IS_WRAPPER_OPEN_STATUS  \
    if (osWrapperIsOpen() == GT_FALSE)  \
    {                                   \
        return GT_FAIL;                 \
    }

#define IS_WRAPPER_OPEN_PTR     \
    if (osWrapperIsOpen() == GT_FALSE)  \
    {                                   \
        return 0;                    \
    }

/************* Functions ******************************************************/
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
GT_STATUS osWrapperOpen(void * osSpec);

/**
* @internal osWrapperClose function
* @endinternal
*
* @brief   Close OS wrapper facility and free all used resources.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
GT_STATUS osWrapperClose(void);

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
GT_INLINE GT_BOOL osWrapperIsOpen(void);

#ifdef __cplusplus
}
#endif

#endif  /* __gtOsGenh */
/* Do Not Add Anything Below This Line */



