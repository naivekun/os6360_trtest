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
* @file gtExtDrvVersion.h
*
* @brief Includes software version information for the External drivers part of
* the Prestera software suite.
*
* @version   1.1.2.1
********************************************************************************
*/

#ifndef __gtExtDrvVersionh
#define __gtExtDrvVersionh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <gtExtDrv/os/extDrvOs.h>

#define EXT_DRV_VERSION_MAX_LEN 30
/**
* @struct GT_EXT_DRV_VERSION
 *
 * @brief This struct holds the package version.
*/
typedef struct{

    GT_U8 version[EXT_DRV_VERSION_MAX_LEN];

} GT_EXT_DRV_VERSION;


/**
* @internal gtExtDrvVersion function
* @endinternal
*
* @brief   This function returns the version of the External driver part of the
*         Prestera SW suite.
*
* @param[out] version                  - External driver software version.
*                                       GT_OK on success,
*                                       GT_BAD_PARAM on bad parameters,
*                                       GT_FAIL otherwise.
*/
GT_STATUS gtExtDrvVersion
(
    OUT GT_EXT_DRV_VERSION   *version
);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __gtExtDrvVersionh */




