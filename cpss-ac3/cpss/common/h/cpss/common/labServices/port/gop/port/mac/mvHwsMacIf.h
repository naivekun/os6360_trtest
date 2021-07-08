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
* @file mvHwsMacIf.h
*
* @brief
*
* @version   13
********************************************************************************
*/

#ifndef __mvHwsMacIf_H
#define __mvHwsMacIf_H

#ifdef __cplusplus
extern "C" {
#endif

#include <cpss/common/labServices/port/gop/port/mvHwsPortCfgIf.h>
#include <cpss/common/labServices/port/gop/port/private/mvHwsPortTypes.h>
#include <cpss/common/labServices/port/gop/port/private/mvHwsPortMiscIf.h>

#include <cpss/common/labServices/port/gop/common/siliconIf/mvSiliconIf.h>

typedef GT_STATUS (*MV_HWS_MAC_RESET_FUNC_PTR)
(
    GT_U8             devNum,
    GT_U32            portGroup,
    GT_U32            portMacNum,
    MV_HWS_PORT_STANDARD    portMode,
    MV_HWS_RESET      action
);

typedef GT_STATUS (*MV_HWS_MAC_PORT_ENABLE_FUNC_PTR)
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  macNum,
    MV_HWS_PORT_STANDARD    portMode,
    GT_BOOL                 enable
);

typedef GT_STATUS (*MV_HWS_MAC_MODE_CFG_FUNC_PTR)
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  portMacNum,
    MV_HWS_PORT_STANDARD    portMode
);

typedef GT_STATUS (*MV_HWS_MAC_LB_CFG_FUNC_PTR)
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  macNum,
    MV_HWS_PORT_STANDARD    portMode,
    MV_HWS_PORT_LB_TYPE     lbType
);

typedef GT_STATUS (*MV_HWS_MAC_LB_GET_FUNC_PTR)
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  macNum,
    MV_HWS_PORT_STANDARD    portMode,
    MV_HWS_PORT_LB_TYPE     *lbType
);

typedef GT_STATUS (*MV_HWS_MAC_LINK_GET_FUNC_PTR)
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  macNum,
    MV_HWS_PORT_STANDARD    portMode,
    GT_BOOL                 *linkStatus
);

typedef GT_STATUS (*MV_HWS_MAC_ACTIVE_STATUS_FUNC_PTR)
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  macNum,
    MV_HWS_PORT_STANDARD    portMode,
    GT_U32                  *numOfLanes
);

typedef GT_STATUS (*MV_HWS_MAC_CLEAR_CHANNEL_CFG)
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  macNum,
    MV_HWS_PORT_STANDARD    portMode,
    GT_U32                  txIpg,
    GT_U32                  txPreamble,
    GT_U32                  rxPreamble,
    MV_HWS_PORT_CRC_MODE    txCrc,
    MV_HWS_PORT_CRC_MODE    rxCrc
);

typedef char* (*MV_HWS_MAC_TYPE_GET_FUNC_PTR)(void);

typedef GT_STATUS (*MV_HWS_MAC_FC_STATE_CFG_FUNC_PTR)
(
    GT_U8                           devNum,
    GT_U32                          portGroup,
    GT_U32                          macNum,
    MV_HWS_PORT_STANDARD            portMode,
    MV_HWS_PORT_FLOW_CONTROL_ENT    fcState
);

typedef struct
{
  MV_HWS_MAC_RESET_FUNC_PTR         macRestFunc;
  MV_HWS_MAC_MODE_CFG_FUNC_PTR      macModeCfgFunc;
  MV_HWS_MAC_LB_CFG_FUNC_PTR        macLbCfgFunc;
  MV_HWS_MAC_LB_GET_FUNC_PTR        macLbStatusGetFunc;
  MV_HWS_MAC_LINK_GET_FUNC_PTR      macLinkGetFunc;
  MV_HWS_MAC_TYPE_GET_FUNC_PTR      macTypeGetFunc;
  MV_HWS_MAC_ACTIVE_STATUS_FUNC_PTR macActiveStatusFunc;
  MV_HWS_MAC_CLEAR_CHANNEL_CFG      macClearChannelCfgFunc;
  MV_HWS_MAC_FC_STATE_CFG_FUNC_PTR  macFcStateCfgFunc;
  MV_HWS_MAC_PORT_ENABLE_FUNC_PTR   macPortEnableFunc;

}MV_HWS_MAC_FUNC_PTRS;

/**
* @internal hwsMacIfInit function
* @endinternal
*
* @brief   Init all supported MAC types.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
void hwsMacIfInit(void);

/**
* @internal mvHwsMacReset function
* @endinternal
*
* @brief   Set the MAC to reset or exit from reset.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] macNum                   - MAC number
* @param[in] macType                  - MAC type according to port configuration
* @param[in] action                   - normal or reset
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsMacReset
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  portMacNum,
    MV_HWS_PORT_STANDARD    portMode,
    MV_HWS_PORT_MAC_TYPE    macType,
    MV_HWS_RESET            action
);

/**
* @internal mvHwsMacModeCfg function
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
GT_STATUS mvHwsMacModeCfg
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  portMacNum,
    MV_HWS_PORT_STANDARD    portMode
);

/**
* @internal mvHwsMacLoopbackCfg function
* @endinternal
*
* @brief   Configure MAC loopback.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] macNum                   - MAC number
* @param[in] macType                  - MAC type according to port configuration
* @param[in] lbType                   - loopback type (no loopback, rx2tx or tx2rx)
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsMacLoopbackCfg
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  macNum,
    MV_HWS_PORT_STANDARD    portMode,
    MV_HWS_PORT_MAC_TYPE    macType,
    MV_HWS_PORT_LB_TYPE     lbType
);

/**
* @internal mvHwsMacLinkStatusGet function
* @endinternal
*
*
* @param[out] linkStatus               - link up, if true
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsMacLinkStatusGet
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  macNum,
    MV_HWS_PORT_STANDARD    portMode,
    MV_HWS_PORT_MAC_TYPE    macType,
    GT_BOOL                 *linkStatus
);

/**
* @internal mvHwsMacLoopbackStatusGet function
* @endinternal
*
* @brief   Retrive MAC loopback status.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] macNum                   - MAC number
* @param[in] macType                  - MAC type according to port configuration
*
* @param[out] lbType                   - supported loopback type
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsMacLoopbackStatusGet
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  macNum,
    MV_HWS_PORT_STANDARD    portMode,
    MV_HWS_PORT_MAC_TYPE    macType,
    MV_HWS_PORT_LB_TYPE     *lbType
);

/**
* @internal mvHwsMacModeSelectDefaultCfg function
* @endinternal
*
* @brief   MAC mode select default value configuration (needed for AP mechanism).
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] macNum                   - MAC number
* @param[in] macType                  - MAC type according to port configuration
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsMacModeSelectDefaultCfg
(
    GT_U8                 devNum,
    GT_U32                portGroup,
    GT_U32                macNum,
    MV_HWS_PORT_MAC_TYPE  macType
);

/**
* @internal mvHwsMacActiveStatusGet function
* @endinternal
*
* @brief   Return number of MAC active lanes or 0, if current MAC isn't active.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] macNum                   - MAC number
* @param[in] macType                  - MAC type according to port configuration
*
* @param[out] numOfLanes               - number of lanes agregated in PCS
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsMacActiveStatusGet
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  macNum,
    MV_HWS_PORT_STANDARD    portMode,
    MV_HWS_PORT_MAC_TYPE    macType,
    GT_U32                  *numOfLanes
);

/**
* @internal mvHwsMacClearChannelCfg function
* @endinternal
*
* @brief   Configures MAC advanced feature accordingly.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] macNum                   - MAC number
* @param[in] macType                  - MAC type according to port configuration
* @param[in] txIpg                    - TX_IPG
* @param[in] txPreamble               - TX Preamble
* @param[in] rxPreamble               - RX Preamble
* @param[in] txCrc                    - TX CRC
* @param[in] rxCrc                    - RX CRC
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsMacClearChannelCfg
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  macNum,
    MV_HWS_PORT_STANDARD    portMode,
    MV_HWS_PORT_MAC_TYPE    macType,
    GT_U32                  txIpg,
    GT_U32                  txPreamble,
    GT_U32                  rxPreamble,
    MV_HWS_PORT_CRC_MODE    txCrc,
    MV_HWS_PORT_CRC_MODE    rxCrc
);

/**
* @internal hwsMacIfClose function
* @endinternal
*
* @brief   Free all resources allocated by supported MAC types.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
void hwsMacIfClose(void);

/**
* @internal mvHwsMacFcStateCfg function
* @endinternal
*
* @brief   Configure Flow Control state.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] macNum                   - MAC number
* @param[in] macType                  - MAC type according to port configuration
* @param[in] fcState                  - flow control state
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsMacFcStateCfg
(
    GT_U8                           devNum,
    GT_U32                          portGroup,
    GT_U32                          macNum,
    MV_HWS_PORT_STANDARD            portMode,
    MV_HWS_PORT_MAC_TYPE            macType,
    MV_HWS_PORT_FLOW_CONTROL_ENT    fcState
);

GT_STATUS hwsMacGetFuncPtr
(
    GT_U8                   devNum,
    MV_HWS_MAC_FUNC_PTRS    **hwsFuncsPtr
);

/**
* @internal mvHwsMacPortEnable function
* @endinternal
*
* @brief   Perform port disable on the port MAC.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] macNum                   - MAC number
* @param[in] macType                  - MAC type according to port configuration
* @param[in] enable                   - GT_TRUE for port enable, GT_FALSE otherwise
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsMacPortEnable
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  macNum,
    MV_HWS_PORT_STANDARD    portMode,
    MV_HWS_PORT_MAC_TYPE    macType,
    GT_BOOL                 enable
);

#ifdef __cplusplus
}
#endif


#endif /* __mvHwsMacIf_H */


