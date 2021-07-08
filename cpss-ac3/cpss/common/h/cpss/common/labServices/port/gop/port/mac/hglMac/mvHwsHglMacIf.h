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
* @file mvHwsHglMacIf.h
*
* @brief XLG MAC interface
*
* @version   6
********************************************************************************
*/

#ifndef __mvHwsHglMacIf_H
#define __mvHwsHglMacIf_H

#include <cpss/common/labServices/port/gop/port/mac/mvHwsMacIf.h>

/**
* @internal hwsHglMacIfInit function
* @endinternal
*
* @brief   Init HGL MAC configuration sequences and IF functions.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS hwsHglMacIfInit(MV_HWS_MAC_FUNC_PTRS *);

/**
* @internal mvHwsHglMacReset function
* @endinternal
*
* @brief   Set the HGL MAC to reset or exit from reset.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] macNum                   - MAC number
* @param[in] action                   - normal or reset
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsHglMacReset
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  portMacNum,
    MV_HWS_PORT_STANDARD    portMode,
    MV_HWS_RESET            action
);

/**
* @internal mvHwsHglMacModeCfg function
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
GT_STATUS mvHwsHglMacModeCfg
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  portMacNum,
    MV_HWS_PORT_STANDARD    portMode
);

/**
* @internal mvHwsHglMacLinkStatus function
* @endinternal
*
* @brief   Get MAC link status.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsHglMacLinkStatus
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  macNum,
    MV_HWS_PORT_STANDARD    portMode,
    GT_BOOL                 *linkStatus
);

/**
* @internal mvHwsHglMacLoopbackCfg function
* @endinternal
*
* @brief   Loopback not supported.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsHglMacLoopbackCfg
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  macNum,
    MV_HWS_PORT_STANDARD    portMode,
    MV_HWS_PORT_LB_TYPE     lbType
);

/**
* @internal mvHwsHglMacLoopbackStatusGet function
* @endinternal
*
* @brief   Loopback not supported.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsHglMacLoopbackStatusGet
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  macNum,
    MV_HWS_PORT_STANDARD    portMode,
    MV_HWS_PORT_LB_TYPE     *lbType
);

/**
* @internal mvHwsHglMacActiveStatusGet function
* @endinternal
*
* @brief   Return number of MAC active lanes or 0, if current MAC isn't active.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] macNum                   - MAC number
*
* @param[out] numOfLanes               - number of lanes agregated
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsHglMacActiveStatusGet
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  macNum,
    MV_HWS_PORT_STANDARD    portMode,
    GT_U32                  *numOfLanes
);

#endif /* __mvHwsHglMacIf_H */


