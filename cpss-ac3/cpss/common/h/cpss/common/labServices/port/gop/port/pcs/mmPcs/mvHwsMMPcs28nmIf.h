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
* @file mvHwsMMPcs28nmIf.h
*
* @brief MMPCS interface API (for bobcat2)
*
* @version   1
********************************************************************************
*/

#ifndef __mvHwsMMPcs28nmIf_H
#define __mvHwsMMPcs28nmIf_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpss/common/labServices/port/gop/port/pcs/mvHwsPcsIf.h>

/**
* @internal mvHwsMMPcs28nmIfInit function
* @endinternal
*
* @brief   Init MMPCS configuration sequences and IF functions.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsMMPcs28nmIfInit(MV_HWS_PCS_FUNC_PTRS *funcPtrArray);

/**
* @internal mvHwsMMPcs28nmRxReset function
* @endinternal
*
* @brief   Set the selected RX PCS type and number to reset or exit from reset.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] pcsNum                   - physical PCS number
* @param[in] action                   - reset / unreset
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsMMPcs28nmRxReset
(
    GT_U8                   devNum,
    GT_UOPT                 portGroup,
    GT_UOPT                 pcsNum,
    MV_HWS_RESET            action
);

/**
* @internal mvHwsMMPcs28nmReset function
* @endinternal
*
* @brief   Set the selected PCS type and number to reset or exit from reset.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] portMacNum               - MAC number
* @param[in] portMode                 - port mode
* @param[in] action                   - reset / unreset
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsMMPcs28nmReset
(
    GT_U8                   devNum,
    GT_UOPT                 portGroup,
    GT_U32                  portMacNum,
    MV_HWS_PORT_STANDARD    portMode,
    MV_HWS_RESET            action
);

/**
* @internal mvHwsMMPcs28nmMode function
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
GT_STATUS mvHwsMMPcs28nmMode
(
    GT_U8                   devNum,
    GT_UOPT                 portGroup,
    GT_U32                  portMacNum,
    MV_HWS_PORT_STANDARD    portMode
);

GT_STATUS mvMmPcs28nmExtPllCfg
(
    GT_U8                   devNum,
    GT_UOPT                 portGroup,
    GT_UOPT                 pcsNum
);

/**
* @internal mvHwsMMPcs28nmAlignLockGet function
* @endinternal
*
* @brief   Read align lock status of given MPCS.
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
GT_STATUS mvHwsMMPcs28nmAlignLockGet
(
    GT_U8                   devNum,
    GT_UOPT                 portGroup,
    GT_UOPT                 pcsNum,
    GT_BOOL                 *lock
);

/**
* @internal mvHwsMMPcs28nmCheckGearBox function
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
GT_STATUS mvHwsMMPcs28nmCheckGearBox
(
    GT_U8                   devNum,
    GT_UOPT                 portGroup,
    GT_U32                  portMacNum,
    MV_HWS_PORT_STANDARD    portMode,
    GT_BOOL                 *laneLock
);

/**
* @internal mvHwsMMPcs28nmSignalDetectMaskSet function
* @endinternal
*
* @brief   Set all related PCS with Signal Detect Mask value (1/0).
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] pcsNum                   - physical PCS number
* @param[in] maskEn                   - if true, enable signal detect mask bit
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsMMPcs28nmSignalDetectMaskSet
(
    GT_U8                   devNum,
    GT_UOPT                 portGroup,
    GT_UOPT                 pcsNum,
    GT_BOOL                 maskEn
);

/**
* @internal mvHwsMMPcs28nmLoopBack function
* @endinternal
*
* @brief   Set PCS loop back.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] pcsNum                   - physical PCS number
*                                      pcsType   - PCS type
* @param[in] lbType                   - loop back type
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsMMPcs28nmLoopBack
(
    GT_U8                   devNum,
    GT_UOPT                 portGroup,
    GT_U32                  phyPortNum,
    MV_HWS_PORT_STANDARD    portMode,
    MV_HWS_PORT_LB_TYPE     lbType
);

/**
* @internal mvHwsMMPcs28nmLoopBackGet function
* @endinternal
*
* @brief   Return the PCS loop back mode state.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] pcsNum                   - physical PCS number
* @param[in] lbType                   - loop back type
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsMMPcs28nmLoopBackGet
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  phyPortNum,
    MV_HWS_PORT_STANDARD    portMode,
    MV_HWS_PORT_LB_TYPE     *lbType
);

/**
* @internal mvHwsMMPcs28nmTestGenCfg function
* @endinternal
*
* @brief   Set PCS internal test generator mechanisms.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] pcsNum                   - physical PCS number
* @param[in] pattern                  -  to generate
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsMMPcs28nmTestGenCfg
(
    GT_U8                   devNum,
    GT_UOPT                 portGroup,
    GT_UOPT                 pcsNum,
    MV_HWS_PORT_TEST_GEN_PATTERN     pattern
);

/**
* @internal mvHwsMMPcs28nmTestGenStatus function
* @endinternal
*
* @brief   Get PCS internal test generator mechanisms error counters and status.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] pcsNum                   - physical PCS number
* @param[in] pattern                  -  to generate
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsMMPcs28nmTestGenStatus
(
    GT_U8                   devNum,
    GT_UOPT                 portGroup,
    GT_UOPT                 pcsNum,
    MV_HWS_PORT_TEST_GEN_PATTERN  pattern,
    MV_HWS_TEST_GEN_STATUS        *status
);

/**
* @internal mvHwsMMPcs28nmFecConfig function
* @endinternal
*
* @brief   Configure FEC disable/enable on PCS.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] pcsNum                   - physical PCS number
*                                      fecEn     - if true, enable FEC
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsMMPcs28nmFecConfig
(
    GT_U8                   devNum,
    GT_UOPT                 portGroup,
    GT_UOPT                 pcsNum,
    MV_HWS_PORT_FEC_MODE    portFecType
);

/**
* @internal mvHwsMMPcs28nmFecConfigGet function
* @endinternal
*
* @brief   Return FEC disable/enable on PCS.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - port mode
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsMMPcs28nmFecConfigGet
(
    GT_U8                   devNum,
    GT_UOPT                 portGroup,
    GT_U32                  phyPortNum,
    MV_HWS_PORT_STANDARD    portMode,
    MV_HWS_PORT_FEC_MODE    *portFecTypePtr
);

/**
* @internal mvHwsMMPcs28nmActiveStatusGet function
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
GT_STATUS mvHwsMMPcs28nmActiveStatusGet
(
    GT_U8                   devNum,
    GT_UOPT                 portGroup,
    GT_UOPT                 pcsNum,
    GT_UOPT                 *numOfLanes
);

/**
* @internal mvHwsMMPcs28nmSendFaultSet function
* @endinternal
*
* @brief   Configure the PCS to start or stop sending fault signals to partner.
*         on single lane, the result will be local-fault on the sender and remote-fault on the receiver,
*         on multi-lane there will be local-fault on both sides, and there won't be align lock
*         at either side.
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] pcsNum                   - PCS number
* @param[in] portMode                 - port mode
* @param[in] send                     - start or stop faults
*       transmit
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsMMPcs28nmSendFaultSet
(
    GT_U8                devNum,
    GT_U32               portGroup,
    GT_U32               pcsNum,
    MV_HWS_PORT_STANDARD portMode,
    GT_BOOL              send
);

/**
* @internal mvHwsFcFecCorrectedError function
* @endinternal
*
* @brief   Return number of bits corrected by FC-FEC.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] portNum                  - port number
*
* @param[out] correctedError          - number of corrected
*                                       errors
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsFcFecCorrectedError
(
    GT_U8                 devNum,
    GT_U32                portGroup,
    GT_U32                portNum,
    GT_U32                *correctedError
);

/**
* @internal mvHwsRsFecCorrectedError function
* @endinternal
*
* @brief   Return number of codewords corrected by RS-FEC.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] portNum                  - port number
*
* @param[out] correctedError          - number of corrected
*                                       errors
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsRsFecCorrectedError
(
    GT_U8                 devNum,
    GT_U32                portGroup,
    GT_U32                portNum,
    GT_U32                *correctedError
);
#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __mvHwsMMPcsV2If_H */


