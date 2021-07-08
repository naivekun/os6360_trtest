/*******************************************************************************
*                Copyright 2001, Marvell International Ltd.
* This code contains confidential information of Marvell semiconductor, inc.
* no rights are granted herein under any patent, mask work right or copyright
* of Marvell or any third party.
* Marvell reserves the right at its sole discretion to request that this code
* be immediately returned to Marvell. This code is provided "as is".
* Marvell makes no warranties, express, implied or otherwise, regarding its
* accuracy, completeness or performance.
********************************************************************************
*/
/**
********************************************************************************
* @file mvHwsBobKPortIf.h
*
* @brief
*
* @version   18
********************************************************************************
*/

#ifndef __mvHwServicesBobKPortIf_H
#define __mvHwServicesBobKPortIf_H

#ifdef __cplusplus
extern "C" {
#endif

#include <gtOs/gtGenTypes.h>
#include <cpss/common/labServices/port/gop/port/mvHwsPortInitIf.h>

/**
* @internal mvHwsBobKPortAutoTuneSetExt function
* @endinternal
*
* @brief   Sets the port Tx and Rx parameters according to different working
*         modes/topologies.
*         Can be run any time after create port.
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - port standard metric
* @param[in] optAlgoMask              - bit mask for optimization algorithms
* @param[in] portTuningMode           - port tuning mode
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsBobKPortAutoTuneSetExt
(
    GT_U8   devNum,
    GT_U32  portGroup,
    GT_U32  phyPortNum,
    MV_HWS_PORT_STANDARD        portMode,
    MV_HWS_PORT_AUTO_TUNE_MODE      portTuningMode,
    GT_U32  optAlgoMask,
    void *  results
);

/**
* @internal mvHwsBobKPortExtendedModeCfg function
* @endinternal
*
* @brief   Enable / disable extended mode on port specified.
*         Extended ports supported only in Lion2 and Alleycat3 devices.
*         For Lion2:   1G, 10GBase-R, 20GBase-R2, RXAUI - can be normal or extended
*         XAUI, DXAUI, 40GBase-R - only extended
*         For Alleycat3: ports 25 and 27 can be 10GBase_KR, 10GBase_SR_LR - normal or extended modes
*         port 27 can be 20GBase_KR, 20GBase_SR_LR - only in extended mode
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - port standard metric
* @param[in] extendedMode             - enable / disable
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsBobKPortExtendedModeCfg
(
    GT_U8   devNum,
    GT_U32  portGroup,
    GT_U32  phyPortNum,
    MV_HWS_PORT_STANDARD    portMode,
    GT_BOOL extendedMode
);

/**
* @internal mvHwsBobKPortExtendedModeCfgGet function
* @endinternal
*
* @brief   Returns the extended mode status on port specified.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - port standard metric
*
* @param[out] extendedMode             - enable / disable
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsBobKPortExtendedModeCfgGet
(
    GT_U8   devNum,
    GT_U32  portGroup,
    GT_U32  phyPortNum,
    MV_HWS_PORT_STANDARD    portMode,
    GT_BOOL *extendedMode
);

/**
* @internal hwsBobKPortsElementsCfg function
* @endinternal
*
* @brief   Init BobK port modes
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
GT_STATUS hwsBobKPortsElementsCfg
(
    GT_U8              devNum,
    GT_U32             portGroup
);

/**
* @internal hwsBobKPortParamsGetLanes function
* @endinternal
*
* @brief   Set the right firstLane and LanesList of the given 
*          entry (portParams)
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in/out] portParams           - the entry to update
*
* @retval GT_OK                    - on success
* @retval GT_NOT_SUPPORTED - on error;
*         
*/
GT_STATUS hwsBobKPortParamsGetLanes
(
    GT_U8                       devNum,
    GT_U32                      portGroup,
    GT_U32                      phyPortNum,
    MV_HWS_PORT_INIT_PARAMS*    portParams
);

/**
* @internal hwsBobKPortParamsSet function
* @endinternal
*
* @brief   Set Port mode
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - port standard metric
*
* @retval GT_OK                    - on success
* @retval GT_NOT_SUPPORTED; GT_OUT_OF_RANGE; GT_NOT_INITIALIZED;
*         - on error
*/
GT_U32 hwsBobKPortParamsSet
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  phyPortNum,
    MV_HWS_PORT_STANDARD    portMode
);

#ifdef __cplusplus
}
#endif

#endif /* __mvHwServicesBobKPortIf_H */



