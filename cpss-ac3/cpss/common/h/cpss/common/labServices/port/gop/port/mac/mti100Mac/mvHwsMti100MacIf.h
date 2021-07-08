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
* @file mvHwsMti100MacIf.h
*
* @brief MTI100 MAC interface
*
* @version   1
********************************************************************************
*/

#ifndef __mvHwsMti100MacIf_H
#define __mvHwsMti100MacIf_H

#include <cpss/common/labServices/port/gop/port/mac/mvHwsMacIf.h>

/**
* @internal hwsMti100MacIfInit function
* @endinternal
*
* @brief   Init CG MAC configuration sequences and IF functions.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsMti100MacIfInit
(
    MV_HWS_MAC_FUNC_PTRS *funcPtrArray
);

/**
* @internal mvHwsMti100MacModeCfg function
* @endinternal
*
* @brief   Set the MTI100 MAC in GOP.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] portMacNum               - MAC number
* @param[in] portMode                 - port mode
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsMti100MacModeCfg
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  portMacNum,
    MV_HWS_PORT_STANDARD    portMode
);

/**
* @internal mvHwsMti100MacReset function
* @endinternal
*
* @brief   Set CG MAC RESET/UNRESET or FULL_RESET action.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] portMacNum               - MAC number
* @param[in] action                   - RESET/UNRESET or FULL_RESET
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsMti100MacReset
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  portMacNum,
    MV_HWS_PORT_STANDARD    portMode,
    MV_HWS_RESET            action
);


/**
* @internal mvHwsMti100MacActiveStatusGet function
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
GT_STATUS mvHwsMti100MacActiveStatusGet
(
    GT_U8                   devNum,
    GT_UOPT                 portGroup,
    GT_UOPT                 macNum,
    MV_HWS_PORT_STANDARD    portMode,
    GT_UOPT                 *numOfLanes
);

/**
* @internal mvHwsMti100MacLinkStatus function
* @endinternal
*
* @brief   Get CG MAC link status.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsMti100MacLinkStatus
(
    GT_U8       devNum,
    GT_U32      portGroup,
    GT_U32      macNum,
    MV_HWS_PORT_STANDARD    portMode,
    GT_BOOL     *linkStatus
);


/**
* @internal mvHwsMti100MacLoopbackStatusGet function
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
GT_STATUS mvHwsMti100MacLoopbackStatusGet
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  macNum,
    MV_HWS_PORT_STANDARD    portMode,
    MV_HWS_PORT_LB_TYPE     *lbType
);

/**
* @internal mvHwsMti100MacIfPortEnable function
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
GT_STATUS mvHwsMti100MacPortEnable
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  macNum,
    MV_HWS_PORT_STANDARD    portMode,
    GT_BOOL                 enable
);

#if 0
/**
* @internal mvHwsMti100MacFcStateCfg function
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
GT_STATUS mvHwsMti100MacFcStateCfg
(
    GT_U8                           devNum,
    GT_UOPT                         portGroup,
    GT_UOPT                         macNum,
    MV_HWS_PORT_STANDARD            portMode,
    MV_HWS_PORT_FLOW_CONTROL_ENT    fcState
);


/**
* @internal hwsMti100MacIfClose function
* @endinternal
*
* @brief   Release all system resources allocated by MAC IF functions.
*/
void hwsMti100MacIfClose(void);

/**
* @internal mvHwsMti100MacAccessLock function
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
void mvHwsMti100MacAccessLock
(
    GT_U8   devNum,
    GT_UOPT macNum
);

/**
* @internal mvHwsMti100MacAccessUnlock function
* @endinternal
*
* @brief   release cg sem
*
* @param[in] devNum                   - device id
* @param[in] macNum                   - mac number
*                                       None
*/
void mvHwsMti100MacAccessUnlock
(
    GT_U8   devNum,
    GT_UOPT macNum
);

/**
* @internal mvHwsMti100MacAccessGet function
* @endinternal
*
* @brief   Check if CG is in reset.
*         cg lock should be taken before the call to this function
* @param[in] devNum                   - device id
* @param[in] portGroup                - port group id
* @param[in] macNum                   - mac number
*                                       None
*/
GT_BOOL mvHwsMti100MacAccessGet
(
    GT_U8                   devNum,
    GT_UOPT                 portGroup,
    GT_UOPT                 macNum
);
#endif
#endif /* __mvHwsMti100MacIf_H */


