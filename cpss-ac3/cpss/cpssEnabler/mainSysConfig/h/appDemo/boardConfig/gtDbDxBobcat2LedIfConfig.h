/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE IS A REFERENCE CODE FOR MARVELL SWITCH PRODUCTS.  IT IS PROVIDED   *
* "AS IS" WITH NO WARRANTIES, EXPRESSED, IMPLIED OR OTHERWISE, REGARDING ITS   *
* ACCURACY, COMPLETENESS OR PERFORMANCE.                                       *
* CUSTOMERS ARE FREE TO MODIFY IT AND USE IT ONLY IN THEIR PRODUCTION          *
* SOFTWARE RELEASES WITH MARVELL SWITCH CHIPSETS.                              *
*******************************************************************************/
#ifndef __GT_DB_DX_BOBCAT2_LEDIF_CONFIGURE_H
#define __GT_DB_DX_BOBCAT2_LEDIF_CONFIGURE_H


#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */



#include <cpss/generic/cpssHwInit/cpssLedCtrl.h>
#include <cpss/generic/config/private/prvCpssConfigTypes.h>
#include <cpss/dxCh/dxChxGen/cpssHwInit/cpssDxChHwInit.h>
#include <cpss/dxCh/dxChxGen/cpssHwInit/cpssDxChHwInitLedCtrl.h>
#include <cpss/dxCh/dxChxGen/cpssHwInit/private/prvCpssDxChHwInit.h>
#include <cpss/dxCh/dxChxGen/port/cpssDxChPortMapping.h>

/**
* @internal appDemoLedInterfacesInit function
* @endinternal
*
* @brief   configure led interface
*
* @param[in] dev                      - device number
* @param[in] bc2BoardType             - board type (used in setBC2 backward compatability
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
GT_STATUS appDemoLedInterfacesInit
(
    IN GT_U8    dev,
    IN GT_U32   bc2BoardType 
);

#ifdef __cplusplus
}
#endif /* __cplusplus */


#endif



