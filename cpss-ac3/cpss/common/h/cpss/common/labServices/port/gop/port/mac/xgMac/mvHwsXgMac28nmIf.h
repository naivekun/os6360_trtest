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
* @file mvHwsXgMac28nmIf.h
*
* @brief XLG MAC interface
*
* @version   1
********************************************************************************
*/

#ifndef __mvHwsXgMac28nmIf_H
#define __mvHwsXgMac28nmIf_H

#include <cpss/common/labServices/port/gop/port/mac/mvHwsMacIf.h>

/**
* @internal hwsXgMac28nmIfInit function
* @endinternal
*
* @brief   Init XG MAC configuration sequences and IF functions.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS hwsXgMac28nmIfInit(MV_HWS_MAC_FUNC_PTRS *funcPtrArray);

/**
* @internal mvHwsXgMac28nmReset function
* @endinternal
*
* @brief   Set the XG MAC to reset or exit from reset.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] macNum                   - MAC number
* @param[in] action                   - normal or reset
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsXgMac28nmReset
(
    GT_U8                   devNum,
    GT_UOPT                 portGroup,
    GT_UOPT                 portMacNum,
    MV_HWS_PORT_STANDARD    portMode,
    MV_HWS_RESET            action
);

/**
* @internal mvHwsXgMac28nmModeCfg function
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
GT_STATUS mvHwsXgMac28nmModeCfg
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  portMacNum,
    MV_HWS_PORT_STANDARD    portMode
);

/**
* @internal mvHwsXgMac28nmLoopbackCfg function
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
GT_STATUS mvHwsXgMac28nmLoopbackCfg
(
    GT_U8                   devNum,
    GT_UOPT                 portGroup,
    GT_UOPT                 macNum,
    MV_HWS_PORT_STANDARD    portMode,
    MV_HWS_PORT_LB_TYPE     lbType
);

/**
* @internal mvHwsXgMac28nmLoopbackStatusGet function
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
GT_STATUS mvHwsXgMac28nmLoopbackStatusGet
(
    GT_U8                   devNum,
    GT_UOPT                 portGroup,
    GT_UOPT                 macNum,
    MV_HWS_PORT_STANDARD    portMode,
    MV_HWS_PORT_LB_TYPE     *lbType
);

/**
* @internal mvHwsXgMac28nmLinkStatus function
* @endinternal
*
* @brief   Get MAC link status.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsXgMac28nmLinkStatus
(
    GT_U8                   devNum,
    GT_UOPT                 portGroup,
    GT_UOPT                 macNum,
    MV_HWS_PORT_STANDARD    portMode,
    GT_BOOL                 *linkStatus
);

/**
* @internal mvHwsXgMac28nmActiveStatusGet function
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
GT_STATUS mvHwsXgMac28nmActiveStatusGet
(
    GT_U8                   devNum,
    GT_UOPT                 portGroup,
    GT_UOPT                 macNum,
    MV_HWS_PORT_STANDARD    portMode,
    GT_UOPT                 *numOfLanes
);

/**
* @internal hwsXgMac28nmIfClose function
* @endinternal
*
* @brief   Release all system resources allocated by GE MAC IF functions.
*/
void hwsXgMac28nmIfClose(void);

/**
* @internal mvHwsXgMac28nmFcStateCfg function
* @endinternal
*
* @brief   Configure Flow Control state.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] macNum                   - MAC number
* @param[in] fcState                  - flow control state
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsXgMac28nmFcStateCfg
(
    GT_U8                           devNum,
    GT_UOPT                         portGroup,
    GT_UOPT                         macNum,
    MV_HWS_PORT_STANDARD            portMode,
    MV_HWS_PORT_FLOW_CONTROL_ENT    fcState
);

/**
* @internal hwsXgMac28nmPortEnable function
* @endinternal
*
* @brief   Perform port enable on the a port MAC.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] macNum                   - MAC number
* @param[in] enable                   - GT_TRUE for port enable, GT_FALSE otherwise
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS hwsXgMac28nmPortEnable
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  macNum,
    MV_HWS_PORT_STANDARD    portMode,
    GT_BOOL                 enable
);

#endif /* __mvHwsXgMacIf_H */


