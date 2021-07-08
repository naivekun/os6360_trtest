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
* @file gtExtDrvVersion.c
*
* @brief Includes software version information for the External drivers part of
* the Prestera software suite.
*
* @version   1.1.2.1
********************************************************************************
*/

#include <gtExtDrv/version/gtExtDrvVersion.h>


#ifdef EXT_DRV_DEBUG
#define DBG_INFO(x) osPrintf x
#else
#define DBG_INFO(x)
#endif


#define EXT_DRV_VERSION   "v1.2a3"

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
)
{
    if(version == NULL)
        return GT_BAD_PARAM;

    if(osStrlen(EXT_DRV_VERSION) > EXT_DRV_VERSION_MAX_LEN)
    {
        DBG_INFO(("EXT_DRV_VERSION exceeded max len\n"));
        return GT_FAIL;
    }

    osMemCpy(version->version,EXT_DRV_VERSION,osStrlen(EXT_DRV_VERSION) + 1);
    return GT_OK;
}


