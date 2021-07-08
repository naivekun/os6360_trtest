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
* @file cpssInit.h
 *
* @brief Initialization function for PP's , regardless to PP's types.
* 
* @version   8
********************************************************************************
*/

#ifndef __cpssInith
#define __cpssInith

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpss/common/cpssTypes.h>

/**
* @internal cpssPpInit function
* @endinternal
*
* @brief   This function initialize the internal DB of the CPSS regarding PPs.
*         This function must be called before any call to a PP functions ,
*         i.e before calling cpssExMxPmHwPpPhase1Init/cpssDxChHwPpPhase1Init.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; Puma2; Puma3; ExMx.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - on failure.
*/
GT_STATUS cpssPpInit
(
    void
);
/**
* @internal cpssPpDestroy function
* @endinternal
*
* @brief   This function destroy the internal DB of the CPSS regarding PPs.
*         This function should be called after all devices have been removed from
*         the CPSS and system need to 'clean up' before restart initialization again.
*         so function must be called before recalling cpssPpInit(...)
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; Puma2; Puma3; ExMx.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - on failure.
*/
GT_STATUS cpssPpDestroy
(
    void
);

/**
* @internal cpssSystemDualDeviceIdModeEnableSet function
* @endinternal
*
* @brief   This function declare 'This system support dual device Id'.
*         The function MUST be called for any system with Lion2 ('Dual device Id' device).
*         This means that application MUST not use 'odd' hwDevNum to any device in the system.
*         Allowed to use only : 0,2,4..30 (also for ch1,2,3,xcat1,2,Lion in this system)
*         For such system this function must be called before any call to a PP functions ,
*         i.e before calling cpssDxChHwPpPhase1Init(...).
*         The function MUST be called only after cpssPpInit(...)
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; Puma2; Puma3; ExMx.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] enable                   - indication that the system is in 'dual deviceId' mode.
*                                      GT_TRUE - This system support dual device Id
*                                      GT_FALSE - This system NOT support dual device Id
*
* @retval GT_OK                    - on success,
* @retval GT_BAD_STATE             - the function called before call to cpssPpInit(...).
*/
GT_STATUS cpssSystemDualDeviceIdModeEnableSet
(
    IN GT_BOOL  enable
);

/**
* @internal cpssSystemDualDeviceIdModeEnableGet function
* @endinternal
*
* @brief   This function retrieve if 'This system support dual device Id'.
*         This means that application MUST not use 'odd' hwDevNum to any device in the system.
*         Allowed to use only : 0,2,4..30 (also for ch1,2,3,xcat1,2,Lion in this system)
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; Puma2; Puma3; ExMx.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @param[out] enablePtr                - (pointer to)indication that the system is in 'dual deviceId' mode.
*                                      GT_TRUE - This system support dual device Id
*                                      GT_FALSE - This system NOT support dual device Id
*
* @retval GT_OK                    - on success,
* @retval GT_BAD_PTR               - on NULL pointer
*/
GT_STATUS cpssSystemDualDeviceIdModeEnableGet
(
    OUT GT_BOOL  *enablePtr
);

/**
* @internal cpssPpDump function
* @endinternal
*
* @brief   Dump function , to print the info on a specific PP.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; Puma2; Puma3; ExMx.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device Number
*
* @retval GT_OK                    - on success,
* @retval GT_BAD_PARAM             - on bad device number
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPpDump
(
    IN GT_U8   devNum
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __cpssInith */


