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
* @file mvHwsXgMacIf.h
*
* @brief XLG MAC interface
*
* @version   6
********************************************************************************
*/

#ifndef __mvHwsXgMacIf_H
#define __mvHwsXgMacIf_H

#include <cpss/common/labServices/port/gop/port/mac/mvHwsMacIf.h>

/**
* @internal hwsXgMacIfInit function
* @endinternal
*
* @brief   Init XG MAC configuration sequences and IF functions.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS hwsXgMacIfInit(MV_HWS_MAC_FUNC_PTRS *funcPtrArray);

/**
* @internal mvHwsXgMacReset function
* @endinternal
*
* @brief   .
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsXgMacReset
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  portMacNum,
    MV_HWS_PORT_STANDARD    portMode,
    MV_HWS_RESET            action
);

/**
* @internal mvHwsXgMacModeCfg function
* @endinternal
*
* @brief   Set the internal mux's to the required MAC in the GOP.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] portMacNum               - MAC number
* @param[in] portMode                 - port mode
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsXgMacModeCfg
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  portMacNum,
    MV_HWS_PORT_STANDARD    portMode
);

/**
* @internal mvHwsXgMacLoopbackCfg function
* @endinternal
*
* @brief   Configure MAC loopback.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] macNum                   - MAC number
* @param[in] lbType                   - supported loopback type
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsXgMacLoopbackCfg
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  macNum,
    MV_HWS_PORT_STANDARD    portMode,
    MV_HWS_PORT_LB_TYPE     lbType
);

/**
* @internal mvHwsXgMacLoopbackStatusGet function
* @endinternal
*
* @brief   Retrive MAC loopback status.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] macNum                   - MAC number
*
* @param[out] lbType                   - supported loopback type
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsXgMacLoopbackStatusGet
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  macNum,
    MV_HWS_PORT_STANDARD    portMode,
    MV_HWS_PORT_LB_TYPE     *lbType
);

/**
* @internal mvHwsXgMacActiveStatusGet function
* @endinternal
*
* @brief   Return number of MAC active lanes or 0, if current MAC isn't active.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] macNum                   - MAC number
*
* @param[out] numOfLanes               - number of lanes agregated in PCS
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsXgMacActiveStatusGet
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  macNum,
    MV_HWS_PORT_STANDARD    portMode,
    GT_U32                  *numOfLanes
);

/**
* @internal mvHwsXgMacLinkStatus function
* @endinternal
*
* @brief   Get MAC link status.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsXgMacLinkStatus
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  macNum,
    MV_HWS_PORT_STANDARD    portMode,
    GT_BOOL                 *linkStatus
);

#endif /* __mvHwsXgMacIf_H */


