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
* @file mvHwsPcsIf.h
*
* @brief
*
* @version   17
********************************************************************************
*/

#ifndef __mvHwsPcsIf_H
#define __mvHwsPcsIf_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* General H Files */
#include <cpss/common/labServices/port/gop/port/private/mvHwsPortTypes.h>
#include <cpss/common/labServices/port/gop/port/private/mvHwsPortMiscIf.h>


typedef GT_STATUS (*MV_HWS_PCS_CHECK_GEAR_BOX)
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  portMacNum,
    MV_HWS_PORT_STANDARD    portMode,
    GT_BOOL                 *laneLock
);

typedef GT_STATUS (*MV_HWS_PCS_ALIGN_LOCK_GET_BOX)
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  pcsNum,
    GT_BOOL                 *alignLock
);

typedef GT_STATUS (*MV_HWS_PCS_RESET_FUNC_PTR)
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  portMacNum,
    MV_HWS_PORT_STANDARD    portMode,
    MV_HWS_RESET            action
);

typedef GT_STATUS (*MV_HWS_PCS_RX_RESET_FUNC_PTR)
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  pcsNum,
    MV_HWS_RESET            action
);

typedef GT_STATUS (*MV_HWS_PCS_MODE_CFG_FUNC_PTR)
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  portMacNum,
    MV_HWS_PORT_STANDARD    portMode
);

typedef GT_STATUS (*MV_HWS_PCS_LB_CFG_FUNC_PTR)
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  phyPortNum,
    MV_HWS_PORT_STANDARD    portMode,
    MV_HWS_PORT_LB_TYPE     lbType
);

typedef GT_STATUS (*MV_HWS_PCS_LB_CFG_GET_FUNC_PTR)
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  phyPortNum,
    MV_HWS_PORT_STANDARD    portMode,
    MV_HWS_PORT_LB_TYPE     *lbType
);

typedef GT_STATUS (*MV_HWS_PCS_TEST_GEN_FUNC_PTR)
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  pcsNum,
    MV_HWS_PORT_TEST_GEN_PATTERN     pattern
);

typedef GT_STATUS (*MV_HWS_PCS_TEST_GEN_STATUS_FUNC_PTR)
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  pcsNum,
    MV_HWS_PORT_TEST_GEN_PATTERN     pattern,
    MV_HWS_TEST_GEN_STATUS        *status
);

typedef GT_STATUS (*MV_HWS_PCS_EXT_PLL_CFG_FUNC_PTR)
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  pcsNum
);

typedef GT_STATUS (*MV_HWS_PCS_SIGNAL_DETECT_MASK_SET_FUNC_PTR)
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  pcsNum,
    GT_BOOL                 maskEn
);

typedef GT_STATUS (*MV_HWS_PCS_FEC_CFG_FUNC_PTR)
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  pcsNum,
    MV_HWS_PORT_FEC_MODE    portFecType
);

typedef GT_STATUS (*MV_HWS_PCS_FEC_CFG_GET_FUNC_PTR)
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  phyPortNum,
    MV_HWS_PORT_STANDARD    portMode,
    MV_HWS_PORT_FEC_MODE    *portFecTypePtr
);

typedef GT_STATUS (*MV_HWS_PCS_ACTIVE_STATUS_FUNC_PTR)
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  pcsNum,
    GT_U32                  *numOfLanes
);

typedef char* (*MV_HWS_PCS_TYPE_GET_FUNC_PTR)(void);

typedef GT_STATUS (*MV_HWS_PCS_SEND_FAULT_SET_PTR)
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  pcsNum,
    MV_HWS_PORT_STANDARD    portMode,
    GT_BOOL                 send
);

typedef GT_STATUS (*MV_HWS_PCS_LINK_STATUS_GET_FUNC_PTR)
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  pcsNum,
    GT_BOOL                 *linkStatus
);

typedef struct
{
    MV_HWS_PCS_RESET_FUNC_PTR           pcsResetFunc;
    MV_HWS_PCS_MODE_CFG_FUNC_PTR        pcsModeCfgFunc;
    MV_HWS_PCS_LB_CFG_FUNC_PTR          pcsLbCfgFunc;
    MV_HWS_PCS_LB_CFG_GET_FUNC_PTR      pcsLbCfgGetFunc;
    MV_HWS_PCS_TEST_GEN_FUNC_PTR        pcsTestGenFunc;
    MV_HWS_PCS_TEST_GEN_STATUS_FUNC_PTR pcsTestGenStatusFunc;
    MV_HWS_PCS_TYPE_GET_FUNC_PTR        pcsTypeGetFunc;
    MV_HWS_PCS_RX_RESET_FUNC_PTR        pcsRxResetFunc;
    MV_HWS_PCS_EXT_PLL_CFG_FUNC_PTR     pcsExtPllCfgFunc;
    MV_HWS_PCS_SIGNAL_DETECT_MASK_SET_FUNC_PTR  pcsSignalDetectMaskEn;
    MV_HWS_PCS_FEC_CFG_FUNC_PTR         pcsFecCfgFunc;
    MV_HWS_PCS_FEC_CFG_GET_FUNC_PTR     pcsFecCfgGetFunc;
    MV_HWS_PCS_ACTIVE_STATUS_FUNC_PTR   pcsActiveStatusGetFunc;
    MV_HWS_PCS_CHECK_GEAR_BOX           pcsCheckGearBoxFunc;
    MV_HWS_PCS_ALIGN_LOCK_GET_BOX       pcsAlignLockGetFunc;
    MV_HWS_PCS_SEND_FAULT_SET_PTR       pcsSendFaultSetFunc;
    MV_HWS_PCS_LINK_STATUS_GET_FUNC_PTR   pcsLinkStatusGetFunc;

}MV_HWS_PCS_FUNC_PTRS;

/**
* @internal hwsPcsIfInit function
* @endinternal
*
* @brief   Init all supported PCS types.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
void hwsPcsIfInit(GT_U8 deviceNum);

/**
* @internal mvHwsPcsReset function
* @endinternal
*
* @brief   Set the selected PCS type and number to reset or exit from reset.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] portMacNum               - MAC number
* @param[in] portMode                 - port mode
* @param[in] pcsType                  - PCS type
* @param[in] action                   - reset / unreset
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPcsReset
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  portMacNum,
    MV_HWS_PORT_STANDARD    portMode,
    MV_HWS_PORT_PCS_TYPE    pcsType,
    MV_HWS_RESET            action
);

/**
* @internal mvHwsPcsModeCfg function
* @endinternal
*
* @brief   Set the internal mux's to the required PCS in the PI.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - MAC number
* @param[in] portMode                 - port mode
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPcsModeCfg
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  phyPortNum,
    MV_HWS_PORT_STANDARD    portMode
);

/**
* @internal mvHwsPcsLoopbackCfg function
* @endinternal
*
* @brief   Set PCS loop back.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - port mode
* @param[in] pcsType                  - PCS type
* @param[in] lbType                   - loop back type
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPcsLoopbackCfg
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  phyPortNum,
    MV_HWS_PORT_STANDARD    portMode,
    MV_HWS_PORT_PCS_TYPE    pcsType,
    MV_HWS_PORT_LB_TYPE     lbType
);

/**
* @internal mvHwsPcsLoopbackCfgGet function
* @endinternal
*
* @brief   Return the PCS loop back mode state.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
*                                      pcsNum    - physical PCS number
*                                      pcsType   - PCS type
* @param[in] lbType                   - loop back type
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPcsLoopbackCfgGet
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  phyPortNum,
    MV_HWS_PORT_STANDARD    portMode,
    MV_HWS_PORT_LB_TYPE     *lbType
);

/**
* @internal mvHwsPcsTestGenCfg function
* @endinternal
*
* @brief   Set PCS internal test generator mechanisms.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] pcsNum                   - physical PCS number
* @param[in] pcsType                  - PCS type
* @param[in] pattern                  -  to generate
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPcsTestGenCfg
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  pcsNum,
    MV_HWS_PORT_PCS_TYPE    pcsType,
    MV_HWS_PORT_TEST_GEN_PATTERN     pattern
);

/**
* @internal mvHwsPcsTestGenStatus function
* @endinternal
*
* @brief   Get PCS internal test generator mechanisms error counters and status.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] pcsNum                   - physical PCS number
* @param[in] pcsType                  - PCS type
* @param[in] pattern                  -  to generate
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPcsTestGenStatus
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  pcsNum,
    MV_HWS_PORT_PCS_TYPE    pcsType,
    MV_HWS_PORT_TEST_GEN_PATTERN     pattern,
    MV_HWS_TEST_GEN_STATUS        *status
);

/**
* @internal mvHwsPcsRxReset function
* @endinternal
*
* @brief   Set the selected RX PCS type and number to reset or exit from reset.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] pcsNum                   - physical PCS number
* @param[in] pcsType                  - PCS type
* @param[in] action                   - reset / unreset
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPcsRxReset
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  pcsNum,
    MV_HWS_PORT_PCS_TYPE    pcsType,
    MV_HWS_RESET            action
);

/**
* @internal mvHwsPcsExtPllCfg function
* @endinternal
*
* @brief   Set the selected PCS type and number to external PLL mode.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] pcsNum                   - physical PCS number
* @param[in] pcsType                  - PCS type
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPcsExtPllCfg
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  pcsNum,
    MV_HWS_PORT_PCS_TYPE    pcsType
);

/**
* @internal mvHwsPcsSignalDetectMaskSet function
* @endinternal
*
* @brief   Set all related PCS with Signal Detect Mask value (1/0).
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] pcsNum                   - physical PCS number
* @param[in] pcsType                  - PCS type
* @param[in] maskEn                   - if true, enable signal detect mask bit
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPcsSignalDetectMaskSet
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  pcsNum,
    MV_HWS_PORT_PCS_TYPE    pcsType,
    GT_BOOL                 maskEn
);

/**
* @internal mvHwsPcsFecConfig function
* @endinternal
*
* @brief   Configure FEC disable/enable on PCS.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] pcsNum                   - physical PCS number
* @param[in] pcsType                  - PCS type
*                                      fecType   - 0-AP_ST_HCD_FEC_RES_NONE, 1-AP_ST_HCD_FEC_RES_FC, 2-AP_ST_HCD_FEC_RES_RS
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPcsFecConfig
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  pcsNum,
    MV_HWS_PORT_PCS_TYPE    pcsType,
    MV_HWS_PORT_FEC_MODE    portFecType
);

/**
* @internal mvHwsPcsFecConfigGet function
* @endinternal
*
* @brief   Return FEC disable/enable status on PCS.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - port mode
*
* @param[out] portFecTypePtr
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPcsFecConfigGet
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  phyPortNum,
    MV_HWS_PORT_STANDARD    portMode,
    MV_HWS_PORT_FEC_MODE    *portFecTypePtr
);

/**
* @internal mvHwsPcsActiveStatusGet function
* @endinternal
*
* @brief   Return number of PCS active lanes or 0, if current PCS isn't active.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - port mode
*
* @param[out] numOfLanes               - number of lanes agregated in PCS
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPcsActiveStatusGet
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  phyPortNum,
    MV_HWS_PORT_STANDARD    portMode,
    GT_U32                  *numOfLanes
);

/**
* @internal mvHwsPcsCheckGearBox function
* @endinternal
*
* @brief   Check Gear Box Status on related lanes (currently used only on MMPCS).
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
GT_STATUS mvHwsPcsCheckGearBox
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  portMacNum,
    MV_HWS_PORT_STANDARD    portMode,
    GT_BOOL                 *laneLock
);

/**
* @internal mvHwsPcsAlignLockGet function
* @endinternal
*
* @brief   Read align status of PCS used by port.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - MAC number
* @param[in] portMode                 - port mode
*
* @param[out] lock                     - true or false.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPcsAlignLockGet
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  phyPortNum,
    MV_HWS_PORT_STANDARD    portMode,
    GT_BOOL                 *lock
);

/**
* @internal hwsPcsIfClose function
* @endinternal
*
* @brief   Free all resources allocated by supported PCS types.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
void hwsPcsIfClose(void);

/**
* @internal hwsPcsGetFuncPtr function
* @endinternal
*
* @brief   Get function structure pointer.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS hwsPcsGetFuncPtr
(
    GT_U8                 devNum,
    MV_HWS_PCS_FUNC_PTRS  **hwsFuncsPtr
);

/**
* @internal mvHwsPcsSendFaultSet function
* @endinternal
*
* @brief   Configure the PCS to start sending fault signals to partner, at both 10G and 40G.
*         On 10G the result will be local-fault on the sender and remote-fault on the receiver,
*         on 40G there will be local-fault on both sides, but there won't be align lock
*         at either side.
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] pcsNum                   - Pcs Number
* @param[in] portMode                 - port mode
* @param[in] send                     - start/ stop send faults
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPcsSendFaultSet
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  pcsNum,
    MV_HWS_PORT_PCS_TYPE    pcsType,
    MV_HWS_PORT_STANDARD    portMode,
    GT_BOOL                 send
);

/**
* @internal mvHwsXpcsPortLinkStatusGet function
* @endinternal
*
* @brief   Read link status of PCS used by port.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - MAC number
* @param[in] portMode                 - port mode
*
* @param[out] linkStatus                - link status.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsXpcsPortLinkStatusGet
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  phyPortNum,
    MV_HWS_PORT_STANDARD    portMode,
    GT_BOOL                 *linkStatus
);
#ifdef __cplusplus
}
#endif

#endif /* __mvHwsPcsIf_H */


