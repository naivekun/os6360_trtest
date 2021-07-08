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
* @file wraplCpssDxChTrunk.h
*
* @brief A lua wrapper for trunk functions
*
* @version   4
********************************************************************************
*/

#ifndef __wraplCpssDxChTrunk__
#define __wraplCpssDxChTrunk__

#include <cpssCommon/wrapCpssDebugInfo.h>


#include <cpss/common/cpssTypes.h>


/**
* @internal prvCpssDxChDoesTrunkExist function
* @endinternal
*
* @brief   Function Relevant mode : All modes
*         Check's that trunk exists.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] trunkId                  - trunk id
*                                      P_CALLING_FORMAL_DATA - general debug information (environment variables
*                                      etc); could be empty
*
* @param[out] doesVlanExist            - vlan existen
* @param[out] errorMessagePtr          - error message
*                                       operation execution status
*/
GT_STATUS prvCpssDxChDoesTrunkExist
(
    IN  GT_U8                   devNum,
    IN  GT_TRUNK_ID             trunkId,
    OUT GT_BOOL                 *doesVlanExist,
    OUT GT_CHAR_PTR             *errorMessagePtr
    P_CALLING_FORMAL_DATA
);


#endif /* __wraplCpssDxChTrunk__ */

