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
* @file appDemoStaticDb.h
*
* @brief App demo STATIC database.
* This DB is not reset by the 'system reset' mechanism.
*
* @version   1
********************************************************************************
*/
#ifndef __appDemoStaticDbh
#define __appDemoStaticDbh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpss/common/cpssTypes.h>
#ifdef CHX_FAMILY
#include <cpss/generic/cpssCommonDefs.h>
#endif /*CHX_FAMILY*/


/**
* @internal appDemoStaticDbEntryAdd function
* @endinternal
*
* @brief   Set AppDemo 'static' database value.This value will be considered during system
*         initialization process.
*         This DB is not reset by the 'system reset' mechanism.
* @param[in] namePtr                  - points to parameter name
* @param[in] value                    - parameter value
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - if name is too long
* @retval GT_BAD_PTR               - if NULL pointer
* @retval GT_NO_RESOURCE           - if Database is full
*/
GT_STATUS appDemoStaticDbEntryAdd
(
    IN  GT_CHAR *namePtr,
    IN  GT_U32  value
);


/**
* @internal appDemoStaticDbEntryGet function
* @endinternal
*
* @brief   Get parameter value from AppDemo 'static' database .
*         This DB is not reset by the 'system reset' mechanism.
* @param[in] namePtr                  - points to parameter name
*
* @param[out] valuePtr                 - points to parameter value
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - if name is too long
* @retval GT_BAD_PTR               - if NULL pointer
* @retval GT_NO_SUCH               - there is no such parameter in Database
*/
GT_STATUS appDemoStaticDbEntryGet
(
    IN  GT_CHAR *namePtr,
    OUT GT_U32  *valuePtr
);


/**
* @internal appDemoStaticDbDump function
* @endinternal
*
* @brief   Dumps entries set in AppDemo 'static' database to console.
*         This DB is not reset by the 'system reset' mechanism.
*
* @retval GT_OK                    - on success
*/
GT_STATUS appDemoStaticDbDump
(
    GT_VOID
);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __appDemoStaticDbh */



