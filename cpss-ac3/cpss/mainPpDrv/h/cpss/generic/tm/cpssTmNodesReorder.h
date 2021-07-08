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
* @file cpssTmNodesReorder.h
*
* @brief TM nodes reshuffling APIs
*
* @version   1
********************************************************************************
*/

#ifndef __cpssTmNodesReorderh
#define __cpssTmNodesReorderh

#include <cpss/generic/tm/cpssTmPublicDefs.h>



/**
* @internal cpssTmNodesSwitch function
* @endinternal
*
* @brief   Switch children between two nodes.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  xCat; xCat3; Lion; xCat2; DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Lion2; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X; Puma2; Puma3; ExMx.
*
* @param[in] devNum                   - Device number.
* @param[in] level                    - Node level.
* @param[in] nodeAIndex               - Node A index in switch.
* @param[in] nodeBIndex               - Node B index in switch.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on hardware error.
* @retval GT_BAD_PARAM             - on wrong device number or wrong parameter value.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/
GT_STATUS cpssTmNodesSwitch
(
    IN GT_U8                        devNum,
    IN CPSS_TM_LEVEL_ENT            level,
    IN GT_U32                       nodeAIndex,
    IN GT_U32                       nodeBIndex
);


#endif 	    /* __cpssTmNodesReorderh */


