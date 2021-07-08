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
* @file prvCpssDxChPortDynamicPAGCDAlgo.h
*
* @brief GCD (greatest common divider) algorithm
*
* @version   4
********************************************************************************
*/
#ifndef __PRV_CPSS_DXCH_DYN_PA_GCD_ALGO_H
#define __PRV_CPSS_DXCH_DYN_PA_GCD_ALGO_H

#ifndef STAND_ALONE
    #include <cpss/common/cpssTypes.h>
#else
    #include "CpssInclude/cpssTypes.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
* @internal prvCpssDxChDynPAGCDAlgo function
* @endinternal
*
* @brief   GCD (greatest common divider) algorithm
*
* @note   APPLICABLE DEVICES:     DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES: None.
*
* @param[in] a                        - first number
* @param[in] b                        - second number
*/
GT_U32 prvCpssDxChDynPAGCDAlgo
(
    IN GT_U32 a, 
    IN GT_U32 b
);

#ifdef __cplusplus
}
#endif /* __cplusplus */


#endif

