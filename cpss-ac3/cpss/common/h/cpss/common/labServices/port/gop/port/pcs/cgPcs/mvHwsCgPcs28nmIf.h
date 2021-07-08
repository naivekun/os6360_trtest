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
* @file mvHwsCgPcs28nmIf.h
*
* @brief CG PCS interface API
*
* @version   1
********************************************************************************
*/

#ifndef __mvHwsCgPcs28nmIf_H
#define __mvHwsCgPcs28nmIf_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpss/common/labServices/port/gop/port/pcs/mvHwsPcsIf.h>

/**
* @internal mvHwsCgPcs28nmIfInit function
* @endinternal
*
* @brief   Init PCS configuration sequences and IF functions.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsCgPcs28nmIfInit(GT_U8 devNum, MV_HWS_PCS_FUNC_PTRS *funcPtrArray);

/**
* @internal mvHwsCgPcs28nmMode function
* @endinternal
*
* @brief   Set the CG mode with RS_FEC mode
*         In CG PCS:
*         - 25G mode must work with RS_FEC
*         - 50G/100G modes can work with/without RS_FEC
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] portMacNum               - MAC number
* @param[in] portMode                 - port mode
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsCgPcs28nmMode
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  portMacNum,
    MV_HWS_PORT_STANDARD    portMode
);

/**
* @internal mvHwsCgPcs28nmFecConfig function
* @endinternal
*
* @brief   Configure the RS_FEC mode on CG PCS.
*         In CG PCS:
*         - 25G mode must work with RS_FEC
*         - 50G/100G modes can work with/without RS_FEC
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] pcsNum                   - physical PCS number
* @param[in] portFecType              -
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsCgPcs28nmFecConfig
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  pcsNum,
    MV_HWS_PORT_FEC_MODE    portFecType
);

/**
* @internal mvHwsCgPcs28nmFecConfigGet function
* @endinternal
*
* @brief   Return the RS_FEC disable/enable status on PCS.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - port mode
*
* @param[out] portFecTypePtr           - pointer to fec mode
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsCgPcs28nmFecConfigGet
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  phyPortNum,
    MV_HWS_PORT_STANDARD    portMode,
    MV_HWS_PORT_FEC_MODE    *portFecTypePtr
);

/**
* @internal mvHwsCgPcs28nmReset function
* @endinternal
*
* @brief   Set CG PCS RESET/UNRESET or FULL_RESET action.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] portMacNum               - MAC number
* @param[in] portMode                 - port mode
* @param[in] action                   - RESET/UNRESET or FULL_RESET
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsCgPcs28nmReset
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  portMacNum,
    MV_HWS_PORT_STANDARD    portMode,
    MV_HWS_RESET            action
);

/**
* @internal mvHwsCgPcs28nmActiveLanesNumGet function
* @endinternal
*
* @brief   Return the number of active lanes of CG PCS number.
*         25G - 1 lane, 50G - 2 lanes, 100G - 4 lanes
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] pcsNum                   - PCS number
*
* @param[out] activeLanes              - Number of activeLanes
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsCgPcs28nmActiveLanesNumGet
(
    GT_U8       devNum,
    GT_U32      portGroup,
    GT_U32      pcsNum,
    GT_U32      *activeLanes
);

/**
* @internal mvHwsCgPcs28nmActiveStatusGet function
* @endinternal
*
* @brief   Return number of PCS active lanes or 0, if current PCS isn't active.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] pcsNum                   - physical PCS number
*
* @param[out] numOfLanes               - number of lanes agregated in PCS
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsCgPcs28nmActiveStatusGet
(
    GT_U8                   devNum,
    GT_UOPT                 portGroup,
    GT_UOPT                 pcsNum,
    GT_UOPT                 *numOfLanes
);

/**
* @internal mvHwsCgPcs28nmCheckGearBox function
* @endinternal
*
* @brief   Check Gear Box Status on related lanes.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] pcsNum                   - physical PCS number
* @param[in] numOfLanes               - number of lanes agregated in PCS
*
* @param[out] laneLock                 - true or false.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsCgPcs28nmCheckGearBox
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  portMacNum,
    MV_HWS_PORT_STANDARD    portMode,
    GT_BOOL                 *laneLock
);

/**
* @internal mvHwsCgPcs28nmAlignLockGet function
* @endinternal
*
* @brief   Read align lock status of given CG PCS.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] pcsNum                   - physical PCS number
*
* @param[out] lock                     - true or false.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsCgPcs28nmAlignLockGet
(
    GT_U8                   devNum,
    GT_UOPT                 portGroup,
    GT_UOPT                 pcsNum,
    GT_BOOL                 *lock
);

/**
* @internal mvHwsCgPcs28nmSendFaultSet function
* @endinternal
*
* @brief   Configure the PCS to start or stop sending fault signals to partner.
*         on single lane, the result will be local-fault on the sender and remote-fault on the receiver,
*         on multi-lane there will be local-fault on both sides, and there won't be align lock
*         at either side.
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] pcsNum                   - pcs number
* @param[in] portMode                 - port mode
* @param[in] send                     - send or stop sending
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsCgPcs28nmSendFaultSet
(
    GT_U8                devNum,
    GT_U32               portGroup,
    GT_U32               pcsNum,
    MV_HWS_PORT_STANDARD portMode,
    GT_BOOL              send
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __mvHwsCgPcs28nmIf_H */


