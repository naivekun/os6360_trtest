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
* @file mvHwsGPcs28nmIf.h
*
* @brief GPCS interface API
*
* @version   2
********************************************************************************
*/

#ifndef __mvHwsGPcs28nmIf_H
#define __mvHwsGPcs28nmIf_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpss/common/labServices/port/gop/port/pcs/mvHwsPcsIf.h>

/**
* @internal mvHwsGPcs28nmIfInit function
* @endinternal
*
* @brief   Init GE MAC configuration sequences and IF functions.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsGPcs28nmIfInit(MV_HWS_PCS_FUNC_PTRS *funcPtrArray);

/**
* @internal mvHwsGPcs28nmLoopBack function
* @endinternal
*
* @brief   Set PCS loop back.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] pcsNum                   - physical PCS number
* @param[in] lbType                   - loop back type
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsGPcs28nmLoopBack
(
    GT_U8                   devNum,
    GT_UOPT                 portGroup,
    GT_U32                  phyPortNum,
    MV_HWS_PORT_STANDARD    portMode,
    MV_HWS_PORT_LB_TYPE     lbType
);

/**
* @internal mvHwsGPcs28nmLoopBackGet function
* @endinternal
*
* @brief   Get the PCS loop back mode state.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] pcsNum                   - physical PCS number
* @param[in] lbType                   - loop back type
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsGPcs28nmLoopBackGet
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  phyPortNum,
    MV_HWS_PORT_STANDARD    portMode,
    MV_HWS_PORT_LB_TYPE     *lbType
);

/**
* @internal mvHwsGPcs28nmSignalDetectMaskSet function
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
GT_STATUS mvHwsGPcs28nmSignalDetectMaskSet
(
    GT_U8                   devNum,
    GT_UOPT                 portGroup,
    GT_UOPT                 pcsNum,
    GT_BOOL                 maskEn
);

/**
* @internal mvHwsQsgmiiPcs28nmSignalDetectMaskSet function
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
GT_STATUS mvHwsQsgmiiPcs28nmSignalDetectMaskSet
(
    GT_U8                   devNum,
    GT_UOPT                 portGroup,
    GT_UOPT                 pcsNum,
    GT_BOOL                 maskEn
);

/**
* @internal mvHwsGPcs28nmTestGenCfg function
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
GT_STATUS mvHwsGPcs28nmTestGenCfg
(
    GT_U8                   devNum,
    GT_UOPT                 portGroup,
    GT_UOPT                 pcsNum,
    MV_HWS_PORT_TEST_GEN_PATTERN     pattern
);

/**
* @internal mvHwsGPcs28nmTestGenStatus function
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
GT_STATUS mvHwsGPcs28nmTestGenStatus
(
    GT_U8                   devNum,
    GT_UOPT                 portGroup,
    GT_UOPT                 pcsNum,
    MV_HWS_PORT_TEST_GEN_PATTERN     pattern,
    MV_HWS_TEST_GEN_STATUS        *status
);

/**
* @internal mvHwsGPcs28nmActiveStatusGet function
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
GT_STATUS mvHwsGPcs28nmActiveStatusGet
(
    GT_U8                   devNum,
    GT_UOPT                 portGroup,
    GT_UOPT                 pcsNum,
    GT_UOPT                 *numOfLanes
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __mvHwsGPcsIf_H */


