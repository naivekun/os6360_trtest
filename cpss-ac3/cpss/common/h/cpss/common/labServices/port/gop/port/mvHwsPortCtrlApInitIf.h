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
* @file mvHwsPortCtrlApInitIf.h
*
* @brief API to configure and run 802.3ap Serdes AutoNeg engine
*
* @version   17
********************************************************************************
*/

#ifndef __mvHwServicesPortCtrlApIf_H
#define __mvHwServicesPortCtrlApIf_H

#ifdef __cplusplus
extern "C" {
#endif

#include <gtOs/gtGenTypes.h>
#include <cpss/common/labServices/port/gop/port/mvHwsPortInitIf.h>
#include <cpss/common/labServices/port/gop/port/private/mvHwsPortTypes.h>
#include <cpss/common/labServices/port/gop/port/mvHwsPortApInitIf.h>

#include <cpss/common/labServices/port/gop/port/private/mvHwsPortMiscIf.h>
/**
* @internal mvHwsApPortCtrlStart function
* @endinternal
*
* @brief   Init AP port capability.
*         Runs AP protocol(802.3ap Serdes AutoNeg) and configures the best port
*         mode and all it's elements accordingly.
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] apCfg                    - AP configuration parameters
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsApPortCtrlStart
(
    GT_U8         devNum,
    GT_U32        portGroup,
    GT_U32        phyPortNum,
    MV_HWS_AP_CFG *apCfg
);

/**
* @internal mvHwsApPortCtrlStop function
* @endinternal
*
* @brief   Disable the AP engine on port and release all its resources.
*         Clears the port mode and release all its resources according to selected.
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsApPortCtrlStop
(
    GT_U8  devNum,
    GT_U32 portGroup,
    GT_U32 phyPortNum
);

/**
* @internal mvHwsApPortCtrlSysAck function
* @endinternal
*
* @brief   Acknowledge port resources were allocated at application level
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsApPortCtrlSysAck
(
    GT_U8                  devNum,
    GT_U32                 portGroup,
    GT_U32                 phyPortNum,
    MV_HWS_PORT_STANDARD   portMode
);

/**
* @internal mvHwsApPortCtrlCfgGet function
* @endinternal
*
* @brief   Returns the AP port configuration.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] apCfg                    - AP configuration parameters
*
* @param[out] apCfg                    - AP configuration parameters
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsApPortCtrlCfgGet
(
    GT_U8         devNum,
    GT_U32        portGroup,
    GT_U32        phyPortNum,
    MV_HWS_AP_CFG *apCfg
);

/**
* @internal mvHwsApPortCtrlStatusGet function
* @endinternal
*
* @brief   Returns the AP port resolution information
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
*                                      phyPortNum - physical port number
* @param[in] apStatus                 - AP status parameters
*
* @param[out] apStatus                 - AP/HCD results
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsApPortCtrlStatusGet
(
    GT_U8                 devNum,
    GT_U32                portGroup,
    GT_U32                apPortNum,
    MV_HWS_AP_PORT_STATUS *apStatus
);

/**
* @internal mvHwsApPortCtrlStatsGet function
* @endinternal
*
* @brief   Returns the AP port statistics information
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
*                                      phyPortNum - physical port number
*
* @param[out] apStats                  - AP stats
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsApPortCtrlStatsGet
(
    GT_U8                devNum,
    GT_U32               portGroup,
    GT_U32               apPortNum,
    MV_HWS_AP_PORT_STATS *apStats
);

/**
* @internal mvHwsApPortCtrlStatsReset function
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
GT_STATUS mvHwsApPortCtrlStatsReset
(
    GT_U8                devNum,
    GT_U32               portGroup,
    GT_U32               apPortNum
);

/**
* @internal mvHwsApPortCtrlIntropSet function
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
GT_STATUS mvHwsApPortCtrlIntropSet
(
    GT_U8                 devNum,
    GT_U32                portGroup,
    GT_U32                phyPortNum,
    MV_HWS_AP_PORT_INTROP *apIntrop
);

/**
* @internal mvHwsApPortCtrlIntropGet function
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
GT_STATUS mvHwsApPortCtrlIntropGet
(
    GT_U8                 devNum,
    GT_U32                portGroup,
    GT_U32                phyPortNum,
    MV_HWS_AP_PORT_INTROP *apIntrop
);

/**
* @internal mvHwsApPortCtrlDebugGet function
* @endinternal
*
* @brief   Return AP debug information
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] apDebug                  - AP debug parameters
*
* @param[out] apDebug                  - AP debug parameters
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsApPortCtrlDebugGet
(
    GT_U8                devNum,
    GT_U32               portGroup,
    GT_U32               phyPortNum,
    MV_HWS_FW_LOG *apDebug
);

/**
* @internal mvHwsFwHwsLogGet function
* @endinternal
*
* @brief   Return FW Hws log information
*
* @param[in] devNum                   - system device number
*                                      fwHwsLog   - FW HWS log parameters pointer
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsFwHwsLogGet
(
    GT_U8           devNum,
    MV_HWS_FW_LOG   *fwHwsLogPtr
);

/**
* @internal mvHwsApPortCtrlAvagoGuiSet function
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
GT_STATUS mvHwsApPortCtrlAvagoGuiSet
(
    GT_U8 devNum,
    GT_U8 state
);

/**
* @internal mvApPortCtrlStatsShow function
* @endinternal
*
* @brief   Print AP port statistics information stored in system
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] apPortNum                - AP port number
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvApPortCtrlStatsShow
(
    GT_U8   devNum,
    GT_U32  portGroup,
    GT_U32  apPortNum
);

/**
* @internal mvApPortCtrlIntropShow function
* @endinternal
*
* @brief   Print AP port introp information stored in system
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] apPortNum                - AP port number
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvApPortCtrlIntropShow
(
    GT_U8   devNum,
    GT_U32  portGroup,
    GT_U32  apPortNum
);

/**
* @internal mvApPortCtrlDebugInfoShow function
* @endinternal
*
* @brief   Print AP port real-time log information stored in system
*
* @param[in] devNum                 - system device number
* @param[in] apPortNum              - AP port number
* @param[in] apCpssLogEnable        - boolean value indicate if
*                                      log will be part of CPSS
*                                      log mechanism or terminal
*                                      print cpssLog   - boolean
*                                      value indicate if log
*                                      will be part of CPSS log
*                                      mechanism or terminal
*                                      print
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvApPortCtrlDebugInfoShow
(
    GT_U8     devNum,
    GT_U32    apPortNum,
    GT_BOOL   apCpssLogEnable
);

/**
* @internal mvHwsLogInfoShow function
* @endinternal
*
* @brief   Print FW Hws real-time log information stored in system.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsLogInfoShow
(
    void
);

/**
* @internal mvHwsApPortCtrlEnablePortCtrlSet function
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
GT_STATUS mvHwsApPortCtrlEnablePortCtrlSet
(
    GT_U8        devNum,
    GT_BOOL      srvCpuEnable
);

/**
* @internal mvHwsApPortCtrlEnablePortCtrlGet function
* @endinternal
*
* @brief   Get the entity responsible to preform port enable after training – Host or Service CPU (default value – service CPU)
*
* @param[in] devNum                   - system device number
*
* @param[out] srvCpuEnablePtr          - (ptr to) port enable parmeter - if TRUE Service CPU responsible to preform port enable after training, if FALSE Host
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsApPortCtrlEnablePortCtrlGet
(
    GT_U8        devNum,
    GT_BOOL      *srvCpuEnablePtr
);

/**
* @internal mvHwsApSerdesTxParametersOffsetSet function
* @endinternal
*
* @brief   Set serdes TX parameters offsets (negative of positive). Those offsets
*         will take place after resolution and prior to running TRX training.
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
*                                      serdesLane  - serdes number
* @param[in] offsets                  - serdes TX values offsets
* @param[in] serdesSpeed              port speed assicoated with the offsets
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsApSerdesTxParametersOffsetSet
(
    GT_U8         devNum,
    GT_U32        portGroup,
    GT_U32        phyPortNum,
    GT_U32        serdesNumber,
    GT_U16        offsets,
    MV_HWS_SERDES_SPEED serdesSpeed
);


/**
* @internal mvHwsApSerdesRxParametersManualSet function
* @endinternal
*
* @brief   Set serdes RX parameters . Those offsets
*         will take place after TRX training.
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
*                                      phyPortNum  - physical port number
* @param[in] rxCfgPtr                 - serdes RX values per speeds 10G and 25G
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsApSerdesRxParametersManualSet
(
    GT_U8                           devNum,
    GT_U32                          portGroup,
    GT_U32                          apPortNum,
    MV_HWS_MAN_TUNE_CTLE_CONFIG_OVERRIDE  *rxCfgPtr
);

#ifdef __cplusplus
}
#endif

#endif /* mvHwServicesPortCtrlApIf_H */



