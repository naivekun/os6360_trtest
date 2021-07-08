/*******************************************************************************
*           Copyright 2001, Marvell International Ltd.
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
* @file mvHwsPortCfgIf.c
*
* @brief This file contains API for port configuartion and tuning parameters
*
* @version   48
********************************************************************************
*/
#include <cpss/common/labServices/port/gop/common/siliconIf/mvSiliconIf.h>
#include <cpss/common/labServices/port/gop/port/mvHwsPortInitIf.h>
#include <cpss/common/labServices/port/gop/port/private/mvHwsPortPrvIf.h>
#include <cpss/common/labServices/port/gop/port/private/mvHwsPortMiscIf.h>
#include <cpss/common/labServices/port/gop/port/private/mvPortModeElements.h>
#include <cpss/common/labServices/port/gop/port/serdes/mvHwsSerdesPrvIf.h>
#include <cpss/common/labServices/port/gop/port/pcs/mvHwsPcsIf.h>
#include <cpss/common/labServices/port/gop/port/mac/mvHwsMacIf.h>
#include <cpss/common/labServices/port/gop/common/siliconIf/siliconAddress.h>
#ifndef MV_HWS_REDUCED_BUILD
#include <cpss/common/labServices/port/gop/port/mvHwsIpcApis.h>
#include <cpssCommon/private/prvCpssEmulatorMode.h>
#endif

#define NEW_PORT_INIT_SEQ
#define HWS_CTLE_BIT_MAP 32
/**************************** Globals ****************************************************/
static GT_U32 align90Delay = 60;

static GT_U32 serdesOptAlgoMask = 0x0;
extern GT_U32 optPrintLevel;

/* in Lion2/Hooper, this variable allows PCS WA or PCS Rx unreset if set to FALSE */
GT_BOOL performPcsWA = GT_TRUE;

#ifndef CM3
/* enables/disabled training results copy */
extern GT_BOOL copyTuneResults;
#endif

GT_U8 portModeNumArr[] =
    {1,     /* SGMII */
     2,     /* _1000Base_X */
     3,     /* SGMII2_5 */
     4,     /* QSGMII */
     5,     /* _10GBase_KX4 */
     7,     /* _10GBase_KR */
     8,     /* _20GBase_KR */
     9,     /* _40GBase_KR */
     14,    /* RXAUI */
     15,    /* _20GBase_KX4 */
     16,    /* _10GBase_SR_LR */
     17,    /* _20GBase_SR_LR */
     18,    /* _40GBase_SR_LR */
     19,    /* _12_5GBase_KR */
     20,    /* XLHGL_KR4 */
     44,    /* _12GBaseR */
     45,    /* _5_625GBaseR */
     46,    /* _48GBaseR */
     47,    /* _12GBase_SR */
     48,    /* _48GBase_SR */
     49,    /* _5GBaseR */
     50};   /*_22GBase_SR */

MV_HWS_MAN_TUNE_TX_CONFIG_DATA hwPortTxDefaults[] =
{
  /* PortMode          TxAmp   TxAmpAdj  TxEmph0  TxEmph1  TxAmpShft */
/* SGMII */            { 19,      1,        0,       0,        0     },
/* _1000Base_X */      { 19,      1,        0,       0,        0     },
/* SGMII2_5 */         { 20,      1,        0,       0,        0     },
/* QSGMII */           { 24,      1,        1,       9,        0     },
/* _10GBase_KX4 */     { 20,      1,        0,       0,        0     },
/* _10GBase_KR */      { 26,      1,        0,       0,        0     },
/* _20GBase_KR */      { 26,      1,        0,       0,        0     },
/* _40GBase_KR */      { 26,      1,        0,       0,        0     },
/* RXAUI */            { 30,      1,        0,       0,        0     },
/* _20GBase_KX4 */     { 30,      1,        0,       0,        0     },
/* _10GBase_SR_LR */   { 17,      1,        2,       8,        0     },
/* _20GBase_SR_LR */   { 17,      1,        2,       8,        0     },
/* _40GBase_SR_LR */   { 17,      1,        2,       8,        0     },
/* _12_5GBase_KR */    { 26,      1,        3,       3,        0     },
/* XLHGL_KR4 */        { 26,      1,        3,       3,        0     },
/* _12GBaseR */        { 26,      1,        3,       3,        0     },
/* _5_625GBaseR */     { 30,      1,        0,       0,        0     },
/* _48GBaseR */        { 26,      1,        3,       3,        0     },
/* _12GBase_SR */      { 26,      1,        3,       3,        0     },
/* _48GBase_SR */      { 26,      1,        3,       3,        0     },
/* _5GBaseR */         { 24,      1,        1,       9,        0     },
/* _22GBase_SR */      { 26,      1,        3,       3,        0     },
};

/**************************** Definition *************************************************/
/* #define GT_DEBUG_HWS */
#ifdef  GT_DEBUG_HWS
#include <common/os/gtOs.h>
#define DEBUG_HWS_FULL(level,s) if (optPrintLevel >= level) {hwsOsPrintf s;}
#else
#define DEBUG_HWS_FULL(level,s)
#endif

/**************************** Pre-Declaration ********************************************/
#ifndef MV_HWS_AVAGO_NO_VOS_WA
extern GT_STATUS mvHwsAvagoSerdesVosOverrideModeSet
(
    GT_U8   devNum,
    GT_BOOL vosOverride
);
extern GT_STATUS mvHwsAvagoSerdesVosOverrideModeGet
(
    GT_U8   devNum,
    GT_BOOL *vosOverridePtr
);
#endif /*#ifndef MV_HWS_AVAGO_NO_VOS_WA*/
extern GT_STATUS mvHwsComHRev2SerdesTrainingOptimization
(
   GT_U8   devNum,
   GT_U32  portGroup,
   GT_U32  *serdesList,
   GT_U32  numOfActLanes,
   MV_HWS_PORT_MAN_TUNE_MODE       tuningMode,
   GT_U32  algoMask
);

GT_STATUS hwsPortFixAlign90Ext
(
   GT_U8   devNum,
   GT_U32  portGroup,
   GT_U32  phyPortNum,
   MV_HWS_PORT_STANDARD    portMode,
   GT_U32  optAlgoMask
);

GT_STATUS mvHwsComHRev2SerdesConfigRegulator
(
   GT_U8   devNum,
   GT_U32  portGroup,
   GT_U32  *serdesList,
   GT_U32  numOfActLanes,
   GT_BOOL setDefault
);

GT_STATUS mvHwsPortPostTraining
(
   GT_U8   devNum,
   GT_U32  portGroup,
   GT_U32  phyPortNum,
   MV_HWS_PORT_STANDARD    portMode
);

#ifndef CM3
#ifndef BC2_DEV_SUPPORT
extern GT_STATUS mvHwsComHRev2SerdesPostTrainingConfig
(
    GT_U8   devNum,
    GT_U32  portGroup,
    GT_U32  numOfSerdes,
    GT_U32  *serdesArr
);
#endif

extern GT_STATUS mvHwsComHRev2SerdesOptAlgoParams
(
    GT_32     ffeFirstTh,
    GT_32     ffeFinalTh,
    GT_U32    f0dStopTh
);

extern GT_STATUS mvHwsComHRev2SerdesSetEyeCheckEn
(
    GT_BOOL     eyeCheckEnVal
);

extern GT_STATUS mvHwsComHRev2SerdesSetPresetCommand
(
    GT_32     presetCmdVal
);
#endif

/**
* @internal mvHwsPCSMarkModeSet function
* @endinternal
*
* @brief   Mark/Un-mark PCS unit
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - port standard metric
* @param[in] enable                   - GT_TRUE  for mark the PCS,
*                                      GT_FALSE for un-mark the PCS
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameter
*/
GT_STATUS mvHwsPCSMarkModeSet
(
   GT_U8   devNum,
   GT_UOPT portGroup,
   GT_U32  phyPortNum,
   MV_HWS_PORT_STANDARD    portMode,
   GT_BOOL enable
)
{
   MV_HWS_DEV_FUNC_PTRS *hwsDevFunc;

   hwsDeviceSpecGetFuncPtr(&hwsDevFunc);

   if (hwsDevFunc[HWS_DEV_SILICON_INDEX(devNum)].pcsMarkModeFunc == NULL)
   {
           return GT_OK;
   }

   return hwsDevFunc[HWS_DEV_SILICON_INDEX(devNum)].pcsMarkModeFunc(devNum, portGroup, phyPortNum, portMode, enable);
}

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
    GT_U8          devNum,
    GT_U32         portGroup,
    GT_U32         phyPortNum,
    MV_HWS_PORT_STANDARD   portMode,
    GT_BOOL        powerDownRx,
    GT_BOOL        powerDownTx
)
{
    MV_HWS_PORT_INIT_PARAMS curPortParams;
    GT_U32  curLanesList[HWS_MAX_SERDES_NUM];
    GT_U32 i;

    if ((phyPortNum >= HWS_CORE_PORTS_NUM(devNum)) || ((GT_U32)portMode >= HWS_DEV_PORT_MODES(devNum)))
    {
        return GT_BAD_PARAM;
    }

    if (GT_OK != hwsPortModeParamsGetToBuffer(devNum, portGroup, phyPortNum, portMode, &curPortParams))
    {
        return GT_NOT_SUPPORTED;
    }

    /* rebuild active lanes list according to current configuration (redundancy) */
    CHECK_STATUS(mvHwsRebuildActiveLaneList(devNum, portGroup, phyPortNum, portMode, curLanesList));

    /* on each related serdes */
    for (i = 0; i < curPortParams.numOfActLanes; i++)
    {
        /* Enable/Disable power down of Tx and Rx on Serdes */
        CHECK_STATUS(mvHwsSerdesPartialPowerDown(devNum, portGroup, (curLanesList[i] & 0xFFFF), HWS_DEV_SERDES_TYPE(devNum), powerDownRx, powerDownTx));
    }

    return GT_OK;
}

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
    GT_U8          devNum,
    GT_U32         portGroup,
    GT_U32         phyPortNum,
    MV_HWS_PORT_STANDARD   portMode,
    GT_BOOL        *powerRxStatus,
    GT_BOOL        *powerTxStatus
)
{
    GT_U32  curLanesList[HWS_MAX_SERDES_NUM];

    if ((phyPortNum >= HWS_CORE_PORTS_NUM(devNum)) || ((GT_U32)portMode >= HWS_DEV_PORT_MODES(devNum)))
    {
        return GT_BAD_PARAM;
    }

    /* rebuild active lanes list according to current configuration (redundancy) */
    CHECK_STATUS(mvHwsRebuildActiveLaneList(devNum, portGroup, phyPortNum, portMode, curLanesList));

    /* Get the status of Serdes Tx and Rx power */
    CHECK_STATUS(mvHwsSerdesPartialPowerStatusGet(devNum, portGroup, (curLanesList[0] & 0xFFFF), HWS_DEV_SERDES_TYPE(devNum),
                                                  powerRxStatus, powerTxStatus));

    return GT_OK;
}

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
)
{
   MV_HWS_PORT_INIT_PARAMS curPortParams;
   MV_HWS_AUTO_TUNE_STATUS rxStatus[HWS_MAX_SERDES_NUM];
   MV_HWS_AUTO_TUNE_STATUS txStatus[HWS_MAX_SERDES_NUM];
   GT_U32  curLanesList[HWS_MAX_SERDES_NUM];
   GT_BOOL autoTunePass[HWS_MAX_SERDES_NUM];
   GT_BOOL laneFail = GT_FALSE;
   GT_U32 i;
   MV_HWS_DEV_FUNC_PTRS *hwsDevFunc;

   /* avoid warnings */
   optAlgoMask = optAlgoMask;

   if ((phyPortNum >= HWS_CORE_PORTS_NUM(devNum)) ||
       ((GT_U32)portMode >= HWS_DEV_PORT_MODES(devNum)))
   {
           return GT_BAD_PARAM;
   }

    if ((portTuningMode == RxTrainingAdative) ||
        (portTuningMode == RxTrainingStopAdative))
    {
        return GT_NOT_SUPPORTED;
    }

   if (GT_OK != hwsPortModeParamsGetToBuffer(devNum, portGroup, phyPortNum, portMode, &curPortParams))
   {
       return GT_NOT_SUPPORTED;
   }

   hwsOsMemSetFuncPtr(autoTunePass, 0, sizeof(autoTunePass));
   hwsOsMemSetFuncPtr(rxStatus, 0, sizeof(rxStatus));
   hwsOsMemSetFuncPtr(txStatus, 0, sizeof(txStatus));

   hwsDeviceSpecGetFuncPtr(&hwsDevFunc);

   /* rebuild active lanes list according to current configuration (redundancy) */
   CHECK_STATUS(mvHwsRebuildActiveLaneList(devNum, portGroup, phyPortNum, portMode, curLanesList));

   /* TRxTuneCfg flow */
   if (portTuningMode == TRxTuneCfg)
   {
        /* on each related serdes */
        for (i = 0; (i < HWS_MAX_SERDES_NUM) && (i < curPortParams.numOfActLanes); i++)
        {
            CHECK_STATUS(mvHwsSerdesAutoTuneCfg(devNum, (portGroup + ((curLanesList[i] >> 16) & 0xFF)),
                                                (curLanesList[i] & 0xFFFF), HWS_DEV_SERDES_TYPE(devNum),
                                                GT_FALSE, GT_TRUE));
        }

#if !defined(BC2_DEV_SUPPORT) && !defined(AC3_DEV_SUPPORT) && !defined(CM3) && defined(CHX_FAMILY)
        if ((HooperA0 == hwsDeviceSpecInfo[devNum].devType) || (Lion2B0 == hwsDeviceSpecInfo[devNum].devType))
        {
            CHECK_STATUS(mvHwsComHRev2SerdesPostTrainingConfig(devNum, portGroup,
                                curPortParams.numOfActLanes, curLanesList));
        }
#endif /* #if !defined(BC2_DEV_SUPPORT) && !defined(AC3_DEV_SUPPORT) && !defined(CM3) && defined(CHX_FAMILY) */

        return GT_OK;
  }

   /* TRxTuneStart flow */
   if (portTuningMode == TRxTuneStart)
   {
           /* on each related serdes */
           for (i = 0; (i < HWS_MAX_SERDES_NUM) && (i < curPortParams.numOfActLanes); i++)
           {
                   if (autoTunePass[i] == GT_FALSE)
                   {
                           /* enable TX tunning */
                           CHECK_STATUS(mvHwsSerdesTxAutoTuneStart(devNum, (portGroup + ((curLanesList[i] >> 16) & 0xFF)),
                                                                   (curLanesList[i] & 0xFFFF), HWS_DEV_SERDES_TYPE(devNum), GT_TRUE));
                   }
           }

           /* mark PCS to be under TRx training */
           CHECK_STATUS(mvHwsPCSMarkModeSet(devNum, portGroup, phyPortNum, portMode, GT_TRUE));

           /* Reset MAC */
           CHECK_STATUS(mvHwsMacReset(devNum, portGroup, curPortParams.portMacNumber, portMode,
                                           curPortParams.portMacType, RESET));

           /* Reset PCS */
           CHECK_STATUS(mvHwsPcsReset(devNum, portGroup, phyPortNum, portMode,
                                           curPortParams.portPcsType, RESET));

           /* on each related Serdes apply Rf reset */
           for (i = 0; (i < HWS_MAX_SERDES_NUM) && (i < curPortParams.numOfActLanes); i++)
           {
                   CHECK_STATUS(mvHwsSerdesDigitalReset(devNum, (portGroup + ((curLanesList[i] >> 16) & 0xFF)),
                                                           (curLanesList[i] & 0xFFFF), HWS_DEV_SERDES_TYPE(devNum), RESET));
           }

           return GT_OK;
   }

   /* TRxTuneStatus flow */
   for (i = 0; (i < HWS_MAX_SERDES_NUM) && (i < curPortParams.numOfActLanes); i++)
   {
           if (autoTunePass[i] == GT_FALSE)
           {
                   mvHwsSerdesAutoTuneStatus(devNum, (portGroup + ((curLanesList[i] >> 16) & 0xFF)),
                                                   (curLanesList[i] & 0xFFFF), HWS_DEV_SERDES_TYPE(devNum), NULL, &txStatus[i]);
                   if (txStatus[i] == TUNE_PASS)
                   {
                           autoTunePass[i] = GT_TRUE;
                   }
                   else
                   {
                           laneFail = GT_TRUE;
                   }
           }
   }

   if (laneFail == GT_TRUE)
   {
           /* if training failed - unreset all units to allow farther configurations */
           CHECK_STATUS(mvHwsPortPostTraining(devNum, portGroup, phyPortNum, portMode));
           return GT_FAIL;
   }

   if (hwsDevFunc[HWS_DEV_SILICON_INDEX(devNum)].portAutoTuneSetWaFunc != NULL)
   {
           hwsDevFunc[HWS_DEV_SILICON_INDEX(devNum)].portAutoTuneSetWaFunc(devNum, portGroup, phyPortNum, portMode, &(curLanesList[0])) ;
   }

   /* UnReset RF on each related Serdes */
   for (i = 0; (i < HWS_MAX_SERDES_NUM) && (i < curPortParams.numOfActLanes); i++)
   {
           CHECK_STATUS(mvHwsSerdesDigitalReset(devNum, (portGroup + ((curLanesList[i] >> 16) & 0xFF)), (curLanesList[i] & 0xFFFF),
                                            HWS_DEV_SERDES_TYPE(devNum), UNRESET));
   }

   /* UnReset PCS because FixAlign90 need signal */
   CHECK_STATUS(mvHwsPcsReset(devNum, portGroup, phyPortNum, portMode,
                                           curPortParams.portPcsType, UNRESET));

   return GT_OK;
}

/**
* @internal hwsPortTxAutoTuneActivateSet function
* @endinternal
*
* @brief   Unreset MAC and PCS after TRx training
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
)
{
   MV_HWS_PORT_INIT_PARAMS curPortParams;

   if (GT_OK != hwsPortModeParamsGetToBuffer(devNum, portGroup, phyPortNum, portMode, &curPortParams))
   {
       return GT_NOT_SUPPORTED;
   }

   /* avoid warnings */
   portTuningMode = portTuningMode;

   /* UnReset MAC: at this point FixAlign90 already finished and we are after TRX-Training */
   CHECK_STATUS(mvHwsMacReset(devNum, portGroup, curPortParams.portMacNumber, portMode,
                                           curPortParams.portMacType, UNRESET));

   /* unmark PCS to be under TRx training */
   CHECK_STATUS(mvHwsPCSMarkModeSet(devNum, portGroup, phyPortNum, portMode, GT_FALSE));

   return GT_OK;
}

/**
* @internal mvHwsPortAutoTuneSet function
* @endinternal
*
* @brief   Sets the port Tx and Rx parameters according to different working
*         modes/topologies.
*         Can be run any time after create port.
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - port standard metric
* @param[in] portTuningMode           - port tuning mode
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPortAutoTuneSet
(
   GT_U8   devNum,
   GT_U32  portGroup,
   GT_U32  phyPortNum,
   MV_HWS_PORT_STANDARD    portMode,
   MV_HWS_PORT_AUTO_TUNE_MODE      portTuningMode,
   void *  results
)
{
   return mvHwsPortAutoTuneSetExt(devNum, portGroup, phyPortNum, portMode,
                                           portTuningMode, serdesOptAlgoMask, results);
}

/**
* @internal mvHwsPortAutoTuneSetExt function
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
GT_STATUS mvHwsPortAutoTuneSetExt
(
   GT_U8   devNum,
   GT_U32  portGroup,
   GT_U32  phyPortNum,
   MV_HWS_PORT_STANDARD        portMode,
   MV_HWS_PORT_AUTO_TUNE_MODE      portTuningMode,
   GT_U32  optAlgoMask,
   void *  results
)
{
    MV_HWS_DEV_FUNC_PTRS *hwsDevFunc;

    hwsDeviceSpecGetFuncPtr(&hwsDevFunc);

    if (hwsDevFunc[HWS_DEV_SILICON_INDEX(devNum)].portAutoTuneSetExtFunc == NULL)
    {
        return GT_OK;
    }

#ifndef MV_HWS_REDUCED_BUILD
    if (hwsDeviceSpecInfo[devNum].ipcConnType == HOST2SERVICE_CPU_IPC_CONNECTION)
        return (mvHwsPortAutoTuneSetExtIpc(devNum, portGroup, phyPortNum, portMode, portTuningMode, optAlgoMask, results));
#endif

    return hwsDevFunc[HWS_DEV_SILICON_INDEX(devNum)].portAutoTuneSetExtFunc(devNum, portGroup, phyPortNum,
                                                                           portMode, (MV_HWS_PORT_AUTO_TUNE_MODE)portTuningMode,
                                                                           optAlgoMask,  results);
}

/**
* @internal mvHwsPortManTuneSet function
* @endinternal
*
* @brief   Sets the port Tx and Rx parameters according to different working
*         modes/topologies.
*         Can be run any time after create port.
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - port standard metric
* @param[in] portTuningMode           - port tuning mode
* @param[in] tunParams                - pointer to tune parameters
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPortManTuneSet
(
   GT_U8   devNum,
   GT_U32  portGroup,
   GT_U32  phyPortNum,
   MV_HWS_PORT_STANDARD    portMode,
   MV_HWS_PORT_MAN_TUNE_MODE       portTuningMode,
   MV_HWS_AUTO_TUNE_RESULTS        *tunParams
)
{
   MV_HWS_PORT_INIT_PARAMS curPortParams;
   GT_U32 curLanesList[HWS_MAX_SERDES_NUM];
   GT_U32 i;

   portTuningMode = portTuningMode;

   if ((phyPortNum >= HWS_CORE_PORTS_NUM(devNum)) ||
           ((GT_U32)portMode >= HWS_DEV_PORT_MODES(devNum)))
   {
           return GT_BAD_PARAM;
   }

   if (GT_OK != hwsPortModeParamsGetToBuffer(devNum, portGroup, phyPortNum, portMode, &curPortParams))
   {
       return GT_NOT_SUPPORTED;
   }

   /* rebuild active lanes list according to current configuration (redundancy) */
   CHECK_STATUS(mvHwsRebuildActiveLaneList(devNum, portGroup, phyPortNum, portMode, curLanesList));

   /* on each related serdes */
   for (i = 0; i < curPortParams.numOfActLanes; i++)
   {
           CHECK_STATUS(mvHwsSerdesManualRxConfig(devNum, (portGroup + ((curLanesList[i] >> 16) & 0xFF)),
                                                   (curLanesList[i] & 0xFFFF), HWS_DEV_SERDES_TYPE(devNum),
                                                   portTuningMode, tunParams->sqleuch, tunParams->ffeR, tunParams->ffeC,
                                                   GT_TRUE, tunParams->align90));
   }

   for (i = 0; i < curPortParams.numOfActLanes; i++)
   {
           CHECK_STATUS(mvHwsSerdesManualTxConfig(devNum, (portGroup + ((curLanesList[i] >> 16) & 0xFF)),
                                                   (curLanesList[i] & 0xFFFF), HWS_DEV_SERDES_TYPE(devNum),
                                                   tunParams->txAmp, GT_TRUE, tunParams->txEmph0, tunParams->txEmph1,
                                                   tunParams->txAmpShft, GT_TRUE));
   }

   return GT_OK;
}

/**
* @internal mvHwsPortManualRxConfig function
* @endinternal
*
* @brief   configures SERDES Rx parameters for all SERDES lanes.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - port standard metric
* @param[in] configParams             pointer to array of the config params structures
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPortManualRxConfig
(
    GT_U8                           devNum,
    GT_U32                          portGroup,
    GT_U32                          phyPortNum,
    MV_HWS_PORT_STANDARD            portMode,
    MV_HWS_PORT_MAN_TUNE_MODE       portTuningMode,
    MV_HWS_MAN_TUNE_RX_CONFIG_DATA  *configParams
)
{
    MV_HWS_PORT_INIT_PARAMS curPortParams;
    GT_U32  curLanesList[HWS_MAX_SERDES_NUM];
    GT_U32 i;

    if ((phyPortNum >= HWS_CORE_PORTS_NUM(devNum)) || ((GT_U32)portMode >= HWS_DEV_PORT_MODES(devNum))
        || (HWS_DEV_SILICON_TYPE(devNum) == BobK))
    {
        return GT_BAD_PARAM;
    }

   if (GT_OK != hwsPortModeParamsGetToBuffer(devNum, portGroup, phyPortNum, portMode, &curPortParams))
   {
       return GT_NOT_SUPPORTED;
   }

    /* rebuild active lanes list according to current configuration (redundancy) */
    CHECK_STATUS(mvHwsRebuildActiveLaneList(devNum, portGroup, phyPortNum, portMode, curLanesList));

    /* on each related serdes */
    for (i = 0; i < curPortParams.numOfActLanes; i++)
    {
        CHECK_STATUS(mvHwsSerdesManualRxConfig(devNum, portGroup,(curLanesList[i] & 0xFFFF), HWS_DEV_SERDES_TYPE(devNum),
                                               portTuningMode, configParams->sqlch, configParams->ffeRes,
                                               configParams->ffeCap, configParams->dfeEn, configParams->alig));
    }

    return GT_OK;
}

/**
* @internal mvHwsAvagoSerdesWaitForEdgeDetect function
* @endinternal
*
* @brief   Detect TxAmp peak for positive or negative changing
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
*                                      phyPortNum  - physical port number
*                                      portMode    - port standard metric
* @param[in] timeout                  -  in miliseconds
* @param[in] threshEO                 - Eye monitor threshold
* @param[in] dwellTime                - define dwell value
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAvagoSerdesWaitForEdgeDetect
(
    GT_U8       devNum,
    GT_UOPT     portGroup,
    GT_UOPT     serdesNum,
    GT_32       timeout,
    GT_32       threshEO,
    GT_U32      dwellTime
);
GT_STATUS mvHwsPortWaitForEdgeDetect
(
    GT_U8       devNum,
    GT_U32      portGroup,
    GT_U32      phyPortNum,
    MV_HWS_PORT_STANDARD    portMode,
    GT_32       timeout,
    GT_32       threshEO,
    GT_U32      dwellTime
)
{
    MV_HWS_PORT_INIT_PARAMS curPortParams;
    GT_U32  curLanesList[HWS_MAX_SERDES_NUM];
    GT_U32 i;

    if ((phyPortNum >= HWS_CORE_PORTS_NUM(devNum)) || ((GT_U32)portMode >= HWS_DEV_PORT_MODES(devNum)))
    {
        return GT_BAD_PARAM;
    }

    if (GT_OK != hwsPortModeParamsGetToBuffer(devNum, portGroup, phyPortNum, portMode, &curPortParams))
    {
        return GT_BAD_PARAM;
    }

    /* rebuild active lanes list according to current configuration (redundancy) */
    CHECK_STATUS(mvHwsRebuildActiveLaneList(devNum, portGroup, phyPortNum, portMode, curLanesList));

    /* on each related serdes */
   for (i = 0; i < curPortParams.numOfActLanes; i++)
   {
       CHECK_STATUS(mvHwsAvagoSerdesWaitForEdgeDetect(devNum, portGroup, (curLanesList[i] & 0xFFFF), timeout, threshEO, dwellTime));
   }

    return GT_OK;
}

/**
* @internal mvHwsPortManualTxConfig function
* @endinternal
*
* @brief   configures SERDES tx parameters for all SERDES lanes.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - port standard metric
* @param[in] configParams             pointer to array of the config params structures
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPortManualTxConfig
(
    GT_U8                           devNum,
    GT_U32                          portGroup,
    GT_U32                          phyPortNum,
    MV_HWS_PORT_STANDARD            portMode,
    MV_HWS_PORT_MAN_TUNE_MODE       portTuningMode,
    MV_HWS_MAN_TUNE_TX_CONFIG_DATA  *configParams
)
{
    MV_HWS_PORT_INIT_PARAMS curPortParams;
    GT_U32  curLanesList[HWS_MAX_SERDES_NUM];
    GT_U32 i;

    portTuningMode = portTuningMode; /* avoid warnings */

    if ((phyPortNum >= HWS_CORE_PORTS_NUM(devNum)) || ((GT_U32)portMode >= HWS_DEV_PORT_MODES(devNum)))
    {
        return GT_BAD_PARAM;
    }

    if (GT_OK != hwsPortModeParamsGetToBuffer(devNum, portGroup, phyPortNum, portMode, &curPortParams))
    {
        return GT_NOT_SUPPORTED;
    }

    /* rebuild active lanes list according to current configuration (redundancy) */
    CHECK_STATUS(mvHwsRebuildActiveLaneList(devNum, portGroup, phyPortNum, portMode, curLanesList));

    /* on each related serdes */
   for (i = 0; i < curPortParams.numOfActLanes; i++)
   {
       CHECK_STATUS(mvHwsSerdesManualTxConfig(devNum, portGroup,(curLanesList[i] & 0xFFFF), HWS_DEV_SERDES_TYPE(devNum),
                                              configParams->txAmp, configParams->txAmpAdj, configParams->txEmph0,
                                              configParams->txEmph1, configParams->txAmpShft, GT_TRUE));
   }

    return GT_OK;
}

/**
* @internal mvHwsPortManualTxConfigGet function
* @endinternal
*
* @brief   Get SERDES tx parameters for first SERDES lane.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - port standard metric
*
* @param[out] configParams             pointer to array of the config params structures:
*                                      txAmp     Tx amplitude
*                                      txAmpAdj  TX adjust
*                                      txemph0   TX emphasis 0
*                                      txemph1   TX emphasis 1
*                                      txAmpShft TX amplitude shift
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPortManualTxConfigGet
(
    GT_U8                           devNum,
    GT_U32                          portGroup,
    GT_U32                          phyPortNum,
    MV_HWS_PORT_STANDARD            portMode,
    MV_HWS_PORT_MAN_TUNE_MODE       portTuningMode,
    MV_HWS_MAN_TUNE_TX_CONFIG_DATA  *configParams
)
{
    GT_U32  curLanesList[HWS_MAX_SERDES_NUM];

    portTuningMode = portTuningMode; /* avoid warnings */

    if ((phyPortNum >= HWS_CORE_PORTS_NUM(devNum)) || ((GT_U32)portMode >= HWS_DEV_PORT_MODES(devNum)))
    {
        return GT_BAD_PARAM;
    }

    /* rebuild active lanes list according to current configuration (redundancy) */
    CHECK_STATUS(mvHwsRebuildActiveLaneList(devNum, portGroup, phyPortNum, portMode, curLanesList));

    /* on first lane related serdes */
    CHECK_STATUS(mvHwsSerdesManualTxConfigGet(devNum, portGroup, (curLanesList[0] & 0xFFFF), HWS_DEV_SERDES_TYPE(devNum), configParams));

    return GT_OK;
}

/**
* @internal mvHwsPortTxDefaultsSet function
* @endinternal
*
* @brief   Set the default Tx parameters on port.
*         Relevant for Bobcat2 and Alleycat3 devices.
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - port standard metric
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPortTxDefaultsSet
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  phyPortNum,
    MV_HWS_PORT_STANDARD    portMode
)
{
    MV_HWS_PORT_MAN_TUNE_MODE       portTuningMode;
    MV_HWS_PORT_INIT_PARAMS         curPortParams;
    GT_U32 curLanesList[HWS_MAX_SERDES_NUM];
    GT_U32 i;

    portTuningMode = StaticLongReach;

    if (GT_OK != hwsPortModeParamsGetToBuffer(devNum, portGroup, phyPortNum, portMode, &curPortParams))
    {
        return GT_NOT_SUPPORTED;
    }

    /* relevant for Bobcat2 and Alleycat3 devices */
    if ((HWS_DEV_GOP_REV(devNum) != GOP_28NM_REV2) && (HWS_DEV_GOP_REV(devNum) != GOP_28NM_REV1))
    {
        return GT_NOT_SUPPORTED;
    }

    /* validation of PortNum and portMode */
    CHECK_STATUS(mvHwsPortValidate(devNum, portGroup, phyPortNum, portMode));

    /* rebuild active lanes list according to current configuration (redundancy) */
    CHECK_STATUS(mvHwsRebuildActiveLaneList(devNum, portGroup, phyPortNum, portMode, curLanesList));

    for (i=0; i < (sizeof(portModeNumArr)/sizeof(GT_U8)); i++)
    {
        if (portMode == portModeNumArr[i])
            break;
    }
    /* configure defaults Tx parameter values on port */
    CHECK_STATUS(mvHwsPortManualTxConfig(devNum, portGroup, phyPortNum, portMode, portTuningMode, &hwPortTxDefaults[i]));

    /* perform Serdes Digital Reset/Unreset */
    for (i = 0; i < curPortParams.numOfActLanes; i++)
    {
        CHECK_STATUS(mvHwsSerdesReset(devNum, portGroup, MV_HWS_SERDES_NUM(curLanesList[i]), HWS_DEV_SERDES_TYPE(devNum),
                                      GT_FALSE, GT_TRUE, GT_FALSE));
        /* delay 1ms */
        hwsOsExactDelayPtr(devNum, portGroup, 1);

        CHECK_STATUS(mvHwsSerdesReset(devNum, portGroup, MV_HWS_SERDES_NUM(curLanesList[i]), HWS_DEV_SERDES_TYPE(devNum),
                                      GT_FALSE, GT_FALSE, GT_FALSE));
    }

    return GT_OK;
}

/**
* @internal mvHwsPortTestGenerator function
* @endinternal
*
* @brief   Activate the port related PCS Tx generator and Rx checker control.
*         Can be run any time after create port.
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - port standard metric
* @param[in] portPattern              - port tuning mode
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPortTestGenerator
(
   GT_U8   devNum,
   GT_U32  portGroup,
   GT_U32  phyPortNum,
   MV_HWS_PORT_STANDARD    portMode,
   MV_HWS_PORT_TEST_GEN_PATTERN    portPattern,
   MV_HWS_PORT_TEST_GEN_ACTION     actionMode
)
{
   MV_HWS_PORT_INIT_PARAMS curPortParams;

   actionMode = actionMode;

   if ((phyPortNum >= HWS_CORE_PORTS_NUM(devNum)) ||
           ((GT_U32)portMode >= HWS_DEV_PORT_MODES(devNum)))
   {
           return GT_BAD_PARAM;
   }

   if (GT_OK != hwsPortModeParamsGetToBuffer(devNum, portGroup, phyPortNum, portMode, &curPortParams))
   {
       return GT_NOT_SUPPORTED;
   }

   /* perform MAC RESET */
   CHECK_STATUS(mvHwsMacReset(devNum, portGroup, curPortParams.portMacNumber, portMode, curPortParams.portMacType, RESET));

   /* perform PCS RESET */
   CHECK_STATUS(mvHwsPcsReset(devNum, portGroup, phyPortNum, portMode, curPortParams.portPcsType, RESET));

   CHECK_STATUS(mvHwsPcsTestGenCfg(devNum, portGroup, curPortParams.portPcsNumber, curPortParams.portPcsType, portPattern));

   /* perform PCS UNRESET */
   CHECK_STATUS(mvHwsPcsReset(devNum, portGroup, phyPortNum, portMode, curPortParams.portPcsType, UNRESET));

   /* perform MAC UNRESET */
   CHECK_STATUS(mvHwsMacReset(devNum, portGroup, curPortParams.portMacNumber, portMode, curPortParams.portMacType, UNRESET));

   return GT_OK;
}

/**
* @internal mvHwsPortTestGeneratorStatus function
* @endinternal
*
* @brief   Get test errors - every get clears the errors.
*         Can be run any time after delete port or after power up
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - port standard metric
*                                      portTuningMode - port tuning mode
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPortTestGeneratorStatus
(
   GT_U8   devNum,
   GT_U32  portGroup,
   GT_U32  phyPortNum,
   MV_HWS_PORT_STANDARD    portMode,
   MV_HWS_PORT_TEST_GEN_PATTERN    portPattern,
   MV_HWS_TEST_GEN_STATUS  *status
)
{
   MV_HWS_PORT_INIT_PARAMS curPortParams;

   if ((phyPortNum >= HWS_CORE_PORTS_NUM(devNum)) || ((GT_U32)portMode >= HWS_DEV_PORT_MODES(devNum)))
   {
           return GT_BAD_PARAM;
   }

   if (GT_OK != hwsPortModeParamsGetToBuffer(devNum, portGroup, phyPortNum, portMode, &curPortParams))
   {
       return GT_NOT_SUPPORTED;
   }

   CHECK_STATUS(mvHwsPcsTestGenStatus(devNum, portGroup, curPortParams.portPcsNumber,
                                                   curPortParams.portPcsType, portPattern, status));

   return GT_OK;
}

/*******************************************************************************
* mvHwsPortPPMSet
*
* DESCRIPTION:
*       Increase/decrease  Tx clock on port (added/sub ppm).
*       Can be run only after create port not under traffic.
*
* INPUTS:
*       devNum    - system device number
*       portGroup - port group (core) number
*       phyPortNum - physical port number
*       portMode   - port standard metric
*       portPPM    - limited to +/- 3 taps
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       0  - on success
*       1  - on error
*
*******************************************************************************/
#if ( !defined(MV_HWS_FREE_RTOS) || (defined (BC2_DEV_SUPPORT) || defined(AC3_DEV_SUPPORT)))
GT_STATUS mvHwsPortPPMSet
(
   GT_U8   devNum,
   GT_U32  portGroup,
   GT_U32  phyPortNum,
   MV_HWS_PORT_STANDARD    portMode,
   MV_HWS_PPM_VALUE        portPPM
)
{
   MV_HWS_PORT_INIT_PARAMS curPortParams;
   GT_U32 curLanesList[HWS_MAX_SERDES_NUM];
   GT_U32 i, tmpPortNum;

   if ((phyPortNum >= HWS_CORE_PORTS_NUM(devNum)) ||
           ((GT_U32)portMode >= HWS_DEV_PORT_MODES(devNum)))
   {
           return GT_BAD_PARAM;
   }

#ifndef MV_HWS_REDUCED_BUILD
   if (hwsDeviceSpecInfo[devNum].ipcConnType == HOST2SERVICE_CPU_IPC_CONNECTION)
           return (mvHwsPortPPMSetIpc(devNum,portGroup, phyPortNum,portMode,portPPM));
#endif

   if(QSGMII == portMode)
   {
       tmpPortNum = phyPortNum & 0xFFFFFFFC; /* only the first port in the quad is available*/
   }
   else
   {
       tmpPortNum = phyPortNum;
   }

   if (GT_OK != hwsPortModeParamsGetToBuffer(devNum, portGroup, phyPortNum, portMode, &curPortParams))
   {
       return GT_NOT_SUPPORTED;
   }

   /* MAC Reset */
   CHECK_STATUS(mvHwsMacReset(devNum, portGroup, curPortParams.portMacNumber, portMode, curPortParams.portMacType, RESET));

   /* PCS Reset */
   CHECK_STATUS(mvHwsPcsReset(devNum, portGroup, tmpPortNum, portMode, curPortParams.portPcsType, RESET));

   /* rebuild active lanes list according to current configuration (redundancy) */
   CHECK_STATUS(mvHwsRebuildActiveLaneList(devNum, portGroup, tmpPortNum, portMode, curLanesList));

   /* on each related serdes */
   for (i = 0; i < curPortParams.numOfActLanes; i++)
   {
           CHECK_STATUS(mvHwsSerdesPpmConfig(devNum, (portGroup + ((curLanesList[i] >> 16) & 0xFF)),
                                            (curLanesList[i] & 0xFFFF), HWS_DEV_SERDES_TYPE(devNum), portPPM));
   }

   /* PCS UnReset */
   CHECK_STATUS(mvHwsPcsReset(devNum, portGroup, tmpPortNum, portMode, curPortParams.portPcsType, UNRESET));

   /* MAC UnReset */
   CHECK_STATUS(mvHwsMacReset(devNum, portGroup, curPortParams.portMacNumber, portMode, curPortParams.portMacType, UNRESET));

   return GT_OK;
}

/**
* @internal mvHwsPortPPMGet function
* @endinternal
*
* @brief   Check the entire line configuration, return ppm value in case of match in all
*         or error in case of different configuration.
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
GT_STATUS mvHwsPortPPMGet
(
   GT_U8   devNum,
   GT_U32  portGroup,
   GT_U32  phyPortNum,
   MV_HWS_PORT_STANDARD    portMode,
   MV_HWS_PPM_VALUE        *portPPM
)
{
   MV_HWS_PORT_INIT_PARAMS curPortParams;
   MV_HWS_PPM_VALUE lanePpm, curPpm;
   GT_U32 curLanesList[HWS_MAX_SERDES_NUM];
   GT_U32 i, tmpPortNum;

   if ((portPPM == NULL) ||
       (phyPortNum >= HWS_CORE_PORTS_NUM(devNum)) ||
       ((GT_U32)portMode >= HWS_DEV_PORT_MODES(devNum)))
   {
           return GT_BAD_PARAM;
   }

#ifndef MV_HWS_REDUCED_BUILD
   if (hwsDeviceSpecInfo[devNum].ipcConnType == HOST2SERVICE_CPU_IPC_CONNECTION)
           return (mvHwsPortPPMGetIpc(devNum,portGroup, phyPortNum,portMode,portPPM));
#endif

    if(QSGMII == portMode)
    {
        tmpPortNum = phyPortNum & 0xFFFFFFFC; /* only the first port in the quad is available*/
    }
    else
    {
        tmpPortNum = phyPortNum;
    }

   if (GT_OK != hwsPortModeParamsGetToBuffer(devNum, portGroup, phyPortNum, portMode, &curPortParams))
   {
       return GT_NOT_SUPPORTED;
   }

   if (curPortParams.numOfActLanes == 0)
   {
           *portPPM = (MV_HWS_PPM_VALUE)0;
           return GT_FAIL;
   }

   /* rebuild active lanes list according to current configuration (redundancy) */
   CHECK_STATUS(mvHwsRebuildActiveLaneList(devNum, portGroup, tmpPortNum, portMode, curLanesList));

   /* read first lane PPM */
   CHECK_STATUS(mvHwsSerdesPpmConfigGet(devNum, (portGroup + ((curLanesList[0] >> 16) & 0xFF)),
                                                   (curLanesList[0] & 0xFFFF), HWS_DEV_SERDES_TYPE(devNum),&lanePpm));
   *portPPM = lanePpm;

   /* on each related serdes */
   for (i = 1; i < curPortParams.numOfActLanes; i++)
   {
           CHECK_STATUS(mvHwsSerdesPpmConfigGet(devNum, (portGroup + ((curLanesList[i] >> 16) & 0xFF)),
                                                           (curLanesList[i] & 0xFFFF), HWS_DEV_SERDES_TYPE(devNum), &curPpm));
           if (lanePpm != curPpm)
           {
                   return GT_FAIL;
           }
   }

   return GT_OK;
}
#endif /* ( !defined(MV_HWS_FREE_RTOS) || (defined (BC2_DEV_SUPPORT) || defined(AC3_DEV_SUPPORT))) */

/**
* @internal hwsPortFaultSendSet function
* @endinternal
*
* @brief   Configure the PCS to start or stop sending fault signals to partner.
*         on single lane, the result will be local-fault on the sender and remote-fault on the receiver,
*         on multi-lane there will be local-fault on both sides, and there won't be align lock
*         at either side.
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - port standard metric
* @param[in] send                     - send or stop sending
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS hwsPortFaultSendSet
(
    GT_U8   devNum,
    GT_U32  portGroup,
    GT_U32  phyPortNum,
    MV_HWS_PORT_STANDARD    portMode,
    GT_BOOL                 send
)
{
    GT_U32 pcsNum;
    MV_HWS_PORT_PCS_TYPE    pcsType;
    MV_HWS_PORT_INIT_PARAMS curPortParams;

    if ((phyPortNum >= HWS_CORE_PORTS_NUM(devNum)) || ((GT_U32)portMode >= HWS_DEV_PORT_MODES(devNum)))
    {
        CHECK_STATUS_EXT(GT_BAD_PARAM,LOG_ARG_GEN_PARAM_MAC(portMode),LOG_ARG_MAC_IDX_MAC(phyPortNum));
    }

#if 0 /* TBD */
#ifndef MV_HWS_REDUCED_BUILD
   if (hwsDeviceSpecInfo[devNum].ipcConnType == HOST2SERVICE_CPU_IPC_CONNECTION)
           return (hwsPortFaultSendSetIpc(devNum,portGroup, phyPortNum,portMode));
#endif
#endif

    if (GT_OK != hwsPortModeParamsGetToBuffer(devNum, portGroup, phyPortNum, portMode, &curPortParams))
    {
        return GT_NOT_SUPPORTED;
    }

    pcsType = curPortParams.portPcsType;
    pcsNum = curPortParams.portPcsNumber;

    if((pcsType == XPCS) && (HWS_DEV_SILICON_TYPE(devNum) == Pipe))
    {   /* Workaround for XGMII/XAUI/RXAUI - remote fault is not implemented for XPCS */
        /* Apllicable only for Pipe */
        CHECK_STATUS_EXT(  genUnitRegisterSet(devNum, portGroup, XLGMAC_UNIT, phyPortNum, MSM_PORT_MAC_CONTROL_REGISTER2, (send == GT_TRUE) ? (1<<12) : 0, (1<<12)),
                           LOG_ARG_MAC_IDX_MAC(phyPortNum), LOG_ARG_GEN_PARAM_MAC(portMode));
        return GT_OK;
    }

    if(HWS_TWO_LANES_MODE_CHECK(portMode))
    {
        pcsType = MMPCS;
    }

    CHECK_STATUS_EXT(mvHwsPcsSendFaultSet(devNum, portGroup, pcsNum, pcsType, portMode, send), LOG_ARG_MAC_IDX_MAC(phyPortNum), LOG_ARG_GEN_PARAM_MAC(portMode));

    return GT_OK;
}

/**
* @internal mvHwsPortLoopbackSet function
* @endinternal
*
* @brief   Activates the port loopback modes.
*         Can be run only after create port not under traffic.
* @param[in] devNum     - system device number
* @param[in] portGroup  - port group (core) number
* @param[in] phyPortNum - physical port number
* @param[in] portMode   - port standard metric
* @param[in] lpPlace    - unit for loopback configuration
* @param[in] lpType     - loopback type
*
* @retval 0 - on success
* @retval 1 - on error
*
* @note  For Serdes Loopback modes:
*        - In 'AN_TX_RX' mode, the Rx & Tx Serdes Polarity
*          configurations should be disabled, for getting LinkUp
*          and Traffic on port.
*        - After return to 'Normal' mode, the Rx & Tx Serdes
*          Polarity configuration should be restored, for
*          re-synch again the PCS and MAC units and getting
*          LinkUp and Traffic on port.
*        - Switching between 'AN_TX_RX'/'DIG_RX_TX' modes
*          requires returning to the 'Normal' mode before.
*
*/
GT_STATUS mvHwsPortLoopbackSet
(
   GT_U8   devNum,
   GT_U32  portGroup,
   GT_U32  phyPortNum,
   MV_HWS_PORT_STANDARD    portMode,
   MV_HWS_UNIT     lpPlace,
   MV_HWS_PORT_LB_TYPE     lbType
)
{
    MV_HWS_PORT_INIT_PARAMS curPortParams;
    MV_HWS_SERDES_LB        serdesLbType;
    GT_U32                  curLanesList[HWS_MAX_SERDES_NUM];
    GT_U32                  i;
    MV_HWS_PORT_MAC_TYPE    macType;
    MV_HWS_PORT_PCS_TYPE    pcsType;
    GT_U32                  macNum, pcsNum;

    if ((phyPortNum >= HWS_CORE_PORTS_NUM(devNum)) || ((GT_U32)portMode >= HWS_DEV_PORT_MODES(devNum)))
    {
        return GT_BAD_PARAM;
    }

#ifndef MV_HWS_REDUCED_BUILD
   if (hwsDeviceSpecInfo[devNum].ipcConnType == HOST2SERVICE_CPU_IPC_CONNECTION)
           return (mvHwsPortLoopbackSetIpc(devNum,portGroup, phyPortNum,portMode,lpPlace,lbType));
#endif

    if (GT_OK != hwsPortModeParamsGetToBuffer(devNum, portGroup, phyPortNum, portMode, &curPortParams))
    {
        return GT_NOT_SUPPORTED;
    }

    /* rebuild active lanes list according to current configuration (redundancy) */
    CHECK_STATUS(mvHwsRebuildActiveLaneList(devNum, portGroup, phyPortNum, portMode, curLanesList));

    macType = curPortParams.portMacType;
    macNum  = curPortParams.portMacNumber;
    pcsType = curPortParams.portPcsType;
    pcsNum  = curPortParams.portPcsNumber;

    switch (lpPlace)
    {
        case HWS_MAC:
            if ((macType == XGMAC) || (macType == XLGMAC) || (macType == CGMAC))
            {
                /* perform MAC RESET */
                CHECK_STATUS(mvHwsMacReset(devNum, portGroup, macNum, portMode, macType, RESET));

                /* perform PCS RESET */
                CHECK_STATUS(mvHwsPcsReset(devNum, portGroup, phyPortNum, portMode, pcsType, RESET));
            }

            CHECK_STATUS(mvHwsMacLoopbackCfg(devNum, portGroup, macNum, portMode, macType, lbType));

            if ((macType == XGMAC) || (macType == XLGMAC) || (macType == CGMAC))
            {
                /* perform PCS UNRESET */
                CHECK_STATUS(mvHwsPcsReset(devNum, portGroup, phyPortNum, portMode, pcsType, UNRESET));

                /* perform MAC UNRESET */
                CHECK_STATUS(mvHwsMacReset(devNum, portGroup, macNum, portMode, macType, UNRESET));
            }
            break;
        case HWS_PCS:
            CHECK_STATUS(mvHwsPcsLoopbackCfg(devNum, portGroup, phyPortNum, portMode, pcsType, lbType));
            break;
        case HWS_PMA:
            /* convert port LB type to Serdes LB type */
            switch (lbType)
            {
                case DISABLE_LB:
                    serdesLbType = SERDES_LP_DISABLE;
                    /* signal detect mask set 0 per pcs lane */
                    mvHwsPcsSignalDetectMaskSet(devNum, portGroup, pcsNum, pcsType, GT_FALSE);
                    break;
                case RX_2_TX_LB:
                    serdesLbType = SERDES_LP_DIG_RX_TX;
                    break;
                case TX_2_RX_LB:
                    serdesLbType = SERDES_LP_AN_TX_RX;
                    /* signal detect mask set 1 per pcs lane */
                    mvHwsPcsSignalDetectMaskSet(devNum, portGroup, pcsNum, pcsType, GT_TRUE);
                    break;
                case TX_2_RX_DIGITAL_LB:
                    serdesLbType = SERDES_LP_DIG_TX_RX;
                    /* signal detect mask set 1 per pcs lane */
                    mvHwsPcsSignalDetectMaskSet(devNum, portGroup, pcsNum, pcsType, GT_TRUE);
                    break;
                default:
                    return GT_BAD_PARAM;
            }

            /* for port modes with Multi-lanes, need to perform Reset/Unreset to PCS units*/
            CHECK_STATUS(mvHwsPcsReset(devNum, portGroup, phyPortNum, portMode, pcsType, RESET));

            /* on each related serdes */
            for (i = 0; i < curPortParams.numOfActLanes; i++)
            {
                CHECK_STATUS(mvHwsSerdesLoopback(devNum, portGroup, MV_HWS_SERDES_NUM(curLanesList[i]), HWS_DEV_SERDES_TYPE(devNum),
                                                 serdesLbType));

                /* disable Polarity on Rx & Tx */
                if (serdesLbType == SERDES_LP_AN_TX_RX)
                {
                    CHECK_STATUS(mvHwsSerdesPolarityConfig(devNum, portGroup, MV_HWS_SERDES_NUM(curLanesList[i]),
                                                           HWS_DEV_SERDES_TYPE(devNum), GT_FALSE, GT_FALSE));
                }
            }

            /* perform PCS UNRESET */
            CHECK_STATUS(mvHwsPcsReset(devNum, portGroup, phyPortNum, portMode, pcsType, UNRESET));
            break;
        default:
            return GT_NOT_SUPPORTED;
    }

    return GT_OK;
}


#if (!defined MV_HWS_REDUCED_BUILD)
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
)
{
    GT_U32 regVal;

    if ((portMode == SGMII) || (portMode == SGMII2_5))
    {
        regVal = 2 * coreClk;
    }
    else if (portMode == _1000Base_X)
    {
        regVal = (GT_U32)(1.5 * coreClk);
    }
    else
    {
         regVal = 0xff; /*default value*/
    }

    /* write timer to register low and high */
    CHECK_STATUS(genUnitRegisterSet(devNum, portGroup, GEMAC_UNIT, phyPortNum, GIG_PORT_pulse_1ms_Low_Reg, regVal&0xff, 0xff));
    CHECK_STATUS(genUnitRegisterSet(devNum, portGroup, GEMAC_UNIT, phyPortNum, GIG_PORT_pulse_1ms_High_Reg, regVal>>8, 0xff));

    return GT_OK;
}

/**
* @internal mvHwsGeneralClkSelCfg function
* @endinternal
*
* @brief   Configures the DP/Core Clock Selector on port according to core clock,
*         port mode and port number.
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - port standard metric
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsGeneralClkSelCfg
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  phyPortNum,
    MV_HWS_PORT_STANDARD    portMode
)
{
#ifndef MV_HWS_FREE_RTOS
    MV_HWS_PORT_INIT_PARAMS   curPortParams;
    GT_U32 coreClk = 0, coreClkHw;
    MV_HWS_DEV_FUNC_PTRS *hwsDevFunc;

    hwsDeviceSpecGetFuncPtr(&hwsDevFunc);
    CHECK_STATUS(hwsDevFunc[HWS_DEV_SILICON_INDEX(devNum)].coreClockGetFunc(devNum,&coreClk,&coreClkHw));

    CHECK_STATUS(hwsPortModeParamsGetToBuffer(devNum, portGroup, phyPortNum, portMode, &curPortParams));

    if ((portMode == SGMII2_5) || (portMode == SGMII) || (portMode == _1000Base_X))
    {
        CHECK_STATUS(hwsPulse1msRegCfg(devNum, portGroup, curPortParams.portMacNumber, portMode, coreClk));
    }
#endif
    return GT_OK;
}
#endif

#ifndef CO_CPU_RUN
/**
* @internal mvHwsPortLoopbackStatusGet function
* @endinternal
*
* @brief   Retrive MAC loopback status.
*         Can be run only after create port not under traffic.
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
GT_STATUS mvHwsPortLoopbackStatusGet
(
   GT_U8   devNum,
   GT_U32  portGroup,
   GT_U32  phyPortNum,
   MV_HWS_PORT_STANDARD    portMode,
   MV_HWS_UNIT     lpPlace,
   MV_HWS_PORT_LB_TYPE     *lbType
)
{
   MV_HWS_PORT_INIT_PARAMS  curPortParams;
   MV_HWS_SERDES_LB         serdesLbType;
   GT_U32 curLanesList[HWS_MAX_SERDES_NUM];

   if ((phyPortNum >= HWS_CORE_PORTS_NUM(devNum)) ||
           ((GT_U32)portMode >= HWS_DEV_PORT_MODES(devNum)))
   {
           return GT_BAD_PARAM;
   }

#ifndef MV_HWS_REDUCED_BUILD
   if (hwsDeviceSpecInfo[devNum].ipcConnType == HOST2SERVICE_CPU_IPC_CONNECTION)
           return (mvHwsPortLoopbackStatusGetIpc(devNum,portGroup, phyPortNum,portMode,lpPlace,lbType));
#endif

    if (GT_OK != hwsPortModeParamsGetToBuffer(devNum, portGroup, phyPortNum, portMode, &curPortParams))
    {
        return GT_NOT_SUPPORTED;
    }

   switch (lpPlace)
   {
   case HWS_MAC:
           CHECK_STATUS(mvHwsMacLoopbackStatusGet(devNum, portGroup, curPortParams.portMacNumber, portMode,
                                                  curPortParams.portMacType, lbType));
           break;
   case HWS_PCS:
           CHECK_STATUS(mvHwsPcsLoopbackCfgGet(devNum, portGroup, phyPortNum,
                                                   portMode, lbType));
           break;
   case HWS_PMA:
           /* rebuild active lanes list according to current configuration (redundancy) */
           CHECK_STATUS(mvHwsRebuildActiveLaneList(devNum, portGroup, phyPortNum, portMode, curLanesList));
           /* read loopback status on first lane */
           CHECK_STATUS(mvHwsSerdesLoopbackGet(devNum, (portGroup + ((curLanesList[0] >> 16) & 0xFF)),
                                                   (curLanesList[0] & 0xFFFF), HWS_DEV_SERDES_TYPE(devNum), &serdesLbType));
           /* convert serdes LB type to port LB type */
           switch (serdesLbType)
           {
           case SERDES_LP_DISABLE:
                   *lbType = DISABLE_LB;
                   break;
           case SERDES_LP_DIG_RX_TX:
                   *lbType = RX_2_TX_LB;
                   break;
           case SERDES_LP_AN_TX_RX:
                   *lbType = TX_2_RX_LB;
                   break;
           case SERDES_LP_DIG_TX_RX:
                   *lbType = TX_2_RX_DIGITAL_LB;
                   break;
           default:
                   return GT_FAIL;
           }
           break;
   default:
           return GT_NOT_SUPPORTED;
   }

   return GT_OK;
}
#endif

/*******************************************************************************
* mvHwsPortLinkStatusGet
*
* DESCRIPTION:
*       Returns the port link status.
*       Can be run any time.
*
* INPUTS:
*       devNum    - system device number
*       portGroup - port group (core) number
*       phyPortNum - physical port number
*       portMode   - port standard metric
*
* OUTPUTS:
*       Port link UP status (true/false).
*
* RETURNS:
*       0  - on success
*       1  - on error
*
*******************************************************************************/
#ifndef BV_DEV_SUPPORT
GT_STATUS mvHwsPortLinkStatusGet
(
   GT_U8   devNum,
   GT_U32  portGroup,
   GT_U32  phyPortNum,
   MV_HWS_PORT_STANDARD    portMode,
   GT_BOOL *linkStatus
)
{

   MV_HWS_PORT_INIT_PARAMS curPortParams;

#ifndef MV_HWS_REDUCED_BUILD
   if (hwsDeviceSpecInfo[devNum].ipcConnType == HOST2SERVICE_CPU_IPC_CONNECTION)
           return (mvHwsPortLinkStatusGetIpc(devNum,portGroup, phyPortNum,portMode,linkStatus));
#endif
   if ((phyPortNum >= HWS_CORE_PORTS_NUM(devNum)) ||
           ((GT_U32)portMode >= HWS_DEV_PORT_MODES(devNum)))
   {
           return GT_BAD_PARAM;
   }

    if (GT_OK != hwsPortModeParamsGetToBuffer(devNum, portGroup, phyPortNum, portMode, &curPortParams))
    {
        return GT_NOT_SUPPORTED;
    }

   CHECK_STATUS(mvHwsMacLinkStatusGet(devNum,portGroup,curPortParams.portMacNumber, portMode,
                                      curPortParams.portMacType,linkStatus));

   return GT_OK;
}
#endif
/**
* @internal mvHwsPortPolaritySet function
* @endinternal
*
* @brief   Defines the port polarity of the Serdes lanes (Tx/Rx).
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
GT_STATUS mvHwsPortPolaritySet
(
   GT_U8   devNum,
   GT_U32  portGroup,
   GT_U32  phyPortNum,
   MV_HWS_PORT_STANDARD    portMode,
   GT_32   txInvMask,
   GT_32   rxInvMask
)
{
   MV_HWS_PORT_INIT_PARAMS curPortParams;
   GT_BOOL txInv, rxInv;
   GT_U32 curLanesList[HWS_MAX_SERDES_NUM];
   GT_U32 i;

   if ((phyPortNum >= HWS_CORE_PORTS_NUM(devNum)) ||
           ((GT_U32)portMode >= HWS_DEV_PORT_MODES(devNum)))
   {
           return GT_BAD_PARAM;
   }

#ifndef MV_HWS_REDUCED_BUILD
   if (hwsDeviceSpecInfo[devNum].ipcConnType == HOST2SERVICE_CPU_IPC_CONNECTION)
           return (mvHwsPortPolaritySetIpc(devNum,portGroup, phyPortNum,portMode,txInvMask,rxInvMask));
#endif
   if (GT_OK != hwsPortModeParamsGetToBuffer(devNum, portGroup, phyPortNum, portMode, &curPortParams))
   {
       return GT_NOT_SUPPORTED;
   }

   /* MAC Reset */
   CHECK_STATUS(mvHwsMacReset(devNum, portGroup, curPortParams.portMacNumber, portMode, curPortParams.portMacType, RESET));

   /* PCS Reset */
   CHECK_STATUS(mvHwsPcsReset(devNum, portGroup, phyPortNum, portMode, curPortParams.portPcsType, RESET));

   /* rebuild active lanes list according to current configuration (redundancy) */
   CHECK_STATUS(mvHwsRebuildActiveLaneList(devNum, portGroup, phyPortNum, portMode, curLanesList));

   /* on each related serdes */
   for (i = 0; i < curPortParams.numOfActLanes; i++)
   {
           txInv = ((txInvMask >> i) & 1);
           rxInv = ((rxInvMask >> i) & 1);
           CHECK_STATUS(mvHwsSerdesPolarityConfig(devNum, (portGroup + ((curLanesList[i] >> 16) & 0xFF)),
                                                   (curLanesList[i] & 0xFFFF), HWS_DEV_SERDES_TYPE(devNum), txInv, rxInv));
   }

   /* PCS UnReset */
   CHECK_STATUS(mvHwsPcsReset(devNum, portGroup, phyPortNum, portMode, curPortParams.portPcsType, UNRESET));

   /* MAC UnReset */
   CHECK_STATUS(mvHwsMacReset(devNum, portGroup, curPortParams.portMacNumber, portMode, curPortParams.portMacType, UNRESET));

   return GT_OK;
}

/**
* @internal mvHwsPortTxEnable function
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
GT_STATUS mvHwsPortTxEnable
(
   GT_U8   devNum,
   GT_U32  portGroup,
   GT_U32  phyPortNum,
   MV_HWS_PORT_STANDARD    portMode,
   GT_BOOL enable
)
{
   MV_HWS_PORT_INIT_PARAMS curPortParams;
   GT_U32 curLanesList[HWS_MAX_SERDES_NUM];
   GT_U32 i;

   if ((phyPortNum >= HWS_CORE_PORTS_NUM(devNum)) ||
           ((GT_U32)portMode >= HWS_DEV_PORT_MODES(devNum)))
   {
           return GT_BAD_PARAM;
   }
#ifndef MV_HWS_REDUCED_BUILD
   if(cpssDeviceRunCheck_onEmulator() )
   {
       return GT_OK;
   }
   if (hwsDeviceSpecInfo[devNum].ipcConnType == HOST2SERVICE_CPU_IPC_CONNECTION)
           return (mvHwsPortTxEnableIpc(devNum,portGroup, phyPortNum,portMode,enable));
#endif

   if (GT_OK != hwsPortModeParamsGetToBuffer(devNum, portGroup, phyPortNum, portMode, &curPortParams))
   {
       return GT_NOT_SUPPORTED;
   }

   /* rebuild active lanes list according to current configuration (redundancy) */
   CHECK_STATUS(mvHwsRebuildActiveLaneList(devNum, portGroup, phyPortNum, portMode, curLanesList));

   /* on each related serdes */
   for (i = 0; i < curPortParams.numOfActLanes; i++)
   {
           CHECK_STATUS(mvHwsSerdesTxEnable(devNum, (portGroup + ((curLanesList[i] >> 16) & 0xFF)),
                                            (curLanesList[i] & 0xFFFF), HWS_DEV_SERDES_TYPE(devNum), enable));
   }

   return GT_OK;
}

/**
* @internal mvHwsPortTxEnableGet function
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
GT_STATUS mvHwsPortTxEnableGet
(
   GT_U8                   devNum,
   GT_U32                  portGroup,
   GT_U32                  phyPortNum,
   MV_HWS_PORT_STANDARD    portMode,
   GT_BOOL                 serdesTxStatus[]
)
{
   MV_HWS_PORT_INIT_PARAMS curPortParams;
   GT_U32 curLanesList[HWS_MAX_SERDES_NUM];
   GT_U32 i;

   if ((phyPortNum >= HWS_CORE_PORTS_NUM(devNum)) ||
           ((GT_U32)portMode >= HWS_DEV_PORT_MODES(devNum)))
   {
           return GT_BAD_PARAM;
   }
#ifndef MV_HWS_REDUCED_BUILD
   if (hwsDeviceSpecInfo[devNum].ipcConnType == HOST2SERVICE_CPU_IPC_CONNECTION)
           return (mvHwsPortTxEnableGetIpc(devNum,portGroup, phyPortNum,portMode,serdesTxStatus));
#endif

   if (GT_OK != hwsPortModeParamsGetToBuffer(devNum, portGroup, phyPortNum, portMode, &curPortParams))
   {
       return GT_NOT_SUPPORTED;
   }

   /* rebuild active lanes list according to current configuration (redundancy) */
   CHECK_STATUS(mvHwsRebuildActiveLaneList(devNum, portGroup, phyPortNum, portMode, curLanesList));

   /* on each related serdes */
   for (i = 0; i < curPortParams.numOfActLanes; i++)
   {
           CHECK_STATUS(mvHwsSerdesTxEnableGet(devNum, portGroup,(curLanesList[i] & 0xFFFF),
                                            HWS_DEV_SERDES_TYPE(devNum), &serdesTxStatus[i]));
   }

   return GT_OK;
}

/**
* @internal mvHwsPortSignalDetectGet function
* @endinternal
*
* @brief   Retrieve the signal detected status of all port serdeses.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - port standard metric
*
* @param[out] signalDet[]              per serdes.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPortSignalDetectGet
(
   GT_U8                   devNum,
   GT_U32                  portGroup,
   GT_U32                  phyPortNum,
   MV_HWS_PORT_STANDARD    portMode,
   GT_BOOL                 signalDet[]
)
{
   MV_HWS_PORT_INIT_PARAMS curPortParams;
   GT_U32 curLanesList[HWS_MAX_SERDES_NUM];
   GT_U32 i;

   if ((phyPortNum >= HWS_CORE_PORTS_NUM(devNum)) ||
           ((GT_U32)portMode >= HWS_DEV_PORT_MODES(devNum)))
   {
           return GT_BAD_PARAM;
   }
#ifndef MV_HWS_REDUCED_BUILD
   if (hwsDeviceSpecInfo[devNum].ipcConnType == HOST2SERVICE_CPU_IPC_CONNECTION)
           return (mvHwsPortSignalDetectGetIpc(devNum,portGroup, phyPortNum,portMode,signalDet));
#endif

   if (GT_OK != hwsPortModeParamsGetToBuffer(devNum, portGroup, phyPortNum, portMode, &curPortParams))
   {
       return GT_NOT_SUPPORTED;
   }

   /* rebuild active lanes list according to current configuration (redundancy) */
   CHECK_STATUS(mvHwsRebuildActiveLaneList(devNum, portGroup, phyPortNum, portMode, curLanesList));

   /* on each related serdes */
   for (i = 0; i < curPortParams.numOfActLanes; i++)
   {
           CHECK_STATUS(mvHwsSerdesSignalDetectGet(devNum, portGroup,(curLanesList[i] & 0xFFFF),
                                                HWS_DEV_SERDES_TYPE(devNum), &signalDet[i]));
   }

   return GT_OK;
}

/*******************************************************************************
* mvHwsPortCdrLockStatusGet
*
* DESCRIPTION:
*       Retrieve the CDR lock status of all port serdeses.
*
* INPUTS:
*       devNum    - system device number
*       portGroup - port group (core) number
*       phyPortNum - physical port number
*       portMode   - port standard metric
*
* OUTPUTS:
*       cdrLockStatus per serdes.
*
* RETURNS:
*       0  - on success
*       1  - on error
*
*******************************************************************************/
#if ( !defined(MV_HWS_FREE_RTOS) || (defined (BC2_DEV_SUPPORT) || defined(AC3_DEV_SUPPORT)))
GT_STATUS mvHwsPortCdrLockStatusGet
(
   GT_U8                   devNum,
   GT_U32                  portGroup,
   GT_U32                  phyPortNum,
   MV_HWS_PORT_STANDARD    portMode,
   GT_BOOL                 cdrLockStatus[]
)
{
   MV_HWS_PORT_INIT_PARAMS curPortParams;
   GT_U32 curLanesList[HWS_MAX_SERDES_NUM];
   GT_U32 i;

   if ((phyPortNum >= HWS_CORE_PORTS_NUM(devNum)) ||
           ((GT_U32)portMode >= HWS_DEV_PORT_MODES(devNum)))
   {
           return GT_BAD_PARAM;
   }
#ifndef MV_HWS_REDUCED_BUILD
   if (hwsDeviceSpecInfo[devNum].ipcConnType == HOST2SERVICE_CPU_IPC_CONNECTION)
           return (mvHwsPortCdrLockStatusGetIpc(devNum,portGroup, phyPortNum,portMode,cdrLockStatus));
#endif

   if (GT_OK != hwsPortModeParamsGetToBuffer(devNum, portGroup, phyPortNum, portMode, &curPortParams))
   {
       return GT_NOT_SUPPORTED;
   }

   /* rebuild active lanes list according to current configuration (redundancy) */
   CHECK_STATUS(mvHwsRebuildActiveLaneList(devNum, portGroup, phyPortNum, portMode, curLanesList));

   /* on each related serdes */
   for (i = 0; i < curPortParams.numOfActLanes; i++)
   {
           CHECK_STATUS(mvHwsSerdesCdrLockStatusGet(devNum, portGroup,(curLanesList[i] & 0xFFFF),
                                                HWS_DEV_SERDES_TYPE(devNum), &cdrLockStatus[i]));
   }

   return GT_OK;
}
#endif /* ( !defined(MV_HWS_FREE_RTOS) || (defined (BC2_DEV_SUPPORT) || defined(AC3_DEV_SUPPORT))) */

/**
* @internal hwsPortExtendedModeCfg function
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
GT_STATUS hwsPortExtendedModeCfg
(
   GT_U8   devNum,
   GT_U32  portGroup,
   GT_U32  phyPortNum,
   MV_HWS_PORT_STANDARD    portMode,
   GT_BOOL extendedMode
)
{
   MV_HWS_DEV_FUNC_PTRS *hwsDevFunc;

   hwsDeviceSpecGetFuncPtr(&hwsDevFunc);

   if (hwsDevFunc[HWS_DEV_SILICON_INDEX(devNum)].portExtModeCfgFunc == NULL)
   {
           return GT_OK;
   }

   return hwsDevFunc[HWS_DEV_SILICON_INDEX(devNum)].portExtModeCfgFunc(devNum, portGroup,
                                                                           phyPortNum, portMode, extendedMode);
}

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
   GT_U8   devNum,
   GT_U32  portGroup,
   GT_U32  phyPortNum,
   MV_HWS_PORT_STANDARD    portMode,
   GT_BOOL *extendedMode
)
{
   MV_HWS_DEV_FUNC_PTRS *hwsDevFunc;

   hwsDeviceSpecGetFuncPtr(&hwsDevFunc);

   if (hwsDevFunc[HWS_DEV_SILICON_INDEX(devNum)].portExtModeCfgGetFunc == NULL)
   {
           return GT_OK;
   }

   return hwsDevFunc[HWS_DEV_SILICON_INDEX(devNum)].portExtModeCfgGetFunc(devNum, portGroup,
                                                                           phyPortNum, portMode, extendedMode);
}

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
)
{
    MV_HWS_DEV_FUNC_PTRS *hwsDevFunc;

    hwsDeviceSpecGetFuncPtr(&hwsDevFunc);

    if (hwsDevFunc[HWS_DEV_SILICON_INDEX(devNum)].portFixAlign90Func == NULL)
    {
        return GT_OK;
    }

    return hwsDevFunc[HWS_DEV_SILICON_INDEX(devNum)].portFixAlign90Func(devNum, portGroup, phyPortNum, portMode);
}

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
   GT_U8   devNum,
   GT_U32  portGroup,
   GT_U32  phyPortNum,
   MV_HWS_PORT_STANDARD    portMode,
   MV_HWS_AUTO_TUNE_STATUS *rxTune,
   MV_HWS_AUTO_TUNE_STATUS *txTune
)
{
   MV_HWS_PORT_INIT_PARAMS curPortParams;
   MV_HWS_AUTO_TUNE_STATUS rxStatus[HWS_MAX_SERDES_NUM];
   MV_HWS_AUTO_TUNE_STATUS txStatus[HWS_MAX_SERDES_NUM];
   GT_U32  curLanesList[HWS_MAX_SERDES_NUM];
   GT_BOOL autoTunePass[HWS_MAX_SERDES_NUM];
   GT_U32 i;
#ifndef MV_HWS_REDUCED_BUILD
   if (hwsDeviceSpecInfo[devNum].ipcConnType == HOST2SERVICE_CPU_IPC_CONNECTION)
           return (mvHwsPortAutoTuneStateCheckIpc(devNum,portGroup, phyPortNum,portMode,rxTune,txTune));

#endif

   if ((phyPortNum >= HWS_CORE_PORTS_NUM(devNum)) ||
           ((GT_U32)portMode >= HWS_DEV_PORT_MODES(devNum)))
   {
           return GT_BAD_PARAM;
   }

   if(GT_OK != hwsPortModeParamsGetToBuffer(devNum, portGroup, phyPortNum, portMode, &curPortParams))
   {
       return GT_NOT_SUPPORTED;
   }

   hwsOsMemSetFuncPtr(autoTunePass, 0, sizeof(autoTunePass));
   hwsOsMemSetFuncPtr(rxStatus, 0, sizeof(rxStatus));
   hwsOsMemSetFuncPtr(txStatus, 0, sizeof(txStatus));

   *rxTune = TUNE_PASS;
   *txTune = TUNE_PASS;

   /* rebuild active lanes list according to current configuration (redundancy) */
   CHECK_STATUS(mvHwsRebuildActiveLaneList(devNum, portGroup, phyPortNum, portMode, curLanesList));

   for (i = 0; (i < HWS_MAX_SERDES_NUM) && (i < curPortParams.numOfActLanes); i++)
   {
       CHECK_STATUS(mvHwsSerdesAutoTuneStatusShort(devNum, (portGroup + ((curLanesList[i] >> 16) & 0xFF)),
                                                   (curLanesList[i] & 0xFFFF), HWS_DEV_SERDES_TYPE(devNum), &rxStatus[i], &txStatus[i]));

       if (txStatus[i] != TUNE_PASS)
       {
           *txTune = txStatus[i];
       }

       if (rxStatus[i] != TUNE_PASS)
       {
           *rxTune = rxStatus[i];
       }
   }

   return GT_OK;
}

/**
* @internal mvHwsPortPostTraining function
* @endinternal
*
* @brief   Perform UnReset for Serdes(RF), PCS and MAC units after performing TRX-Training
*         Note: this function is used mainly used by AP process where
*         training is executed in non-blocking mode (unlike "regular"
*         port mode training where status functionality is not completed
*         till all lanes finished their training).
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - port standard metric
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPortPostTraining
(
   GT_U8   devNum,
   GT_U32  portGroup,
   GT_U32  phyPortNum,
   MV_HWS_PORT_STANDARD    portMode
)
{
   MV_HWS_PORT_INIT_PARAMS curPortParams;
   GT_U32 curLanesList[HWS_MAX_SERDES_NUM];
   MV_HWS_DEV_FUNC_PTRS *hwsDevFunc;
   GT_32  j;

   if (GT_OK != hwsPortModeParamsGetToBuffer(devNum, portGroup, phyPortNum, portMode, &curPortParams))
   {
       return GT_NOT_SUPPORTED;
   }

   if (performPcsWA == GT_FALSE)
   {
        /* UnReset PCS Rx*/
        CHECK_STATUS(mvHwsPcsRxReset(devNum, portGroup, curPortParams.portPcsNumber,
                                                curPortParams.portPcsType, UNRESET));
   }
   else
   {
        hwsDeviceSpecGetFuncPtr(&hwsDevFunc);

        /* rebuild active lanes list according to current configuration (redundancy) */
        CHECK_STATUS(mvHwsRebuildActiveLaneList(devNum, portGroup, phyPortNum, portMode, curLanesList));

        /* Serdes digital UnReset */
        for (j = curPortParams.numOfActLanes-1; j >= 0; j--)
        {
            CHECK_STATUS(mvHwsSerdesDigitalReset(devNum, (portGroup + ((curLanesList[j] >> 16) & 0xFF)),
                                    (curLanesList[j] & 0xFFFF), HWS_DEV_SERDES_TYPE(devNum), UNRESET));
        }

        /* UnReset PCS */
        CHECK_STATUS(mvHwsPcsReset(devNum, portGroup, phyPortNum, portMode,
                                                curPortParams.portPcsType, UNRESET));

        /* Run fix Align90 */
        CHECK_STATUS(mvHwsPortFixAlign90Ext(devNum, portGroup, phyPortNum, portMode));

        /* UnReset MAC */
        CHECK_STATUS(mvHwsMacReset(devNum, portGroup, curPortParams.portMacNumber, portMode,
                                                curPortParams.portMacType, UNRESET));
   }

   /* un-mark PCS to be under TRx training */
   CHECK_STATUS(mvHwsPCSMarkModeSet(devNum, portGroup, phyPortNum, portMode, GT_FALSE));

   return GT_OK;
}

/**
* @internal mvHwsPortAutoTuneStop function
* @endinternal
*
* @brief   Stop Tx and Rx training.
*         Note: this function is used mainly used by AP process where
*         training is executed in non-blocking mode (unlike "regular"
*         port mode training where status functionality is not completed
*         till all lanes finished their training).
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
   GT_U8   devNum,
   GT_U32  portGroup,
   GT_U32  phyPortNum,
   MV_HWS_PORT_STANDARD    portMode,
   GT_BOOL stopRx,
   GT_BOOL stopTx
)
{
   MV_HWS_DEV_FUNC_PTRS *hwsDevFunc;
   MV_HWS_PORT_INIT_PARAMS curPortParams;
   GT_U32  curLanesList[HWS_MAX_SERDES_NUM];
   GT_U32  i;

    if ((phyPortNum >= HWS_CORE_PORTS_NUM(devNum)) ||
           ((GT_U32)portMode >= HWS_DEV_PORT_MODES(devNum)))
   {
           return GT_BAD_PARAM;
   }

#ifndef MV_HWS_REDUCED_BUILD
   if (hwsDeviceSpecInfo[devNum].ipcConnType == HOST2SERVICE_CPU_IPC_CONNECTION)
           return (mvHwsPortAutoTuneStopIpc(devNum,portGroup, phyPortNum, portMode, stopRx,stopTx));
#endif

    hwsDeviceSpecGetFuncPtr(&hwsDevFunc);

    if (hwsDevFunc[HWS_DEV_SILICON_INDEX(devNum)].portAutoTuneStopFunc != NULL)
    {
        return(hwsDevFunc[HWS_DEV_SILICON_INDEX(devNum)].portAutoTuneStopFunc(devNum, portGroup, phyPortNum, portMode, stopRx,stopTx));
    }

   /* perform UnReset for Serdes(RF), PCS and MAC units BEFORE TRX-Training is stopped */
   CHECK_STATUS(mvHwsPortPostTraining(devNum, portGroup, phyPortNum, portMode));

   /* rebuild active lanes list according to current configuration (redundancy) */
   CHECK_STATUS(mvHwsRebuildActiveLaneList(devNum, portGroup, phyPortNum, portMode,curLanesList));

   if (GT_OK != hwsPortModeParamsGetToBuffer(devNum, portGroup, phyPortNum, portMode, &curPortParams))
   {
       return GT_NOT_SUPPORTED;
   }

   /* on each related serdes */
   for (i = 0; (i < HWS_MAX_SERDES_NUM) && (i < curPortParams.numOfActLanes); i++)
   {
           /* stop an appropriate engine */
           if (stopTx == GT_TRUE)
           {
                   CHECK_STATUS(mvHwsSerdesTxAutoTuneStop(devNum, (portGroup + ((curLanesList[i] >> 16) & 0xFF)),
                                                           (curLanesList[i] & 0xFFFF), HWS_DEV_SERDES_TYPE(devNum)));
           }

           if (stopRx == GT_TRUE)
           {
                   CHECK_STATUS(mvHwsSerdesRxAutoTuneStart(devNum, (portGroup + ((curLanesList[i] >> 16) & 0xFF)),
                                                           (curLanesList[i] & 0xFFFF), HWS_DEV_SERDES_TYPE(devNum), GT_FALSE));
           }
   }

   return GT_OK;
}

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
* @param[in] portFecType              - 0-AP_ST_HCD_FEC_RES_NONE, 1-AP_ST_HCD_FEC_RES_FC, 2-AP_ST_HCD_FEC_RES_RS
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPortFecCofig
(
   GT_U8                devNum,
   GT_U32               portGroup,
   GT_U32               phyPortNum,
   MV_HWS_PORT_STANDARD portMode,
   MV_HWS_PORT_FEC_MODE portFecType
)
{
    MV_HWS_PORT_INIT_PARAMS curPortParams;

    if ((phyPortNum >= HWS_CORE_PORTS_NUM(devNum)) ||
        ((GT_U32)portMode >= HWS_DEV_PORT_MODES(devNum)))
    {
        return GT_BAD_PARAM;
    }

#ifndef MV_HWS_REDUCED_BUILD
   if (hwsDeviceSpecInfo[devNum].ipcConnType == HOST2SERVICE_CPU_IPC_CONNECTION)
           return (mvHwsPortFecConfigIpc(devNum,portGroup, phyPortNum,portMode, portFecType));
#endif

   if (GT_OK != hwsPortModeParamsGetToBuffer(devNum, portGroup, phyPortNum, portMode, &curPortParams))
   {
       return GT_NOT_SUPPORTED;
   }

    /* MAC Reset */
    CHECK_STATUS(mvHwsMacReset(devNum, portGroup, curPortParams.portMacNumber, portMode, curPortParams.portMacType, RESET));

    /* PCS Reset */
    CHECK_STATUS(mvHwsPcsReset(devNum, portGroup, phyPortNum, portMode, curPortParams.portPcsType, RESET));

    CHECK_STATUS(mvHwsPcsFecConfig(devNum,portGroup, curPortParams.portPcsNumber, curPortParams.portPcsType, portFecType));

    /* PCS UnReset */
    CHECK_STATUS(mvHwsPcsReset(devNum, portGroup, phyPortNum, portMode, curPortParams.portPcsType, UNRESET));

    /* MAC UnReset */
    CHECK_STATUS(mvHwsMacReset(devNum, portGroup, curPortParams.portMacNumber, portMode, curPortParams.portMacType, UNRESET));

    return GT_OK;
}

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
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPortFecCofigGet
(
   GT_U8   devNum,
   GT_U32  portGroup,
   GT_U32  phyPortNum,
   MV_HWS_PORT_STANDARD    portMode,
   MV_HWS_PORT_FEC_MODE    *fecMode
)
{
    MV_HWS_PORT_INIT_PARAMS curPortParams;
    /* GT_BOOL                 fecEn; */
    GT_UREG_DATA            data;

    if ((phyPortNum >= HWS_CORE_PORTS_NUM(devNum)) ||
        ((GT_U32)portMode >= HWS_DEV_PORT_MODES(devNum)))
    {
        return GT_BAD_PARAM;
    }

#ifndef MV_HWS_REDUCED_BUILD
    if (hwsDeviceSpecInfo[devNum].ipcConnType == HOST2SERVICE_CPU_IPC_CONNECTION)
        return (mvHwsPortFecConfigGetIpc(devNum, portGroup, phyPortNum, portMode, fecMode));
#endif

    if (GT_OK != hwsPortModeParamsGetToBuffer(devNum, portGroup, phyPortNum, portMode, &curPortParams))
    {
        return GT_NOT_SUPPORTED;
    }

    /* check if 25G port mode use CG unit, if yes RS_FEC mode is the only available option */
    if(HWS_25G_MODE_CHECK(portMode))
    {
        /* check that RS-FEC in CG unit in use */
        CHECK_STATUS(genUnitRegisterGet(devNum, portGroup, CG_UNIT, (phyPortNum & HWS_2_LSB_MASK_CNS), CG_CONTROL_2, &data, 0));
        data = (data >> 17) & 0xF;
        if(data & (1 << phyPortNum % 4))
        {
            *fecMode = RS_FEC;
            return GT_OK;
        }
    }

    /* check if 50G port mode use CG unit, if yes RS_FEC or NO-FEC mode are available and should be checked in CGPCS FEC get function.
       if CG unit not in use - only FC-FEC mode is the only available option  */
    if(HWS_TWO_LANES_MODE_CHECK(portMode))
    {
        /* check that CG unit in use */
        CHECK_STATUS(genUnitRegisterGet(devNum, portGroup, CG_UNIT, (phyPortNum & HWS_2_LSB_MASK_CNS), CG_CONTROL_2, &data, 0));
        data = (data >> 13) & 0xF;
        if(data == 0)
        {
            *fecMode = FC_FEC;
            return GT_OK;
        }
    }

    CHECK_STATUS(mvHwsPcsFecConfigGet(devNum, portGroup, phyPortNum, portMode, fecMode));

    return GT_OK;
}

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
)
{
   MV_HWS_PORT_INIT_PARAMS curPortParams;
   MV_HWS_ALIGN90_PARAMS serdesParams[HWS_MAX_SERDES_NUM];
   GT_U32  curLanesList[HWS_MAX_SERDES_NUM];
   MV_HWS_AUTO_TUNE_STATUS status[HWS_MAX_SERDES_NUM];
   GT_BOOL allLanesPass;
   GT_U32 i, loopNum;

   /* avoid warnings */
   dummyForCompilation = dummyForCompilation;

   if ((phyPortNum >= HWS_CORE_PORTS_NUM(devNum)) ||
           ((GT_U32)portMode >= HWS_DEV_PORT_MODES(devNum)))
   {
           return GT_BAD_PARAM;
   }

   if (GT_OK != hwsPortModeParamsGetToBuffer(devNum, portGroup, phyPortNum, portMode, &curPortParams))
   {
       return GT_NOT_SUPPORTED;
   }

   hwsOsMemSetFuncPtr(serdesParams, 0, sizeof(serdesParams));
   hwsOsMemSetFuncPtr(status, TUNE_NOT_COMPLITED, sizeof(status));

   /* rebuild active lanes list according to current configuration (redundancy) */
   CHECK_STATUS(mvHwsRebuildActiveLaneList(devNum, portGroup, phyPortNum, portMode, curLanesList));

   /* start Align90 on each related serdes */
   for (i = 0; (i < HWS_MAX_SERDES_NUM) && (i < curPortParams.numOfActLanes); i++)
   {
           mvHwsSerdesFixAlign90Start(devNum, (portGroup + ((curLanesList[i] >> 16) & 0xFF)),
                                      (curLanesList[i] & 0xFFFF), HWS_DEV_SERDES_TYPE(devNum), &(serdesParams[i]));
   }

   loopNum = 0;
   do
   {
           allLanesPass = GT_TRUE;
           hwsOsExactDelayPtr(devNum, portGroup, align90Delay);
           /* check status on each related serdes */
           for (i = 0; (i < HWS_MAX_SERDES_NUM) && (i < curPortParams.numOfActLanes); i++)
           {
                   mvHwsSerdesFixAlign90Status(devNum, (portGroup + ((curLanesList[i] >> 16) & 0xFF)),
                                               (curLanesList[i] & 0xFFFF), HWS_DEV_SERDES_TYPE(devNum), &status[i]);
                   if (status[i] != TUNE_PASS)
                   {
                           DEBUG_HWS_FULL(0, ("\nFix Align90 failed on serdes %d (device %d).",(curLanesList[i] & 0xFFFF), devNum));
                           allLanesPass = GT_FALSE;
                   }
           }
           loopNum++;
   } while ((!allLanesPass) && (loopNum < 10));

   /* stop Align90 on each related serdes */
   for (i = 0; (i < HWS_MAX_SERDES_NUM) && (i < curPortParams.numOfActLanes); i++)
   {
           mvHwsSerdesFixAlign90Stop(devNum, (portGroup + ((curLanesList[i] >> 16) & 0xFF)),
                                           (curLanesList[i] & 0xFFFF), HWS_DEV_SERDES_TYPE(devNum), &(serdesParams[i]),
                                           ((status[i] == TUNE_PASS) ? GT_TRUE : GT_FALSE));
           mvHwsSerdesRev2DfeCheck(devNum, (portGroup + ((curLanesList[i] >> 16) & 0xFF)),
                                           (curLanesList[i] & 0xFFFF), (MV_HWS_SERDES_TYPE)HWS_DEV_SERDES_TYPE(devNum));
   }

   if (allLanesPass)
   {
           return GT_OK;
   }

   return GT_FAIL;
}

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
   GT_U8               devNum,
   GT_U32              portGroup,
   GT_U32              phyPortNum,
   MV_HWS_PORT_STANDARD    portMode,
   MV_HWS_ALIGN90_PARAMS   *serdesParams
)
{
   MV_HWS_PORT_INIT_PARAMS curPortParams;
   GT_U32 curLanesList[HWS_MAX_SERDES_NUM];
   GT_U32 numOfActLanes;
   GT_U32 i;

   if ((phyPortNum >= HWS_CORE_PORTS_NUM(devNum)) ||
           ((GT_U32)portMode >= HWS_DEV_PORT_MODES(devNum)))
   {
           return GT_BAD_PARAM;
   }

   if (GT_OK != hwsPortModeParamsGetToBuffer(devNum, portGroup, phyPortNum, portMode, &curPortParams))
   {
       return GT_NOT_SUPPORTED;
   }

   /* rebuild active lanes list according to current configuration (redundancy) */
   CHECK_STATUS(mvHwsRebuildActiveLaneList(devNum, portGroup, phyPortNum, portMode, curLanesList));

   numOfActLanes = curPortParams.numOfActLanes;
   if (numOfActLanes > HWS_MAX_SERDES_NUM)
   {
           return GT_BAD_PARAM;
   }

   /* start Align90 on each related serdes */
   for (i = 0; i < numOfActLanes; i++)
   {
           DEBUG_HWS_FULL(2, ("Start Align90 for Serdes %d\n", curLanesList[i] & 0xFFFF));

           CHECK_STATUS(mvHwsSerdesFixAlign90Start(devNum, (portGroup + ((curLanesList[i] >> 16) & 0xFF)),
                                                           (curLanesList[i] & 0xFFFF), HWS_DEV_SERDES_TYPE(devNum), &(serdesParams[i])));
   }

   return GT_OK;
}

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
   GT_U8   devNum,
   GT_U32  portGroup,
   GT_U32  phyPortNum,
   MV_HWS_PORT_STANDARD    portMode,
   MV_HWS_ALIGN90_PARAMS   *serdesParams
)
{
   MV_HWS_PORT_INIT_PARAMS curPortParams;
   GT_U32 curLanesList[HWS_MAX_SERDES_NUM];
   GT_U32 numOfActLanes;
   GT_U32 i;
   MV_HWS_AUTO_TUNE_STATUS status[HWS_MAX_SERDES_NUM];

   if ((phyPortNum >= HWS_CORE_PORTS_NUM(devNum)) ||
           ((GT_U32)portMode >= HWS_DEV_PORT_MODES(devNum)))
   {
           return GT_BAD_PARAM;
   }

   if (GT_OK != hwsPortModeParamsGetToBuffer(devNum, portGroup, phyPortNum, portMode, &curPortParams))
   {
       return GT_NOT_SUPPORTED;
   }

   /* rebuild active lanes list according to current configuration (redundancy) */
   CHECK_STATUS(mvHwsRebuildActiveLaneList(devNum, portGroup, phyPortNum, portMode, curLanesList));

   numOfActLanes = curPortParams.numOfActLanes;

   /* stop Align90 on each related serdes */
   for (i = 0; i < numOfActLanes; i++)
   {
           DEBUG_HWS_FULL(2, ("Fix Align90 stop for serdes %d (device %d)\n", (curLanesList[i] & 0xFFFF), devNum));


           CHECK_STATUS(mvHwsSerdesFixAlign90Status(devNum,(portGroup + ((curLanesList[i] >> 16) & 0xFF)),
                                                           (curLanesList[i] & 0xFFFF), HWS_DEV_SERDES_TYPE(devNum), &status[i]));

           CHECK_STATUS(mvHwsSerdesFixAlign90Stop(devNum, (portGroup + ((curLanesList[i] >> 16) & 0xFF)),
                                                   (curLanesList[i] & 0xFFFF), HWS_DEV_SERDES_TYPE(devNum), &(serdesParams[i]),
                                                   ((status[i] == TUNE_PASS) ? GT_TRUE : GT_FALSE)));
   }

   return GT_OK;
}

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
   GT_U8   devNum,
   GT_U32  portGroup,
   GT_U32  phyPortNum,
   MV_HWS_PORT_STANDARD    portMode,
   MV_HWS_AUTO_TUNE_STATUS *statusPtr
)
{
   MV_HWS_PORT_INIT_PARAMS curPortParams;
   GT_U32 curLanesList[HWS_MAX_SERDES_NUM];
   GT_U32 numOfActLanes;
   GT_U32 i;
   MV_HWS_AUTO_TUNE_STATUS status[HWS_MAX_SERDES_NUM];

   if ((phyPortNum >= HWS_CORE_PORTS_NUM(devNum)) ||
           ((GT_U32)portMode >= HWS_DEV_PORT_MODES(devNum)))
   {
           return GT_BAD_PARAM;
   }

   if (GT_OK != hwsPortModeParamsGetToBuffer(devNum, portGroup, phyPortNum, portMode, &curPortParams))
   {
       return GT_NOT_SUPPORTED;
   }

   /* rebuild active lanes list according to current configuration (redundancy) */
   CHECK_STATUS(mvHwsRebuildActiveLaneList(devNum, portGroup, phyPortNum, portMode, curLanesList));
   numOfActLanes = curPortParams.numOfActLanes;

   *statusPtr = TUNE_PASS;
   /* check status on each related serdes */
   for (i = 0; i < numOfActLanes; i++)
   {
           CHECK_STATUS(mvHwsSerdesFixAlign90Status(devNum, (portGroup + ((curLanesList[i] >> 16) & 0xFF)),
                                                           (curLanesList[i] & 0xFFFF), HWS_DEV_SERDES_TYPE(devNum), &status[i]));

           if (status[i] != TUNE_PASS)
           {
                   if (status[i] == TUNE_FAIL)
                   {
                           *statusPtr = TUNE_FAIL;
                           DEBUG_HWS_FULL(2, ("Fix Align90 failed for serdes %d (device %d)\n", (curLanesList[i] & 0xFFFF), devNum));

                           /* if one lane failed no reason to continue */
                           return GT_OK;
                   }
                   else if (status[i] == TUNE_NOT_COMPLITED)
                   {
                           *statusPtr = TUNE_NOT_COMPLITED;

                           DEBUG_HWS_FULL(2, ("Fix Align90 not completed for serdes %d (device %d)\n", (curLanesList[i] & 0xFFFF), devNum));
                   }
           }
   }

   return GT_OK;
}

/**
* @internal hwsPortFixAlign90Flow function
* @endinternal
*
* @brief   Run Align90 flow.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
*                                      phyPortNum - physical port number
*                                      portMode   - port standard metric
*                                      optAlgoMask- bit mask of optimization algorithms
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS hwsPortFixAlign90Flow
(
   GT_U8   devNum,
   GT_U32  portGroup,
   GT_U32  *serdesList,
   GT_U32  numOfActLanes,
   GT_BOOL *allLanesPass
)
{
   GT_U32 i;
   GT_U32 loopNum;
   MV_HWS_ALIGN90_PARAMS serdesParams[HWS_MAX_SERDES_NUM];
   MV_HWS_AUTO_TUNE_STATUS status[HWS_MAX_SERDES_NUM];

   if (numOfActLanes > HWS_MAX_SERDES_NUM)
   {
           return GT_BAD_PARAM;
   }

   hwsOsMemSetFuncPtr(serdesParams, 0, sizeof(serdesParams));
   hwsOsMemSetFuncPtr(status, TUNE_NOT_COMPLITED, sizeof(status));

   /* start Align90 on each related serdes */
   for (i = 0; i < numOfActLanes; i++)
   {
           if (serdesList[i] == MV_HWS_SERDES_NOT_ACTIVE)
           {
                   continue;
           }

           DEBUG_HWS_FULL(2, ("Start Align90 for Serdes %d\n", serdesList[i] & 0xFFFF));

           mvHwsSerdesFixAlign90Start(devNum, (portGroup + ((serdesList[i] >> 16) & 0xFF)),
                                      (serdesList[i] & 0xFFFF), HWS_DEV_SERDES_TYPE(devNum), &(serdesParams[i]));
   }

   loopNum = 0;

   do
   {
           *allLanesPass = GT_TRUE;
           hwsOsExactDelayPtr(devNum, portGroup, align90Delay);
           /* check status on each related serdes */
           for (i = 0; i < numOfActLanes; i++)
           {
                   if ((serdesList[i] == MV_HWS_SERDES_NOT_ACTIVE) || (status[i] == TUNE_PASS))
                   {
                           continue;
                   }

                   mvHwsSerdesFixAlign90Status(devNum, (portGroup + ((serdesList[i] >> 16) & 0xFF)),
                                                   (serdesList[i] & 0xFFFF), HWS_DEV_SERDES_TYPE(devNum), &status[i]);

                   if (status[i] != TUNE_PASS)
                   {
                           *allLanesPass = GT_FALSE;

                           if (status[i] == TUNE_FAIL)
                           {
                                   DEBUG_HWS_FULL(2, ("Fix Align90 failed for serdes %d (device %d)\n",
                                                           (serdesList[i] & 0xFFFF), devNum));
                           }
                           else if (status[i] == TUNE_NOT_COMPLITED)
                           {
                                   DEBUG_HWS_FULL(2, ("Fix Align90 no completed for serdes %d (device %d)\n",
                                                           (serdesList[i] & 0xFFFF), devNum));
                           }
                   }
           }

           loopNum++;

   } while ((!(*allLanesPass)) && (loopNum < 10));

   /* stop Align90 on each related serdes */
   for (i = 0; i < numOfActLanes; i++)
   {
           if (serdesList[i] == MV_HWS_SERDES_NOT_ACTIVE)
           {
                   continue;
           }

           mvHwsSerdesFixAlign90Stop(devNum, (portGroup + ((serdesList[i] >> 16) & 0xFF)),
                                           (serdesList[i] & 0xFFFF), HWS_DEV_SERDES_TYPE(devNum), &(serdesParams[i]),
                                           ((status[i] == TUNE_PASS) ? GT_TRUE : GT_FALSE));

           /* after we stop the Serdes, we can mark it as not active */
           if (status[i] == TUNE_PASS)
           {
                   DEBUG_HWS_FULL(2, ("Align90 completed for Serdes %d\n", serdesList[i] & 0xFFFF));
                   serdesList[i] = MV_HWS_SERDES_NOT_ACTIVE;
           }
   }

   return GT_OK;
}

/**
* @internal mvHwsPortAutoTuneOptimization function
* @endinternal
*
* @brief   Run optimization for the training results.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - port standard metric
* @param[in] optAlgoMask              bit mask of optimization algorithms
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPortAutoTuneOptimization
(
   GT_U8   devNum,
   GT_U32  portGroup,
   GT_U32  phyPortNum,
   MV_HWS_PORT_STANDARD    portMode,
   GT_U32  optAlgoMask
)
{
   MV_HWS_PORT_MAN_TUNE_MODE       tuningMode;
   GT_BOOL isTrainingMode;
   MV_HWS_PORT_INIT_PARAMS curPortParams;
   GT_U32  curLanesList[HWS_MAX_SERDES_NUM];
   GT_STATUS rc = GT_OK;

   CHECK_STATUS(hwsPortGetTuneMode(portMode, &tuningMode, &isTrainingMode));
   if (isTrainingMode == GT_FALSE)
   {
           return GT_BAD_PARAM;
   }

   if (GT_OK != hwsPortModeParamsGetToBuffer(devNum, portGroup, phyPortNum, portMode, &curPortParams))
   {
       return GT_NOT_SUPPORTED;
   }

   /* rebuild active lanes list according to current configuration (redundancy) */
   CHECK_STATUS(mvHwsRebuildActiveLaneList(devNum, portGroup, phyPortNum, portMode, curLanesList));
#ifdef CHX_FAMILY
   rc = mvHwsComHRev2SerdesTrainingOptimization(devNum, portGroup, curLanesList,
                                                   curPortParams.numOfActLanes, tuningMode, optAlgoMask);
#else
   optAlgoMask = optAlgoMask;
#endif
   return rc;
}

/**
* @internal hwsPortGetTuneMode function
* @endinternal
*
* @brief   returns the tune mode according to port mode..
*
* @param[in] portMode                 - port standard metric
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS hwsPortGetTuneMode
(
   MV_HWS_PORT_STANDARD    portMode,
   MV_HWS_PORT_MAN_TUNE_MODE       *tuneMode,
   GT_BOOL *isTrainingMode
)
{
   *isTrainingMode = GT_TRUE;

   if (IS_KR_MODE(portMode))
   {
           /*hwsOsPrintf("Long Reach\n");*/
           *tuneMode = StaticLongReach;
   }
   else if (IS_SR_MODE(portMode))
   {
           /*hwsOsPrintf("Short Reach\n");*/
           *tuneMode = StaticShortReach;
   }
   else
   {
           /*hwsOsPrintf("Not a training mode\n");*/
           *isTrainingMode = GT_FALSE;
   }

   return GT_OK;
}

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
   GT_U8   devNum,
   GT_U32  portGroup,
   GT_U32  phyPortNum,
   MV_HWS_PORT_STANDARD    *portModePtr
)
{
   MV_HWS_SERDES_SPEED rate;
   MV_HWS_PORT_INIT_PARAMS curPortParams;
   GT_U32  curLanesList[HWS_MAX_SERDES_NUM];
   GT_U32 pcsLaneNums, macLaneNums;
   GT_U32 i;
   GT_STATUS rc = GT_OK;
#ifndef CM3
   MV_HWS_SERDES_MEDIA     serdesMediaType;
   GT_UREG_DATA  data = 0;
#endif /* CM3 */

   if ((phyPortNum >= HWS_CORE_PORTS_NUM(devNum)) || (portModePtr == NULL))
   {
           return GT_BAD_PARAM;
   }

#ifndef MV_HWS_REDUCED_BUILD
   if (hwsDeviceSpecInfo[devNum].ipcConnType == HOST2SERVICE_CPU_IPC_CONNECTION)
           return (mvHwsPortInterfaceGetIpc(devNum,portGroup, phyPortNum,portModePtr));
#endif

   *portModePtr = NON_SUP_MODE;
   for (i = 0; i < hwsDeviceSpecInfo[devNum].lastSupPortMode; i++)
   {
       if (GT_OK != hwsPortModeParamsGetToBuffer(devNum, portGroup, phyPortNum, i, &curPortParams))
       {
           continue;
       }
       if ((curPortParams.numOfActLanes != 0) && (curPortParams.portMacNumber != NA_NUM) && (curPortParams.serdesSpeed != SPEED_NA))
       {
           /* rebuild active lanes list according to current configuration (redundancy) */
           CHECK_STATUS(mvHwsRebuildActiveLaneList(devNum, portGroup, phyPortNum,
                                                   curPortParams.portStandard, curLanesList));
           /* get serdes rate */
           rc = mvHwsSerdesSpeedGet(devNum, (portGroup + ((curLanesList[0] >> 16) & 0xFF)), (curLanesList[0] & 0xFFFF),
                                    HWS_DEV_SERDES_TYPE(devNum), &rate);
           if (rc == GT_NOT_INITIALIZED)
           {
               /* It may be not initialized for current port mode but initialized in the next one*/
               /* So should continue in the loop                                                 */
               continue;
           }
           if (rc != GT_OK)
           {
               return rc;
           }


           if ((rate == SPEED_NA) || (rate != curPortParams.serdesSpeed))
           {
               continue;
           }

           /* get number of active PCSs */
           mvHwsPcsActiveStatusGet(devNum, portGroup, phyPortNum, curPortParams.portStandard, &pcsLaneNums);
           if ((pcsLaneNums == 0) || ((pcsLaneNums != curPortParams.numOfActLanes)
                                      && (curPortParams.portPcsType  != XPCS)/* RXAUI uses 2 serdes but 4 XPCS lanes */))
           {
               if ((curPortParams.portPcsType == HGLPCS) && ((rate == _3_125G) || (rate == _6_25G)))
               {
                   pcsLaneNums = 6;
               }
               else
               {
                   continue;
               }
           }

           /* get number of active MACs */
           mvHwsMacActiveStatusGet(devNum, portGroup, curPortParams.portMacNumber, curPortParams.portStandard, curPortParams.portMacType, &macLaneNums);
           if (macLaneNums == 0)
           {
               continue;
           }
#ifndef CM3
           if ((hwsDeviceSpecInfo[devNum].devType == BobcatA0) || (hwsDeviceSpecInfo[devNum].devType == Alleycat3A0))
           {
               /* check current serdes media type matches configured in hw one */
               CHECK_STATUS(hwsSerdesRegGetFuncPtr(devNum, portGroup, EXTERNAL_REG, curPortParams.firstLaneNum,
                                                   SERDES_EXTERNAL_CONFIGURATION_0, &data, 0));
               serdesMediaType = (((data >> 15) & 0x1) == GT_TRUE) ? RXAUI_MEDIA : XAUI_MEDIA;
               if (curPortParams.serdesMediaType != serdesMediaType)
               {
                   continue;
               }
           }
           else if ((HooperA0 == hwsDeviceSpecInfo[devNum].devType) || (Lion2B0 == hwsDeviceSpecInfo[devNum].devType))
           {
               /* check if it is RXAUI_MEDIA */
               CHECK_STATUS(hwsRegisterGetFuncPtr(devNum, portGroup, MG_Media_Interface_Reg, &data, 0));
               serdesMediaType = (((data >> curPortParams.firstLaneNum) & 1) == GT_TRUE) ? RXAUI_MEDIA : XAUI_MEDIA;
               if (curPortParams.serdesMediaType != serdesMediaType)
               {
                   continue;
               }
           }
           else if (Pipe == hwsDeviceSpecInfo[devNum].devType)
           {
               /* check current serdes media type matches configured in hw one */
               CHECK_STATUS(hwsSerdesRegGetFuncPtr(devNum, portGroup, EXTERNAL_REG, curPortParams.firstLaneNum,
                                                   SERDES_EXTERNAL_CONFIGURATION_0, &data, 0));
               serdesMediaType = (((data >> 2) & 0x1) == GT_TRUE) ? RXAUI_MEDIA : XAUI_MEDIA;
               if (curPortParams.serdesMediaType != serdesMediaType)
               {
                   continue;
               }
           }
#endif /* CM3 */
           *portModePtr = (MV_HWS_PORT_STANDARD)i;
           return GT_OK;
       }
   }

   return GT_OK;
}

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
   GT_U8   devNum,
   GT_U32  portGroup,
   GT_U32  phyPortNum,
   MV_HWS_PORT_STANDARD    portMode,
   GT_U32  txIpg,
   GT_U32  txPreamble,
   GT_U32  rxPreamble,
   MV_HWS_PORT_CRC_MODE    txCrc,
   MV_HWS_PORT_CRC_MODE    rxCrc
)
{
   MV_HWS_PORT_INIT_PARAMS curPortParams;

   if ((phyPortNum >= HWS_CORE_PORTS_NUM(devNum)) ||
           (portMode >= (MV_HWS_PORT_STANDARD)HWS_DEV_PORT_MODES(devNum)))
   {
           return GT_BAD_PARAM;
   }

   if (GT_OK != hwsPortModeParamsGetToBuffer(devNum, portGroup, phyPortNum, portMode, &curPortParams))
   {
       return GT_NOT_SUPPORTED;
   }

   CHECK_STATUS(mvHwsMacClearChannelCfg(devNum, portGroup,curPortParams.portMacNumber, portMode,
                                           curPortParams.portMacType,
                                           txIpg,txPreamble,rxPreamble,txCrc,rxCrc));

   return GT_OK;
}

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
   GT_U8   devNum,
   GT_U32  portGroup,
   GT_U32  phyPortNum,
   MV_HWS_PORT_STANDARD    portMode,
   GT_BOOL portAcTermEn
)
{
   MV_HWS_PORT_INIT_PARAMS curPortParams;
   GT_U32  curLanesList[HWS_MAX_SERDES_NUM];
   GT_U32 i;

   if ((phyPortNum >= HWS_CORE_PORTS_NUM(devNum)) ||
           ((GT_U32)portMode >= HWS_DEV_PORT_MODES(devNum)))
   {
           return GT_BAD_PARAM;
   }

   if ( (GT_OK != hwsPortModeParamsGetToBuffer(devNum, portGroup, phyPortNum, portMode, &curPortParams)) || curPortParams.numOfActLanes == 0)
   {
           return GT_NOT_SUPPORTED;
   }

   /* rebuild active lanes list according to current configuration (redundancy) */
   CHECK_STATUS(mvHwsRebuildActiveLaneList(devNum, portGroup, phyPortNum, portMode, curLanesList));

   for (i = 0; i < curPortParams.numOfActLanes; i++)
   {
           CHECK_STATUS(mvHwsSerdesAcTerminationCfg(devNum, (portGroup + ((curLanesList[i] >> 16) & 0xFF)),
                                                           (curLanesList[i] & 0xFFFF), (MV_HWS_SERDES_TYPE)(HWS_DEV_SERDES_TYPE(devNum)),
                                                           portAcTermEn));
   }

   return GT_OK;
}

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
   GT_U8   devNum,
   GT_U32  portGroup,
   GT_U32  phyPortNum,
   MV_HWS_PORT_STANDARD    portMode,
   GT_BOOL *laneLock
)
{
   if ((phyPortNum >= HWS_CORE_PORTS_NUM(devNum)) || ((GT_U32)portMode >= HWS_DEV_PORT_MODES(devNum)))
   {
       return GT_BAD_PARAM;
   }

   CHECK_STATUS(mvHwsPcsCheckGearBox(devNum, portGroup, phyPortNum, portMode,laneLock));

   return GT_OK;
}

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
   GT_U8   devNum,
   GT_U32  portGroup,
   GT_U32  phyPortNum,
   MV_HWS_PORT_STANDARD    portMode
)
{
   /* relevant for ports 25 and 27 */
   switch (phyPortNum)
   {
   case 25:
           switch (portMode)
           {
           case SGMII:
           case SGMII2_5:
           case _1000Base_X:
           case _5GBase_DQX:
           case _10GBase_KR:
           case _10GBase_SR_LR:
           case _12_1GBase_KR:
           case _5_625GBaseR:
           case _12GBaseR:
           case _5GBaseR:
                   CHECK_STATUS(mvHwsSerdesTxIfSelect(devNum, portGroup, 10,
                                                           (MV_HWS_SERDES_TYPE)(HWS_DEV_SERDES_TYPE(devNum)), 0));
                   break;
           default:
                   return GT_NOT_SUPPORTED;
           }
           break;
   case 27:
           switch (portMode)
           {
           case SGMII:
           case SGMII2_5:
           case _1000Base_X:
           case _5GBase_DQX:
           case _10GBase_KR:
           case _10GBase_SR_LR:
           case _12_1GBase_KR:
           case _5_625GBaseR:
           case _12GBaseR:
           case _2_5GBase_QX:
           case _5GBaseR:
                   CHECK_STATUS(mvHwsSerdesTxIfSelect(devNum, portGroup, 11,
                                                           (MV_HWS_SERDES_TYPE)(HWS_DEV_SERDES_TYPE(devNum)), 0));
                   break;
           case _5GBase_HX:
           case _20GBase_KR:
           case _20GBase_SR_LR:
                   CHECK_STATUS(mvHwsSerdesTxIfSelect(devNum, portGroup, 10,
                                                           (MV_HWS_SERDES_TYPE)(HWS_DEV_SERDES_TYPE(devNum)), 1));
                   CHECK_STATUS(mvHwsSerdesTxIfSelect(devNum, portGroup, 11,
                                                           (MV_HWS_SERDES_TYPE)(HWS_DEV_SERDES_TYPE(devNum)), 0));
                   break;
           default:
                   return GT_NOT_SUPPORTED;
           }
           break;
   default:
           return GT_NOT_SUPPORTED;
   }

   return GT_OK;
}

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
)
{
    MV_HWS_PORT_INIT_PARAMS curPortParams;
    GT_U32 curLanesList[HWS_MAX_SERDES_NUM];
    GT_U32 i;
    MV_HWS_SERDES_CONFIG_STR    serdesConfig;

    if (GT_OK != hwsPortModeParamsGetToBuffer(devNum, portGroup, phyPortNum, portMode, &curPortParams))
    {
        return GT_NOT_SUPPORTED;
    }

    switch (unitType)
    {
        case HWS_MAC:
            CHECK_STATUS(mvHwsMacReset(devNum, portGroup, curPortParams.portMacNumber, portMode, curPortParams.portMacType, action));
            break;
        case HWS_PCS:
            CHECK_STATUS(mvHwsPcsReset(devNum, portGroup, phyPortNum, portMode, curPortParams.portPcsType, action));
            break;
        case HWS_PMA: /* Serdes */
            if (action == UNRESET) /* for Serdes only Power-down is enabled */
            {
                return GT_NOT_SUPPORTED;
            }
            /* rebuild active lanes list according to current configuration (redundancy) */
            CHECK_STATUS(mvHwsRebuildActiveLaneList(devNum, portGroup, phyPortNum, portMode, curLanesList));

            for (i = 0; i < curPortParams.numOfActLanes; i++)
            {
                serdesConfig.baudRate = curPortParams.serdesSpeed;
                serdesConfig.media = curPortParams.serdesMediaType;
                serdesConfig.busWidth = curPortParams.serdes10BitStatus;
                serdesConfig.refClock = _156dot25Mhz;
                serdesConfig.refClockSource = PRIMARY;
                serdesConfig.rxEncoding = SERDES_ENCODING_NA;
                serdesConfig.serdesType = (MV_HWS_SERDES_TYPE)(HWS_DEV_SERDES_TYPE(devNum));
                serdesConfig.txEncoding = SERDES_ENCODING_NA;

                /* for Serdes Power-Down operation, refClock and refClockSource values are not relevant */
                CHECK_STATUS(mvHwsSerdesPowerCtrl(devNum, portGroup, MV_HWS_SERDES_NUM(curLanesList[i]), GT_FALSE, &serdesConfig));
            }
            break;
        default:
            return GT_NOT_SUPPORTED;
    }

    return GT_OK;
}

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
)
{
    MV_HWS_PORT_INIT_PARAMS curPortParams;
    GT_U32 pcsLaneNums;

#ifndef MV_HWS_REDUCED_BUILD
    if (hwsDeviceSpecInfo[devNum].ipcConnType == HOST2SERVICE_CPU_IPC_CONNECTION)
        return (mvHwsPortPcsActiveStatusGetIpc(devNum,portGroup, phyPortNum, portMode, numOfLanes));
#endif

    if (GT_OK != hwsPortModeParamsGetToBuffer(devNum, portGroup, phyPortNum, portMode, &curPortParams))
    {
        return GT_NOT_SUPPORTED;
    }

    CHECK_STATUS(mvHwsPcsActiveStatusGet(devNum, portGroup, phyPortNum, portMode, &pcsLaneNums));

    *numOfLanes = pcsLaneNums;

    return GT_OK;
}

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
    GT_U8                  devNum,
    GT_U32                 portGroup,
    GT_U32                 phyPortNum,
    MV_HWS_PORT_STANDARD   portMode
)
{
    MV_HWS_PORT_INIT_PARAMS   curPortParams;
    GT_UOPT macNum;

    if (GT_OK != hwsPortModeParamsGetToBuffer(devNum, portGroup, phyPortNum, portMode, &curPortParams))
    {
        CHECK_STATUS_EXT(GT_NOT_SUPPORTED, LOG_ARG_MAC_IDX_MAC(phyPortNum));
    }

    macNum = curPortParams.portMacNumber;

    /**************************************************/
    /* in BC3 the minimum interface width is 64 bits: */
    /* relevant for: SGMII/1000BaseX/SGMII2.5G/QSGMII */
    /**************************************************/
    if(((HWS_DEV_SILICON_TYPE(devNum) == Bobcat3) || (HWS_DEV_SILICON_TYPE(devNum) == Pipe)
        || (HWS_DEV_SILICON_TYPE(devNum) == Aldrin2)) &&
        (portMode <= QSGMII))
    {
        /* set P Sync Bypass */
        CHECK_STATUS(genUnitRegisterSet(devNum, portGroup, GEMAC_UNIT, macNum,
            GIG_PORT_MAC_CONTROL_REGISTER4, (1 << 6), (1 << 6)));
        return GT_OK;
    }

    if (portMode == SGMII2_5)
    {
        /* set P Sync Bypass */
        CHECK_STATUS(genUnitRegisterSet(devNum, portGroup, GEMAC_UNIT, macNum,
            GIG_PORT_MAC_CONTROL_REGISTER4, (1 << 6), (1 << 6)));
        return GT_OK;
    }

    /* unset P Sync Bypass */
    CHECK_STATUS(genUnitRegisterSet(devNum, portGroup, GEMAC_UNIT, curPortParams.portMacNumber,
        GIG_PORT_MAC_CONTROL_REGISTER4, 0, (1 << 6)));

    return GT_OK;
}

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
    GT_U32                     portGroup,
    GT_U32                     phyPortNum,
    MV_HWS_PORT_STANDARD    portMode,
    GT_BOOL                    *betterAlign90
)
{
    MV_HWS_PORT_INIT_PARAMS *curPortParams;
    GT_U32 curLanesList[HWS_MAX_SERDES_NUM];
    MV_HWS_DEV_FUNC_PTRS *hwsDevFunc;
    GT_U32 i;

    hwsDeviceSpecGetFuncPtr(&hwsDevFunc);

    if (hwsDevFunc[HWS_DEV_SILICON_INDEX(devNum)].portBetterAdapSetFunc == NULL)
    {
        return GT_OK;
    }

    *betterAlign90 = GT_FALSE;

    if ((phyPortNum >= HWS_CORE_PORTS_NUM(devNum)) ||
        ((GT_U32)portMode >= HWS_DEV_PORT_MODES(devNum)))
    {
        return GT_BAD_PARAM;
    }

    curPortParams = hwsPortModeParamsGet(devNum, portGroup, phyPortNum, portMode);

    /* rebuild active lanes list according to current configuration (redundancy) */
    CHECK_STATUS(mvHwsRebuildActiveLaneList(devNum, portGroup, phyPortNum, portMode, curLanesList));

    /* on each related serdes */
    for (i = 0; i < curPortParams->numOfActLanes; i++)
    {
        /* find a better Align90 adaptation value when temperature on device is changed from cold->hot or from hot->cold */
        CHECK_STATUS(hwsDevFunc[HWS_DEV_SILICON_INDEX(devNum)].portBetterAdapSetFunc(devNum, portGroup, (curLanesList[i] & 0xFFFF), betterAlign90));
    }

    return GT_OK;
}
#endif

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
)
{
   MV_HWS_PORT_INIT_PARAMS curPortParams;

   if ((phyPortNum >= HWS_CORE_PORTS_NUM(devNum)) ||
           ((GT_U32)portMode >= HWS_DEV_PORT_MODES(devNum)))
   {
       return GT_BAD_PARAM;
   }

#ifndef MV_HWS_REDUCED_BUILD
   if (hwsDeviceSpecInfo[devNum].ipcConnType == HOST2SERVICE_CPU_IPC_CONNECTION)
       return (mvHwsPortFlowControlStateSetIpc(devNum,portGroup, phyPortNum,portMode,fcState));
#endif

   if (GT_OK != hwsPortModeParamsGetToBuffer(devNum, portGroup, phyPortNum, portMode, &curPortParams))
   {
       return GT_NOT_SUPPORTED;
   }

   CHECK_STATUS(mvHwsMacFcStateCfg(devNum, portGroup, curPortParams.portMacNumber, portMode,
                                               curPortParams.portMacType, fcState));

   return GT_OK;
}
#ifndef MV_HWS_AVAGO_NO_VOS_WA
/**
* @internal mvHwsPortVosOverrideControlModeSet function
* @endinternal
*
* @brief   Set the override mode of the VOS parameters for all ports. If the override mode
*         is set to true, the VOS parameters will be overriden, if set to false, they will
*         not be overriden.
* @param[in] devNum                   - device number.
* @param[in] vosOverride              - GT_TRUE means override, GT_FALSE means no override.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPortVosOverrideControlModeSet
(
    GT_U8 devNum,
    GT_BOOL vosOverride
)
{
    CHECK_STATUS(mvHwsAvagoSerdesVosOverrideModeSet(devNum, vosOverride));
#ifndef MV_HWS_REDUCED_BUILD
    if (mvHwsServiceCpuEnableGet(devNum))
    {
        CHECK_STATUS(mvHwsPortVosOverrideModeSetIpc(devNum, vosOverride));
    }
#endif

    return GT_OK;
}

/**
* @internal mvHwsPortVosOverrideControlModeGet function
* @endinternal
*
* @brief   Set the override mode of the VOS parameters for all ports. If the override mode
*         is set to true, the VOS parameters will be overriden, if set to false, they will
*         not be overriden.
* @param[in] devNum                   - device number.
*
* @param[out] vosOverridePtr           - (pointer to) current VOS override mode.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPortVosOverrideControlModeGet
(
    GT_U8   devNum,
    GT_BOOL *vosOverridePtr
)
{
    CHECK_STATUS(mvHwsAvagoSerdesVosOverrideModeGet(devNum, vosOverridePtr));

    return GT_OK;
}
#endif /*#ifndef MV_HWS_AVAGO_NO_VOS_WA*/

#ifndef BV_DEV_SUPPORT
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
)
{
#ifndef BC2_DEV_SUPPORT /* not applicable for BC2 and AC3 */
    GT_U32      ptpResetMask;      /* ptp reset bits mask */
    GT_U32      ptpResetData;      /* ptp reset bits data */
#endif
    MV_HWS_PORT_INIT_PARAMS curPortParams;

    /* Currently The operation is supported on several devices, with have no
       errata nor external phys need furthur port enable implementation. */
    if (HWS_DEV_SILICON_TYPE(devNum) == Alleycat3A0)
    {
        return GT_OK;
    }

    /* Validity checks */
    if ((phyPortNum >= HWS_CORE_PORTS_NUM(devNum)) ||
            ((GT_U32)portMode >= HWS_DEV_PORT_MODES(devNum)))
    {
        return GT_BAD_PARAM;
    }

    /* Getting port params */
    if (GT_OK != hwsPortModeParamsGetToBuffer(devNum, portGroup, phyPortNum, portMode, &curPortParams))
    {
        return GT_NOT_INITIALIZED;
    }

    /* Checking MAC type so that 'mvHwsMacPortEnable()'  that is called next, will not be called with
       unsupported MAC after PTP (which is MAC type independant) was reset */
    switch (curPortParams.portMacType)
    {
        case GEMAC_X:
        case XGMAC:
        case XLGMAC:
        case CGMAC:
            break;
        default:
            return GT_NOT_SUPPORTED;
    }

#ifndef BC2_DEV_SUPPORT
    /* Performing port PTP reset
         Bobcat2:
            bit[1] PTP reset
         BobK, Aldrin, AC3X:
            bit[1] PTP TX reset, bit[13] PTP RX reset
         Bobcat3:
            bit[1] PTP TX reset, bit[8] PTP RX reset
    */

    /* Calculating PTP reset mask and bits */
    ptpResetData = 0;
    ptpResetMask = 2;
    /* First, PTP reset bit (or PTP TX reset bit on supported devices) */
    if (enable)
    {
        ptpResetData |= 2;
    }
    /* Second, PTP RX reset bit on supported devices */
    if ( HWS_DEV_SILICON_TYPE(devNum) == BobK || HWS_DEV_SILICON_TYPE(devNum) == Aldrin )
    {
        ptpResetMask |= 0x2000;
        if (enable)
        {
            ptpResetData |= 0x2000;
        }
    }
    else if ((HWS_DEV_SILICON_TYPE(devNum) == Bobcat3) || (HWS_DEV_SILICON_TYPE(devNum) == Pipe) )
    {
        ptpResetMask |= 0x100;
        if (enable)
        {
            ptpResetData |= 0x100;
        }
    }

    /* If the operation is Enable, first Unreset the PTP then perform MAC enable */
    if (enable)
    {
        CHECK_STATUS(genUnitRegisterSet(devNum, portGroup, PTP_UNIT, curPortParams.portMacNumber,
                PTP_PORT_CONTROL_REGISTER, ptpResetData, ptpResetMask));
    }
#endif /* ndef BC2_DEV_SUPPORT */

    /* Performing MAC port enable */
    CHECK_STATUS(mvHwsMacPortEnable(devNum, portGroup, curPortParams.portMacNumber,  portMode,
                                                curPortParams.portMacType, enable));
#ifndef BC2_DEV_SUPPORT
    /* If the operation is Disable, first disable the MAC, then reset PTP */
    if (!enable)
    {
        CHECK_STATUS(genUnitRegisterSet(devNum, portGroup, PTP_UNIT, curPortParams.portMacNumber,
                PTP_PORT_CONTROL_REGISTER, ptpResetData, ptpResetMask));
    }
#endif

    return GT_OK;
}
#endif

#if !defined(BC2_DEV_SUPPORT) && !defined(AC3_DEV_SUPPORT) && !defined(CM3) && defined(CHX_FAMILY)
/**
* @internal mvHwsPortTuneResCopyEnable function
* @endinternal
*
* @brief   Enable copy of auto tune results.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPortTuneResCopyEnable
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_BOOL                 enable
)
{
    /* avoid warnings */
    devNum = devNum;
    portGroup = portGroup;

    copyTuneResults = enable;

    return GT_OK;
}

/**
* @internal mvHwsPortSetEyeCheckEn function
* @endinternal
*
* @brief   Enables/Disabled eye check according to the receive value.
*         Actual configuration (to the device) will be done in mvHwsComHRev2SerdesPostTrainingConfig().
* @param[in] eyeCheckEn               - value for enables/disabled check eye
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPortSetEyeCheckEn
(
    GT_U8     devNum,
    GT_U32    portGroup,
    GT_BOOL   eyeCheckEn
)
{
    /* avoid warnings */
    devNum = devNum;
    portGroup = portGroup;

    return mvHwsComHRev2SerdesSetEyeCheckEn(eyeCheckEn);
}

/**
* @internal mvHwsPortSetPresetCommand function
* @endinternal
*
* @brief   Setting PRESET command according to the receive value.
*         If PRESET command configure to PRESET, eye check will disable.
*         Actual configuration (to the device) will be done in mvHwsComHRev2SerdesPostTrainingConfig().
* @param[in] presetCmdVal             - value for setting PRESET command:
*                                      1 = INIT
*                                      2 = PRESET
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPortSetPresetCommand
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    MV_HWS_PORT_PRESET_CMD  presetCmdVal
)
{
    /* avoid warnings */
    devNum = devNum;
    portGroup = portGroup;

    CHECK_STATUS(mvHwsComHRev2SerdesSetPresetCommand(presetCmdVal));

    /* in case Preset command == PRESET -> disable eye check */
    if (presetCmdVal == PRESET)
        return mvHwsComHRev2SerdesSetEyeCheckEn(GT_FALSE);

    return GT_OK;
}

#endif /* #if !defined(BC2_DEV_SUPPORT) && !defined(AC3_DEV_SUPPORT) && !defined(CM3) && defined(CHX_FAMILY) */

#ifndef MV_HWS_REDUCED_BUILD_EXT_CM3
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
)
{
    GT_U32 bitMapIndex;

    if ((HWS_DEV_SILICON_TYPE(devNum) == BobK) && (phyPortNum < 48))
    {
        phyPortNum /= 4;
    }

    bitMapIndex = phyPortNum / HWS_CTLE_BIT_MAP;

    if (ctleBiasOverride)
    {
        hwsDeviceSpecInfo[devNum].serdesInfo.ctleBiasOverride[bitMapIndex] |= (0x1 << (phyPortNum%HWS_CTLE_BIT_MAP));
    }
    else
    {
        hwsDeviceSpecInfo[devNum].serdesInfo.ctleBiasOverride[bitMapIndex] &= ~(0x1 << (phyPortNum%HWS_CTLE_BIT_MAP));
    }

    if (ctleBiasValue)
    {
        hwsDeviceSpecInfo[devNum].serdesInfo.ctleBiasValue[bitMapIndex] |= (0x1 << (phyPortNum%HWS_CTLE_BIT_MAP));
    }
    else
    {
        hwsDeviceSpecInfo[devNum].serdesInfo.ctleBiasValue[bitMapIndex] &= ~(0x1 << (phyPortNum%HWS_CTLE_BIT_MAP));
    }

    return GT_OK;
}

/**
* @internal mvHwsPortCtleBiasOverrideEnableGet function
* @endinternal
*
* @brief   read from hws dataBase the values of override mode and CTLE Bias.
*
* @param[in] devNum                   - device number.
* @param[in] phyPortNum               - physical port number.
*
* @param[out] ctleBiasOverridePtr      - pointer to override mode.
* @param[out] ctleBiasValuePtr         - pointer to CTLE Bias value.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPortCtleBiasOverrideEnableGet
(
    GT_U8      devNum,
    GT_U32     phyPortNum,
    GT_U32    *ctleBiasOverridePtr,
    GT_U32    *ctleBiasValuePtr
)
{
    GT_U32 bitMapIndex;

    if ((HWS_DEV_SILICON_TYPE(devNum) == BobK) && (phyPortNum < 48))
    {
        phyPortNum /= 4;
    }

    bitMapIndex = phyPortNum/HWS_CTLE_BIT_MAP;

    *ctleBiasOverridePtr = (hwsDeviceSpecInfo[devNum].serdesInfo.ctleBiasOverride[bitMapIndex] >> (phyPortNum%HWS_CTLE_BIT_MAP)) & 0x1;
    *ctleBiasValuePtr    = (hwsDeviceSpecInfo[devNum].serdesInfo.ctleBiasValue[bitMapIndex] >> (phyPortNum%HWS_CTLE_BIT_MAP)) & 0x1;

    return GT_OK;
}
#endif /* MV_HWS_REDUCED_BUILD_EXT_CM3 */

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
)
{
    GT_U16          tmpCheckingCounterArray[] = {0,0,0,0};
    GT_U16          i;
    GT_U16          macNum;
    GT_U16          serdesNum;
    GT_UREG_DATA    tempRegValue = 0;

    GT_UREG_DATA    serdesToMacTxRegValue[4][4] = /* [serdesNum][macNum] */
    { /*  mac0      mac1       mac2        mac3 */
        { 0x0<<0,   0,         0x1<<0,     0x2<<0  },   /*serdes0*/
        { 0,        0x0<<4,    0x1<<4,     0x2<<4  },   /*serdes1*/
        { 0x1<<8,   0x2<<8,    0x0<<8,     0       },   /*serdes2*/
        { 0x1<<12,  0x2<<12,   0,          0x0<<12 }    /*serdes3*/
    };

    GT_UREG_DATA    macToSerdesRxRegValue[4][4] = /* [macNum][serdesNum] */
    { /*  serdes0   serdes1    serdes2     serdes3 */
        { 0x0<<2,   0,         0x1<<2,     0x2<<2  },   /*mac0*/
        { 0,        0x0<<6,    0x1<<6,     0x2<<6  },   /*mac1*/
        { 0x1<<10,  0x2<<10,   0x0<<10,    0       },   /*mac2*/
        { 0x1<<14,  0x2<<14,   0,          0x0<<14 }    /*mac3*/
    };

    /* serdes muxing work only on ports 0-23 on Aldrin2*/
    if(!((HWS_DEV_SILICON_TYPE(devNum) == Aldrin2) && (phyPortNum < 24)))
    {
        return GT_NOT_SUPPORTED;
    }
    /* serdes muxing work only on ports that are multiple of 4 */
    if ((phyPortNum % 4) != 0)
    {
        return GT_NOT_SUPPORTED;
    }

    /* checking for legal connectivity for each serdes and that its in the range between 0-3
     mac lane 0 -> Serdes lane 0,2,3 Return GT_BAD_PARAM for 1
     mac lane 1 -> Serdes lane 1,2,3 Return GT_BAD_PARAM for 0
     mac lane 2 -> Serdes lane 0,1,2 Return GT_BAD_PARAM for 3
     mac lane 3 -> Serdes lane 0,1,3 Return GT_BAD_PARAM for 2 */
    if ((serdesToMacMuxStc->serdesLanes[0] == 1) || (serdesToMacMuxStc->serdesLanes[0] > 3))
    {
                return GT_BAD_PARAM;
    }
    if ((serdesToMacMuxStc->serdesLanes[1] == 0) || (serdesToMacMuxStc->serdesLanes[1] > 3))
    {
                return GT_BAD_PARAM;
    }
    if ((serdesToMacMuxStc->serdesLanes[2] == 3) || (serdesToMacMuxStc->serdesLanes[2] > 3))
    {
                return GT_BAD_PARAM;
    }
    if ((serdesToMacMuxStc->serdesLanes[3] == 2) || (serdesToMacMuxStc->serdesLanes[3] > 3))
    {
                return GT_BAD_PARAM;
    }

    /* checking validity of the values from the input: each number 0,1,2,3 should appear EXACTLY one time */
    for(i = 0; i < 4; i++)
    {   /* counting the number of occurrences of each of one the serdes numbers */
        tmpCheckingCounterArray[serdesToMacMuxStc->serdesLanes[i]]++;
    }

    for (i = 0; i < 4; i++)
    {
        if (tmpCheckingCounterArray[i] != 1)
        {
            return GT_BAD_PARAM;
        }
    }

    /* Aldrin2 Serdes Muxing has sillicon bug in the following permutations:
       mac[0,1,2,3] -> serdes[3,2,0,1]
       mac[0,1,2,3] -> serdes[2,3,1,0] */
    if((serdesToMacMuxStc->serdesLanes[0] == 3 && serdesToMacMuxStc->serdesLanes[1] == 2 && serdesToMacMuxStc->serdesLanes[2] == 0 && serdesToMacMuxStc->serdesLanes[3] == 1) ||
       (serdesToMacMuxStc->serdesLanes[0] == 2 && serdesToMacMuxStc->serdesLanes[1] == 3 && serdesToMacMuxStc->serdesLanes[2] == 1 && serdesToMacMuxStc->serdesLanes[3] == 0))
    {
       return GT_NOT_SUPPORTED;
    }

    for(macNum = 0 ; macNum < 4; macNum++)
    {
        serdesNum       = serdesToMacMuxStc->serdesLanes[macNum];

        tempRegValue    |= serdesToMacTxRegValue[serdesNum][macNum];
        tempRegValue    |= macToSerdesRxRegValue[macNum][serdesNum];
    }

    CHECK_STATUS(genUnitRegisterSet(devNum, portGroup, XLGMAC_UNIT, phyPortNum, MACRO_CONTROL, tempRegValue, 0xffff));
    CHECK_STATUS(genUnitRegisterSet(devNum, portGroup, GEMAC_UNIT,  phyPortNum, GIG_PORT_MAC_MACRO_CONTROL, tempRegValue, 0xffff));

    return GT_OK;
}

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
)
{
    GT_UREG_DATA    data;
    GT_U8           tmpCheckingCounterArray[] = {0,0,0,0};
    GT_U16          i;
    GT_U16          macNum;

    if (serdesToMacMuxStc == NULL)
    {
        return GT_BAD_PTR;
    }

    serdesToMacMuxStc->enableSerdesMuxing = GT_FALSE;
    if(!((HWS_DEV_SILICON_TYPE(devNum) == Aldrin2) && (phyPortNum < 24)))
    {
        return GT_NOT_SUPPORTED;
    }

    if ((phyPortNum % 4) != 0)
    {    /* serdes muxing work only on ports that are multiple of 4 */
        phyPortNum = phyPortNum & 0xFFFC; /* use the first port in the quad instead */
    }

    CHECK_STATUS(genUnitRegisterGet(devNum, portGroup, GEMAC_UNIT, phyPortNum, GIG_PORT_MAC_MACRO_CONTROL, &data, 0x0000FFFF));
    if (data == 0)
    {
        serdesToMacMuxStc->serdesLanes[0] = 0;
        serdesToMacMuxStc->serdesLanes[1] = 0;
        serdesToMacMuxStc->serdesLanes[2] = 0;
        serdesToMacMuxStc->serdesLanes[3] = 0;
        return GT_NOT_INITIALIZED;
    }

    /* Convert the data from the register */
    /*serdes 0*/
    if ((data & 0x3) == 0)
    {
        macNum = 0;
    }
    else if ((data & 0x3) == 1)
    {
        macNum = 2;
    }
    else
    {
        macNum = 3;
    }
    serdesToMacMuxStc->serdesLanes[macNum] = 0;

    /*serdes 1*/
    if (((data & 0x30) >> 4) == 0)
    {
        macNum = 1;
    }
    if (((data & 0x30) >> 4) == 1)
    {
        macNum = 2;
    }
    if ((((data & 0x30) >> 4) == 2)||(((data & 0x30) >> 4) == 3))
    {
        macNum = 3;
    }
    serdesToMacMuxStc->serdesLanes[macNum] = 1;

    /*serdes 2*/
    if (((data & 0x300) >> 8) == 0)
    {
        macNum = 2;
    }
    if (((data & 0x300) >> 8) == 1)
    {
        macNum = 0;
    }
    if ((((data & 0x300) >> 8) == 2)||(((data & 0x300) >> 8) == 3))
    {
        macNum = 1;
    }
    serdesToMacMuxStc->serdesLanes[macNum] = 2;

    /*serdes 3*/
    if (((data & 0x3000) >> 12) == 0)
    {
        macNum = 3;
    }
    if (((data & 0x3000) >> 12) == 1)
    {
        macNum = 0;
    }
    if ((((data & 0x3000) >> 12) == 2)||(((data & 0x3000) >> 12) == 3))
    {
        macNum = 1;
    }
    serdesToMacMuxStc->serdesLanes[macNum] = 3;

    /* checking validity of the values from resgisers: each number 0,1,2,3 should appear EXACTLY one time */
    for(i = 0; i < 4; i++)
    {   /* counting the number of occurrences of each of one the serdes numbers */
        tmpCheckingCounterArray[serdesToMacMuxStc->serdesLanes[i]]++;
    }
    for (i = 0; i < 4; i++)
    {
        if (tmpCheckingCounterArray[i] != 1)
        {
            serdesToMacMuxStc->serdesLanes[0] = 0;
            serdesToMacMuxStc->serdesLanes[1] = 0;
            serdesToMacMuxStc->serdesLanes[2] = 0;
            serdesToMacMuxStc->serdesLanes[3] = 0;
            return GT_NOT_INITIALIZED;
        }
    }

    serdesToMacMuxStc->enableSerdesMuxing = GT_TRUE;
    return GT_OK;
}

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
)
{
    GT_U8                           i;
    GT_U8                           serdesIndex;
    GT_U8                           serdesToMac[4];
    MV_HWS_PORT_SERDES_TO_MAC_MUX   macToSerdesStc;
    if( GT_OK != mvHwsPortLaneMacToSerdesMuxGet(devNum,portGroup,phyPortNum,&macToSerdesStc))
    {
        return serdesNum;
    }
    for(i = 0; i < 4 ; i++)
    {
        serdesToMac[macToSerdesStc.serdesLanes[i]] = i;
    }
    serdesIndex = serdesNum & 0x3;
    return (phyPortNum & 0xFFFC) /* first num in quad of MACs*/ + serdesToMac[serdesIndex] /* relative MAC num in the quad */;
}
