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
* @file mvHwsCgMac28nmIf.h
*
* @brief CG MAC interface
*
* @version   1
********************************************************************************
*/

#ifndef __mvHwsCgMac28nmIf_H
#define __mvHwsCgMac28nmIf_H

#include <cpss/common/labServices/port/gop/port/mac/mvHwsMacIf.h>

/**
* @internal hwsCgMac28nmIfInit function
* @endinternal
*
* @brief   Init CG MAC configuration sequences and IF functions.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS hwsCgMac28nmIfInit
(
    GT_U8 devNum,
    MV_HWS_MAC_FUNC_PTRS *funcPtrArray
);

/**
* @internal mvHwsCgMac28nmModeCfg function
* @endinternal
*
* @brief   Set the CG MAC in GOP.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] portMacNum               - MAC number
* @param[in] portMode                 - port mode
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsCgMac28nmModeCfg
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  portMacNum,
    MV_HWS_PORT_STANDARD    portMode
);

/**
* @internal mvHwsCgMac28nmReset function
* @endinternal
*
* @brief   Set CG MAC RESET/UNRESET or FULL_RESET action.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] macNum                   - MAC number
* @param[in] portMode                 - port mode
* @param[in] action                   - RESET/UNRESET or FULL_RESET
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsCgMac28nmReset
(
    GT_U8           devNum,
    GT_U32          portGroup,
    GT_U32          portMacNum,
    MV_HWS_PORT_STANDARD    portMode,
    MV_HWS_RESET    action
);

/**
* @internal mvHwsCgMac28nmActiveStatusGet function
* @endinternal
*
* @brief   Return number of MAC active lanes or 0, if current MAC isn't active.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
*                                      pcsNum    - physical MAC number
*
* @param[out] numOfLanes               - number of lanes agregated in MAC
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsCgMac28nmActiveStatusGet
(
    GT_U8                   devNum,
    GT_UOPT                 portGroup,
    GT_UOPT                 macNum,
    MV_HWS_PORT_STANDARD    portMode,
    GT_UOPT                 *numOfLanes
);

/**
* @internal mvHwsCgMac28nmLinkStatus function
* @endinternal
*
* @brief   Get CG MAC link status.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsCgMac28nmLinkStatus
(
    GT_U8       devNum,
    GT_U32      portGroup,
    GT_U32      macNum,
    MV_HWS_PORT_STANDARD    portMode,
    GT_BOOL     *linkStatus
);


/**
* @internal mvHwsCgMac28nmLoopbackStatusGet function
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
GT_STATUS mvHwsCgMac28nmLoopbackStatusGet
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  macNum,
    MV_HWS_PORT_STANDARD    portMode,
    MV_HWS_PORT_LB_TYPE     *lbType
);

/**
* @internal mvHwsCgMac28nmLoopbackSet function
* @endinternal
*
* @brief   Set MAC loopback .
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
GT_STATUS mvHwsCgMac28nmLoopbackSet
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  macNum,
    MV_HWS_PORT_STANDARD    portMode,
    MV_HWS_PORT_LB_TYPE     lbType
);

/**
* @internal mvHwsCgMac28nmIfPortEnable function
* @endinternal
*
* @brief   Perform port enable on the a port MAC.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] macNum                   - MAC number
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsCgMac28nmIfPortEnable
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  macNum,
    MV_HWS_PORT_STANDARD    portMode,
    GT_BOOL                 enable
);

/**
* @internal mvHwsCgMac28nmFcStateCfg function
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
GT_STATUS mvHwsCgMac28nmFcStateCfg
(
    GT_U8                           devNum,
    GT_UOPT                         portGroup,
    GT_UOPT                         macNum,
    MV_HWS_PORT_STANDARD            portMode,
    MV_HWS_PORT_FLOW_CONTROL_ENT    fcState
);

/**
* @internal mvHwsCgMac28nmAccessLock function
* @endinternal
*
* @brief   Protection Definition
*         =====================
*         Multi-Processor Environment
*         This case is protected by HW Semaphore
*         HW Semaphore is defined based in MSYS / CM3 resources
*         In case customer does not use MSYS / CM3 resources,
*         the customer will need to implement its own HW Semaphore
*         This protection is relevant ONLY in case Service CPU Firmware is loaded to CM3
* @param[in] devNum                   - device id
* @param[in] macNum                   - mac number
*                                       None
*/
void mvHwsCgMac28nmAccessLock
(
    GT_U8   devNum,
    GT_UOPT macNum
);

/**
* @internal mvHwsCgMac28nmAccessUnlock function
* @endinternal
*
* @brief   release cg sem
*
* @param[in] devNum                   - device id
* @param[in] macNum                   - mac number
*                                       None
*/
void mvHwsCgMac28nmAccessUnlock
(
    GT_U8   devNum,
    GT_UOPT macNum
);

/**
* @internal mvHwsCgMac28nmAccessGet function
* @endinternal
*
* @brief   Check if CG is in reset.
*         cg lock should be taken before the call to this function
* @param[in] devNum                   - device id
* @param[in] portGroup                - port group id
* @param[in] macNum                   - mac number
*                                       None
*/
GT_BOOL mvHwsCgMac28nmAccessGet
(
    GT_U8                   devNum,
    GT_UOPT                 portGroup,
    GT_UOPT                 macNum
);

#endif /* __mvHwsCgMac28nmIf_H */


