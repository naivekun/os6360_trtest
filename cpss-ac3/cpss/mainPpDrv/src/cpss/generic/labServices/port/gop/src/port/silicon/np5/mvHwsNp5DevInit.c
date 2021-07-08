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
* mvHwsLion2IfDevInit.c
*
* DESCRIPTION:
*       Lion2 specific HW Services init
*
* FILE REVISION NUMBER:
*       $Revision: 23 $
*
*******************************************************************************/
#include <gtOs/gtGenTypes.h>
#include <cpss/common/labServices/port/gop/common/siliconIf/mvSiliconIf.h>
#include <cpss/common/labServices/port/gop/common/siliconIf/siliconAddress.h>
#include <cpss/common/labServices/port/gop/port/serdes/comPhyH28nm/mvComPhyH28nmIf.h>
#include <cpss/common/labServices/port/gop/port/serdes/mvHwsSerdesPrvIf.h>
#include <trainingIp/mvDdr3TrainingIpPrvIf.h>
#include <trainingIp/silicon/mvHwsDdr3NP5.h>

#define MILOS5_SERDES_BASE      (0)
#define MILOS5_SERDES_PHY_BASE  (0x800)
#define SERDES_IND_OFFSET       (0)

/* init per device */
static GT_BOOL hwsInitDone[HWS_MAX_DEVICE_NUM] = {0};
static GT_BOOL np5DbInitDone = GT_FALSE;

#ifndef FreeRTOS
extern GT_STATUS hwsComPhyH28nmSeqInit(void);
extern GT_STATUS hwsComPhyH28nmRev3SeqInit(void);
#endif

/**
* @internal serdesIfInit function
* @endinternal
*
* @brief   Init all supported PCS types.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
static GT_STATUS serdesIfInit(MV_HWS_SERDES_TYPE type, GT_U32 devRevision)
{
#ifdef FreeRTOS
    MV_HWS_SERDES_FUNC_PTRS *hwsSerdesFuncsPtr;

    hwsSerdesGetFuncPtr(0, &hwsSerdesFuncsPtr);

    if ((devRevision == DevVersion_P1_A0)|| (devRevision == DevVersion_P1_A1)) {
        hwsOsPrintf("Serdes Rev A\n");
        CHECK_STATUS(mvHwsComH28nmIfInit(hwsSerdesFuncsPtr));
    } else if (devRevision == DevVersion_P2_B0) {
        hwsOsPrintf("Serdes Rev B\n");
        CHECK_STATUS(mvHwsComH28nmRev3IfInit(hwsSerdesFuncsPtr));
    } else {
        hwsOsPrintf("Serdes Rev C\n");
        CHECK_STATUS(mvHwsComH28nmRev3IfInit(hwsSerdesFuncsPtr));
    }
#else
    if ((devRevision == DevVersion_P1_A0)|| (devRevision == DevVersion_P1_A1)) {
        hwsOsPrintf("Serdes Rev A\n");
        CHECK_STATUS(hwsComPhyH28nmSeqInit());
    } else {
        hwsOsPrintf("Serdes Rev B\n");
        CHECK_STATUS(hwsComPhyH28nmRev3SeqInit());
    }
#endif
    return GT_OK;
}

/**
* @internal serdesIfClose function
* @endinternal
*
* @brief   Free all resources allocated by supported serdes types.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
static void serdesIfClose(MV_HWS_SERDES_TYPE type)
{

}

/**
* @internal TrainingIpIfInit function
* @endinternal
*
* @brief   Init training IP functionality.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
static void TrainingIpIfInit(GT_U32 devNum)
{
    MV_HWS_TRAINING_IP_FUNC_PTRS *funcPtrArray;
    GT_U32 board = 0;

    ddr3TipInitNp5(devNum, board);
    mvHwsTrainingIpGetFuncPtr(&funcPtrArray);
    mvHwsTrainingIpIfInit(funcPtrArray);
}


/**
* @internal hwsNp5IfInit function
* @endinternal
*
* @brief   Init all supported units needed for port initialization.
*         Must be called per device.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS hwsNp5IfInit(GT_U8 devNum, HWS_OS_FUNC_PTR *funcPtr)
{
    GT_U32  data, version;
    MV_HWS_DEV_TYPE devType = NP5A0;
    GT_U32 i;

    if (hwsInitDone[devNum]) {
        return GT_ALREADY_EXIST;
    }

    if (funcPtr == NULL)
        return GT_FAIL;

    if ((funcPtr->osTimerWkPtr == NULL) ||
        (funcPtr->osMemSetPtr == NULL) ||
        (funcPtr->osFreePtr == NULL) ||
        (funcPtr->osMallocPtr == NULL) ||
        (funcPtr->osExactDelayPtr == NULL) ||
        (funcPtr->sysDeviceInfo == NULL) ||
        (funcPtr->serdesRegSetAccess == NULL) ||
        (funcPtr->serdesRegGetAccess == NULL) ||
        (funcPtr->serverRegSetAccess == NULL) ||
        (funcPtr->serverRegGetAccess == NULL) ||
        (funcPtr->osMemCopyPtr == NULL) ||
        (funcPtr->registerSetAccess == NULL) ||
        (funcPtr->registerGetAccess == NULL))
            return GT_BAD_PARAM;

    /* Init configuration sequence executer */
    /*mvCfgSeqExecInit();*/

    hwsOsExactDelayPtr = funcPtr->osExactDelayPtr;
    hwsOsTimerWkFuncPtr = funcPtr->osTimerWkPtr;
    hwsOsMemSetFuncPtr = funcPtr->osMemSetPtr;
    hwsOsFreeFuncPtr = funcPtr->osFreePtr;
    hwsOsMallocFuncPtr = funcPtr->osMallocPtr;
    hwsSerdesRegSetFuncPtr = funcPtr->serdesRegSetAccess;
    hwsSerdesRegGetFuncPtr = funcPtr->serdesRegGetAccess;
    hwsServerRegSetFuncPtr = funcPtr->serverRegSetAccess;
    hwsServerRegGetFuncPtr = funcPtr->serverRegGetAccess;
    hwsRegisterSetFuncPtr = funcPtrsStc.registerSetAccess;
    hwsRegisterGetFuncPtr = funcPtrsStc.registerGetAccess;
    hwsOsMemCopyFuncPtr = funcPtr->osMemCopyPtr;

    /*
    hwsDeviceSpecInfo[devNum].portsNum = 12;
    hwsDeviceSpecInfo[devNum].lastSupPortMode = _100GBase_SR10;
    */

    /* get device id and version */
    funcPtr->sysDeviceInfo(devNum, &data, &version);
    hwsOsPrintf("Version data is 0x%x (version is 0x%x)\n", data, version);
    if (version == DevVersion_P3_C0) {
        hwsOsPrintf("Type: NP5C0\n");
        devType = NP5B0; /*C0 have a similar bihevior*/
    }else if (version == DevVersion_P2_B0) {
        hwsOsPrintf("Type: NP5B0\n");
        devType = NP5B0;
    } else {
        hwsOsPrintf("Type: NP5A0\n");
        devType = NP5A0;
    }

    hwsDeviceSpecInfo[devNum].devNum = devNum;
    hwsDeviceSpecInfo[devNum].devType = devType;
    hwsDeviceSpecInfo[devNum].gopRev = GOP_40NM_REV1;
    hwsDeviceSpecInfo[devNum].serdesType = COM_PHY_28NM;
    for(i = 0; i < MV_HWS_MAX_CPLL_NUMBER; i++)
    {
        hwsDeviceSpecInfo[devNum].serdesInfo.cpllInitDoneStatusArr[i] = GT_FALSE;
        hwsDeviceSpecInfo[devNum].serdesInfo.cpllCurrentOutFreqArr[i] = MV_HWS_MAX_OUTPUT_FREQUENCY;
    }
    hwsDeviceSpecInfo[devNum].serdesInfo.serdesNumber = 0;
    hwsDeviceSpecInfo[devNum].serdesInfo.serdesToAvagoMapPtr = NULL;
    hwsDeviceSpecInfo[devNum].serdesInfo.spicoNumber = 0;
    hwsDeviceSpecInfo[devNum].serdesInfo.avagoToSerdesMapPtr = NULL;
    hwsDeviceSpecInfo[devNum].serdesInfo.serdesVosOverride = GT_FALSE;
    hwsDeviceSpecInfo[devNum].serdesInfo.sbusAddrToSerdesFuncPtr = NULL;


    /* define Host to HWS connection type  - now Legacy mode*/
    hwsDeviceSpecInfo[devNum].ipcConnType = HOST2HWS_LEGACY_MODE;

    /* Init SERDES unit relevant for current device */
    CHECK_STATUS(serdesIfInit(hwsDeviceSpecInfo[devNum].devType, version));

    mvUnitInfoSet(hwsDeviceSpecInfo[devNum].devType, SERDES_UNIT, MILOS5_SERDES_BASE, SERDES_IND_OFFSET);
    mvUnitInfoSet(hwsDeviceSpecInfo[devNum].devType, SERDES_PHY_UNIT, MILOS5_SERDES_PHY_BASE, SERDES_IND_OFFSET);

    /* init Training IP functionality */
    TrainingIpIfInit(devNum);

    np5DbInitDone = GT_TRUE;
    hwsInitDone[devNum] = GT_TRUE;

    return GT_OK;
}

/**
* @internal hwsNp5IfClose function
* @endinternal
*
* @brief   Free all resource allocated for ports initialization.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
void hwsNp5IfClose(GT_U8 devNum)
{
    GT_U32  i;

    if (hwsInitDone[devNum]) {
        hwsInitDone[devNum] = GT_FALSE;
    }

    /* if there is no active device remove DB */
    for (i = 0; i < HWS_MAX_DEVICE_NUM; i++) {
        if (hwsInitDone[i]) {
        return;
        }
    }

    np5DbInitDone = GT_FALSE;

    for(i = 0; i < MV_HWS_MAX_CPLL_NUMBER; i++)
    {
        hwsDeviceSpecInfo[devNum].serdesInfo.cpllInitDoneStatusArr[i] = GT_FALSE;
        hwsDeviceSpecInfo[devNum].serdesInfo.cpllCurrentOutFreqArr[i] = MV_HWS_MAX_OUTPUT_FREQUENCY;
    }

    serdesIfClose(hwsDeviceSpecInfo[devNum].devType);
}


