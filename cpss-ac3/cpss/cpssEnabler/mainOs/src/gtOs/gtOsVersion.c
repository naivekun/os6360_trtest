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
* @file gtOsVersion.c
*
* @brief Operating System wrapper. Version facility.
*
* @version   1.1.2.1
********************************************************************************
*/


/************* Includes *******************************************************/

#include <gtOs/gtOsVersion.h>
#include <gtOs/gtOsMem.h>
#include <gtOs/gtOsStr.h>

#define GT_OS_VERSTRING   "v1.2a3"

/************* Functions ******************************************************/
/**
* @internal osVersion function
* @endinternal
*
* @brief   Returns OS Layer wrapper version number.
*
* @param[in] verString                - pointer to buffer for return string
*
* @param[out] verString                - string with version number
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer
*
* @note User of this function should allocate at less 10 bytes for version
*       string.
*
*/
GT_STATUS osVersion
(
    IN GT_OS_VERSION * verString
)
{
    if (verString == NULL)
        return GT_BAD_PTR;

    osMemCpy(&(verString->version[0]),GT_OS_VERSTRING,
             osStrlen(GT_OS_VERSTRING) + 1);

    return GT_OK;
}

/**
* @internal osIsThreadLocalSupported function
* @endinternal
*
* @brief   Returns true if thread local storage is supproted
*
* @retval GT_TRUE                  - if supported
* @retval GT_FALSE                 - if not supported
*/
GT_BOOL osIsThreadLocalSupported
(
    GT_VOID
)
{
#if defined(PPC_CPU) || defined(POWERPC_CPU) || defined(LINUX_NOKM)
    return GT_FALSE;
#else
    return GT_TRUE;
#endif
}


