/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
* wraplCpssDxChBrgFdb.c
*
* DESCRIPTION:
*       A lua wrapper for bridge multicast entries.
*
* DEPENDENCIES:
*
* COMMENTS:
*
* FILE REVISION NUMBER:
*       $Revision: 2 $
*******************************************************************************/


#include <cpss/common/cpssTypes.h>


/**
* @internal prvCpssDxChDoesMulticastGroupIndexExist function
* @endinternal
*
* @brief   Function Relevant mode : All modes
*         Check's that trunk exists.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] vidx                     - multicast group index
*
* @param[out] doesMulticastGroupIndexExist
*                                      - multicast group index existence
* @param[out] errorMessagePtr          - error message
*                                       operation execution status
*/
GT_STATUS prvCpssDxChDoesMulticastGroupIndexExist
(
    IN  GT_U8                   devNum,
    IN  GT_U16                  vidx,
    OUT GT_BOOL                 *doesMulticastGroupIndexExist, 
    OUT GT_CHAR_PTR             *errorMessagePtr
);


