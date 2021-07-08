/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
* mvHwsGPcsIf_2.h
*
* DESCRIPTION:
*       GPCS interface API
*
* FILE REVISION NUMBER:
*       $Revision: 1 $
*
*******************************************************************************/

#ifndef __mvHwsGPcsRev2If_H
#define __mvHwsGPcsRev2If_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpss/common/labServices/port/gop/port/pcs/gPcs/mvHwsGPcsIf.h>

/**
* @internal mvHwsGPcsRev2IfInit function
* @endinternal
*
* @brief   Init GE MAC configuration sequences and IF functions.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsGPcsRev2IfInit(MV_HWS_PCS_FUNC_PTRS *funcPtrArray);

/**
* @internal mvHwsGPcsRev2ModeCfg function
* @endinternal
*
* @brief   Set the internal mux's to the required PCS in the PI.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] portMacNum               - MAC number
* @param[in] portMode                 - port mode
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsGPcsRev2ModeCfg
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  portMacNum,
    MV_HWS_PORT_STANDARD    portMode
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __mvHwsGPcsIf_H */


