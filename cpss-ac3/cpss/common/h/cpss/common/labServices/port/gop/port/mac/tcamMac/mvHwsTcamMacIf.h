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
* @file mvHwsTcamMacIf.h
*
* @brief TCAM MAC interface
*
* @version   1
********************************************************************************
*/

#ifndef __mvHwsTcamMacIf_H
#define __mvHwsTcamMacIf_H

#include <cpss/common/labServices/port/gop/port/mac/mvHwsMacIf.h>

/**
* @internal hwsTcamMacIfInit function
* @endinternal
*
* @brief   Init TCAM MAC configuration sequences and IF functions.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS hwsTcamMacIfInit(MV_HWS_MAC_FUNC_PTRS *funcPtrArray);

/**
* @internal mvHwsTcamMacReset function
* @endinternal
*
* @brief   Set the TCAM MAC to reset or exit from reset.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsTcamMacReset
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  portMacNum,
    MV_HWS_PORT_STANDARD    portMode,
    MV_HWS_RESET            action
);

/**
* @internal mvHwsTcamMacModeCfg function
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
GT_STATUS mvHwsTcamMacModeCfg
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  portMacNum,
    MV_HWS_PORT_STANDARD    portMode
);

/**
* @internal mvHwsTcamMacLoopbackCfg function
* @endinternal
*
* @brief   .
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsTcamMacLoopbackCfg
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  macNum,
    MV_HWS_PORT_LB_TYPE     lbType
);

/**
* @internal mvHwsTcamMacLinkStatus function
* @endinternal
*
* @brief   Get MAC link status.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsTcamMacLinkStatus
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  macNum,
    MV_HWS_PORT_STANDARD    portMode,
    GT_BOOL                 *linkStatus
);

#endif /* __mvHwsTcamMacIf_H */


