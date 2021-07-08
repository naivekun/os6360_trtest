/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
*/
/**
********************************************************************************
* @file cstIp.h
*
* @brief Common IP APIs for customer UTs
*
* @version   1
********************************************************************************
*/
#ifndef __cstiph
#define __cstiph

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#ifdef CHX_FAMILY
#include <cpss/dxCh/dxChxGen/ipLpmEngine/cpssDxChIpLpm.h>

/**
* @internal cstIpCleanAppDemoConfig function
* @endinternal
*
* @brief   This function cleans CPSS appDemo default configurations -
*         LPM DB id 0 with VR id 0
*
* @retval GT_NOT_FOUND             - if the LPM DB id is not found
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cstIpCleanAppDemoConfig
(
    GT_VOID
);
#endif /*CHX_FAMILY*/

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __cstiph */


