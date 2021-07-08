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
* @file cpssTmEligPrioFunc.h
*
* @brief Miscellaneous functions for Eligible Priority Functions Tables configuration.
*
* @version   1
********************************************************************************
*/

#ifndef __cpssTmEligPrioFunch
#define __cpssTmEligPrioFunch

#include <cpss/generic/tm/cpssTmPublicDefs.h>


/**
* @internal cpssTmEligPrioFuncQueueConfig function
* @endinternal
*
* @brief   Configure the Queue Eligible Priority Function according
*         to the User Application parameters
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  xCat; xCat3; Lion; xCat2; DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Lion2; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X; Puma2; Puma3; ExMx.
*
* @param[in] devNum                   - Device number.
* @param[in] eligPrioFuncPtr          - The new configured eligible function pointer (APPLICABLE RANGES: 0..63).
* @param[in] funcOutArr               - The Eligible Priority Function structure array pointer.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on hardware error.
* @retval GT_BAD_PARAM             - on wrong device number or wrong parameter value.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*
* @note The use of Eligible Function Pointer 63 on both Queue and Node level is forbidden.
*       Eligible Function 63 is reserved to indicate that Queue/Node is not eligible.
*
*/
GT_STATUS cpssTmEligPrioFuncQueueConfig
(
    IN GT_U8                        devNum,
    IN GT_U32                       eligPrioFuncPtr,
    IN CPSS_TM_ELIG_PRIO_FUNC_STC   *funcOutArr
);


/**
* @internal cpssTmEligPrioFuncNodeConfig function
* @endinternal
*
* @brief   Configure the Node Eligible Priority Function according
*         to the User Application parameters
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  xCat; xCat3; Lion; xCat2; DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Lion2; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X; Puma2; Puma3; ExMx.
*
* @param[in] devNum                   - Device number.
* @param[in] eligPrioFuncPtr          - The new configured eligible function pointer (APPLICABLE RANGES: 0..63).
* @param[in] level                    - A  to configure the Eligible function with.
* @param[in] funcOutArr               - The Eligible Priority Function structure array pointer.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on hardware error.
* @retval GT_BAD_PARAM             - on wrong device number or wrong parameter value.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*
* @note The use of Eligible Function Pointer 63 on both Queue and Node level is forbidden.
*       Eligible Function 63 is reserved to indicate that Queue/Node is not eligible.
*
*/
GT_STATUS cpssTmEligPrioFuncNodeConfig
(
    IN GT_U8                        devNum,
    IN GT_U32                       eligPrioFuncPtr,
    IN CPSS_TM_LEVEL_ENT            level,
    IN CPSS_TM_ELIG_PRIO_FUNC_STC   *funcOutArr
);


/**
* @internal cpssTmEligPrioFuncConfigAllLevels function
* @endinternal
*
* @brief   Configure the Eligible Priority Function according
*         to the User Application parameters
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  xCat; xCat3; Lion; xCat2; DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Lion2; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X; Puma2; Puma3; ExMx.
*
* @param[in] devNum                   - Device number.
* @param[in] eligPrioFuncPtr          - The new configured eligible function pointer (APPLICABLE RANGES: 0..63).
* @param[in] funcOutArr               - The Eligible Priority Function structure array pointer.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on hardware error.
* @retval GT_BAD_PARAM             - on wrong device number or wrong parameter value.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*
* @note The following API configures the same Eligible Priority Functions
*       at all nodes (A, B, C, and Port) levels Elig. Prio. Tables
*       according to the user’s parameters.
*       It has the same functionality as tm_elig_prio_func_config()
*       and can be used at the user convenience to configure the
*       same eligible function to all the Nodes levels (except for Q level)
*       Note: The use of Eligible Function Pointer 63 on both Queue and Node level is forbidden.
*       Eligible Function 63 is reserved to indicate that Queue/Node is not eligible.
*
*/
GT_STATUS cpssTmEligPrioFuncConfigAllLevels
(
    IN GT_U8                        devNum,
    IN GT_U32                       eligPrioFuncPtr,
    IN CPSS_TM_ELIG_PRIO_FUNC_STC   *funcOutArr
);


#endif 	    /* __cpssTmEligPrioFunch */


