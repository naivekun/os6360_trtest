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
* @file mvHwsPortApInitIf.h
*
* @brief API to configure and run 802.3ap Serdes AutoNeg engine
*
* @version   17
********************************************************************************
*/

#ifndef __mvHwServicesPortApIf_H
#define __mvHwServicesPortApIf_H

#ifdef __cplusplus
extern "C" {
#endif

#include <cpss/common/labServices/port/gop/port/mvHwsPortInitIf.h>
#include <cpss/common/labServices/port/gop/port/private/mvHwsPortTypes.h>

/*
 * IEEE capability vector - Lion2
 *  if ($macro_mode eq '1x40G_KR4') {$adv_bitmap=$adv_bitmap+(1<<4);}
 *  if ($macro_mode eq '2x20G_KR2') {$adv_bitmap=$adv_bitmap+(1<<8);}
 *  if ($macro_mode eq '4x10G_KR') {$adv_bitmap=$adv_bitmap+(1<<5);}
 *  if ($macro_mode eq '4x1G') {$adv_bitmap=$adv_bitmap+(1<<7);}
 *  if ($macro_mode eq '2x10G_KX2') {$adv_bitmap=$adv_bitmap+(1<<9);}
 *  if ($macro_mode eq '1x10G_KX4') {$adv_bitmap=$adv_bitmap+(1<<6);}
 *  $master->Reg_Write('address' => 0x088c0000 + ($port*0x1000) + 0x400 + 0x13c,
 *
 *  WARNING: This enum applicable for Lion2 only!
 *
 */
typedef enum
{
    _1000Base_KX_Bit0       = 1, /* _1000Base_KX */
    _10GBase_KX4_Bit1       = 2, /* _10GBase_KX4 */
    _10GBase_KR_Bit2        = 4, /* _10GBase_KR */
    _40GBase_KR4_Bit3       = 8, /* _40GBase_KR */
    _40GBase_CR4_Bit4       = 16, /* not supported */
    _100GBase_KR10_Bit5     = (1<<5), /* not supported */
    _100GBase_KP4_Bit6      = (1<<6), /* not supported */
    _100GBase_KR4_Bit7      = (1<<7), /* not supported */
    _100GBase_CR4_Bit8      = (1<<8), /* not supported */
    _25GBase_KR_S_Bit9      = (1<<9), /* _25GBase_KR_S */
    _25GBase_KR_Bit10       = (1<<10), /* _25GBase_KR */
    _25GBase_KR_C_Bit11     = (1<<11), /* _25GBase_KR_CONSORTIUM */
    _25GBase_CR_C_Bit12     = (1<<12), /* _25GBase_CR_CONSORTIUM */
    _50GBase_KR2_C_Bit13    = (1<<13), /* _50GBase_KR2_CONSORTIUM */
    _50GBase_CR2_C_Bit14    = (1<<14), /* _50GBase_CR2_CONSORTIUM */
    _25GBase_CR_S_Bit15      = (1<<15), /* _25GBase_CR_S */
    _25GBase_CR_Bit16       = (1<<16), /* _25GBase_CR */
    LAST_ADV_MODE

}MV_HWS_AP_CAPABILITY;


/* FEC advinced options (RS/FC) shifts for relevant KR interfaces and for
   consortium
*/
#define FEC_ADVANCE_BASE_R_SHIFT        (0)
#define FEC_ADVANCE_CONSORTIUM_SHIFT    (2) /* _25G/50G_CONSORTIUM */

/*
 * Reference Clock configuration
 *   refClockFreq   - Reference clock frequency
 *   refClockSource - Reference clock source
 *   cpllOutFreq    - cpll out frequency
 *   isValid        - valid serdes number
 */
typedef struct
{
    MV_HWS_REF_CLOCK_SUP_VAL refClockFreq;
    MV_HWS_REF_CLOCK_SOURCE  refClockSource;
    GT_U8                    cpllOutFreq;
    GT_BOOL                  isValid;
}MV_HWS_REF_CLOCK_CFG;


/*
 * AP configuration parameters:
 *   apLaneNum   - lane number inside a port (if not 0, pre AP port number isn't guarantee)
 *   modesVector - bitmap of supported port modes (IEEE capabilities):Lion 2
 *                     _1000Base_KX_Bit0 = 1,
 *                     _10GBase_KX4_Bit1 = 2,
 *                     _10GBase_KR_Bit2  = 4,
 *                     _40GBase_KR4_Bit3 = 8,
 *                     _40GBase_CR4_Bit4 = 16,
 *                     _100GBase_KR10_Bit5 = 32
 *                 in Sip 5.10 modes are
 *     [00:00] Advertisement 40GBase KR4
 *     [01:01] Advertisement 10GBase KR
 *     [02:02] Advertisement 10GBase KX4
 *     [03:03] Advertisement 1000Base KX
 *     [04:04] Advertisement 20GBase KR2
 *     [05:05] Advertisement 10GBase KX2
 *     [06:06] Advertisement 100GBase KR4
 *     [07:07] Advertisement 100GBase CR4
 *     [08:08] Advertisement 25GBASE-KR-S or 25GBASE-CR-S           0x0100
 *     [09:09] Advertisement 25GBASE-KR or 25GBASE-CR               0x0200
 *     [10:10] Advertisement consortium 25GBase KR1                 0x0400
 *     [11:11] Advertisement consortium 25GBase CR1                 0x0800
 *     [12:12] Advertisement consortium 50GBase KR2                 0x1000
 *     [13:13] Advertisement consortium 50GBase CR2                 0x2000

 *   fcPause     - FC pause (true/false)
 *   fcAsmDir    - FC ASM_DIR (Annex 28B)
 *   fecSup      - FEC ability (true/false)
 *   fecReq      - Request link partner to enable FEC (true/false)
 *   nonceDis    - indicates the port mode for the AP-Init (In loopback
 *                 Nonce detection is disabled)
 *   refClockCfg - Reference clock configuration
 *   fecAdvanceAbil - fec ability for advance speed 25,50.. (each speed has 2 bits)
 *   fecAdvanceReq - fec request for advance speed 25,50..  (each speed has 2 bits)
 *   ctleBiasCfg   - Ctle Bias value
*/
typedef struct
{
    GT_U32                apLaneNum;
    GT_U32                modesVector;
    GT_BOOL               fcPause;
    GT_BOOL               fcAsmDir;
    GT_BOOL               fecSup;
    GT_BOOL               fecReq;
    GT_BOOL               nonceDis;
    MV_HWS_REF_CLOCK_CFG  refClockCfg;
    GT_U8                 polarityVector;
    GT_U32                fecAdvanceAbil;
    GT_U32                fecAdvanceReq;
    GT_U32                ctleBiasValue;
}MV_HWS_AP_CFG;

/*
 * Highest common denominator (HCD) results parameters:
 *   hcdFound      - indicating AP resolution completed
 *   hcdLinkStatus - link status
 *   hcdResult     - the auto negotiation resolution
 *                   (bit location in IEEE capabilities vector)
 *   hcdPcsLockStatus - link/lock status form the relevant PCS/MAC
 *   hcdFecEn      - whether FEC negotiation resolution is enabled
 *   hcdFcRxPauseEn   - FC Rx pause resolution (true/false)
 *   hcdFcTxPauseEn   - FC Tx pause resolution (true/false)
 *   hcdFecType     - FEC FC/RS/NONE
*/
typedef struct
{
    GT_BOOL                 hcdFound;
    GT_U32                  hcdLinkStatus;
    GT_U32                  hcdResult;
    GT_U32                  hcdPcsLockStatus;
    GT_BOOL                 hcdFecEn;
    GT_BOOL                 hcdFcRxPauseEn;
    GT_BOOL                 hcdFcTxPauseEn;
    GT_U32                  hcdFecType;

}MV_HWS_HCD_INFO;

/*
 * AP results information:
 *    preApPortNum - requested AP port number (not garante, if apLaneNum != 0)
 *    apLaneNum    - relative lane number
 *    postApPortNum - AP resolved port number (lane swap result)
 *    postApPortMode - port mode - result of auto negotiation
 *    hcdResult     - (HCD) results parameters
 *    smState       - State machine state
 *    smStatus      - State machine status
 *    arbStatus       ARB State machine status 
*/
typedef struct
{
    GT_U32                  preApPortNum;
    GT_U32                  apLaneNum;
    GT_U32                  postApPortNum;
    MV_HWS_PORT_STANDARD    postApPortMode;
    MV_HWS_HCD_INFO         hcdResult;
    GT_U32                  smState;
    GT_U32                  smStatus;
    GT_U32                  arbStatus;

}MV_HWS_AP_PORT_STATUS;

/*
 * AP statistics information:
 *    abilityCnt        - Number of Ability detect intervals executed
 *    abilitySuccessCnt - Number of Ability detect successfull intervals executed
 *    linkFailCnt       - Number of Link check fail intervals executed
 *    linkSuccessCnt    - Number of Link check successfull intervals executed
 *    hcdResoultionTime - Time duration for HCD resolution
 *    linkUpTime        - Time duration for Link up
*/
typedef struct
{
    GT_U16 txDisCnt;
    GT_U16 abilityCnt;
    GT_U16 abilitySuccessCnt;
    GT_U16 linkFailCnt;
    GT_U16 linkSuccessCnt;
    GT_U32 hcdResoultionTime;
    GT_U32 linkUpTime;

}MV_HWS_AP_PORT_STATS;

/*
 * AP introp information: 
 *    attrBitMask            - Bit mask for attribute configuration:
 *                             0x1 - TX Disable,....,0x80 - PD_Max Interval
 *    txDisDuration          - Tx Disable duration in msec - default 60msec
 *    abilityDuration        - Ability detect duration in msec - default 5 msec
 *    abilityMaxInterval     - Ability detect max intervals - default 25 intervals ==> 5msec x 25 intervals = 125msec
 *    abilityFailMaxInterval - Ability detect max number of failed intervals where ST_AN_GOOD_CK was detected
 *                             But not resolution was found, trigger move to INIT state, instead of TX Disable in normal case
 *    apLinkDuration         - AP Link check duration in msec - default 10 msec
 *    apLinkMaxInterval      - AP Link check max intervals - default 50 intervals ==> 10msec x 50 intervals = 500msec
 *    pdLinkDuration         - PD Link check duration in msec - default 10 msec
 *    pdLinkMaxInterval      - PD Link check max intervals - default 25 intervals ==> 10msec x 25 intervals = 250msec
*/
typedef struct
{
    GT_U16 attrBitMask;
    GT_U16 txDisDuration;
    GT_U16 abilityDuration;
    GT_U16 abilityMaxInterval;
    GT_U16 abilityFailMaxInterval;
    GT_U16 apLinkDuration;
    GT_U16 apLinkMaxInterval;
    GT_U16 pdLinkDuration;
    GT_U16 pdLinkMaxInterval;

}MV_HWS_AP_PORT_INTROP;

/*
 * Service CPU log information:
 *    fwBaseAddr       - AP Firmware Base address
 *    fwLogBaseAddr    - AP Firmware Real-time log base address
 *    fwLogCountAddr   - AP Firmware Real-time log count address
 *    fwLogPointerAddr - AP Firmware Real-time log pointer address
 *    fwLogResetAddr   - AP Firmware Real-time log reset address
*/
typedef struct
{
    GT_U32 fwBaseAddr;
    GT_U32 fwLogBaseAddr;
    GT_U32 fwLogCountAddr;
    GT_U32 fwLogPointerAddr;
    GT_U32 fwLogResetAddr;

}MV_HWS_FW_LOG;

/**
* @internal mvHwsApEngineInit function
* @endinternal
*
* @brief   Initialize AP engine.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsApEngineInit
(
    GT_U8                    devNum,
    GT_U32                   portGroup
);

/**
* @internal mvHwsApEngineStop function
* @endinternal
*
* @brief   Disable whole AP engine.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsApEngineStop
(
    GT_U8                    devNum,
    GT_U32                   portGroup
);

/**
* @internal mvHwsApEngineInitGet function
* @endinternal
*
* @brief   Checks if AP engine is enabled whole AP engine.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] engineEnabled            - pointer to boolean that indicated whether the engine
*                                      is enabled
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsApEngineInitGet
(
    GT_U8                    devNum,
    GT_U32                   portGroup,
    GT_BOOL                  *engineEnabled
);

/**
* @internal mvHwsApPortStart function
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
GT_STATUS mvHwsApPortStart
(
    GT_U8                    devNum,
    GT_U32                   portGroup,
    GT_U32                   phyPortNum,
    MV_HWS_AP_CFG            *apCfg
);

/**
* @internal mvHwsApPortStop function
* @endinternal
*
* @brief   Disable the AP engine on port and release all its resources.
*         Clears the port mode and release all its resources according to selected.
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] action                   - Power down or reset
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsApPortStop
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  phyPortNum,
    MV_HWS_PORT_ACTION      action

);

/**
* @internal mvHwsApPortConfigGet function
* @endinternal
*
* @brief   Returns the AP port configuration.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsApPortConfigGet
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  phyPortNum,
    GT_BOOL                 *apPortEnabled,
    MV_HWS_AP_CFG           *apCfg

);

/**
* @internal mvHwsApPortSetActiveLanes function
* @endinternal
*
* @brief   Disable the AP engine on port and release all its resources.
*         Clears the port mode and release all its resources according to selected.
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - port mode
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsApPortSetActiveLanes
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  phyPortNum,
    MV_HWS_PORT_STANDARD    portMode

);

/**
* @internal mvHwsApPortResolutionMaskGet function
* @endinternal
*
* @brief   Returns the port's resolution bit mask
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
*
* @param[out] portBitmask              - port's resolution bit mask
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsApPortResolutionMaskGet
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  *portBitmask
);

/**
* @internal mvApLockGet function
* @endinternal
*
* @brief   Acquires lock so host and AP machine won't access the same
*         resource at the same time.
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvApLockGet
(
    GT_U8   devNum,
    GT_U32  portGroup,
    GT_U32  phyPortNum
);

/**
* @internal mvApLockRelease function
* @endinternal
*
* @brief   Releases the synchronization lock (between Host and AP machine.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvApLockRelease
(
    GT_U8   devNum,
    GT_U32  portGroup,
    GT_U32  phyPortNum
);

/**
* @internal mvHwsApPortStatusGet function
* @endinternal
*
* @brief   Returns the AP port resolution info.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
*
* @param[out] apResult                 - AP/HCD results
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsApPortStatusGet
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  phyPortNum,
    MV_HWS_AP_PORT_STATUS   *apResult

);

/**
* @internal mvHwsApEngineStatusGet function
* @endinternal
*
* @brief   Returns the AP status for all actives AP ports.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] apPortActNum             - size of result array
* @param[in] apResult                 - pointer to array of AP ports results
*
* @param[out] apResult                 - AP results
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsApEngineStatusGet
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  apPortActNum,
    MV_HWS_AP_PORT_STATUS   *apResult

);
/**
* @internal mvApCheckCounterGet function
* @endinternal
*
* @brief   Read checkCounter value display AP engine activity.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvApCheckCounterGet
(
  GT_U8   devNum,
  GT_U32  portGroup,
  GT_U32 *counter
);

/**
* @internal mvApPortStatusShow function
* @endinternal
*
* @brief   Print AP port status information stored in system.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] apPortNum                - AP port number
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvApPortStatusShow
(
  GT_U8   devNum,
  GT_U32  portGroup,
  GT_U32  apPortNum
);

#endif /* mvHwServicesPortApIf_H */



