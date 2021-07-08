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
* @file mvHwsIpcApis.h
*
* @brief Definitions for HWS IPS feature
*
* @version   1
********************************************************************************
*/

#ifndef __mvHwsIpcApis_H
#define __mvHwsIpcApis_H


#include <cpss/common/labServices/port/gop/port/private/mvHwsPortTypes.h>
#include <cpss/common/labServices/port/gop/port/mvHwsServiceCpuFwIf.h>
#if defined(CHX_FAMILY) || defined(PX_FAMILY)
#include <cpss/common/srvCpu/prvCpssGenericSrvCpuIpc.h>
#endif


/**
* @internal mvHwsIpcDbInit function
* @endinternal
*
* @brief   HW Services Ipc database initialization
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_VOID mvHwsIpcDbInit(GT_U8 devNum);

/**
* @internal mvHwsPortInitIpc function
* @endinternal
*
* @brief   Sends to HWS request to init physical port.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - port standard metric
* @param[in] lbPort                   - if true, init port without serdes activity
* @param[in] refClock                 - Reference clock frequency
* @param[in] refClock                 - Reference clock source line
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPortInitIpc
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  phyPortNum,
    MV_HWS_PORT_STANDARD    portMode,
    GT_BOOL                 lbPort,
    MV_HWS_REF_CLOCK_SUP_VAL refClock,
    MV_HWS_REF_CLOCK_SOURCE  refClockSource

);

/**
* @internal mvHwsPortResetIpc function
* @endinternal
*
* @brief   Sends to HWS request to power down or reset physical port.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - port standard metric
*                                      lbPort     - if true, init port without serdes activity
* @param[in] action                   - Power down or reset
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPortResetIpc
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  phyPortNum,
    MV_HWS_PORT_STANDARD    portMode,
    MV_HWS_PORT_ACTION	    action
);

/**
* @internal mvHwsPortAutoTuneStateCheckIpc function
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
GT_STATUS mvHwsPortAutoTuneStateCheckIpc
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  phyPortNum,
    MV_HWS_PORT_STANDARD    portMode,
    MV_HWS_AUTO_TUNE_STATUS *rxTune,
    MV_HWS_AUTO_TUNE_STATUS *txTune
);


/**
* @internal mvHwsPortFecConfigIpc function
* @endinternal
*
* @brief   Send message to configure FEC disable/enable on port.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - port standard metric
* @param[in] portFecType              - GT_TRUE for FEC enable, GT_FALSE otherwise
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPortFecConfigIpc
(
    GT_U8                devNum,
    GT_U32               portGroup,
    GT_U32               phyPortNum,
    MV_HWS_PORT_STANDARD portMode,
    MV_HWS_PORT_FEC_MODE portFecType

);

/**
* @internal mvHwsPortFecConfigGetIpc function
* @endinternal
*
* @brief   Send message to get FEC status.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - port standard metric
*
* @param[out] fecMode                  - (pointer to) FEC mode
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPortFecConfigGetIpc
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  phyPortNum,
    MV_HWS_PORT_STANDARD    portMode,
    MV_HWS_PORT_FEC_MODE    *fecMode
);

/**
* @internal mvHwsPortLinkStatusGetIpc function
* @endinternal
*
* @brief   Sends message to get the port link status.
*         Can be run any time.
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - port standard metric
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPortLinkStatusGetIpc
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  phyPortNum,
    MV_HWS_PORT_STANDARD    portMode,
    GT_BOOL                 *linkStatus
);

/**
* @internal mvHwsPortAutoTuneSetExtIpc function
* @endinternal
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - port standard metric
* @param[in] portTuningMode           - port TX related tuning mode
* @param[in] optAlgoMask              - bit mask for optimization algorithms
* @param[in] portTuningMode           - port tuning mode
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPortAutoTuneSetExtIpc
(
    GT_U8                           devNum,
    GT_U32                          portGroup,
    GT_U32                          phyPortNum,
    MV_HWS_PORT_STANDARD            portMode,
    MV_HWS_PORT_AUTO_TUNE_MODE      portTuningMode,
    GT_U32                          optAlgoMask,
    void                            *results
);

/**
* @internal mvHwsPortAutoTuneStopIpc function
* @endinternal
*
* @brief   Send IPC message to stop Tx and Rx training
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - port standard metric
* @param[in] stopRx                   - stop RX
* @param[in] stopTx                   - stop Tx
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPortAutoTuneStopIpc
(
    GT_U8                           devNum,
    GT_U32                          portGroup,
    GT_U32                          phyPortNum,
    MV_HWS_PORT_STANDARD            portMode,
	GT_BOOL                         stopRx,
	GT_BOOL                         stopTx
);

/**
* @internal mvHwsSerdesManualRxConfigIpc function
* @endinternal
*
* @brief   Send IPC message to configure SERDES Rx parameters for all SERDES lanes.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
*                                      phyPortNum  - physical port number
*                                      portMode    - port standard metric
*                                      config params:
* @param[in] alig
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsSerdesManualRxConfigIpc
(
    GT_U8                       devNum,
    GT_U32                      portGroup,
    GT_U32                      serdesNum,
	MV_HWS_SERDES_TYPE			serdesType,
    MV_HWS_PORT_MAN_TUNE_MODE   portTuningMode,
    GT_U32                      sqlch,
    GT_U32                      ffeRes,
    GT_U32                      ffeCap,
    GT_BOOL                     dfeEn,
    GT_U32                      alig
);

/**
* @internal mvHwsSerdesManualTxConfigIpc function
* @endinternal
*
* @brief   Send IPC message to configure SERDES Tx parameters for all SERDES lanes.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
*                                      phyPortNum  - physical port number
*                                      portMode    - port standard metric
*                                      config params:
* @param[in] txAmpShft
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsSerdesManualTxConfigIpc
(
    GT_U8               devNum,
    GT_U32              portGroup,
    GT_U32              serdesNum,
	MV_HWS_SERDES_TYPE  serdesType,
    GT_U32              txAmp,
    GT_BOOL             txAmpAdj,
    GT_U32              emph0,
    GT_U32              emph1,
    GT_BOOL             txAmpShft
);

/**
* @internal mvHwsPortPolaritySetIpc function
* @endinternal
*
* @brief   Send message to set the port polarity of the Serdes lanes (Tx/Rx).
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - port standard metric
* @param[in] txInvMask                - bitmap of 32 bit, each bit represent Serdes
* @param[in] rxInvMask                - bitmap of 32 bit, each bit represent Serdes
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPortPolaritySetIpc
(
	GT_U8                   devNum,
	GT_U32                  portGroup,
	GT_U32                  phyPortNum,
	MV_HWS_PORT_STANDARD    portMode,
	GT_32                   txInvMask,
	GT_32                   rxInvMask
);

/**
* @internal mvHwsPortTxEnableIpc function
* @endinternal
*
* @brief   Turn of the port Tx according to selection.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - port standard metric
* @param[in] enable                   - enable/disable port Tx
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPortTxEnableIpc
(
	GT_U8                   devNum,
	GT_U32                  portGroup,
	GT_U32                  phyPortNum,
	MV_HWS_PORT_STANDARD    portMode,
	GT_BOOL                 enable
);

/**
* @internal mvHwsPortTxEnableGetIpc function
* @endinternal
*
* @brief   Retrieve the TX status of all port serdeses.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - port standard metric
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPortTxEnableGetIpc
(
	GT_U8   devNum,
	GT_U32  portGroup,
	GT_U32  phyPortNum,
	MV_HWS_PORT_STANDARD    portMode,
	GT_BOOL serdesTxStatus[]
);

/**
* @internal mvHwsPortLoopbackSetIpc function
* @endinternal
*
* @brief   Send message to activates the port loopback modes.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - port standard metric
* @param[in] lpPlace                  - unit for loopback configuration
*                                      lpType     - loopback type
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPortLoopbackSetIpc
(
	GT_U8                   devNum,
	GT_U32                  portGroup,
	GT_U32                  phyPortNum,
	MV_HWS_PORT_STANDARD    portMode,
	MV_HWS_UNIT             lpPlace,
	MV_HWS_PORT_LB_TYPE     lbType
);

/**
* @internal mvHwsPortLoopbackStatusGetIpc function
* @endinternal
*
* @brief   Send IPC message to retrive MAC loopback status.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - port standard metric
* @param[in] lpPlace                  - unit for loopback configuration
*
* @param[out] lbType                   - supported loopback type
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPortLoopbackStatusGetIpc
(
	GT_U8                   devNum,
	GT_U32                  portGroup,
	GT_U32                  phyPortNum,
	MV_HWS_PORT_STANDARD    portMode,
	MV_HWS_UNIT             lpPlace,
	MV_HWS_PORT_LB_TYPE     *lbType
);

/**
* @internal mvHwsPortPPMSetIpc function
* @endinternal
*
* @brief   Send IPC message to increase/decrease Tx clock on port (added/sub ppm).
*         Can be run only after create port not under traffic.
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - port standard metric
* @param[in] portPPM                  - limited to +/- 3 taps
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPortPPMSetIpc
(
	GT_U8                   devNum,
	GT_U32                  portGroup,
	GT_U32                  phyPortNum,
	MV_HWS_PORT_STANDARD    portMode,
	MV_HWS_PPM_VALUE	    portPPM
);

/**
* @internal mvHwsPortPPMGetIpc function
* @endinternal
*
* @brief   Send message to check the entire line configuration
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - port standard metric
*
* @param[out] portPPM                  - current PPM
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPortPPMGetIpc
(
	GT_U8                   devNum,
	GT_U32                  portGroup,
	GT_U32                  phyPortNum,
	MV_HWS_PORT_STANDARD    portMode,
	MV_HWS_PPM_VALUE	    *portPPM
);

/**
* @internal mvHwsPortInterfaceGetIpc function
* @endinternal
*
* @brief   Send message to gets Interface mode and speed of a specified port.
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
GT_STATUS mvHwsPortInterfaceGetIpc
(
	GT_U8	                devNum,
	GT_U32	                portGroup,
	GT_U32	                phyPortNum,
	MV_HWS_PORT_STANDARD	*portModePtr
);

/**
* @internal mvHwsPortPcsActiveStatusGetIpc function
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
GT_STATUS mvHwsPortPcsActiveStatusGetIpc
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  phyPortNum,
    MV_HWS_PORT_STANDARD    portMode,
    GT_U32                  *numOfLanes
);

/**
* @internal mvHwsPortSignalDetectGetIpc function
* @endinternal
*
* @brief   Retrieve the signalDetect status of all port serdeses.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - port standard metric
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPortSignalDetectGetIpc
(
	GT_U8   devNum,
	GT_U32  portGroup,
	GT_U32  phyPortNum,
	MV_HWS_PORT_STANDARD    portMode,
	GT_BOOL signalDetect[]
);

/**
* @internal mvHwsPortCdrLockStatusGetIpc function
* @endinternal
*
* @brief   Send message to get the CDR lock status of all port serdeses.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - port standard metric
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPortCdrLockStatusGetIpc
(
	GT_U8   devNum,
	GT_U32  portGroup,
	GT_U32  phyPortNum,
	MV_HWS_PORT_STANDARD    portMode,
	GT_BOOL cdrLockStatus[]
);

/**
* @internal mvHwsPortFlowControlStateSetIpc function
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
GT_STATUS mvHwsPortFlowControlStateSetIpc
(
    GT_U8                           devNum,
    GT_U32                          portGroup,
    GT_U32                          phyPortNum,
    MV_HWS_PORT_STANDARD            portMode,
    MV_HWS_PORT_FLOW_CONTROL_ENT    fcState
);

/**
* @internal mvHwsPortApStartIpc function
* @endinternal
*
* @brief   Send message to configure AP port parameters and start execution
*
* @param[in] devNum                   - physical device number
* @param[in] portGroup                - core number
* @param[in] phyPortNum               - physical port number
* @param[in] apCfg                    - AP configuration parameters
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPortApStartIpc
(
    GT_U8  devNum,
    GT_U32 portGroup,
    GT_U32 phyPortNum,
    GT_U32 *apCfg
);

/**
* @internal mvHwsPortApStopIpc function
* @endinternal
*
* @brief   Send message to stop AP port execution
*
* @param[in] devNum                   - physical device number
* @param[in] portGroup                - core number
* @param[in] phyPortNum               - physical port number
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPortApStopIpc
(
    GT_U8  devNum,
    GT_U32 portGroup,
    GT_U32 phyPortNum
);

/**
* @internal mvHwsPortApSysAckIpc function
* @endinternal
*
* @brief   Send message to notify AP state machine that port resource allocation
*         was executed by the Host and it can continue execution
* @param[in] devNum                   - physical device number
* @param[in] portGroup                - core number
* @param[in] phyPortNum               - physical port number
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPortApSysAckIpc
(
    GT_U8                  devNum,
    GT_U32                 portGroup,
    GT_U32                 phyPortNum,
    MV_HWS_PORT_STANDARD   portMode
);

/**
* @internal mvHwsPortApCfgGetIpc function
* @endinternal
*
* @brief   Send message to retrive AP port configuration parameters
*
* @param[in] devNum                   - physical device number
* @param[in] portGroup                - core number
* @param[in] phyPortNum               - physical port number
*                                      portApCfg  - AP configuration parameters
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPortApCfgGetIpc
(
    GT_U8  devNum,
    GT_U32 portGroup,
    GT_U32 phyPortNum,
    GT_U32 *apCfg
);

/**
* @internal mvHwsPortApStatusGetIpc function
* @endinternal
*
* @brief   Send message to retrive AP port status parameters
*
* @param[in] devNum                   - physical device number
* @param[in] portGroup                - core number
* @param[in] phyPortNum               - physical port number
*                                      apStatus   - AP Status parameters
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPortApStatusGetIpc
(
    GT_U8  devNum,
    GT_U32 portGroup,
    GT_U32 phyPortNum,
    GT_U32 *portApStatus
);

/**
* @internal mvHwsPortApStatsGetIpc function
* @endinternal
*
* @brief   Send message to retrive AP port stats parameters
*
* @param[in] devNum                   - physical device number
* @param[in] portGroup                - core number
* @param[in] phyPortNum               - physical port number
* @param[in] portApStats              - AP Stats parameters
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPortApStatsGetIpc
(
    GT_U8  devNum,
    GT_U32 portGroup,
    GT_U32 phyPortNum,
    GT_U32 *portApStats
);

/**
* @internal mvHwsPortApStatsResetIpc function
* @endinternal
*
* @brief   Reset AP port statistics information
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] apPortNum                - AP port number
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPortApStatsResetIpc
(
    GT_U8  devNum,
    GT_U32 portGroup,
    GT_U32 apPortNum
);

/**
* @internal mvHwsPortApIntropSetIpc function
* @endinternal
*
* @brief   Set AP port introp information
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
*                                      apintrop    - AP introp parameters
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPortApIntropSetIpc
(
    GT_U8  devNum,
    GT_U32 portGroup,
    GT_U32 phyPortNum,
    GT_U32 *portApIntrop
);

/**
* @internal mvHwsPortApIntropGetIpc function
* @endinternal
*
* @brief   Return AP port introp information
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
*                                      apintrop    - AP introp parameters
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPortApIntropGetIpc
(
    GT_U8  devNum,
    GT_U32 portGroup,
    GT_U32 phyPortNum,
    GT_U32 *portApIntrop
);

/**
* @internal mvHwsPortApDebugGetIpc function
* @endinternal
*
* @brief   Return AP debug information
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
*                                      apDebug    - AP debug parameters
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPortApDebugGetIpc
(
    GT_U8  devNum,
    GT_U32 portGroup,
    GT_U32 phyPortNum,
    GT_U32 *portApDebug
);

/**
* @internal mvHwsLogGetIpc function
* @endinternal
*
* @brief   Return FW Hws log information
*
* @param[in] devNum                   - system device number
* @param[in] hwsLogPtr                - hws log parameters pointer
*
* @param[out] hwsLogPtr                - hws log parameters pointer
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsLogGetIpc
(
    GT_U8  devNum,
    GT_U32 *hwsLogPtr
);

/**
* @internal mvHwsPortApAvagoGuiSetIpc function
* @endinternal
*
* @brief   Set AP state machine state when Avago GUI is enabled
*         Avago GUI access Avago Firmware as SBUS command level
*         Therefore it is required to stop the periodic behiviour of AP state
*         machine when Avago GUI is enabled
* @param[in] devNum                   - system device number
* @param[in] state                    - Avago GUI state
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPortApAvagoGuiSetIpc
(
    GT_U8 devNum,
    GT_U8 state
);

/**
* @internal mvHwsPortAvagoCfgAddrGetIpc function
* @endinternal
*
* @brief   Return Avago Serdes Configuration structure address
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
*
* @param[out] avagoCfgAddr             - Avago Serdes Configuration structure address
* @param[out] avagoCfgSize             - Avago Serdes Configuration structure size
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPortAvagoCfgAddrGetIpc
(
    GT_U8  devNum,
    GT_U32 portGroup,
    GT_U32 phyPortNum,
    GT_U32 *avagoCfgAddr,
    GT_U32 *avagoCfgSize
);


/**
* @internal mvHwsPortAvagoAaplInitIpc function
* @endinternal
*
* @brief   Init Avago Serdes Configuration structure pointers parameters, on service CPU, with NULL values.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS  mvHwsPortAvagoAaplInitIpc
(
    GT_U8  devNum,
    GT_U32 portGroup,
    GT_U32 phyPortNum
);

/**
* @internal mvHwsPortVosOVerrideParamsSetIpc function
* @endinternal
*
* @brief   Set the VOS Override params for the service CPU in the DB. This API allows
*         the overall VOS params to be sent in small buffers in order to keep
*         low memory consumption in the IPC.
* @param[in] devNum                   - system device number
* @param[in] vosOverrideParamsBufPtr  - (pointer to) VOS Override params to set
* @param[in] vosOverrideParamsBufIdx  - relative index of the params buffer in the
*                                      overall VOS Override params DB.
* @param[in] vosOverrideParamsBufLength - length of the buffer array.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPortVosOVerrideParamsSetIpc
(
    GT_U8  devNum,
    GT_U32 *vosOverrideParamsBufPtr,
    GT_U32  vosOverrideParamsBufIdx,
    GT_U32  vosOverrideParamsBufLength
);

/**
* @internal mvHwsPortVosOverrideModeSetIpc function
* @endinternal
*
* @brief   Set the VOS Override mode for the service CPU.
*
* @param[in] devNum                   - system device number
* @param[in] vosOverride              - Vos override mode.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPortVosOverrideModeSetIpc
(
    GT_U8  devNum,
    GT_BOOL vosOverride
);

/**
* @internal mvHwsExecuteCommandLineIpc function
* @endinternal
*
* @brief   Execute CLI command by IPC message
*
* @param[in] devNum                   - system device number
* @param[in] ipcCLIPtr                - command line to execute
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter
*/
GT_STATUS mvHwsExecuteCommandLineIpc
(
    GT_U8                           devNum,
    GT_U8                          *ipcCLIPtr
);


/**
* @internal mvHwsPortApEnablePortCtrlSetIpc function
* @endinternal
*
* @brief   Set the entity responsible to preform port enable after training – Host or Service CPU (default value – service CPU)
*
* @param[in] devNum                   - system device number
* @param[in] srvCpuEnable             - if TRUE Service CPU responsible to preform port enable after training, if FALSE Host
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPortApEnablePortCtrlSetIpc
(
    GT_U8        devNum,
    GT_BOOL      srvCpuEnable
);

/**
* @internal mvHwsPortApEnablePortCtrlGetIpc function
* @endinternal
*
* @brief   Get the entity responsible to preform port enable after training – Host or Service CPU (default value – service CPU)
*
* @param[in] devNum                   - system device number
*
* @param[out] srvCpuEnablePtr          - (ptr to) port enable parmeter - if TRUE Service CPU responsible to preform port
*                                      enable after training, if FALSE Host
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPortApEnablePortCtrlGetIpc
(
    GT_U8        devNum,
    GT_BOOL      *srvCpuEnablePtr
);

/**
* @internal mvHwsPortApSerdesTxParametersOffsetSetIpc function
* @endinternal
*
* @brief   Set serdes TX parameters offsets (negative of positive). Those offsets
*         will take place after resolution and prior to running TRX training.
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] serdesNumber             - serdes number
* @param[in] offsets                  - serdes TX values offsets
*                                      portMode    - port mode assicoated with the offsets
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPortApSerdesTxParametersOffsetSetIpc
(
    GT_U8         devNum,
    GT_U32        portGroup,
    GT_U32        phyPortNum,
    GT_U32        serdesNumber,
    GT_U16        offsets,
    MV_HWS_SERDES_SPEED serdesSpeed
);

/**
* @internal mvHwsPortApSerdesRxParametersManualSetIpc function
* @endinternal
*
* @brief   Set serdes RX parameters . Those offsets
*         will take place after TRX training.
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] apPortNum                - physical port number
* @param[in] rxCfgPtr                 - serdes RX values per speeds 10G and 25G
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPortApSerdesRxParametersManualSetIpc
(
    GT_U8                           devNum,
    GT_U32                          portGroup,
    GT_U32                          apPortNum,
    MV_HWS_MAN_TUNE_CTLE_CONFIG_OVERRIDE *rxCfgPtr
);

#ifdef __cplusplus
}
#endif

#endif /* __mvHwsIpcApis_H */



