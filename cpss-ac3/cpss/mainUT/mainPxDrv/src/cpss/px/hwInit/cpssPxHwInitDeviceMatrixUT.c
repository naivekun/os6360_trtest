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
* @file cpssPxHwInitDeviceMatrixUT.c
*
* @brief Unit tests for Device Matrix
*
* @version   1
********************************************************************************
*/
/* includes */

#include <cpss/px/config/private/prvCpssPxInfo.h>
#include <cpss/px/config/cpssPxCfgInit.h>
#include <cpss/px/cpssHwInit/cpssPxHwInit.h>
#include <cpssDriver/pp/config/prvCpssDrvPpCfgInit.h>

#include <utf/utfMain.h>
#include <utf/private/prvUtfExtras.h>
#include <common/tgfCommon.h>

#include <cpssCommon/cpssBuffManagerPool.h>

/* Include appDemo */
#include <appDemo/boardConfig/appDemoBoardConfig.h>
#include <appDemo/boardConfig/gtDbPxPipe.h>
#include <appDemo/sysHwConfig/gtAppDemoSysConfig.h>

#define SET_MATRIX_PTR_AND_SIZE_MAC(_ptrMatrixData,_matrixDataEntriesNum,_matrixTable) \
    _ptrMatrixData = &_matrixTable[0];                                          \
    _matrixDataEntriesNum = sizeof(_matrixTable)/sizeof(_matrixTable[0])

/* macro to convert : CPSS_98PXC401_CNS (0xC40111AB)  to be : 0xC401 */
#define DEVICE_ID_FROM_DEVICE_TYPE_MAC(devType) (0xFFFF & (devType >> 16))

#ifdef ASIC_SIMULATION
extern GT_VOID simCoreClockOverwrite_px
(
    GT_U32 simUserDefinedCoreClockMHz
);
#endif /*ASIC_SIMULATION*/


/**
* @struct PRV_UTF_DEVICE_MATRIX_DATA_STC
 *
 * @brief Holds Pp related data derived from the device matrix.
*/
typedef struct{

    /** HW device ID */
    GT_U32 deviceId;

    /** @brief buffer memory size
     *  transmitDescriptors - number of transmit descriptors
     */
    GT_U32 bufferMemory;

    GT_U32 transmitDescr;

    /** number of centralized counters */
    GT_U32 cnc;

    /** number of network ports */
    GT_U32 networkPorts;

    /** existing ports bmp */
    GT_U32 existingPortsBmp;

} PRV_UTF_DEVICE_MATRIX_DATA_STC;

static PRV_UTF_DEVICE_MATRIX_DATA_STC prvUtfDeviceMatrixDataForPipe[] =
{
     /* deviceId, */
     {  DEVICE_ID_FROM_DEVICE_TYPE_MAC(CPSS_98PX1012_CNS),
        /*bufferMemory, transmitDescr, cnc,  networkPorts, existingPortsBmp */
          16*_1M,        _8K,           2048, 0,            0xFFFF },
     /* deviceId, */
     {  DEVICE_ID_FROM_DEVICE_TYPE_MAC(CPSS_98PX1008_CNS),
        /*bufferMemory, transmitDescr, cnc,  networkPorts, existingPortsBmp */
          16*_1M,        _8K,           2048, 0,            0x3FF0 },
     /* deviceId, */
     {  DEVICE_ID_FROM_DEVICE_TYPE_MAC(CPSS_98PX1022_CNS),
        /*bufferMemory, transmitDescr, cnc,  networkPorts, existingPortsBmp */
          16*_1M,        _8K,           2048, 0,            0xA0F0 },
     /* deviceId, */
     {  DEVICE_ID_FROM_DEVICE_TYPE_MAC(CPSS_98PX1024_CNS),
        /*bufferMemory, transmitDescr, cnc,  networkPorts, existingPortsBmp */
          16*_1M,        _8K,           2048, 0,            0xFFF0 },
     {  DEVICE_ID_FROM_DEVICE_TYPE_MAC(CPSS_98PX1016_CNS),
        /*bufferMemory, transmitDescr, cnc,  networkPorts, existingPortsBmp */
          16*_1M,        _8K,           2048, 0,            0xFFFF },
     {  DEVICE_ID_FROM_DEVICE_TYPE_MAC(CPSS_98PX1015_CNS),
        /*bufferMemory, transmitDescr, cnc,  networkPorts, existingPortsBmp */
          16*_1M,        _8K,           2048, 0,            0xFFFF }
};

/**
* @internal prvCpssPxHwDeviceMatrixCheckResults function
* @endinternal
*
* @brief   Check device parameters according to device matrix file.
*
* @param[in] deviceType               - device type
* @param[in] matrixDataPtr            - (pointer to) device parameters list to
*                                      verify.
*                                       None
*/
static void prvCpssPxHwDeviceMatrixCheckResults
(
    IN CPSS_PP_DEVICE_TYPE             deviceType,
    IN PRV_UTF_DEVICE_MATRIX_DATA_STC  *matrixDataPtr
)
{
    GT_STATUS                st = GT_OK;
    GT_U8                    dev = prvTgfDevNum;
    CPSS_PX_CFG_DEV_INFO_STC devInfo;

    st = cpssPxCfgDevInfoGet(dev, &devInfo);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "Device get info for device 0x%x",
                                 deviceType);
    if( GT_OK != st )
    {
        return;
    }

    /* Buffer memory */
    UTF_VERIFY_EQUAL1_STRING_MAC(matrixDataPtr->bufferMemory,
                                 PRV_CPSS_PX_PP_MAC(dev)->fineTuning.tableSize.
                                    bufferMemory,
                                 "Buffer memory for device 0x%x",
                                 deviceType);

    /* Transmit descriptors */
    UTF_VERIFY_EQUAL1_STRING_MAC(matrixDataPtr->transmitDescr,
                                 PRV_CPSS_PX_PP_MAC(dev)->fineTuning.tableSize.
                                    transmitDescr,
                                 "Transmit descriptors for device 0x%x",
                                 deviceType);

    /* CNC */
    UTF_VERIFY_EQUAL1_STRING_MAC(matrixDataPtr->cnc,
                                 PRV_CPSS_PX_PP_MAC(dev)->fineTuning.tableSize.
                                 cncBlocks * PRV_CPSS_PX_PP_MAC(dev)->
                                    fineTuning.tableSize.cncBlockNumEntries,
                                    "CNC for device 0x%x", deviceType);

    /* compare the existing ports bmp */
    UTF_VERIFY_EQUAL2_STRING_MAC(
        matrixDataPtr->existingPortsBmp,
        devInfo.genDevInfo.existingPorts.ports[0],
        "expected in range of ports bmp[0x%8.8x] got [0x%8.8x] \n",
        matrixDataPtr->existingPortsBmp,/*expected*/
        devInfo.genDevInfo.existingPorts.ports[0] /*actual*/
    );
}

/**
* @internal prvCpssPxHwPipeDeviceMatrixTest function
* @endinternal
*
* @brief   Test Pipe parameters according to device matrix file.
*
* @param[in] matrixDataPtr            - (pointer to) device parameters list to
*                                      verify.
* @param[in] matrixDataEntriesNum     - number of matrix entries
*                                       None
*/
static void prvCpssPxHwPipeDeviceMatrixTest
(
    IN PRV_UTF_DEVICE_MATRIX_DATA_STC  *matrixDataPtr,
    IN GT_U32                          matrixDataEntriesNum
)
{
    GT_STATUS               st = GT_OK;
    GT_U32                  ii;
    CPSS_PP_DEVICE_TYPE     deviceType = 0x11AB;

    for( ii = 0 ; ii < matrixDataEntriesNum ; ii++ )
    {
        U32_SET_FIELD_MAC(deviceType, 16, 16, (matrixDataPtr->deviceId));
        /* Override device ID by value from device matrix */
        st = appDemoDebugDeviceIdSet(prvTgfDevNum, deviceType);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
            "prvCpssDrvDebugDeviceIdSet: entry %d, device 0x%x", ii,
            deviceType);
        if( GT_OK != st )
        {
            break;
        }

        /* Reset system for new device ID */
        st = prvTgfResetAndInitSystem();
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
            "prvTgfResetAndInitSystem: entry %d, device 0x%x", ii, deviceType);
        if( GT_OK != st )
        {
            break;
        }

        /* Verify device info according to matrix data */
        prvCpssPxHwDeviceMatrixCheckResults(deviceType, matrixDataPtr);

        matrixDataPtr++;

        /* wait until the reset is done */
        osTimerWkAfter(10);
    }
    appDemoDebugDeviceIdReset();
}

GT_STATUS px_getBoardInfoSimple(
    OUT CPSS_HW_INFO_STC *hwInfoPtr,
    OUT GT_PCI_INFO *pciInfo);

extern GT_STATUS   px_is_forced_port_list_ok(IN GT_U32    coreClock);
extern GT_STATUS   px_unforce_Ports(void);
extern GT_STATUS   px_force_Ports_50Gx4_0_4_12_14(void);
extern GT_STATUS   px_force_Ports_48HGS_0_4_8_12(void);
extern GT_STATUS   px_force_Ports_48HGS_0_4_12(void);
extern GT_STATUS   px_force_Ports_25G_12_13_14_15(void);
extern GT_STATUS   px_force_Ports_48HGS_0_4_8_100G_12(void);
extern GT_STATUS   px_force_Ports_25G_0_2_25G_12_13(void);
extern GT_STATUS   px_force_Ports_25G_0_2_4_6_25G_12_to_15(void);
extern GT_STATUS   px_force_Ports_12_5G_0_to_7_11_to_15(void);
extern GT_STATUS   px_force_Ports_10G_0_to_9_25G_12_to_15(void);
#define FUNC_AND_NAME(func) func , #func
typedef GT_STATUS (*PX_FORCE_PORTS_TO_USE)(void);
typedef struct {
    PX_FORCE_PORTS_TO_USE   forcFunc;
    GT_CHAR*                forcFunc_Name;
    GT_U32                  coreClocks[8];/* terminated by '0' */
}PX_FORCE_PORTS_TO_USE_STC;
static PX_FORCE_PORTS_TO_USE_STC    pxForcedPortsToUseArr[] =
{
     {  FUNC_AND_NAME(px_force_Ports_50Gx4_0_4_12_14             )     , {500,450,350,      0}}
    ,{  FUNC_AND_NAME(px_force_Ports_48HGS_0_4_8_12              )     , {500,450,          0}}
    ,{  FUNC_AND_NAME(px_force_Ports_48HGS_0_4_12                )     , {500,450,350,      0}}
    ,{  FUNC_AND_NAME(px_force_Ports_25G_12_13_14_15             )     , {        350,      0}}
    ,{  FUNC_AND_NAME(px_force_Ports_48HGS_0_4_8_100G_12         )     , {500,450,          0}}
    ,{  FUNC_AND_NAME(px_force_Ports_25G_0_2_25G_12_13           )     , {            288,  0}}
    ,{  FUNC_AND_NAME(px_force_Ports_25G_0_2_4_6_25G_12_to_15    )     , {        350,      0}}
    ,{  FUNC_AND_NAME(px_force_Ports_12_5G_0_to_7_11_to_15       )     , {        350,      0}}
    ,{  FUNC_AND_NAME(px_force_Ports_10G_0_to_9_25G_12_to_15     )     , {        350,      0}}

    /* must be last */
    ,{NULL, NULL ,{0}}
};

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssPxHwInitDeviceMatrixCheckTest
(
    IN GT_U8    devNum
)
*/
static void cpssPxHwInitDeviceMatrixCheckTest(IN GT_U8 dev)
{
    GT_STATUS                       st = GT_OK;
    CPSS_PX_CFG_DEV_INFO_STC        devInfo;
    PRV_UTF_DEVICE_MATRIX_DATA_STC  *ptrMatrixData;
    GT_U32                          matrixDataEntriesNum;
    CPSS_PP_DEVICE_TYPE             deviceType;
    GT_U32                          ii,jj;
    CPSS_PX_INIT_INFO_STC           initInfo;
    CPSS_PP_DEVICE_TYPE             px_devType;
    PRV_CPSS_GEN_PP_CONFIG_STC* devPtr = PRV_CPSS_PP_MAC(dev);
    GT_U32  queue;/*rx/tx queue*/
    GT_BOOL is_ASIC_SIMULATION;

    /* NOTE: the 350 and 288 tested explicitly at pxForcedPortsToUseArr[] */
    GT_U32  coreClockArr[] = {500,450, /*350,288*/     0/*0 must be last*/};
    GT_U32  currCoreClock;

    PRV_TGF_SKIP_TO_REDUCE_LOG_SIZE_MAC;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    if( GT_OK != prvUtfNextDeviceGet(&dev, GT_TRUE) )
    {
        return;
    }

    st = cpssPxCfgDevInfoGet(dev, &devInfo);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

#ifdef ASIC_SIMULATION
    is_ASIC_SIMULATION = GT_TRUE;
#else
    is_ASIC_SIMULATION = GT_FALSE;
#endif

    SET_MATRIX_PTR_AND_SIZE_MAC(ptrMatrixData, matrixDataEntriesNum,
        prvUtfDeviceMatrixDataForPipe);

    if(prvTgfResetModeGet() == GT_TRUE  && is_ASIC_SIMULATION == GT_TRUE)
    {
        for(ii = 0 ; coreClockArr[ii] > 0 ; ii++)
        {
            currCoreClock = coreClockArr[ii];
            cpssOsPrintf("====== start Testing Core Clock [%d] ========== \n",
                currCoreClock);
#ifdef ASIC_SIMULATION
            simCoreClockOverwrite_px(currCoreClock);
#endif /*ASIC_SIMULATION*/
            prvCpssPxHwPipeDeviceMatrixTest(ptrMatrixData,
                matrixDataEntriesNum);
        }

        /* start to check specific cases needed to be supported by specific core-clocks */
        for(ii = 0 ; pxForcedPortsToUseArr[ii].forcFunc != NULL ; ii++)
        {
            /* activate the specific port speed needed */
            pxForcedPortsToUseArr[ii].forcFunc();

            cpssOsPrintf("====== start Testing [%s] ========== \n",
                pxForcedPortsToUseArr[ii].forcFunc_Name);

            for(jj = 0 ; pxForcedPortsToUseArr[ii].coreClocks[jj] != 0 ; jj++)
            {
                currCoreClock = pxForcedPortsToUseArr[ii].coreClocks[jj];
                cpssOsPrintf("====== start Testing Core Clock [%d] ========== \n",
                    currCoreClock);

#ifdef ASIC_SIMULATION
                simCoreClockOverwrite_px(currCoreClock);
#endif /*ASIC_SIMULATION*/

                /* check that expected to PASS */
                (void)px_is_forced_port_list_ok(currCoreClock);

                prvCpssPxHwPipeDeviceMatrixTest(ptrMatrixData,
                    1);
            }
        }

        px_unforce_Ports();
        currCoreClock = 500;
#ifdef ASIC_SIMULATION
        simCoreClockOverwrite_px(currCoreClock);
#endif /*ASIC_SIMULATION*/

        return;
    }

    st = appDemoPxInitInfoGet(&initInfo);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

    /* Disable interrupts */
    PRV_CPSS_INT_SCAN_LOCK();

    /* clear buffers pools due to buffers allocated but not yet freed by former UTs*/
    for(queue = 0; queue < NUM_OF_TX_QUEUES; queue++)
    {
        if(devPtr->intCtrl.txDescList[queue].poolId)
        {
            st = cpssBmPoolReCreate(devPtr->intCtrl.txDescList[queue].poolId);
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);
        }
    }

    for( ii = 0 ; ii < matrixDataEntriesNum ; ii++ )
    {
        st = cpssPxCfgDevRemove(dev);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
            "cpssPxCfgDevRemove: entry %d, device 0x%x", ii,
            (ptrMatrixData->deviceId));

        if( GT_OK != st )
        {
            break;
        }

        deviceType = 0x11AB;
        U32_SET_FIELD_MAC(deviceType, 16, 16, (ptrMatrixData->deviceId));
        st = prvCpssDrvDebugDeviceIdSet(dev, deviceType);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
            "prvCpssDrvDebugDeviceIdSet: entry %d, device 0x%x", ii,
            deviceType);
        if( GT_OK != st )
        {
            break;
        }

        st = cpssPxHwInit(dev, &initInfo, &px_devType);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        prvCpssPxHwDeviceMatrixCheckResults(deviceType, ptrMatrixData);

        ptrMatrixData++;
    }

    if(is_ASIC_SIMULATION == GT_TRUE)
    {
        /* Enable interrupts */
        PRV_CPSS_INT_SCAN_UNLOCK();
    }
}
extern void cpssInitSystemGet
(
    OUT GT_U32  *boardIdxPtr,
    OUT GT_U32  *boardRevIdPtr,
    OUT GT_U32  *reloadEepromPtr
);

UTF_TEST_CASE_MAC(cpssPxHwInitDeviceMatrixCheckTest)
{
    GT_U8       dev;

    /* the test too hard for GM because it use multiple system
       resets each of them allocates more and more not freeable memory.
       There is no added value to check the test on GM also. */
    GM_NOT_SUPPORT_THIS_TEST_MAC

    /* skip test for systems with TM because it's already done without TM */
    PRV_TGF_SKIP_TEST_WHEN_TM_USED_MAC;

    if(prvUtfIsAc3xWithPipeSystem())
    {
        prvUtfSkipTestsSet();
        return;
    }

    /* reduce run of logic per single device of the family */
    prvUtfSetSingleIterationPerFamily();

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        cpssPxHwInitDeviceMatrixCheckTest(dev);
    }
}

/*----------------------------------------------------------------------------*/
/*
 * Configuration of cpssPxHwInitDeviceMatrix suit
 */
UTF_SUIT_BEGIN_TESTS_MAC(cpssPxHwInitDeviceMatrix)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxHwInitDeviceMatrixCheckTest)
UTF_SUIT_END_TESTS_MAC(cpssPxHwInitDeviceMatrix)

