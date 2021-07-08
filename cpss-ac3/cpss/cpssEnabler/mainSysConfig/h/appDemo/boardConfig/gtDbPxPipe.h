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
* @file gtDbPxPipe.h
*
* @brief Utils for Pipe devices
*
* @version   1
********************************************************************************
*/

#include <cpss/px/cpssHwInit/cpssPxHwInit.h>

/**
* @internal appDemoPxInitInfoGet function
* @endinternal
*
* @brief   Gets parameters used during Pipe initialization.
*
* @param[out] initInfo                 - (pointer to) parameters used during Pipe initialization.
*
* @retval GT_OK                    - on success,
* @retval GT_BAD_PTR               - bad pointer.
*/
GT_STATUS appDemoPxInitInfoGet
(
    IN CPSS_PX_INIT_INFO_STC    *initInfo
);


