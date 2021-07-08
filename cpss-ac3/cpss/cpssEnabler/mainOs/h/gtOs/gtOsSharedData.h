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
* @file gtOsSharedData.h
*
* @brief
*
* @version   3
********************************************************************************
*/

#ifndef __gtOsSharedDatah
#define __gtOsSharedDatah

#ifdef __cplusplus
extern "C" {
#endif

#ifndef SHARED_MEMORY
#  define __SHARED_DATA_MAINOS

#else /* defined(SHARED_MEMORY) */
/************* Includes *******************************************************/

#include <gtOs/gtGenTypes.h>

/* for uintptr_t */
#include <stdint.h>

/* __SHARED_DATA_MAINOS
 *    This macro defines a variable attribute for process shared data
 *    The data placed to special section to be mapped for all processes
 *    Usage example:
 *       static int variable_name __SHARED_DATA_MAINOS = 10;
 *
 * Please note: this is applicable for libhelper/mainOs only
 */
#ifndef __GNUC__
#  error "GNUC supported only for shared library"
#endif
#define __SHARED_DATA_MAINOS __attribute__ ((section ("MAINOS")))


typedef enum {
    CPSS_SHARED_DATA_ETHPORT_POOLARRAY_PTR_E = 0,
    CPSS_SHARED_DATA_MAX_E
} CPSS_SHARED_DATA_TYPE;

/**
* @internal cpssMultiProcGetSharedData function
* @endinternal
*
* @brief   return enumerated shared data
*
* @param[in] id                       - data identifier
*                                       data (integer or pointer)
*/
uintptr_t cpssMultiProcGetSharedData
(
    IN CPSS_SHARED_DATA_TYPE id
);

/**
* @internal cpssMultiProcSetSharedData function
* @endinternal
*
* @brief   Set enumerated shared data
*
* @param[in] id                       - data identifier
*                                       GT_OK
*                                       GT_FAIL
*/
GT_STATUS cpssMultiProcSetSharedData
(
    IN CPSS_SHARED_DATA_TYPE id,
    IN uintptr_t data
);

/**
* @internal cpssMultiProcSharedDataLock function
* @endinternal
*
* @brief   Lock shared data for critical operations (data alloc)
*/
GT_STATUS cpssMultiProcSharedDataLock(void);

/**
* @internal cpssMultiProcSharedDataUnlock function
* @endinternal
*
* @brief   Unlock shared data
*/
GT_STATUS cpssMultiProcSharedDataUnlock(void);

#endif /* defined(SHARED_MEMORY) */

#ifdef __cplusplus
}
#endif

#endif  /* __gtOsSharedDatah */
/* Do Not Add Anything Below This Line */


