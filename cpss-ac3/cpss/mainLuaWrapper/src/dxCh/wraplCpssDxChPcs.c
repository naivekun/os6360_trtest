/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
* wraplCpssDxChPcs.c
*
* DESCRIPTION:
*       A lua wrapper for pcs functions.
*
*
* DEPENDENCIES:
*
* COMMENTS:
*
* FILE REVISION NUMBER:
*       $Revision: 1 $
*******************************************************************************/

#include <cpss/common/cpssTypes.h>
#include <cpss/dxCh/dxChxGen/port/cpssDxChPortCtrl.h>
#include <generic/private/prvWraplGeneral.h>

#include <extUtils/luaCLI/luaCLIEngine_genWrapper.h>



/*******************************************************************************
* wrlDxChPortXgPcsLanesSwapSet
*
* DESCRIPTION:
*       Set swapping configuration of XAUI or HyperG.Stack port SERDES Lanes.
*
* APPLICABLE DEVICES:
*       DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X.
*
* NOT APPLICABLE DEVICES:
*       DxCh1; DxCh1_Diamond; DxCh2; Bobcat3; Aldrin2; Falcon.
*
* INPUTS:
*       L                - lua state
*
* OUTPUTS:
*       None
*
* RETURNS:
*       1 - on success
*       2 - on error with error code and message
*
* COMMENTS:
*       Supported for XAUI or HyperG.Stack ports only.
*
*******************************************************************************/
int wrlDxChPortXgPcsLanesSwapSet
(
    IN lua_State *L
)
{
    GT_U8                   devNum          = (GT_U8)
                                                            lua_tonumber(L, 1);
                                                                    /* devId */
    GT_PHYSICAL_PORT_NUM    portNum         = (GT_PHYSICAL_PORT_NUM)
                                                            lua_tonumber(L, 2);
                                                                /* portNum */
    GT_U32                  rxSerdesLaneArr[CPSS_DXCH_PORT_XG_PSC_LANES_NUM_CNS];
    GT_U32                  txSerdesLaneArr[CPSS_DXCH_PORT_XG_PSC_LANES_NUM_CNS];
    GT_STATUS               status          = GT_OK;
    GT_CHAR_PTR             error_message   = NULL;
    GT_U32                  pcsLane;

    for (pcsLane = 0; pcsLane < CPSS_DXCH_PORT_XG_PSC_LANES_NUM_CNS; pcsLane++)
    {
        rxSerdesLaneArr[pcsLane] = (GT_U32) lua_tonumber(L, 3 + pcsLane);
        txSerdesLaneArr[pcsLane] = (GT_U32) lua_tonumber(L, 7 + pcsLane);
    }

    status = cpssDxChPortXgPscLanesSwapSet(devNum, portNum, rxSerdesLaneArr, txSerdesLaneArr);

    if (GT_OK != status)
    {
        error_message   = "Could not set lane swapping";
    }

    return prvWrlCpssStatusToLuaPush(status, L, error_message);
}


