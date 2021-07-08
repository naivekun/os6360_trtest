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
#ifndef __wraplCpssDxChBrgFdb_h__
#define __wraplCpssDxChBrgFdb_h__


/**
* @internal pvrCpssFdbActionModeFromLuaGet
*
* @brief  Function Relevant mode : All modes
*         CPSS wrapper of fdb action mode getting from lua stack.
*
* @note APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
* @note NOT APPLICABLE DEVICES:  None.
*
* @param[in] L                   - lua state
* @param[in] L_index             - entry lua stack index
*
* @param[out] fdbActionModePtr   - fdb action mode
* @param[out] errorMessagePtr    - error message
*/
GT_STATUS pvrCpssFdbActionModeFromLuaGet
(
    IN  lua_State                   *L,
    IN  GT_32                       L_index,
    OUT CPSS_FDB_ACTION_MODE_ENT    *fdbActionModePtr,
    OUT GT_CHAR_PTR                 *errorMessagePtr
);


#endif /* __wraplCpssDxChBrgFdb_h__ */
