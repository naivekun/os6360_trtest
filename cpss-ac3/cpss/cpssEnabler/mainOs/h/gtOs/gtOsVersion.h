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
* @file gtOsVersion.h
*
* @brief Operating System wrapper. Random facility.
*
* @version   4
********************************************************************************
*/

#ifndef __gtOsVersionh
#define __gtOsVersionh

#ifdef __cplusplus
extern "C" {
#endif

/************* Includes *******************************************************/

#include <gtOs/gtGenTypes.h>

#define OS_VERSION_MAX_LEN 30
/**
* @struct GT_OS_VERSION
 *
 * @brief This struct holds the package version.
*/
typedef struct{

    GT_U8 version[OS_VERSION_MAX_LEN];

} GT_OS_VERSION;

/************* Functions ******************************************************/
/**
* @internal osVersion function
* @endinternal
*
* @brief   Returns OS Layer wrapper version number.
*
* @param[in] verString                - pointer to buffer struct with buffer for string
*
* @param[out] verString                - string with version number
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer
*/
GT_STATUS osVersion
(
    IN GT_OS_VERSION * verString
);


#ifdef __cplusplus
}
#endif

#endif  /* __gtOsVersionh */
/* Do Not Add Anything Below This Line */



