/*******************************************************************************
*            Copyright 2001, Marvell International Ltd.
* This code contains confidential information of Marvell semiconductor, inc.
* no rights are granted herein under any patent, mask work right or copyright
* of Marvell or any third party.
* Marvell reserves the right at its sole discretion to request that this code
* be immediately returned to Marvell. This code is provided "as is".
* Marvell makes no warranties, express, implied or otherwise, regarding its
* accuracy, completeness or performance.
********************************************************************************
* mvHwsBobcatIfDevInit.c
*
* DESCRIPTION:
*       Bobcat specific HW Services init
*
* FILE REVISION NUMBER:
*       $Revision: 20 $
*
*******************************************************************************/
#include <FreeRTOS.h>
#include <portmacro.h>
#include <stdint.h>
#include "cli.h"
#include <FreeRTOS_CLI.h>

#include <cpss/common/labServices/port/gop/common/siliconIf/mvSiliconIf.h>
#include <cpss/common/labServices/port/gop/common/siliconIf/siliconAddress.h>
#include <cpss/common/labServices/port/gop/port/private/mvHwsPortPrvIf.h>
#include <cpss/common/labServices/port/gop/port/mvHwsPortCfgIf.h>
#include <cpss/common/labServices/port/gop/port/mvHwsIpcApis.h>
#include <cpss/common/labServices/port/gop/port/mvHwsServiceCpuFwIf.h>
#include <silicon/bobk/mvHwsBobKPortIf.h>

#include <cpss/common/labServices/port/gop/port/serdes/mvHwsSerdesPrvIf.h>
#include <mcdApiTypes.h>
#include <mcdApiRegs.h>
#include <mcdHwCntl.h>
#include <mcdInitialization.h>
#include <mcdDiagnostics.h>
#include <mcdAPI.h>
#include <mcdAPIInternal.h>
#include <mcdHwSerdesCntl.h>
#include <mcdInternalIpcApis.h>


#define BV_MPCS_BASE       (0x61180000)

#define BV_SERDES_BASE     (0x63000000)

#define IND_OFFSET (0x1000)

static MCD_BOOL hwsInitDone[HWS_MAX_DEVICE_NUM] = {0};
static MCD_BOOL BearValleyDbInitDone = MCD_FALSE;
static MV_HWS_PORT_INIT_PARAMS        bvPortParams;
MCD_SER_DEV  aaplSerdesDbDef[1];
MCD_SER_DEV_PTR aaplSerdesDb[1];

MCD_DEV mcdDevDb;

MCD_STATUS mcdInitDriverCm3(MCD_DEV_PTR pDev);

extern MCD_SERDES_CONFIG_DATA serdesElectricalParams[];
extern MCD_SERDES_TXRX_TUNE_PARAMS mcdSerdesTxRxTuneParamsArray[];
extern MCD_MAN_TUNE_TX_CONFIG_OVERRIDE_DB serdesTxApOverrideParams[8][2];

extern MCD_U32 hwsBvPortParamIndexGet(MCD_U8 devNum, MCD_U32 portGroup, MCD_U32 phyPortNum, MV_HWS_PORT_STANDARD portMode);

typedef MCD_STATUS (* MCD_PORT_CTRL_AP_HCD_FOUND)
(
    IN MCD_U32              devNum,
    IN MCD_U16              portNum,
    IN MV_HWS_PORT_STANDARD portMode
);

typedef MCD_STATUS (* MCD_PORT_CTRL_AP_TRX_TRAINING_END)
(
    IN MCD_U32              devNum,
    IN MCD_U16              portNum,
    IN MV_HWS_PORT_STANDARD portMode
);
typedef MCD_STATUS (* MCD_PORT_CTRL_AP_LINK_FALLING)
(
    IN MCD_U32              devNum,
    IN MCD_U16              portNum,
    IN MV_HWS_PORT_STANDARD portMode

);

typedef struct
{
    MCD_PORT_CTRL_AP_HCD_FOUND          apHcdFoundClbk; /* called when resolution is found */
    MCD_PORT_CTRL_AP_TRX_TRAINING_END   apTrxTrainingEndClbk; /* called after training completed */
    MCD_PORT_CTRL_AP_LINK_FALLING       apLinkFallingClbk; /* called when link is falling */

}MCD_PORT_CTRL_AP_CALLBACKS;

extern MCD_PORT_CTRL_AP_CALLBACKS mvPortCtrlApCallbacks;

extern MCD_STATUS mcdSerdesRxAutoTuneStatusShort
(
    MCD_DEV_PTR                     pDev,
    MCD_U8                          serdesNum,
    MCD_AUTO_TUNE_STATUS            *rxStatus
);

extern MCD_STATUS mcdSerdesEnhanceTuneLitePhase2
(
    MCD_DEV_PTR    pDev,
    MCD_U8      *serdesArr,
    MCD_U8     numOfSerdes,
    MCD_U8       *best_dly
);

extern MCD_STATUS mcdSerdesEnhanceTuneLitePhase1
(
    MCD_DEV_PTR       pDev,
    MCD_U8      *serdesArr,
    MCD_U8     numOfSerdes,
    MCD_SERDES_SPEED     serdesSpeed,
    MCD_U8       currentDelaySize,
    MCD_U8       *currentDelayPtr,
    MCD_U8       *inOutI,
    MCD_U16      *best_eye,
    MCD_U8       *best_dly,
    MCD_U8       *subPhase,
    MCD_U8    min_dly,
    MCD_U8    max_dly
);

extern MCD_STATUS mcdSerdesDefaultMinMaxDlyGet
(
    MCD_SERDES_SPEED         laneSpeed,
    MCD_U8              *min_dly_ptr,
    MCD_U8              *max_dly_ptr
);


extern MCD_STATUS mcdSerdesAutoTuneStatus
(
    MCD_DEV_PTR                     pDev,
    MCD_U8                          serdesNum,
    MCD_AUTO_TUNE_STATUS            *rxStatus,
    MCD_AUTO_TUNE_STATUS            *txStatus
);

extern MCD_STATUS mcdSerdesAutoTuneStartExt
(
    MCD_DEV_PTR                     pDev,
    MCD_U8                          serdesNum,
    MCD_RX_TRAINING_MODES           rxTraining,
    MCD_TX_TRAINING_MODES           txTraining
);

extern MCD_STATUS mcdSerdesAutoTuneStartExtAp
(
    MCD_DEV_PTR                     pDev,
    MCD_U8                          serdesNum,
    MCD_RX_TRAINING_MODES           rxTraining,
    MCD_TX_TRAINING_MODES           txTraining,
    MCD_U32                         laneSpeed,
    MCD_U32                         laneNum
);

extern MCD_STATUS mcdSerdesAutoTuneStatusShort
(
    MCD_DEV_PTR                     pDev,
    MCD_U8                          serdesNum,
    MCD_AUTO_TUNE_STATUS            *rxStatus,
    MCD_AUTO_TUNE_STATUS            *txStatus
);

MCD_STATUS mvBvPortManucalCtleConfigGet
(
    MCD_U8 serdesNum,
    MCD_MAN_TUNE_CTLE_CONFIG_DATA *ctleData
);

MCD_STATUS mvBvPortEnhanceTuneLiteByPhaseInitDb
(
    MCD_U8   devNum,
    MCD_U32  apPortIndex
);

MCD_STATUS mvBvPortManualCtleConfig
(
    MCD_U32 phyPortNum,
    MCD_U8  laneNum,
    MCD_U32 bandWidth,
    MCD_U32 dcGain,
    MCD_U32 highFrequency,
    MCD_U32 loopBandwidth,
    MCD_U32 lowFrequency,
    MCD_U32 squelch
);

MCD_STATUS mvBvPortEnhanceTuneLiteSetByPhase
(
    MCD_U8       devNum,
    MCD_U32      portGroup,
    MCD_U32      phyPortNum,
    MV_HWS_PORT_STANDARD    portMode,
    MCD_U8       phase,
    MCD_U8       *phaseFinishedPtr,
    MCD_U8       min_dly,
    MCD_U8       max_dly
);

GT_STATUS mvBvPortAvagoRxAutoTuneStateCheck
(
    GT_U8   devNum,
    GT_U32  portGroup,
    GT_U32  phyPortNum,
    MCD_OP_MODE    portMode,
    MCD_AUTO_TUNE_STATUS *rxTune
);

GT_STATUS mvApHcdFecParamsGet(GT_U8 portNum, GT_U8 *fecType);

/**
* @internal hwsBearValleyIfPreInit function
* @endinternal
*
* @brief   Init all supported units needed for port initialization.
*         Must be called per device.
* @param[in] funcPtr                  - pointer to structure that hold the "os"
*                                      functions needed be bound to HWS.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
MCD_STATUS hwsBearValleyIfPreInit
(
    MCD_U8 devNum,
    HWS_OS_FUNC_PTR *funcPtr
)
{

    if (funcPtr == NULL)
        return MCD_FAIL;

    if ((funcPtr->osTimerWkPtr == NULL) || (funcPtr->osMemSetPtr == NULL) ||
        (funcPtr->osFreePtr == NULL) || (funcPtr->osMallocPtr == NULL) ||
        (funcPtr->osExactDelayPtr == NULL) || (funcPtr->sysDeviceInfo == NULL) ||
        (funcPtr->osMemCopyPtr == NULL) ||
        (funcPtr->serdesRegSetAccess == NULL) ||(funcPtr->serdesRegGetAccess == NULL) ||
        (funcPtr->serverRegSetAccess == NULL) || (funcPtr->serverRegGetAccess == NULL) ||
        (funcPtr->registerSetAccess == NULL) || (funcPtr->registerGetAccess == NULL))
    {
        return MCD_STATUS_BAD_PTR;
    }

    hwsOsExactDelayPtr = funcPtr->osExactDelayPtr;
    hwsOsTimerWkFuncPtr = funcPtr->osTimerWkPtr;
    hwsOsMemSetFuncPtr = funcPtr->osMemSetPtr;
    hwsOsFreeFuncPtr = funcPtr->osFreePtr;
    hwsOsMallocFuncPtr = funcPtr->osMallocPtr;
    hwsSerdesRegSetFuncPtr = funcPtr->serdesRegSetAccess;
    hwsSerdesRegGetFuncPtr = funcPtr->serdesRegGetAccess;
    hwsServerRegSetFuncPtr = funcPtr->serverRegSetAccess;
    hwsServerRegGetFuncPtr = funcPtr->serverRegGetAccess;
    hwsRegisterSetFuncPtr = funcPtr->registerSetAccess;
    hwsRegisterGetFuncPtr = funcPtr->registerGetAccess;
    hwsOsMemCopyFuncPtr = funcPtr->osMemCopyPtr;
    hwsServerRegFieldSetFuncPtr = funcPtr->serverRegFieldSetAccess;
    hwsServerRegFieldGetFuncPtr = funcPtr->serverRegFieldGetAccess;

    hwsDeviceSpecInfo[devNum].devType = BearValley;

    return MCD_OK;
}

int mvHwsAvagoSerdesSpicoInterrupt
(
    unsigned char   devNum,
    unsigned int    portGroup,
    unsigned int    serdesNum,
    unsigned int    interruptCode,
    unsigned int    interruptData,
    int             *result
)
{
    return (mcdSerdesSpicoInterrupt(&mcdDevDb, serdesNum, interruptCode,interruptData,result));
}


GT_STATUS mcdSerdesPowerCtrlWr
(
    GT_U8                       devNum,
    GT_UOPT                     retimerMode,
    GT_UOPT                     serdesNum,
    GT_BOOL                     powerUp,
    MV_HWS_SERDES_CONFIG_STR    *serdesConfigPtr
)
{
   return( mcdSerdesPowerCtrl(&mcdDevDb, serdesNum, powerUp, serdesConfigPtr->baudRate, retimerMode, 0, 1));
}


#if 0
GT_STATUS mcdSerdesLoopbackWr
(
    GT_U8               devNum,
    GT_UOPT             portGroup,
    GT_UOPT             serdesNum,
    MCD_U16             lbMode
)
{

    return (mcdSerdesSetLoopback(&mcdDevDb, (serdesNum + 1),lbMode));
}
#endif
GT_STATUS mcdSerdesRxAutoTuneStartWr
(
    GT_U8   devNum,
    GT_UOPT portGroup,
    GT_UOPT serdesNum,
    GT_BOOL training
)
{
    return (mcdSerdesRxAutoTuneStart(&mcdDevDb, serdesNum,training));
}

GT_STATUS mcdSerdesTxAutoTuneStartWr
(
    GT_U8   devNum,
    GT_UOPT portGroup,
    GT_UOPT serdesNum,
    GT_BOOL training
)

{
    return (mcdSerdesTxAutoTuneStart(&mcdDevDb, serdesNum,training));
}

GT_STATUS mcdSerdesAutoTuneStartWr
(
    GT_U8   devNum,
    GT_UOPT portGroup,
    GT_UOPT serdesNum,
    GT_BOOL rxTraining,
    GT_BOOL txTraining
)
{
    return (mcdSerdesAutoTuneStart(&mcdDevDb, serdesNum,rxTraining,txTraining));
}

GT_STATUS mcdSerdesAutoTuneResultWr
(
    GT_U8                   devNum,
    GT_UOPT                 portGroup,
    GT_UOPT                 serdesNum,
    MV_HWS_AUTO_TUNE_RESULTS *results
)
{
    return (mcdSerdesAutoTuneResult(&mcdDevDb, serdesNum,(MCD_AUTO_TUNE_RESULTS*)results));
}
GT_STATUS mcdSerdesAutoTuneStatusWr
(
    GT_U8                   devNum,
    GT_UOPT                 portGroup,
    GT_UOPT                 serdesNum,
    MV_HWS_AUTO_TUNE_STATUS *rxStatus,
    MV_HWS_AUTO_TUNE_STATUS *txStatus
)
{
    return (mcdSerdesAutoTuneStatus(&mcdDevDb, serdesNum,(MCD_AUTO_TUNE_STATUS *)rxStatus,(MCD_AUTO_TUNE_STATUS *)txStatus));
}

GT_STATUS mcdSerdesResetWr
(
    GT_U8      devNum,
    GT_UOPT    portGroup,
    GT_UOPT    serdesNum,
    GT_BOOL    analogReset,
    GT_BOOL    digitalReset,
    GT_BOOL    syncEReset
)
{
    return (mcdSerdesResetImpl(&mcdDevDb,serdesNum,analogReset,digitalReset,syncEReset));
}

GT_STATUS mcdSerdesPolarityConfigWr
(
    GT_U8   devNum,
    GT_UOPT portGroup,
    GT_UOPT serdesNum,
    GT_BOOL invertTx,
    GT_BOOL invertRx
)
{
    return (mcdSerdesPolarityConfigImpl(&mcdDevDb,serdesNum,invertTx,invertRx));
}
#if 0
GT_STATUS mcdSerdesDigitalResetWr
(
    GT_U8               devNum,
    GT_UOPT             portGroup,
    GT_UOPT             serdesNum,
    MV_HWS_RESET        digitalReset
)
{
    return (mcdSerdesDigitalReset(&mcdDevDb,serdesNum,(MCD_BOOL)digitalReset));
}
#endif
GT_STATUS mcdSerdesAutoTuneStatusShortWr
(
    GT_U8                   devNum,
    GT_UOPT                 portGroup,
    GT_UOPT                 serdesNum,
    MV_HWS_AUTO_TUNE_STATUS    *rxStatus,
    MV_HWS_AUTO_TUNE_STATUS    *txStatus
)
{
    return (mcdSerdesAutoTuneStatusShort(&mcdDevDb,serdesNum,(MCD_AUTO_TUNE_STATUS *)rxStatus,(MCD_AUTO_TUNE_STATUS *)txStatus));
}

GT_STATUS mcdSerdesTxAutoTuneStopWr
(
    GT_U8    devNum,
    GT_UOPT  portGroup,
    GT_UOPT  serdesNum
)
{
    return( mcdSerdesTxAutoTuneStop(&mcdDevDb,serdesNum));
}

GT_STATUS mvHwsAvagoSerdesTxEnable
(
    GT_U8   devNum,
    GT_UOPT portGroup,
    GT_UOPT serdesNum,
    GT_BOOL enable
)
{
    return(mcdSerdesTxEnable(&mcdDevDb,serdesNum, enable));
}
/*******************************************************************************
* mvHwsPortTxEnable
*
* DESCRIPTION:
*       Turn of the port Tx according to selection.
*
* INPUTS:
*       devNum    - system device number
*       portGroup - port group (core) number
*       phyPortNum - physical port number
*       portMode   - port standard metric
*       enable     - enable/disable port Tx
*
* OUTPUTS:
*       Tuning results for recommended settings.
*
* RETURNS:
*       0  - on success
*       1  - on error
*
*******************************************************************************/


/**
* @internal mcdAvagoIfInit function
* @endinternal
*
* @brief   Init Avago Serdes IF functions.
*
* @param[in] devNum                   - system device number
* @param[in] funcPtrArray             - array for function registration
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mcdAvagoIfInit
(
    GT_U8  devNum,
    MV_HWS_SERDES_FUNC_PTRS *funcPtrArray
)
{
#ifdef ASIC_SIMULATION
    /* avoid warning */
    devNum = devNum;
#else
    aaplSerdesDb[devNum] = &(aaplSerdesDbDef[devNum]);
#endif


    /* Avago implementation */
    funcPtrArray[AVAGO].serdesArrayPowerCntrlFunc    = NULL;
    funcPtrArray[AVAGO].serdesPowerCntrlFunc         = mcdSerdesPowerCtrlWr;
    funcPtrArray[AVAGO].serdesManualRxCfgFunc        = NULL;
    funcPtrArray[AVAGO].serdesAutoTuneCfgFunc        = NULL;
    funcPtrArray[AVAGO].serdesRxAutoTuneStartFunc    = mcdSerdesRxAutoTuneStartWr;
    funcPtrArray[AVAGO].serdesTxAutoTuneStartFunc    = mcdSerdesTxAutoTuneStartWr;
    funcPtrArray[AVAGO].serdesAutoTuneStartFunc      = mcdSerdesAutoTuneStartWr;
    funcPtrArray[AVAGO].serdesAutoTuneResultFunc     = mcdSerdesAutoTuneResultWr;
    funcPtrArray[AVAGO].serdesAutoTuneStatusFunc     = mcdSerdesAutoTuneStatusWr;
    funcPtrArray[AVAGO].serdesAutoTuneStatusShortFunc = mcdSerdesAutoTuneStatusShortWr;
    funcPtrArray[AVAGO].serdesResetFunc              = mcdSerdesResetWr;
    funcPtrArray[AVAGO].serdesDigitalReset           = NULL;
    funcPtrArray[AVAGO].serdesCoreReset              = NULL;
#ifndef CO_CPU_RUN
    funcPtrArray[AVAGO].serdesSeqGetFunc             = NULL;
    funcPtrArray[AVAGO].serdesSeqSetFunc             = NULL;
    funcPtrArray[AVAGO].serdesDfeCfgFunc             = NULL;
    funcPtrArray[AVAGO].serdesLoopbackCfgFunc        = NULL;
    funcPtrArray[AVAGO].serdesLoopbackGetFunc        = NULL;
    funcPtrArray[AVAGO].serdesPpmCfgFunc             = NULL;
    funcPtrArray[AVAGO].serdesPpmGetFunc             = NULL;
    funcPtrArray[AVAGO].serdesTestGenFunc            = NULL;
    funcPtrArray[AVAGO].serdesTestGenGetFunc         = NULL;
    funcPtrArray[AVAGO].serdesTestGenStatusFunc      = NULL;
    funcPtrArray[AVAGO].serdesEomGetFunc             = NULL;
    funcPtrArray[AVAGO].serdesDfeStatusFunc          = NULL;
    funcPtrArray[AVAGO].serdesDfeStatusExtFunc       = NULL;
    funcPtrArray[AVAGO].serdesPolarityCfgFunc        = mcdSerdesPolarityConfigWr;
    funcPtrArray[AVAGO].serdesPolarityCfgGetFunc     = NULL;
    funcPtrArray[AVAGO].serdesDfeOptiFunc            = NULL;
    funcPtrArray[AVAGO].serdesFfeCfgFunc             = NULL;
    funcPtrArray[AVAGO].serdesCalibrationStatusFunc  = NULL;
    funcPtrArray[AVAGO].serdesTxEnableFunc           = mvHwsAvagoSerdesTxEnable;
    funcPtrArray[AVAGO].serdesTxEnableGetFunc        = NULL;
    funcPtrArray[AVAGO].serdesTxIfSelectFunc         = NULL;
    funcPtrArray[AVAGO].serdesTxIfSelectGetFunc      = NULL;
    funcPtrArray[AVAGO].serdesSqlchCfgFunc           = NULL;
    funcPtrArray[AVAGO].serdesAcTermCfgFunc          = NULL;
    funcPtrArray[AVAGO].serdesAutoTuneStopFunc       = mcdSerdesTxAutoTuneStopWr;
    funcPtrArray[AVAGO].serdesDfeCheckFunc           = NULL;
    funcPtrArray[AVAGO].serdesSpeedGetFunc           = NULL;
    funcPtrArray[AVAGO].serdesManualRxCfgGetFunc     = NULL;
    funcPtrArray[AVAGO].serdesManualTxCfgGetFunc     = NULL;
    funcPtrArray[AVAGO].serdesSignalDetectGetFunc    = NULL;
    funcPtrArray[AVAGO].serdesCdrLockStatusGetFunc   = NULL;
    funcPtrArray[AVAGO].serdesScanSamplerFunc        = NULL;
    funcPtrArray[AVAGO].serdesFixAlign90Start        = NULL;
    funcPtrArray[AVAGO].serdesFixAlign90Stop         = NULL;
    funcPtrArray[AVAGO].serdesFixAlign90Status       = NULL;
    funcPtrArray[AVAGO].serdesTypeGetFunc            = NULL;
#endif /* CO_CPU_RUN */

    return GT_OK;
}



/**
* @internal hwsBearValleySerdesIfInit function
* @endinternal
*
* @brief   Init all supported Serdes types.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
MCD_STATUS hwsBearValleySerdesIfInit(MCD_U8 devNum)
{
  MV_HWS_SERDES_FUNC_PTRS *hwsSerdesFuncsPtr;

  CHECK_STATUS(hwsSerdesGetFuncPtr(devNum, &hwsSerdesFuncsPtr));

  CHECK_STATUS(mcdAvagoIfInit(devNum, hwsSerdesFuncsPtr));

  return MCD_OK;
}
/*******************************************************************************/
MCD_STATUS mcdGetPortParams
(
    IN MCD_DEV_PTR pDev,
    IN MCD_U32 mdioPort,
    IN MCD_OP_MODE portMode,
    OUT MCD_U32  *laneSpeed,
    OUT MCD_U32 *numOfActLanes
)
{
    switch(portMode)
    {
        case MCD_MODE_P100L:
        case MCD_MODE_P100S:
        case MCD_MODE_P100C:
        case MCD_MODE_P100K:
        case MCD_MODE_R100L:
        case MCD_MODE_R100C:
            *laneSpeed = MCD_25_78125G;
            *numOfActLanes = 4;
            break;
        case MCD_MODE_P40L:
        case MCD_MODE_P40C:
        case MCD_MODE_P40S:
        case MCD_MODE_P40K:
        case MCD_MODE_R40L:
        case MCD_MODE_R40C:
            *laneSpeed = MCD_10_3125G;
            *numOfActLanes = 4;
            break;
        case MCD_MODE_G21L:
        case MCD_MODE_G21SK:
            *laneSpeed = MCD_10_3125G;
            *numOfActLanes = 4;
            break;

        case MCD_MODE_P50R2L:
        case MCD_MODE_P50R2S:
            *laneSpeed = MCD_25_78125G;
            *numOfActLanes = 2;
            break;

        case MCD_MODE_P40R2L:
        case MCD_MODE_P40R2S:
            *laneSpeed = MCD_20_625G;
            *numOfActLanes = 2;
            break;
        case MCD_MODE_P25L:
        case MCD_MODE_P25S:
            *laneSpeed = MCD_25_78125G;
            *numOfActLanes = 1;
            break;
        case MCD_MODE_P10L:
        case MCD_MODE_P10S:
        case MCD_MODE_R10L:
        case MCD_MODE_R10K:
            *laneSpeed = MCD_10_3125G;
            *numOfActLanes = 1;
            break;
        case MCD_MODE_P2_5:
        case MCD_MODE_P2_5S:
            *laneSpeed = MCD_3_125G;
            *numOfActLanes = 1;
            break;
        case MCD_MODE_P1:
        case MCD_MODE_R1L:
        case MCD_MODE_R1C:
            *laneSpeed = MCD_1_25G;
            *numOfActLanes = 1;
            break;
        default:
            hwsOsPrintf("mcdGetPortParams: incorrect mode selection: %u\n", portMode);
            return MCD_FAIL;
    }
    return MCD_OK;

}
MV_HWS_PORT_INIT_PARAMS * hwsPortModeParamsGet
(
   GT_U8                    devNum,
   GT_U32                   portGroup,
   GT_U32                   portNum,
   MV_HWS_PORT_STANDARD     portMode
)
{
    MCD_U32  laneSpeed,numOfActLanes,i;

   mcdGetPortParams(&mcdDevDb, portNum, portMode,&laneSpeed,&numOfActLanes);

   for (i = 0; i < numOfActLanes; i++)
   {
       bvPortParams.activeLanesList[i] = portNum + i;
   }
   bvPortParams.numOfActLanes = numOfActLanes;
   bvPortParams.serdesSpeed = laneSpeed;

   return &bvPortParams;
}

GT_STATUS hwsPortModeParamsGetToBuffer
(
   GT_U8                    devNum,
   GT_U32                   portGroup,
   GT_U32                   portNum,
   MV_HWS_PORT_STANDARD     portMode,
   MV_HWS_PORT_INIT_PARAMS  *portParamsBuffer
)
{
    MV_HWS_PORT_INIT_PARAMS *srcPortParams = hwsPortModeParamsGet(devNum, portGroup, portNum, portMode);
    if (NULL == srcPortParams || NULL == portParamsBuffer)
    {
        return GT_FAIL;
    }

    /* copy the port parameters into the buffer*/
    hwsOsMemCopyFuncPtr(portParamsBuffer, srcPortParams, sizeof(MV_HWS_PORT_INIT_PARAMS));
    return GT_OK;
}

GT_STATUS hwsPortModeParamsSet
(
   GT_U8                    devNum,
   GT_U32                   portGroup,
   GT_U32                   portNum,
   MV_HWS_PORT_INIT_PARAMS  *portParams
)
{

    mcdDevDb.apCfg[portNum].apLaneNum = portParams->firstLaneNum;
    mcdDevDb.portConfig[portNum].fecCorrect    = portParams->portFecMode;
    mcdDevDb.portConfig[portNum].portMode = portParams->portStandard;
    return GT_OK;
}


/**
* @internal mvHwsPortInit function
* @endinternal
*
* @brief   Init physical port. Configures the port mode and all it's elements
*         accordingly.
*         Does not verify that the selected mode/port number is valid at the
*         core level.
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
GT_STATUS mvHwsPortInit
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  phyPortNum,
    MV_HWS_PORT_STANDARD    portMode,
    MV_HWS_PORT_INIT_INPUT_PARAMS   *portInitInParamPtr
)
{

    return MCD_OK;
}

MCD_STATUS mcdSetPcsMode
(
     MCD_U16 mdioPort,
     MCD_U32 portMode,
     MCD_U16 host_or_line,
     MCD_FEC_TYPE fecCorrect
)
{
    MCD_U16 slice,chan;
    MCD_DEV_PTR pDev = &mcdDevDb;

    /* check port is valid */
    if (mcdIsPortValid(pDev, mdioPort) != MCD_OK)
    {
        printf("mcdSetPcsMode: incorrect port selection: %u\n", mdioPort);
        return MCD_FAIL;
    }

    slice = mdioPort / MCD_MAX_CHANNEL_NUM_PER_SLICE;

    chan = CHAN_IN_SLICE_NUM(mdioPort);
    if (host_or_line == MCD_LINE_SIDE)
        chan = chan + 4;
    switch(portMode)
    {
        case MCD_MODE_P100L:
        case MCD_MODE_P100S:
        case MCD_MODE_P100C:
        case MCD_MODE_P100K:
            if (((host_or_line == MCD_HOST_SIDE) && (chan != 0)) || /* 100G  only on chan 0*/
                ((host_or_line == MCD_LINE_SIDE) && (chan != 4)))
            {
                printf("mcdSetPcsMode: wrong channel number %u\n", chan);
                return MCD_FAIL;
            }
            CHECK_STATUS(mcdConfigurePcs100gR4(pDev, MCD_MODE_P100L, slice, chan, fecCorrect));
            break;
        case  MCD_MODE_P40L:
        case MCD_MODE_P40S:
        case  MCD_MODE_P40C:
        case  MCD_MODE_P40K:
            if (((host_or_line == MCD_HOST_SIDE) && (chan != 0)) || /* 40G  only on chan 0*/
                ((host_or_line == MCD_LINE_SIDE) && (chan != 4)))
            {
                printf("mcdSetPcsMode: wrong channel number %u side %d\n", chan, host_or_line);
                return MCD_FAIL;
            }
            CHECK_STATUS(mcdConfigurePcs40g(pDev, MCD_MODE_P40L, slice, chan, MCD_BASE_R4, fecCorrect));
            break;
        case  MCD_MODE_P25L:
        case  MCD_MODE_P25S:
           CHECK_STATUS(mcdConfigurePcs10_25gR1(pDev, slice, chan, fecCorrect, MCD_MODE_P25L));
           break;
        case MCD_MODE_P10L:
        case MCD_MODE_P10S:
            CHECK_STATUS(mcdConfigurePcs10_25gR1(pDev, slice, chan, fecCorrect, MCD_MODE_P10L));
            break;
        case MCD_MODE_P1:
            CHECK_STATUS(mcdConfigurePcs1gR1(pDev, slice, chan, 1/*BASEx*/, MCD_FALSE));
            break;
        case MCD_MODE_P50R2L:
        case MCD_MODE_P50R2S:
            CHECK_STATUS(mcdConfigurePcs40g(pDev, MCD_MODE_P50R2L, slice, chan, MCD_BASE_R2, fecCorrect));
            break;
        default:
            printf("mcdSetPcsMode: incorrect mode selection: %u\n", portMode);
            return MCD_FAIL;
    }

    return MCD_OK;
}

MCD_STATUS mvHwsBearValleySetLinePcsMode
(
     MCD_U8                devNum,
     MCD_U16               mdioPort,
     MCD_U32               portMode,
     MCD_FEC_TYPE          fecCorrect
)
{
    if ((portMode == MCD_MODE_P100S) || (portMode == MCD_MODE_P100C))
    {
        fecCorrect = MCD_RS_FEC;
    }
    CHECK_STATUS(mcdSetPcsMode(mdioPort, portMode, MCD_LINE_SIDE, fecCorrect));
    return MCD_OK;
}

/**
* @internal mvBvPortSerdesPowerUp function
* @endinternal
*
* @brief   power up the serdes lanes
*         assumes .parameters validation in the calling function
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
MCD_STATUS mvBvPortSerdesPowerUp
(
    MCD_U32  phyPortNum,
    MV_HWS_REF_CLOCK_SUP_VAL    refClock,
    MV_HWS_REF_CLOCK_SOURCE refClockSource,
    MCD_U32 laneSpeed,
    MCD_U32 numOfActLanes

)
{
    MCD_U32 i;
    MCD_U32 initAmpValue = 0;
    MCD_U32 initPreCursor = 0, initPostCursor = 0;
    MCD_U8  ctleBiasValue, index = 0;
    MCD_U8 txOverrideSup = 0;
    MCD_BOOL retimerEnable = MCD_FALSE;
                                                                    /* txAmp  emph0  emph1 */
    MV_HWS_TX_TUNE_PARAMS serdesTxTuneInitParams[2] = {/*_10_3125G   */{2,    18,     4},
                                                       /* _25_78125G */{1,     0,     6}};
    ctleBiasValue = mcdDevDb.serdesStatus.ctleBiasData[phyPortNum];
    if (laneSpeed == MCD_25_78125G)
    {
        index = 1;
        txOverrideSup = 1;
    }
    else if (laneSpeed == MCD_10_3125G)
    {
        index = 0;
        txOverrideSup = 1;
    }

    if (mcdDevDb.apCfg[phyPortNum].retimerMode)
    {
        retimerEnable = MCD_TRUE;
    }

    for (i = 0; i < numOfActLanes; i++)
    {
        mcdDevDb.serdesStatus.ctleBiasData[phyPortNum + i] = ctleBiasValue;
        if ((serdesTxApOverrideParams[phyPortNum + i][index].txOverrideBmp & MCD_MAN_TUNE_TX_CONFIG_AMP_CNS) && txOverrideSup)
        {
            serdesElectricalParams[phyPortNum + i].attenuation = serdesTxApOverrideParams[phyPortNum + i][index].serdesTxParams & 0x1F;
        }
        else
        {
             serdesElectricalParams[phyPortNum + i].attenuation = mcdSerdesTxRxTuneParamsArray[laneSpeed].attenuation;
        }
        if ((serdesTxApOverrideParams[phyPortNum + i][index].txOverrideBmp & MCD_MAN_TUNE_TX_CONFIG_EMPH0_CNS) && txOverrideSup)
        {
            serdesElectricalParams[phyPortNum + i].postCursor =
                ((serdesTxApOverrideParams[phyPortNum + i][index].serdesTxParams >> MCD_MAN_TX_TUNE_EMPH0_OVERRIDE_ENABLE_SHIFT) & 0x1F);
        }
        else
        {
            serdesElectricalParams[phyPortNum + i].postCursor   = mcdSerdesTxRxTuneParamsArray[laneSpeed].postCursor;
        }
        if ((serdesTxApOverrideParams[phyPortNum + i][index].txOverrideBmp & MCD_MAN_TUNE_TX_CONFIG_EMPH1_CNS) && txOverrideSup)
        {
            serdesElectricalParams[phyPortNum + i].preCursor =
                ((serdesTxApOverrideParams[phyPortNum + i][index].serdesTxParams >> MCD_MAN_TX_TUNE_EMPH1_OVERRIDE_ENABLE_SHIFT) & 0x1F);
        }
        else
        {
            serdesElectricalParams[phyPortNum + i].preCursor  = mcdSerdesTxRxTuneParamsArray[laneSpeed].preCursor;
        }
    }
    /* must be in additional loop to support lane remap */

    for (i = 0; i < numOfActLanes; i++)
    {
        CHECK_STATUS(mcdSerdesPowerCtrl(&mcdDevDb,(phyPortNum + i),MCD_FALSE, laneSpeed,retimerEnable, 0 /* primary clock */, 1 /* no division */));
    }
    for (i = 0; i < numOfActLanes; i++)
    {
        CHECK_STATUS(mcdSerdesPowerCtrl(&mcdDevDb,(phyPortNum + i),MCD_TRUE, laneSpeed,retimerEnable, 0 /* primary clock */, 1 /* no division */));
    }

    for (i = 0; i < numOfActLanes; i++)
    {
        /* set amplitude value for Initialize mode */
        if ((serdesTxApOverrideParams[phyPortNum + i][index].txOverrideBmp & MCD_MAN_TUNE_TX_CONFIG_AMP_CNS) && txOverrideSup)
        {
            initAmpValue = (0xA<<12) | (serdesTxApOverrideParams[phyPortNum + i][index].serdesTxParams & 0x1F);
        }
        else
        {
            initAmpValue   = (0xA<<12) | serdesTxTuneInitParams[index].txAmp;
        }
        CHECK_STATUS(mcdSerdesSpicoInterrupt(&mcdDevDb, (phyPortNum + i), 0x3D, initAmpValue, NULL));
        /* set emph1-Pre-cursor setting value for Initialize mode */
        if ((serdesTxApOverrideParams[phyPortNum + i][index].txOverrideBmp & MCD_MAN_TUNE_TX_CONFIG_EMPH1_CNS) && txOverrideSup)
        {
            initPreCursor  = (0x9<<12) | ((serdesTxApOverrideParams[phyPortNum + i][index].serdesTxParams >> MCD_MAN_TX_TUNE_EMPH1_OVERRIDE_ENABLE_SHIFT) & 0x1F);
        }
        else
        {
            initPreCursor = (0x9<<12) | serdesTxTuneInitParams[index].txEmph1;
        }
        CHECK_STATUS(mcdSerdesSpicoInterrupt(&mcdDevDb, (phyPortNum + i), 0x3D, initPreCursor, NULL));
        /* set emph0-Post-cursor setting value for Initialize mode */
         if ((serdesTxApOverrideParams[phyPortNum + i][index].txOverrideBmp & MCD_MAN_TUNE_TX_CONFIG_EMPH0_CNS) && txOverrideSup)
         {
             initPostCursor = (0xB<<12) |
                 ((serdesTxApOverrideParams[phyPortNum + i][index].serdesTxParams >> MCD_MAN_TX_TUNE_EMPH0_OVERRIDE_ENABLE_SHIFT) & 0x1F);
         }
         else
         {
             initPostCursor  = (0xB<<12) | serdesTxTuneInitParams[index].txEmph0;
         }
        CHECK_STATUS(mcdSerdesSpicoInterrupt(&mcdDevDb, (phyPortNum + i), 0x3D, initPostCursor, NULL));
    }
    return MCD_OK;
}

MCD_STATUS mvBvPortSerdesPowerDown
(
    MCD_U32  phyPortNum,
    MV_HWS_PORT_STANDARD    portMode,
    MCD_BOOL retimerMode
)
{
    MCD_U32 i,laneSpeed,numOfActLanes;
    MCD_U32 serdesArr[4] = {0};
    CHECK_STATUS(mcdGetPortParams(&mcdDevDb,phyPortNum,portMode,&laneSpeed,&numOfActLanes));
    CHECK_STATUS(mcdPortBuildLanesArray(phyPortNum,portMode,MCD_LINE_SIDE, serdesArr));
    CHECK_STATUS(mcdSerdesHaltDfeTraining(&mcdDevDb, serdesArr, (MCD_U16)numOfActLanes));
    for (i = 0; i < numOfActLanes; i++)
    {
        CHECK_STATUS(mcdSerdesPowerCtrl(&mcdDevDb,(phyPortNum + i),MCD_FALSE, laneSpeed,retimerMode, 0/* primary */, 1 /* no division */));
    }

    return MCD_OK;
}


/**
* @internal mvBvPortApSerdesPowerUp function
* @endinternal
*
* @brief   power up the serdes lanes for 802.3AP protocol.
*         assumes: parameters validation in the calling function
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - port standard metric
* @param[in] refClock                 - Reference clock frequency
* @param[in] refClock                 - Reference clock source line
* @param[in] curLanesList             - active Serdes lanes list according to configuration
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
MCD_STATUS mvBvPortApSerdesPowerUp
(
    MCD_U8   devNum,
    MCD_U32  portGroup,
    MCD_U32  phyPortNum,
    MV_HWS_PORT_STANDARD    portMode,
    MV_HWS_REF_CLOCK_SUP_VAL    refClock,
    MV_HWS_REF_CLOCK_SOURCE refClockSource,
    MCD_U32 *curLanesList
)
{
    MCD_U32 i,laneSpeed,numOfActLanes;

    CHECK_STATUS(mcdGetPortParams(&mcdDevDb,phyPortNum,portMode,&laneSpeed,&numOfActLanes));
    for (i = 0; i < numOfActLanes; i++)
    {
        CHECK_STATUS(mcdSerdesPowerCtrl(&mcdDevDb,(phyPortNum + i),MCD_TRUE, portMode,MCD_FALSE, 0/* primary */, 1/* no division */));
    }
    return MCD_OK;
}
/**
* @internal mvBvPortTxEnable function
* @endinternal
*
* @brief   Turn of the port Tx according to selection.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
MCD_STATUS mvBvPortTxEnable
(
   MCD_U32  phyPortNum,
   MV_HWS_PORT_STANDARD    portMode,
   MCD_BOOL enable
)
{
    MCD_U32 i,laneSpeed,numOfActLanes;
    CHECK_STATUS(mcdGetPortParams(&mcdDevDb,phyPortNum,portMode,&laneSpeed,&numOfActLanes));
    for (i = 0; i < numOfActLanes; i++)
        /* on each related serdes */
    {
           CHECK_STATUS(mcdSerdesTxEnable(&mcdDevDb,(phyPortNum + i), enable));
    }

   return MCD_OK;
}
GT_STATUS mvHwsPortTxEnable
(
   GT_U8   devNum,
   GT_U32  portGroup,
   GT_U32  phyPortNum,
   MV_HWS_PORT_STANDARD    portMode,
   GT_BOOL enable
)
{
    return (mvBvPortTxEnable (phyPortNum, portMode,enable));
}


/**
* @internal mvHwsBvApPortSignalDetectMask function
* @endinternal
*
* @brief   mask/unmask-signal detect interrupt
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
*                                      portMode     - port standard metric
* @param[in] enable                   -  / disable
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
MCD_STATUS mvHwsBvApPortSignalDetectMask
(
    MCD_U8                   devNum,
    MCD_U32                  portGroup,
    MCD_U32                  phyPortNum,
    MCD_U32                  laneSpeed,
    MCD_U32                  numOfActLanes,
    MCD_BOOL                 enable
)
{

    MCD_U32 i, phySerdesNum, serdesNum;
    /* get slice number */
    MCD_U32 slice = phyPortNum/4;
    for (i = 0; i < numOfActLanes; i++)
    {
        serdesNum = phyPortNum + i;
        /*convert logic serdes number to physical serdes number */
        MCD_FW_LOGIC_TO_PHYSICAL_SERDES_MAC(serdesNum,phySerdesNum);
        CHECK_STATUS(hwsRegisterSetFuncPtr(devNum, portGroup,  MCD_SERDES_EXTERNAL_CONFIGURATION_0(phySerdesNum), (enable << 7), (1 << 7)));
    }
    /*hwsOsPrintf("mvHwsBvApPortSignalDetectMask OK\n");*/
    return MCD_OK;
}


/**
* @internal mvHwsApPortInit function
* @endinternal
*
* @brief   Init port for 802.3AP protocol.
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
GT_STATUS mvHwsApPortInit
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  phyPortNum,
    MV_HWS_PORT_STANDARD    portMode,
    MV_HWS_PORT_INIT_INPUT_PARAMS   *portInitInParamPtr
)
{
    MCD_BOOL mask;
    MCD_U32 laneSpeed,numOfActLanes;
    MCD_FEC_TYPE fecType;
    MV_HWS_REF_CLOCK_SUP_VAL  refClock;
    MV_HWS_REF_CLOCK_SOURCE   refClockSource;
    if(NULL == portInitInParamPtr)
    {
       return MCD_FAIL;
    }
    refClock = portInitInParamPtr->refClock;
    refClockSource = portInitInParamPtr->refClockSource;


    CHECK_STATUS(mcdGetPortParams(&mcdDevDb,phyPortNum,portMode,&laneSpeed,&numOfActLanes));
    /* power up the serdes */
    mcdDevDb.portConfig[phyPortNum].portMode = portMode;
    CHECK_STATUS(mvBvPortSerdesPowerUp( phyPortNum, refClock, refClockSource,laneSpeed,numOfActLanes));
    mask = (portMode == 0x31) ? MCD_TRUE : MCD_FALSE;
    /* mask/unmask signal detect */
    CHECK_STATUS(mvHwsBvApPortSignalDetectMask(devNum, portGroup, phyPortNum, laneSpeed,numOfActLanes, mask));
    if (!mcdDevDb.apCfg[phyPortNum].retimerMode)
    {
        /* Configure MAC/PCS */
        CHECK_STATUS(mvApHcdFecParamsGet(phyPortNum, &fecType));

        CHECK_STATUS(mvHwsBearValleySetLinePcsMode(devNum, phyPortNum, (MCD_U32)portMode, fecType));
        /* Enable the Tx signal, the signal was disabled during Serdes init */
        CHECK_STATUS(mvBvPortTxEnable(phyPortNum, portMode, MCD_TRUE));
        if (mvPortCtrlApCallbacks.apHcdFoundClbk != NULL)
            mvPortCtrlApCallbacks.apHcdFoundClbk(0, phyPortNum, portMode);
    }
    else
    {
        if (mvPortCtrlApCallbacks.apHcdFoundClbk != NULL)
        {
           /* AP_PRINT_MAC(("mvHwsApPortInit port:%d  calling apHcdFoundClbk portMode %d \n",recvMsg->phyPortNum, portMode));*/
            mvPortCtrlApCallbacks.apHcdFoundClbk(0/*devNum*/, phyPortNum, portMode);

            /* On HCD found - config line port
              1.    Configure external serdes clock mux and data mux at line side
              2.    Check if host RX PLL is ready
              3.    Switch clock (int 30 then int 0x5) at line side
              4.    Wait 10 msec overall
              5.    Un_reset RF_RESET at line sides
            */
        }
    }
    return MCD_OK;
}

GT_STATUS mvHwsPortReset
(
    MCD_U8  devNum,
    MCD_U32 portGroup,
    MCD_U32 phyPortNum,
    MV_HWS_PORT_STANDARD    portMode,
    MV_HWS_PORT_ACTION  action
)
{
    CHECK_STATUS(mvBvPortSerdesPowerDown( phyPortNum,portMode,mcdDevDb.apCfg[phyPortNum].retimerMode));
    CHECK_STATUS(mcdPortReset(&mcdDevDb, phyPortNum, MCD_BOTH_SIDE, MCD_SOFT_RESET,0));
    return MCD_OK;
}
GT_STATUS mvApPortReset
(
    MCD_U32 phyPortNum
)
{
    /*hwsOsPrintf("mvApPortReset mode %d\n",mcdDevDb.portConfig[phyPortNum].portMode);*/
    CHECK_STATUS(mcdSerdesPowerCtrl(&mcdDevDb,phyPortNum ,MCD_FALSE, MCD_3_125G, MCD_FALSE, 0/* primary */, 1/* no division */));
    /*CHECK_STATUS(mcdPortReset(&mcdDevDb, phyPortNum, MCD_BOTH_SIDE, MCD_HARD_RESET,0));*/
    return MCD_OK;
}

int hwsBearValleyConvertSbusAddrToSerdes
(
    unsigned char devNum,
    MCD_U32 * serdesNum,
    unsigned int  sbusAddr
)
{
    if(sbusAddr > hwsDeviceSpecInfo[devNum].serdesInfo.spicoNumber)
    {
        return MCD_STATUS_BAD_PARAM;
    }
    if(sbusAddr < 1)
    {
        return MCD_STATUS_BAD_PARAM;
    }

    *serdesNum = sbusAddr - 1;
    return MCD_OK;
}
GT_STATUS mvBvSerdesAutoTuneStartExt
(
    MCD_U8  phyPortNum,
    MCD_OP_MODE    portMode,
    MCD_RX_TRAINING_MODES           rxTraining,
    MCD_TX_TRAINING_MODES           txTraining
)
{
    MCD_U32 i,laneSpeed,numOfActLanes;
    CHECK_STATUS(mcdGetPortParams(&mcdDevDb,phyPortNum,portMode,&laneSpeed,&numOfActLanes));
    for (i = 0; i < numOfActLanes; i++)
     /* on each related serdes */
    {
        if (laneSpeed == MCD_25_78125G)
        {
            /*Set PMD PRBS Sequence  based on per lane*/
            CHECK_STATUS(mcdSerdesSpicoInterrupt(&mcdDevDb, (phyPortNum + i), 0x3D, (0x3000 | i), NULL));
        }
        else
            /*Set PMD PRBS Sequence to default*/
            CHECK_STATUS(mcdSerdesSpicoInterrupt(&mcdDevDb, (phyPortNum + i), 0x3D, (0x3000 | 4), NULL));
        }
    for (i = 0; i < numOfActLanes; i++)
     /* on each related serdes */
    {
         CHECK_STATUS(mcdSerdesAutoTuneStartExt(&mcdDevDb,(phyPortNum + i), rxTraining, txTraining));
    }

   return MCD_OK;

}

GT_STATUS mvBvPortAvagoRxAutoTuneStateCheck
(
    GT_U8   devNum,
    GT_U32  portGroup,
    GT_U32  phyPortNum,
    MCD_OP_MODE    portMode,
    MCD_AUTO_TUNE_STATUS *rxTune
)
{
    MCD_U32 i,laneSpeed,numOfActLanes;
    MCD_AUTO_TUNE_STATUS rxStatus[4] = {0};
    CHECK_STATUS(mcdGetPortParams(&mcdDevDb,phyPortNum,portMode,&laneSpeed,&numOfActLanes));

    *rxTune = MCD_TUNE_PASS;
    for (i = 0; i < numOfActLanes; i++)
    {

        CHECK_STATUS(mcdSerdesRxAutoTuneStatusShort(&mcdDevDb, (phyPortNum + i), &rxStatus[i]));
        if (rxStatus[i] != MCD_TUNE_PASS)
        {
            *rxTune = rxStatus[i];
            return GT_OK;
        }
    }

    return GT_OK;
}

GT_STATUS mvBvPortAvagoTxAutoTuneStatus
(
    GT_U8          devNum,
    GT_U32         portGroup,
    GT_U32         phyPortNum,
    MCD_OP_MODE    portMode
)
{
    MCD_U32 i,laneSpeed,numOfActLanes;
    MCD_AUTO_TUNE_STATUS  txStatus;
    CHECK_STATUS(mcdGetPortParams(&mcdDevDb,phyPortNum,portMode,&laneSpeed,&numOfActLanes));
    for (i = 0; i < numOfActLanes; i++)
        /* on each related serdes */
    {
        CHECK_STATUS(mcdSerdesTxAutoTuneStatusShort(&mcdDevDb, (phyPortNum + i), &txStatus));

        if (txStatus == MCD_TUNE_FAIL)
        {
            return GT_FAIL;
        }
    }

    return GT_OK;
}

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
    if (txTune != NULL)
    {
        hwsOsPrintf("\n not implemented for txTune");
    }

    if (rxTune != NULL)
    {
        CHECK_STATUS(mvBvPortAvagoRxAutoTuneStateCheck(devNum,portGroup,phyPortNum,(MCD_OP_MODE)portMode, (MCD_AUTO_TUNE_STATUS*)rxTune));
    }

    return GT_OK;
}
GT_STATUS mvBvPortAvagoTxAutoTuneStateCheck
(
    GT_U8   devNum,
    GT_U32  portGroup,
    GT_U32  phyPortNum,
    MCD_OP_MODE    portMode,
    MCD_AUTO_TUNE_STATUS *txTune
)
{
    MCD_U32 i,laneSpeed,numOfActLanes;
    MCD_AUTO_TUNE_STATUS txStatus[4] = {0};
    CHECK_STATUS(mcdGetPortParams(&mcdDevDb,phyPortNum,portMode,&laneSpeed,&numOfActLanes));

    *txTune = MCD_TUNE_PASS;
    for (i = 0; i < numOfActLanes; i++)
    {

        CHECK_STATUS(mcdSerdesTxAutoTuneStatusShort(&mcdDevDb, (phyPortNum + i), &txStatus[i]));
        if (txStatus[i] != MCD_TUNE_PASS)
        {
            *txTune = txStatus[i];
            return GT_OK;
        }
    }

    return GT_OK;
}
GT_STATUS mvBvPortSerdesRxSignalCheck
(
    GT_U8   devNum,
    GT_U32  phyPortNum,
    MCD_OP_MODE    portMode,
    MCD_AUTO_TUNE_STATUS *resTune
)
{
    MCD_U32 i,laneSpeed,numOfActLanes;
    MCD_BOOL rxSignal = 0;

    CHECK_STATUS(mcdGetPortParams(&mcdDevDb,phyPortNum,portMode,&laneSpeed,&numOfActLanes));

    *resTune = MCD_TUNE_NOT_READY;
    for (i = 0; i < numOfActLanes; i++)
        /* on each related serdes */
    {
        CHECK_STATUS(mcdSerdesSignalDetectGet(&mcdDevDb, phyPortNum+i,&rxSignal));
        if (!rxSignal)
        {
           /*hwsOsPrintf("Port %d RxSignalCheck not detected\n",(phyPortNum + i));*/
           return GT_OK;
        }
    }

    *resTune = MCD_TUNE_READY;
    return GT_OK;

}
GT_STATUS mvBvPortAvagoDfeCfgSet
(
    GT_U8   devNum,
    GT_U8  phyPortNum,
    MCD_OP_MODE    portMode,
    MCD_DFE_MODE         dfeMode
)
{
    MCD_U32 i,laneSpeed,numOfActLanes;
    CHECK_STATUS(mcdGetPortParams(&mcdDevDb,phyPortNum,portMode,&laneSpeed,&numOfActLanes));
    for (i = 0; i < numOfActLanes; i++)
        /* on each related serdes */
    {
        CHECK_STATUS(mcdSerdesDfeConfig(&mcdDevDb,(phyPortNum + i), dfeMode));
    }

    return GT_OK;


}
/**
* @internal mcdBvPortAvagoTRxTuneStop function
* @endinternal
*
* @brief   Stop the TX training
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - mode of port
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
GT_STATUS mcdBvPortAvagoTRxTuneStop
(
    GT_U8   devNum,
    GT_U32  portGroup,
    GT_U32  phyPortNum,
    MV_HWS_PORT_STANDARD    portMode
)
{
    MCD_U32 i,laneSpeed,numOfActLanes;
    CHECK_STATUS(mcdGetPortParams(&mcdDevDb,phyPortNum,portMode,&laneSpeed,&numOfActLanes));
    for (i = 0; i < numOfActLanes; i++)
    /* on each related serdes */
    {
        CHECK_STATUS(mcdSerdesTxAutoTuneStop(&mcdDevDb,(phyPortNum + i)));
    }

    return GT_OK;
}



GT_STATUS mvBvPortAvagoAutoTuneSet
(
    GT_U8   devNum,
    GT_U32  portGroup,
    GT_U32  phyPortNum,
    MCD_OP_MODE        portMode,
    MV_HWS_PORT_AUTO_TUNE_MODE  portTuningMode,
    void                        *results
)
{
    MV_HWS_AUTO_TUNE_STATUS_RES *tuneRes;
    MCD_AUTO_TUNE_STATUS resTune;
    switch (portTuningMode)
    {
        case TRxTuneCfg:
            return GT_OK; /* TRxTuneCfg mode is not relevant for Avago Serdes */
        case RxTrainingOnly:
            CHECK_STATUS(mvBvPortAvagoDfeCfgSet(devNum,phyPortNum, portMode,DFE_ICAL));
            break;
        case TRxTuneStart:
            tuneRes = (MV_HWS_AUTO_TUNE_STATUS_RES*)results;
            tuneRes->rxTune = TUNE_PASS; /* rxTune is not relevant for TxTune mode */
            CHECK_STATUS(mvBvPortSerdesRxSignalCheck(devNum, phyPortNum, portMode, (MCD_AUTO_TUNE_STATUS*)&tuneRes->txTune));
            if((MCD_AUTO_TUNE_STATUS)tuneRes->txTune == MCD_TUNE_READY)
            {
                CHECK_STATUS(mvBvSerdesAutoTuneStartExt(phyPortNum, portMode, MCD_IGNORE_RX_TRAINING, MCD_START_TRAINING));
            }
            if ((MCD_AUTO_TUNE_STATUS)tuneRes->txTune == MCD_TUNE_NOT_READY)
            {
                return GT_OK;
            }
            break;
        case TRxTuneStatus:
            CHECK_STATUS(mvBvPortAvagoTxAutoTuneStatus(devNum, portGroup, phyPortNum, portMode));
            break;
        case TRxTuneStatusNonBlocking: /* for AP mode */
            tuneRes = (MV_HWS_AUTO_TUNE_STATUS_RES*)results;
            tuneRes->rxTune = MCD_TUNE_PASS; /* rxTune is not relevant for TxTune mode */
            CHECK_STATUS(mvBvPortAvagoTxAutoTuneStateCheck(devNum, portGroup, phyPortNum, portMode, &resTune));
            tuneRes->txTune = resTune;
            break;
        case TRxTuneStop:
            CHECK_STATUS(mcdBvPortAvagoTRxTuneStop(devNum, portGroup, phyPortNum, portMode));
            return GT_OK; /* TRxTuneStop mode is not relevant for AP mode in Avago Serdes */
        case RxTrainingAdative: /* running Adaptive pCal */
            CHECK_STATUS(mvBvPortAvagoDfeCfgSet(devNum,  phyPortNum, portMode, DFE_START_ADAPTIVE));
            break;
        case RxTrainingStopAdative:
            CHECK_STATUS(mvBvPortAvagoDfeCfgSet(devNum,  phyPortNum, portMode, DFE_STOP_ADAPTIVE));
            break;
        default:
            return GT_NOT_SUPPORTED;
    }

    return GT_OK;
}
GT_STATUS mvHwsPortAutoTuneSet
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  phyPortNum,
    MV_HWS_PORT_STANDARD    portMode,
    MV_HWS_PORT_AUTO_TUNE_MODE  portTuningMode,
    void *                      results
)
{
    GT_STATUS rc;
    int ret;
    rc = mvBvPortAvagoAutoTuneSet(devNum, portGroup,phyPortNum,(MCD_OP_MODE)portMode,portTuningMode,results);
    ret = aapl_get_return_code(mcdDevDb.serdesDev);
    if( ret < 0 )
        hwsOsPrintf("found aapl->return_code negative (%d) implying a prior error; value cleared\n",ret);

    return rc;
}

GT_STATUS mvHwsPortManualCtleConfig
(
    GT_U8                               devNum,
    GT_U32                              portGroup,
    GT_U32                              phyPortNum,
    MV_HWS_PORT_STANDARD                portMode,
    GT_U8                               laneNum,
    MV_HWS_MAN_TUNE_CTLE_CONFIG_DATA    *configParams
)
{
    CHECK_STATUS(mvBvPortManualCtleConfig(phyPortNum, laneNum, configParams->bandWidth, configParams->dcGain, configParams->highFrequency
                             , configParams->loopBandwidth, configParams->lowFrequency, configParams->squelch));
    return GT_OK;
}

GT_STATUS mvHwsPortManualCtleConfigGet
(
    GT_U8    devNum,
    GT_U32    portGroup,
    GT_U32    phyPortNum,
    MV_HWS_PORT_STANDARD portMode,
    GT_U8    laneNum,
    MV_HWS_MAN_TUNE_CTLE_CONFIG_DATA ctleData[]
)
{
    MCD_U32 laneSpeed,numOfActLanes;
    MCD_U32 i;
    CHECK_STATUS(mcdGetPortParams(&mcdDevDb, phyPortNum, (MCD_OP_MODE)portMode, &laneSpeed, &numOfActLanes));

    for (i = 0; i < numOfActLanes; i++)
    {
        CHECK_STATUS(mvBvPortManucalCtleConfigGet((MCD_U8)(i+phyPortNum), (MCD_MAN_TUNE_CTLE_CONFIG_DATA*)&ctleData[i]));
    }

    return GT_OK;
}


GT_STATUS mvHwsPortEnhanceTuneLiteByPhaseInitDb
(
    GT_U8                   devNum,
    GT_U32                  phyPortNum
)
{
    CHECK_STATUS(mvBvPortEnhanceTuneLiteByPhaseInitDb(devNum, phyPortNum));

    return GT_OK;
}


GT_STATUS mvHwsPortEnhanceTuneLiteSetByPhase
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  phyPortNum,
    MV_HWS_PORT_STANDARD    portMode,
    GT_U8       phase,
    GT_U8       *phaseFinishedPtr,
    GT_U8       min_dly,
    GT_U8       max_dly
)
{
    GT_STATUS rc;
    rc = mvBvPortEnhanceTuneLiteSetByPhase(devNum, portGroup, phyPortNum, (MCD_OP_MODE)portMode, phase, phaseFinishedPtr, min_dly, max_dly);
    return rc;
}

/**
* @internal mvBvPortEnhanceTuneLiteSetByPhase function
* @endinternal
*
* @brief   This function calls Enhance-Tune Lite algorithm phases.
*         The operation is relevant only for AP port with Avago Serdes
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - mode type of port
* @param[in] phase                    -  to perform
* @param[in] min_dly                  - Minimum delay_cal value: (rang: 0-31)
* @param[in] max_dly                  - Maximum delay_cal value: (rang: 0-31)
*
* @param[out] phaseFinishedPtr         - whether or not phase is finished
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
MCD_STATUS mvBvPortEnhanceTuneLiteSetByPhase
(
    MCD_U8       devNum,
    MCD_U32      portGroup,
    MCD_U32      phyPortNum,
    MV_HWS_PORT_STANDARD    portMode,
    MCD_U8       phase,
    MCD_U8       *phaseFinishedPtr,
    MCD_U8       min_dly,
    MCD_U8       max_dly
)
{
#ifdef ASIC_SIMULATION
    devNum = devNum;
    portGroup = portGroup;
    phyPortNum = phyPortNum;
    portMode = portMode;
    phase = phase;
    phaseFinishedPtr = phaseFinishedPtr;
#else
    MCD_STATUS rc = MCD_OK;
    MV_HWS_PORT_ENHANCE_TUNE_LITE_STC   *enhanceTuneLiteStcPtr;
    MCD_U8 default_min_dly, default_max_dly;
    MCD_U32 laneSpeed, numOfActLanes;
    MCD_U8 serdesNum;

    /* For AP port: delay values for EnhanceTuneLite configuration */
    MCD_U8  static_delay_10G[] = {25, 26, 28, 30}; /* best delay values for 10G speed */
    MCD_U8  static_delay_25G[] = {15, 17, 19, 21}; /* best delay values for 25G speed */

    enhanceTuneLiteStcPtr = &(hwsDeviceSpecInfo[devNum].portEnhanceTuneLiteDbPtr
                            [phyPortNum]);
    if (enhanceTuneLiteStcPtr->parametersInitialized == MCD_FALSE)
    {
        CHECK_STATUS(mvBvPortEnhanceTuneLiteByPhaseInitDb(devNum,
                            phyPortNum));
        enhanceTuneLiteStcPtr->parametersInitialized = MCD_TRUE;
    }

    CHECK_STATUS(mcdGetPortParams(&mcdDevDb,phyPortNum,(MCD_OP_MODE)portMode,&laneSpeed,&numOfActLanes));

    if(laneSpeed >= MCD_20_625G)
    {
        enhanceTuneLiteStcPtr->phase_CurrentDelayPtr = static_delay_25G;
        enhanceTuneLiteStcPtr->phase_CurrentDelaySize = sizeof(static_delay_25G)/sizeof(MCD_U8);
    }
    else
    {
        enhanceTuneLiteStcPtr->phase_CurrentDelayPtr = static_delay_10G;
        enhanceTuneLiteStcPtr->phase_CurrentDelaySize = sizeof(static_delay_10G)/sizeof(MCD_U8);
    }

    serdesNum = (MCD_U8)phyPortNum;

    CHECK_STATUS(mcdSerdesDefaultMinMaxDlyGet(laneSpeed, &default_min_dly, &default_max_dly));

    if ( (min_dly>>7) & 1 )
    {
        min_dly = min_dly & 0x7F;
    }
    else
    {
        min_dly = default_min_dly;
    }

    if ( (max_dly>>7) & 1 )
    {
        max_dly = max_dly & 0x7F;
    }
    else
    {
        max_dly = default_max_dly;
    }

    switch (phase)
    {
        case 1:
            rc = mcdSerdesEnhanceTuneLitePhase1(
                &mcdDevDb,
                &serdesNum,  /* equals to SerDes number */
                numOfActLanes,
                /* phase dependant args */
                laneSpeed,
                enhanceTuneLiteStcPtr->phase_CurrentDelaySize,
                enhanceTuneLiteStcPtr->phase_CurrentDelayPtr,
                &enhanceTuneLiteStcPtr->phase_InOutI,
                &(enhanceTuneLiteStcPtr->phase_Best_eye[0]),
                &(enhanceTuneLiteStcPtr->phase_Best_dly[0]),
                &(enhanceTuneLiteStcPtr->subPhase),
                min_dly,
                max_dly);
            if (rc != MCD_OK)
            {
                printf("enhanceTuneLitePhase1 failed\n");
                return MCD_FAIL;
            }
            else
            {
                if (enhanceTuneLiteStcPtr->phase_InOutI == 0xFF)
                {
                    *phaseFinishedPtr = GT_TRUE;
                }
                else
                {
                    *phaseFinishedPtr = GT_FALSE;
                }
            }
            break;

        case 2:
            rc =  mcdSerdesEnhanceTuneLitePhase2(
                &mcdDevDb,
                &serdesNum,
                numOfActLanes,
                /* phase dependant args */
                &(enhanceTuneLiteStcPtr->phase_Best_dly[0]));
            if (rc != MCD_OK)
            {
                printf("\n mcdSerdesEnhancedTuneLitePhase2 failed");
                return MCD_FAIL;
            }
            else
            {
                *phaseFinishedPtr = GT_TRUE;
            }
            break;

        default:
            hwsOsPrintf("\n EnhancedTuneLite: phase number is not correct \n");
        break;
    }
#endif

    return GT_OK;
}

MCD_STATUS mvBvPortManualCtleConfig
(
    MCD_U32 phyPortNum,
    MCD_U8  laneNum,
    MCD_U32 bandWidth,
    MCD_U32 dcGain,
    MCD_U32 highFrequency,
    MCD_U32 loopBandwidth,
    MCD_U32 lowFrequency,
    MCD_U32 squelch
)
{
    MCD_U8 serdesNum = laneNum+(MCD_U8)phyPortNum;


    CHECK_STATUS(mcdSerdesManualCtleConfig(&mcdDevDb, serdesNum, (MCD_U8)dcGain, (MCD_U8)lowFrequency,
                                               (MCD_U8)highFrequency, (MCD_U8)bandWidth,
                                               (MCD_U8)loopBandwidth, (MCD_U8)squelch));


    return MCD_OK;
}

MCD_STATUS mvBvPortManucalCtleConfigGet
(
    MCD_U8 serdesNum,
    MCD_MAN_TUNE_CTLE_CONFIG_DATA *ctleData
)
{

    CHECK_STATUS(mcdSerdesManualCtleConfigGet(&mcdDevDb, serdesNum, ctleData));
    return MCD_OK;
}



/**
* @internal mvBvPortEnhanceTuneLiteByPhaseInitDb function
* @endinternal
*
* @brief   Init HWS DB of EnhanceTuneLite by phase algorithm used for AP port
*
* @param[in] devNum                   - system device number
* @param[in] apPortIndex              - ap port number index
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
MCD_STATUS mvBvPortEnhanceTuneLiteByPhaseInitDb
(
    MCD_U8   devNum,
    MCD_U32  apPortIndex
)
{
#ifdef ASIC_SIMULATION
    devNum = devNum;
    apPortIndex = apPortIndex;
#else
    MCD_U32 i;

    hwsOsMemSetFuncPtr(&hwsDeviceSpecInfo[devNum].portEnhanceTuneLiteDbPtr[apPortIndex], 0, sizeof(MV_HWS_PORT_ENHANCE_TUNE_LITE_STC));
    /*hwsDeviceSpecInfo[devNum].portEnhanceTuneLiteDbPtr[apPortIndex].phase_CurrentDelaySize = 0;
    hwsDeviceSpecInfo[devNum].portEnhanceTuneLiteDbPtr[apPortIndex].phase_CurrentDelayPtr = 0;
    hwsDeviceSpecInfo[devNum].portEnhanceTuneLiteDbPtr[apPortIndex].phase_InOutI = 0;
    hwsDeviceSpecInfo[devNum].portEnhanceTuneLiteDbPtr[apPortIndex].subPhase = 0;
    hwsDeviceSpecInfo[devNum].portEnhanceTuneLiteDbPtr[apPortIndex].phaseEnhanceTuneLiteParams.phase = 0;
    hwsDeviceSpecInfo[devNum].portEnhanceTuneLiteDbPtr[apPortIndex].phaseEnhanceTuneLiteParams.phaseFinished = GT_FALSE;*/
    hwsDeviceSpecInfo[devNum].portEnhanceTuneLiteDbPtr[apPortIndex].parametersInitialized = GT_TRUE;

    for (i=0; i < MCD_MAX_CHANNEL_NUM_PER_SLICE; i++)
    {
        hwsDeviceSpecInfo[devNum].portEnhanceTuneLiteDbPtr[apPortIndex].phase_Best_eye[i] = 4;
        /*hwsDeviceSpecInfo[devNum].portEnhanceTuneLiteDbPtr[apPortIndex].phase_Best_dly[i] = 0;*/
    }
#endif

    return MCD_OK;
}

MCD_STATUS mvBvPortEnhanceTuneLitePhaseDeviceInit
(
    MCD_U8   devNum
)
{
#ifdef ASIC_SIMULATION
    devNum = devNum;
#else
    MCD_U32 phyPortNum;

    for (phyPortNum=0; phyPortNum < MCD_MAX_PORT_NUM; phyPortNum++)
    {
        CHECK_STATUS(mvBvPortEnhanceTuneLiteByPhaseInitDb(devNum, phyPortNum));
    }
#endif

    return MCD_OK;
}

/**
* @internal hwsBearValleyInit function
* @endinternal
*
* @brief   Init all supported units needed for port initialization.
*         Must be called per device.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
MCD_STATUS hwsBearValleyInit(MCD_U8 devNum, HWS_OS_FUNC_PTR *funcPtr)
{
    MCD_U32  data, version;
    MV_HWS_DEV_TYPE devType;
    MV_HWS_DEV_FUNC_PTRS *hwsDevFunc;

    if(BearValleyDbInitDone == MCD_FALSE)
    {
        CHECK_STATUS(hwsBearValleyIfPreInit(devNum, funcPtr));
    }


    hwsDeviceSpecInfo[devNum].devType = 0;
    hwsDeviceSpecInfo[devNum].devNum = 0;
    hwsDeviceSpecInfo[devNum].portsNum = 8;
    hwsDeviceSpecInfo[devNum].lastSupPortMode = NON_SUP_MODE;
    hwsDeviceSpecInfo[devNum].serdesType = 3; /*AVAGO*/;
    hwsDeviceSpecInfo[devNum].serdesInfo.serdesNumber = 15;
    hwsDeviceSpecInfo[devNum].serdesInfo.serdesToAvagoMapPtr = NULL;
    hwsDeviceSpecInfo[devNum].serdesInfo.spicoNumber = 16;
    hwsDeviceSpecInfo[devNum].serdesInfo.avagoToSerdesMapPtr = NULL;
    hwsDeviceSpecInfo[devNum].serdesInfo.sbusAddrToSerdesFuncPtr = hwsBearValleyConvertSbusAddrToSerdes;
    devType = hwsDeviceSpecInfo[devNum].devType;

    hwsDeviceSpecGetFuncPtr(&hwsDevFunc);

    if (hwsDevFunc[devType].deviceInfoFunc == NULL)
    {
        hwsDevFunc[devType].deviceInfoFunc = funcPtr->sysDeviceInfo;
    }

    /* define IPC connection Type Service CPU side */
    hwsDeviceSpecInfo[devNum].ipcConnType = HOST2HWS_LEGACY_MODE;


    /* get device id and version */
    hwsDevFunc[devType].deviceInfoFunc(devNum, &data, &version);

    hwsDeviceSpecInfo[devNum].serdesInfo.serdesVosOverride = MCD_FALSE;

    hwsDevFunc[devType].pcsMarkModeFunc = NULL;

    hwsDevFunc[devType].redundVectorGetFunc = NULL;

    /* set same as Bobk to avoid problems in getting port parameters definition*/
    hwsDeviceSpecInfo[devNum].gopRev = GOP_28NM_REV3;

    hwsDevFunc[devType].portExtModeCfgFunc     = NULL;                         /* TBD - Need to be implemented */
    hwsDevFunc[devType].portExtModeCfgGetFunc  = NULL;                         /* TBD - Need to be implemented */
    hwsDevFunc[devType].portParamsIndexGetFunc = hwsBvPortParamIndexGet;
    hwsDevFunc[devType].portLbSetWaFunc        = NULL;
    hwsDevFunc[devType].portAutoTuneStopFunc   = NULL;
    hwsDevFunc[devType].portAutoTuneSetWaFunc  = NULL;
    hwsDevFunc[devType].portFixAlign90Func     = NULL;

    mcdInitDriverCm3(&mcdDevDb);


    CHECK_STATUS(hwsBearValleySerdesIfInit(devNum));


    mvUnitInfoSet(devType, MMPCS_UNIT,  BV_MPCS_BASE,   0x400);
    mvUnitInfoSet(devType, SERDES_UNIT, BV_SERDES_BASE, 0x400);

    BearValleyDbInitDone = MCD_TRUE;
    hwsInitDone[devNum] = MCD_TRUE;

    CHECK_STATUS(mvBvPortEnhanceTuneLitePhaseDeviceInit(devNum));

    return MCD_OK;
}
#if 0
MV_HWA_AP_PORT_MODE mvHwsApConvertPortMode(MV_HWS_PORT_STANDARD portMode)
{
    switch (portMode)
    {
    case _1000Base_X:    return Port_1000Base_X;
    case _10GBase_KX4:   return Port_10GBase_KX4;
    case _10GBase_KR:    return Port_10GBase_KR;
    case _40GBase_KR:    return Port_40GBase_KR;
    case _100GBase_KR4:  return Port_100GBase_KR;

    default:           return MODE_NOT_SUPPORTED;
    }
}
#endif
MCD_STATUS mvHwsPcsReset
(
    MCD_U8                   devNum,
    MCD_U32                  portGroup,
    MCD_U32                  phyPortNum,
    MCD_U32                  pcsType,
    MV_HWS_RESET            action
)
{
    MCD_BOOL reset;
    if (action == UNRESET)
        reset = MCD_FALSE;
    else
        reset = MCD_TRUE;

    CHECK_STATUS(mcdChannelSwReset(&mcdDevDb, phyPortNum, reset));
    return MCD_OK;
}
MCD_STATUS mvHwsMacReset
(
    MCD_U8                   devNum,
    MCD_U32                  portGroup,
    MCD_U32                  macNum,
    MCD_U32                  macType,
    MV_HWS_RESET            action
)
{
    return MCD_OK;
}
MCD_STATUS mvHwsMacLoopbackCfg
(
    MCD_U8                   devNum,
    MCD_U32                  portGroup,
    MCD_U32                  macNum,
    MV_HWS_PORT_MAC_TYPE    macType,
    MV_HWS_PORT_LB_TYPE     lbType
)
{
    return MCD_OK;
}
MCD_STATUS mvHwsPcsLoopbackCfg
(
    MCD_U8                   devNum,
    MCD_U32                  portGroup,
    MCD_U32                  pcsNum,
    MV_HWS_PORT_PCS_TYPE    pcsType,
    MV_HWS_PORT_LB_TYPE     lbType
)
{

    switch (lbType)
    {
    case     DISABLE_LB:
        mcdSetLineLoopback(&mcdDevDb, pcsNum, MCD_DEEP_PCS_LB, 0);
        break;
    case     TX_2_RX_LB:
        mcdSetLineLoopback(&mcdDevDb, pcsNum, MCD_DEEP_PCS_LB, 0);
        break;
    default:
        printf("BV mvHwsPcsLoopbackCfg: incorrect loopback selection: %u\n", lbType);
        return MCD_FAIL;
    }
    return MCD_OK;
}
MCD_STATUS mvHwsPcsSignalDetectMaskSet
(
    MCD_U8                   devNum,
    MCD_U32                  portGroup,
    MCD_U32                  pcsNum,
    MV_HWS_PORT_PCS_TYPE    pcsType,
    MCD_BOOL                 maskEn
)
{
       return MCD_OK;
}

MCD_STATUS mvHwsMacLoopbackStatusGet
(
    MCD_U8                   devNum,
    MCD_U32                  portGroup,
    MCD_U32                  macNum,
    MV_HWS_PORT_MAC_TYPE    macType,
    MV_HWS_PORT_LB_TYPE     *lbType
)
{
    *lbType = DISABLE_LB;
    return MCD_OK;
}
MCD_STATUS mvHwsPcsLoopbackCfgGet
(
    MCD_U8                   devNum,
    MCD_U32                  portGroup,
    MCD_U32                  pcsNum,
    MV_HWS_PORT_PCS_TYPE    pcsType,
    MV_HWS_PORT_LB_TYPE     *lbType
)
{
        return MCD_STATUS_NOT_SUPPORTED;
}

GT_STATUS mvHwsPortLinkStatusGet
(
    GT_U8                  devNum,
    GT_U32                 portGroup,
    GT_U32                 portNum,
    MV_HWS_PORT_STANDARD   portMode,
    GT_BOOL                *linkStatus
)
{
    MCD_U32 lineRegAddr;
    MCD_U32 currentStatus;
    MCD_U32 slice, chan;

    slice = portNum / MCD_MAX_CHANNEL_NUM_PER_SLICE;
    chan  = CHAN_IN_SLICE_NUM(portNum);
    if (mcdDevDb.apCfg[portNum].retimerMode)
    {     /* repeater modes */
             mcdCheckSerdesSignalStability(&mcdDevDb, 3,slice, chan, 5, &currentStatus);
    }
    else
    {
        if (portMode == 0x31)
        {
            lineRegAddr = UMAC_LSMCPCS_LINE_REG_ADDR(UMAC_LSMCPCS_sts(slice,chan));
            mcdHwGetPhyRegField32bit(&mcdDevDb, portNum, lineRegAddr, 1, 1, &currentStatus);
        }
        else
        {
            lineRegAddr = UMAC_MCPCS_LINE_REG_ADDR(UMAC_MCPCS_BASE_R_and_10GBASE_T_PCS_Status1(slice,chan));
            mcdHwGetPhyRegField32bit(&mcdDevDb, portNum, lineRegAddr, 12, 1, &currentStatus);
        }
    }
    /*printf("%d mvHwsPortLinkStatusGet: : %x\n", portNum,currentStatus);*/

    if (currentStatus)
    {
        *linkStatus =  MCD_TRUE;
    }
    else
    {
        *linkStatus =  MCD_FALSE;
    }
    return MCD_OK;
}

MCD_STATUS mvHwsPcsFecConfig
(
    MCD_U8                   devNum,
    MCD_U32                  portGroup,
    MCD_U32                  pcsNum,
    MV_HWS_PORT_PCS_TYPE    pcsType,
    MCD_BOOL                 fecEn
)
{
       return MCD_OK;
}
MCD_STATUS mvHwsPcsFecConfigGet
(
    MCD_U8                   devNum,
    MCD_U32                  portGroup,
    MCD_U32                  pcsNum,
    MV_HWS_PORT_PCS_TYPE    pcsType,
    MCD_BOOL                 *fecEn
)
{
       *fecEn = MCD_FALSE;
       return MCD_OK;
}
MCD_STATUS mvHwsPcsActiveStatusGet
(
    MCD_U8                   devNum,
    MCD_U32                  portGroup,
    MCD_U32                  pcsNum,
    MV_HWS_PORT_PCS_TYPE    pcsType,
    MCD_U32                  *numOfLanes
)
{
    *numOfLanes = mcdPortGetLanesNum(&mcdDevDb, pcsNum, MCD_LINE_SIDE);
    return MCD_OK;
}
MCD_STATUS mvHwsMacActiveStatusGet
(
    MCD_U8                   devNum,
    MCD_U32                  portGroup,
    MCD_U32                  macNum,
    MV_HWS_PORT_MAC_TYPE    macType,
    MCD_U32                  *numOfLanes
)
{
       *numOfLanes = 0;
       return MCD_OK;
}
MCD_STATUS mvHwsMacFcStateCfg
(
    MCD_U8                           devNum,
    MCD_U32                          portGroup,
    MCD_U32                          macNum,
    MV_HWS_PORT_MAC_TYPE            macType,
    MV_HWS_PORT_FLOW_CONTROL_ENT    fcState
)
{
       return MCD_OK;
}


