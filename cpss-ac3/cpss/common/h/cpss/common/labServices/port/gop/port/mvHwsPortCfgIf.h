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
* @file mvHwsPortCfgIf.h
*
* @brief This file contains API for port configuartion and tuning parameters
*
* @version   9
********************************************************************************
*/

#ifndef __mvHwsPortCfgIf_H
#define __mvHwsPortCfgIf_H

#ifdef __cplusplus
extern "C" {
#endif

#include <cpss/common/labServices/port/gop/port/mvHwsPortInitIf.h>
#include <cpss/common/labServices/port/gop/port/private/mvHwsPortMiscIf.h>
#include <cpss/common/labServices/port/gop/port/serdes/mvHwsSerdesIf.h>
#include <cpss/common/labServices/port/gop/port/silicon/general/mvHwsD2dIf.h>

#define IS_SR_MODE(portMode) (portMode == _10GBase_SR_LR || \
                 portMode == _20GBase_SR_LR || \
                 portMode == _40GBase_SR_LR || \
                 portMode == _100GBase_SR10 || \
                 portMode == INTLKN_12Lanes_10_3125G || \
                 portMode == INTLKN_16Lanes_10_3125G || \
                 portMode == INTLKN_12Lanes_12_5G || \
                 portMode == INTLKN_16Lanes_12_5G || \
                 portMode == TCAM || \
                 portMode == _12GBase_SR || \
                 portMode == _48GBase_SR || \
                 portMode == _25GBase_SR || \
                 portMode == _50GBase_SR2 || \
                 portMode == _100GBase_SR4 || \
                 portMode == _22GBase_SR   || \
                 portMode == _29_09GBase_SR)

#define IS_KR_MODE(portMode) (portMode == _10GBase_KR || \
                 portMode == _20GBase_KR || \
                 portMode == _40GBase_KR || \
                 portMode == _40GBase_KR4 || \
                 portMode == _100GBase_KR10 || \
                 portMode == _12_1GBase_KR || \
                 portMode == XLHGL_KR4 || \
                 portMode == CHGL11_LR12 || \
                 portMode == _12GBaseR || \
                 portMode == _5GBaseR || \
                 portMode == _24GBase_KR2 || \
                 portMode == _12_5GBase_KR || \
                 portMode == _25GBase_KR2 || \
                 portMode == _50GBase_KR4 || \
                 portMode == _25GBase_KR  || \
                 portMode == _26_7GBase_KR  || \
                 portMode == _50GBase_KR2 || \
                 portMode == _40GBase_KR2 || \
                 portMode == _100GBase_KR4 || \
                 portMode == _100GBase_MLG || \
                 portMode == _107GBase_KR4 || \
                 portMode == _102GBase_KR4 || \
                 portMode == _48GBaseR     || \
                 portMode == _25GBase_KR_C || \
                 portMode == _50GBase_KR2_C || \
                 portMode == _25GBase_KR_S  || \
                 portMode ==  _50GBase_KR   || \
                 portMode ==  _100GBase_KR2 || \
                 portMode ==  _200GBase_KR4 || \
                 portMode ==  _200GBase_KR8 || \
                 portMode ==  _400GBase_KR8 )


#define IS_CR_MODE(portMode) (portMode == _40GBase_CR4 || \
                 portMode == _25GBase_CR  || \
                 portMode == _50GBase_CR2  || \
                 portMode == _100GBase_CR4 || \
                 portMode == _25GBase_CR_C || \
                 portMode == _50GBase_CR2_C || \
                 portMode == _25GBase_CR_S)

/**
* @enum MV_HWS_PORT_CRC_MODE
 *
 * @brief Defines the different port CRC modes.
 * The XG port is capable of working at four CRC modes,
 * the standard four-bytes mode and the proprietary
 * one-byte/two-bytes/three-bytes CRC mode.
*/
typedef enum{

    HWS_1Byte_CRC,

    HWS_2Bytes_CRC,

    HWS_3Bytes_CRC,

    HWS_4Bytes_CRC

} MV_HWS_PORT_CRC_MODE;

/**
* @internal hwsPortExtendedModeCfg function
* @endinternal
*
* @brief   Enable / disable extended mode on port specified.
*         Extended ports supported only in Lion2 and Alleycat3 devices.
*         For Lion2: 1G, 10GBase-R, 20GBase-R2, RXAUI - can be normal or extended
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
GT_STATUS hwsPortExtendedModeCfg
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  phyPortNum,
    MV_HWS_PORT_STANDARD    portMode,
    GT_BOOL                 extendedMode
);

/**
* @internal mvHwsPortExtendedModeCfgGet function
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
GT_STATUS mvHwsPortExtendedModeCfgGet
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  phyPortNum,
    MV_HWS_PORT_STANDARD    portMode,
    GT_BOOL                 *extendedMode
);

/**
* @internal mvHwsPortFixAlign90Ext function
* @endinternal
*
* @brief   Fix Align90 parameters
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - port standard metric
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPortFixAlign90Ext
(
    GT_U8   devNum,
    GT_U32  portGroup,
    GT_U32  phyPortNum,
    MV_HWS_PORT_STANDARD    portMode
);

/**
* @internal mvHwsExtendedPortSerdesTxIfSelectSet function
* @endinternal
*
* @brief   Configures the Serdes Tx interface selector for Extended Ports
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - port standard metric
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsExtendedPortSerdesTxIfSelectSet
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  phyPortNum,
    MV_HWS_PORT_STANDARD    portMode
);

/**
* @internal hwsPortTxAutoTuneStartSet function
* @endinternal
*
* @brief   Sets the port Tx only parameters according to different working
*         modes/topologies.
*         Can be run any time after create port.
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - port standard metric
* @param[in] portTuningMode           - port TX related tuning mode
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS hwsPortTxAutoTuneStartSet
(
    GT_U8   devNum,
    GT_U32  portGroup,
    GT_U32  phyPortNum,
    MV_HWS_PORT_STANDARD    portMode,
    MV_HWS_PORT_AUTO_TUNE_MODE      portTuningMode,
    GT_U32  optAlgoMask
);

/**
* @internal hwsPortTxAutoTuneActivateSet function
* @endinternal
*
* @brief   Unreset Mac and PCS.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - port standard metric
* @param[in] portTuningMode           - port TX related tuning mode
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS hwsPortTxAutoTuneActivateSet
(
    GT_U8   devNum,
    GT_U32  portGroup,
    GT_U32  phyPortNum,
    MV_HWS_PORT_STANDARD    portMode,
    MV_HWS_PORT_AUTO_TUNE_MODE      portTuningMode
);

/**
* @internal mvHwsPortAutoTuneStop function
* @endinternal
*
* @brief   Stop Tx and Rx training.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - port standard metric
*                                      portTuningMode - port tuning mode
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPortAutoTuneStop
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  phyPortNum,
    MV_HWS_PORT_STANDARD    portMode,
    GT_BOOL                 stopRx,
    GT_BOOL                 stopTx
);

/**
* @internal mvHwsPortAutoTuneStateCheck function
* @endinternal
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - port standard metric
*                                      portTuningMode - port TX related tuning mode
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPortAutoTuneStateCheck
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  phyPortNum,
    MV_HWS_PORT_STANDARD    portMode,
    MV_HWS_AUTO_TUNE_STATUS *rxTune,
    MV_HWS_AUTO_TUNE_STATUS *txTune
);

/**
* @internal mvHwsPortFecCofig function
* @endinternal
*
* @brief   Configure FEC disable/enable on port.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - port standard metric
*                                      portFecEn  - GT_TRUE for FEC enable, GT_FALSE otherwise
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPortFecCofig
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  phyPortNum,
    MV_HWS_PORT_STANDARD    portMode,
    MV_HWS_PORT_FEC_MODE    portFecType
);

/**
* @internal mvHwsPortFecCofigGet function
* @endinternal
*
* @brief   Return FEC status disable/enable on port.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - port standard metric
*
* @param[out] fecMode                  - pointer to FEC mode
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPortFecCofigGet
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  phyPortNum,
    MV_HWS_PORT_STANDARD    portMode,
    MV_HWS_PORT_FEC_MODE    *fecMode
);

/**
* @internal mvHwsPortInterfaceGet function
* @endinternal
*
* @brief   Gets Interface mode and speed of a specified port.
*
* @param[in] devNum                   - physical device number
* @param[in] portGroup                - core number
* @param[in] phyPortNum               - physical port number (or CPU port)
*
* @param[out] portModePtr              - interface mode
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
*/
GT_STATUS mvHwsPortInterfaceGet
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  phyPortNum,
    MV_HWS_PORT_STANDARD    *portModePtr
);

/**
* @internal mvHwsPortClearChannelCfg function
* @endinternal
*
* @brief   Configures MAC advanced feature accordingly.
*         Can be run before create port or after delete port.
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - port standard metric
* @param[in] txIpg                    - TX_IPG
* @param[in] txPreamble               - TX Preamble
* @param[in] rxPreamble               - RX Preamble
* @param[in] txCrc                    - TX CRC
* @param[in] rxCrc                    - RX CRC
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPortClearChannelCfg
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  phyPortNum,
    MV_HWS_PORT_STANDARD    portMode,
    GT_U32                  txIpg,
    GT_U32                  txPreamble,
    GT_U32                  rxPreamble,
    MV_HWS_PORT_CRC_MODE    txCrc,
    MV_HWS_PORT_CRC_MODE    rxCrc
);

/**
* @internal mvHwsPortAcTerminationCfg function
* @endinternal
*
* @brief   Configures AC termination in all port serdes lanes according to mode.
*         Can be run after create port only.
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - port standard metric
* @param[in] portAcTermEn             - enable or disable AC termination
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPortAcTerminationCfg
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  phyPortNum,
    MV_HWS_PORT_STANDARD    portMode,
    GT_BOOL                 portAcTermEn
);

/**
* @internal mvHwsPortCheckGearBox function
* @endinternal
*
* @brief   Check Gear Box Status on related lanes.
*         Can be run after create port only.
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - port standard metric
*
* @param[out] laneLock                 - true or false.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPortCheckGearBox
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  phyPortNum,
    MV_HWS_PORT_STANDARD    portMode,
    GT_BOOL                 *laneLock
);

/**
* @internal hwsForceGearBoxExt function
* @endinternal
*
*/
GT_STATUS hwsForceGearBoxExt
(
    void
);

/**
* @internal mvHwsPortFixAlign90Start function
* @endinternal
*
* @brief   Start fix Align90 process on current port.
*         Can be run any time after create port.
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - port standard metric
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPortFixAlign90Start
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  phyPortNum,
    MV_HWS_PORT_STANDARD    portMode,
    MV_HWS_ALIGN90_PARAMS   *serdesParams
);

/**
* @internal mvHwsPortFixAlign90Status function
* @endinternal
*
* @brief   Run fix Align90 process on current port.
*         Can be run any time after create port.
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - port standard metric
*
* @param[out] statusPtr                - do all lanes of port passed align90 successfully
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPortFixAlign90Status
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  phyPortNum,
    MV_HWS_PORT_STANDARD    portMode,
    MV_HWS_AUTO_TUNE_STATUS *statusPtr
);

/**
* @internal mvHwsPortFixAlign90Stop function
* @endinternal
*
* @brief   Stop fix Align90 process on current port.
*         Can be run any time after create port.
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - port standard metric
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPortFixAlign90Stop
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  phyPortNum,
    MV_HWS_PORT_STANDARD    portMode,
    MV_HWS_ALIGN90_PARAMS   *serdesParams
);

/**
* @internal hwsPortFixAlign90Ext function
* @endinternal
*
* @brief   Run fix Align90 process on current port.
*         Can be run any time after create port.
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - port standard metric
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS hwsPortFixAlign90Ext
(
    GT_U8   devNum,
    GT_U32  portGroup,
    GT_U32  phyPortNum,
    MV_HWS_PORT_STANDARD    portMode,
    GT_U32  optAlgoMask
);

#if 0
/**
* @internal mvHwsPortBetterAdaptationSet function
* @endinternal
*
* @brief   Run the better Align90 adaptation algorithm on specific port number.
*         The function returns indication if the Better adaptation algorithm
*         succeeded to run on port or not.
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - port standard metric
*
* @param[out] betterAlign90            - indicates if the Better adaptation algorithm run on
*                                      specific port or not
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
GT_STATUS mvHwsPortBetterAdaptationSet
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  phyPortNum,
    MV_HWS_PORT_STANDARD    portMode,
    GT_BOOL                 *betterAlign90
);
#endif

/**
* @internal mvHwsPortPartialPowerDown function
* @endinternal
*
* @brief   Enable/Disable the power down Tx and Rx of on Port.
*         The configuration performs Enable/Disable of Tx and Rx on specific Serdes.
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - port standard metric
* @param[in] powerDownRx              - Enable/Disable the Port Rx power down
* @param[in] powerDownTx              - Enable/Disable the Port Tx power down
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPortPartialPowerDown
(
    GT_U8       devNum,
    GT_U32      portGroup,
    GT_U32      phyPortNum,
    MV_HWS_PORT_STANDARD    portMode,
    GT_BOOL     powerDownRx,
    GT_BOOL     powerDownTx
);

/**
* @internal mvHwsPortPartialPowerStatusGet function
* @endinternal
*
* @brief   Get the status of power Tx and Rx on port
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - port standard metric
* @param[in] powerRxStatus            - GT_TRUE - Serdes power Rx is down
*                                      GT_FALSE - Serdes power Rx is up
* @param[in] powerTxStatus            - GT_TRUE - Serdes power Tx is down
*                                      GT_FALSE - Serdes power Tx is up
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPortPartialPowerStatusGet
(
    GT_U8       devNum,
    GT_U32      portGroup,
    GT_U32      phyPortNum,
    MV_HWS_PORT_STANDARD    portMode,
    GT_BOOL     *powerRxStatus,
    GT_BOOL     *powerTxStatus
);

/**
* @internal mvHwsPortFixAlign90 function
* @endinternal
*
* @brief   Run fix Align90 process on current port.
*         Can be run any time after create port.
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - port standard metric
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPortFixAlign90
(
    GT_U8   devNum,
    GT_U32  portGroup,
    GT_U32  phyPortNum,
    MV_HWS_PORT_STANDARD    portMode,
    GT_U32 dummyForCompilation
);

/**
* @internal mvHwsPortUnitReset function
* @endinternal
*
* @brief   Reset/Unreset the MAC/PCS unit number of port.
*         For PMA (Serdes) unit, performs Power-Down for all port lanes
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - port standard metric
* @param[in] unitType                 - Type of unit: MAC, PCS or PMA(Serdes)
* @param[in] action                   - RESET:   Reset the MAC/PCS.  Power-down for PMA(Serdes)
*                                      UNRESET: Unreset the MAC/PCS.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPortUnitReset
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  phyPortNum,
    MV_HWS_PORT_STANDARD    portMode,
    MV_HWS_UNIT             unitType,
    MV_HWS_RESET            action
);

/**
* @internal mvHwsPortPcsActiveStatusGet function
* @endinternal
*
* @brief   Return number of PCS active lanes or 0 if PCS unit is under RESET.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - port standard metric
*
* @param[out] numOfLanes               - number of lanes agregated in PCS
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPortPcsActiveStatusGet
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  phyPortNum,
    MV_HWS_PORT_STANDARD    portMode,
    GT_U32                  *numOfLanes
);

/**
* @internal mvHwsPortPsyncBypassCfg function
* @endinternal
*
* @brief   Configures the bypass of synchronization module.
*         Controls bypass of synchronization module. When working at 8 bits interface,
*         bypass is used. When working at 64 bits interface, bypass is not used.
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - port standard metric
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPortPsyncBypassCfg
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  phyPortNum,
    MV_HWS_PORT_STANDARD    portMode
);

/**
* @internal mvHwsPortEnhanceTuneSet function
* @endinternal
*
* @brief   Perform Enhance Tuning for finding the best peak of the eye
*         on specific port.
*         This API executes the iCAL (Rx-Training) with Max Delay value.
*         Max Delay value set the maximum iterations for running pCAL in
*         mvHwsAvagoSerdesAutoTuneStatus which included the delay.
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - mode type of port
* @param[in] min_LF                   - Minimum LF value that can be set on Serdes (0...15)
* @param[in] max_LF                   - Maximum LF value that can be set on Serdes (0...15)
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPortEnhanceTuneSet
(
    GT_U8       devNum,
    GT_U32      portGroup,
    GT_U32      phyPortNum,
    MV_HWS_PORT_STANDARD    portMode,
    GT_U8       min_LF,
    GT_U8       max_LF
);

/**
* @internal mvHwsPortVoltageGet function
* @endinternal
*
* @brief   Get the voltage (in mV) of BobK device
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
*
* @param[out] voltage                  - device  value (in mV)
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPortVoltageGet
(
    GT_U8       devNum,
    GT_U32      portGroup,
    GT_U32      *voltage
);

/**
* @internal mvHwsPortTemperatureGet function
* @endinternal
*
* @brief   Get the Temperature (in C) of BobK device
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
*
* @param[out] temperature              - device  (in C)
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPortTemperatureGet
(
    GT_U8       devNum,
    GT_U32      portGroup,
    GT_32       *temperature
);

/**
* @internal mvHwsPortFlowControlStateSet function
* @endinternal
*
* @brief   Activates the port loopback modes.
*         Can be run only after create port not under traffic.
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - port standard metric
* @param[in] fcState                  - flow control state
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPortFlowControlStateSet
(
    GT_U8                           devNum,
    GT_U32                          portGroup,
    GT_U32                          phyPortNum,
    MV_HWS_PORT_STANDARD            portMode,
    MV_HWS_PORT_FLOW_CONTROL_ENT    fcState
);

/**
* @internal mvHwsPortEnableSet function
* @endinternal
*
* @brief   Reseting port PTP unit then performing port enable on port MAC unit.
*         This API currently support only port-enable and not port-disable,
*         and only for MAC units supported in AP, and for devices without PHYs
*         that requires furthur port enable configurations and erratas.
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - port standard metric
* @param[in] enable                   -  or disable
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPortEnableSet
(
    GT_U8                           devNum,
    GT_U32                          portGroup,
    GT_U32                          phyPortNum,
    MV_HWS_PORT_STANDARD            portMode,
    GT_BOOL                         enable
);

/*************************************************************************
* @internal mvHwsPortLaneMacToSerdesMuxSet
* @endinternal
 *
* @param[in] devNum              - system device number
* @param[in] portGroup           - port group (core) number
* @param[in] phyPortNum          - physical port number
* @param[in] serdesToMacMuxStc   - struct that contain the connectivity of the mux
************************************************************************/
GT_STATUS mvHwsPortLaneMacToSerdesMuxSet
(
    GT_U8                           devNum,
    GT_U8                           portGroup,
    GT_U8                           phyPortNum,
    MV_HWS_PORT_SERDES_TO_MAC_MUX   *serdesToMacMuxStc
);

/*************************************************************************
* @internal mvHwsPortLaneMacToSerdesMuxGet
* @endinternal
 *
* @param[in] devNum             - physical device number
* @param[in] portGroup          - port group (core) number
* @param[in] phyPortNum         - physical port number
* @param[in] *serdesToMacMuxStc -ptr to struct that will contaion the connectivity of the mux we got from the register
************************************************************************/
GT_STATUS mvHwsPortLaneMacToSerdesMuxGet
(
    GT_U8                           devNum,
    GT_U8                           portGroup,
    GT_U32                          phyPortNum,
    MV_HWS_PORT_SERDES_TO_MAC_MUX   *serdesToMacMuxStc
);

/*************************************************************************
* @internal mvHwsPortLaneMuxGetMacFromSerdes
* @endinternal
 *
* @param[in] devNum             - physical device number
* @param[in] portGroup          - port group (core) number
* @param[in] phyPortNum         - physical port number
* @param[in] serdesNum          - the num of serdes for which to return the macNum
************************************************************************/
GT_U16 mvHwsPortLaneMuxingGetMacFromSerdes
(
    GT_U8                           devNum,
    GT_U8                           portGroup,
    GT_U32                          phyPortNum,
    GT_U16                          serdesNum
);

#if !defined MV_HWS_REDUCED_BUILD_EXT_CM3 || defined MICRO_INIT
GT_STATUS mvHwsAvagoSerdesEdgeDetectByPhase
(
    GT_U8       devNum,
    GT_U32      portGroup,
    GT_U32      phyPortNum,
    MV_HWS_PORT_STANDARD    portMode,
    GT_U32      phase,
    GT_BOOL     *phaseFinishedPtr
);

GT_STATUS mvHwsAvagoSerdesEnhanceTuneSetByPhase
(
    GT_U8       devNum,
    GT_U32      portGroup,
    GT_U32      phyPortNum,
    MV_HWS_PORT_STANDARD    portMode,
    GT_U8       min_LF,
    GT_U8       max_LF,
    GT_U32      phase,
    GT_BOOL     *phaseFinishedPtr
);
/**
* @internal mvHwsAvagoSerdesOneShotTuneByPhase function
* @endinternal
*
* @brief   running one shot tune (iCal) by phases, it take care
* of KR2/CR2 cases with Fec mode disabled, to unharm the
* sequence of port manager.
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - mode type of port
* @param[in] phase....................- current phase
*
* @param[out] phaseFinishedPtr                 - (pointer to)
*       whether or not we finished the last phase.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAvagoSerdesOneShotTuneByPhase
(
    GT_U8       devNum,
    GT_U32      portGroup,
    GT_U32      phyPortNum,
    MV_HWS_PORT_STANDARD    portMode,
    GT_U32      phase,
    GT_BOOL     *phaseFinishedPtr
);

/**
* @internal mvHwsAvagoSerdesLunchOneShotiCal function
* @endinternal
*
* @brief   run one shot dfe tune without checking the tune
*          status
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - mode type of port
*
*
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAvagoSerdesLunchOneShotiCal
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  phyPortNum,
    MV_HWS_PORT_STANDARD    portMode
);

/**
* @internal mvHwsPortAvagoSerdesRxSignalOkStateGet function
* @endinternal
*
* @brief   Get the rx_signal_ok indication from the port lanes. If all lanes
*         have rx_signal_ok indication, then output value is true. Else,
*         output value is false.
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - mode type of port
*
* @param[out] signalOk                 - (pointer to) whether or not rx_signal_ok is up
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPortAvagoSerdesRxSignalOkStateGet
(
    GT_U8   devNum,
    GT_U32  portGroup,
    GT_U32  phyPortNum,
    MV_HWS_PORT_STANDARD    portMode,
    GT_BOOL *signalOk
);

/**
* @internal mvHwsPortAvagoSerdesRxSignalOkChangeGet function
* @endinternal
*
* @brief   Get the rx_signal_ok change indication from the port
*         lanes. If all lanes have rx_signal_ok indication, then
*         output value is true. Else, output value is false.
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - mode type of port
*
* @param[out] signalOk                 - (pointer to) whether or not rx_signal_ok is up
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPortAvagoSerdesRxSignalOkChangeGet
(
    GT_U8   devNum,
    GT_U32  portGroup,
    GT_U32  phyPortNum,
    MV_HWS_PORT_STANDARD    portMode,
    GT_BOOL *signalOkChange
);

/**
* @internal mvHwsPortCtleBiasOverrideEnableSet function
* @endinternal
*
* @brief   write to hws dataBase the override mode and value of Ctle Bias Parameter
*         per port.
* @param[in] devNum                   - device number.
* @param[in] phyPortNum               - physical port number.
* @param[in] ctleBiasOverride         - override mode.
* @param[in] ctleBiasValue            - CTLE Bias value [0..1].
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPortCtleBiasOverrideEnableSet
(
    GT_U8      devNum,
    GT_U32     phyPortNum,
    GT_U32    ctleBiasOverride,
    GT_U32    ctleBiasValue
);

GT_STATUS mvHwsPortCtleBiasOverrideEnableGet
(
    GT_U8      devNum,
    GT_U32     phyPortNum,
    GT_U32    *ctleBiasOverridePtr,
    GT_U32    *ctleBiasValuePtr
);

GT_STATUS mvHwsMsdbFcuEnable
(
    GT_U8   devNum,
    GT_U32 portNum,
    GT_BOOL enable
);

GT_STATUS mvHwsMpfConfigChannel
(
    GT_U8                   devNum,
    GT_U32                  portNum,
    MV_HWS_PORT_STANDARD    portMode,
    GT_BOOL                 enable
);

/**
* @internal mvHwsMtipExtFecTypeGet function
* @endinternal
*
* @brief  Get port fec type
*
* @param[in] devNum          - system device number
* @param[in] portNum         - physical port number
* @param[in] portMode        - port mode
* @param[in] width           - lane width
*
* @retval 0                  - on success
* @retval 1                  - on error
*/
GT_STATUS mvHwsMtipExtFecTypeGet
(
    GT_U8                       devNum,
    GT_U32                      portGroup,
    GT_U32                      portNum,
    MV_HWS_PORT_STANDARD        portMode,
    MV_HWS_PORT_FEC_MODE        *fecTypePtr
);

/**
* @internal mvHwsMpfTsuEnable function
* @endinternal
*
* @brief  set TSU in mpf unit
*
* @param[in] devNum                - system device number
* @param[in] portNum               - physical port number
* @param[in] portMode              - port mode
* @param[in] enable                -  / disable
*
* @retval 0                        - on success
* @retval 1                        - on error
*
* m_RAL.tsu_ip_units_reg_model[i].tsu_ip_units_RegFile.PTP_Tx_Pipe_Status_Delay.Tx_Pipe_Status_Delay.set(2);
* m_RAL.tsu_ip_units_reg_model[i].tsu_ip_units_RegFile.PTP_Tx_Pipe_Status_Delay.update(status);
* m_RAL.tsu_ip_units_reg_model[i].tsu_ip_units_RegFile.PTP_general_control.PTP_unit_enable.set(1);
* m_RAL.tsu_ip_units_reg_model[i].tsu_ip_units_RegFile.PTP_general_control.update(status);
*/

GT_STATUS mvHwsMpfTsuEnable
(
    GT_U8                   devNum,
    GT_U32                  portNum,
    MV_HWS_PORT_STANDARD    portMode,
    GT_BOOL                 enable
);

/**
* @internal mvHwsD2dConfigChannel function
* @endinternal
*
* @brief  Enable Port , D2D TX and D2D RX.
*         The configuration performs enable of Tx and Rx on for
*         specific channel for PCS calendar , MAC TDM RX
*         calendar, and credits. this function should be called
*         twice, from eagle side and from raven side.
*
* @param[in] devNum                   - system device number
* @param[in] d2dNum                   - d2d number
* @param[in] channel                  - d2d local channel index
* @param[in] serdesSpeed              - serdes speed
* @param[in] numOfActLanes            - active lanes number
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
extern GT_STATUS mvHwsD2dConfigChannel
(
    GT_U8               devNum,
    GT_U32              d2dNum,
    GT_U32              channel,
    MV_HWS_SERDES_SPEED serdesSpeed,
    GT_U8               numOfActLanes
);

/**
* @internal mvHwsD2dDisableChannel function
* @endinternal
*
* @brief  Disable Port , D2D TX and D2D RX.The configuration
*         performs Disable of Tx and Rx on for specific channel
*         for PCS calendar , MAC TDM RX calendar, credits reset.
*         this function should be called twice, from eagle side
*         and from raven side.
*
* @param[in] devNum                   - system device number
* @param[in] d2dNum                   - d2d number
* @param[in] channel                  - d2d local channel index
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
extern GT_STATUS mvHwsD2dDisableChannel
(
    GT_U8           devNum,
    GT_U32          d2dNum,
    GT_U32          channel
);

/**
* @internal mvHwsD2dDisableAll50GChannel function
* @endinternal
*
* @brief  Disable all 50G bandwidth channels 0 ..7 for specific
*         d2d. this function should be called twise, from eagle
*         side and from raven side.
*
* @param[in] devNum                   - system device number
* @param[in] d2dNum                   - d2d number
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
extern GT_STATUS mvHwsD2dDisableAll50GChannel
(
    IN  GT_U8   devNum,
    IN  GT_U32  d2dNum
);

/**
* @internal mvHwsD2dGetChannelsConfigurations function
* @endinternal
*
* @brief  count the slices each channel holds in pcs or mac
*         calendars.
*
* @param[in] devNum                   - system device number
* @param[in] d2dNum                   - d2d number
*
* @param[out] channelSlicesArr           - pointer to matrix of
*       counters. the first row contains mac calendar channels
*       slices counters . the second row contains pcs calendars
*       channels slices counters .in each row the channel is the
*       cell index. the cell holds the number of the channel
*       slices
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
extern GT_STATUS mvHwsD2dGetChannelsConfigurations
(
    GT_U8   devNum,
    GT_U32  d2dNum,
    GT_U32  channelSlicesArr[HWS_D2D_PORTS_NUM_CNS+1][2]
);


/**
* @internal mvHwsD2dMacCreditsGet function
* @endinternal
*
* @brief  Get Tx and Rx mac credits for specific channel index
*         and D2D number
*
* @param[in] devNum                   - system device number
* @param[in] d2dNum                   - d2d number
* @param[in] channel                  - d2d local channel index
*
* @param[out] txChannelCreditsArr     - array of tx credits
*       parameters
* @param[out] rxChannelCreditsArr     - array of rx credits
*       parameters
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
extern GT_STATUS mvHwsD2dMacCreditsGet
(
    GT_U8                  devNum,
    GT_U32                 d2dNum,
    GT_U32                 channel,
    GT_U32                 *txChannelCreditsArr,
    GT_U32                 *rxChannelCreditsArr
);

/**
* @internal hwsPulse1msRegCfg function
* @endinternal
*
* @brief   configure the pulse 1ms register according the
*          portmode and core clock
*/
GT_STATUS hwsPulse1msRegCfg
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  phyPortNum,
    MV_HWS_PORT_STANDARD    portMode,
    GT_U32                  coreClk
);


#endif /* MV_HWS_REDUCED_BUILD_EXT_CM3 */

#ifdef __cplusplus
}
#endif

#endif /* __mvHwsPortMiscIf_H */



