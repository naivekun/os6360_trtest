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
* @file cpssTmNodesUpdateUT.c
*
* @brief Unit tests for cpssTmNodesUpdate.
*
* @version   2
********************************************************************************
*/

#include <cpss/generic/tm/cpssTmCtl.h>
#include <cpss/generic/tm/cpssTmDrop.h>
#include <cpss/generic/tm/cpssTmSched.h>
#include <cpss/generic/tm/cpssTmShaping.h>
#include <cpss/generic/tm/cpssTmNodesCreate.h>
#include <cpss/generic/tm/cpssTmNodesTree.h>
#include <cpss/generic/tm/cpssTmNodesUpdate.h>
#include <cpss/generic/tm/cpssTmNodesRead.h>
#include <cpssCommon/private/prvCpssMath.h>
#include <cpss/generic/tm/cpssTmUtDefs.h>
#include <utf/utfMain.h>
#include <utf/private/prvUtfExtras.h>




/*******************************************************************************
GT_STATUS cpssTmAnodeUpdate
(
    IN GT_U8                            devNum,
    IN GT_U32                           index,
    IN CPSS_TM_A_NODE_PARAMS_STC        *paramsPtr
)
*/
UTF_TEST_CASE_MAC(cpssTmAnodeUpdate)
{
    GT_STATUS                   st = GT_OK;

    GT_U8                       dev;
    GT_U32                      portInd;
    CPSS_TM_A_NODE_PARAMS_STC   aParamsPtr;
    CPSS_TM_B_NODE_PARAMS_STC   bParamsPtr;
    CPSS_TM_C_NODE_PARAMS_STC   cParamsPtr;
    GT_U32                      aNodeIndPtr = 1;
    GT_U32                      bNodeIndPtr;
    GT_U32                      cNodeIndPtr;
    CPSS_TM_PORT_PARAMS_STC     paramsPtr;
    GT_U32                      cNodesNum;
    GT_U32                      bNodesNum;
    GT_U32                      aNodesNum;
    GT_U32                      queuesNum;
    CPSS_PP_FAMILY_TYPE_ENT     devFamily;
    GT_U32                      ii;
    CPSS_TM_QUANTUM_LIMITS_STC  cpssQuantumLimits;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev,
                                 UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_LION_E |
                                     UTF_PUMA2_E | UTF_PUMA3_E | UTF_XCAT2_E | UTF_LION2_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E | UTF_ALDRIN_E | UTF_AC3X_E);

    cpssOsBzero((GT_VOID*)&aParamsPtr, sizeof(aParamsPtr));
    cpssOsBzero((GT_VOID*)&bParamsPtr, sizeof(bParamsPtr));
    cpssOsBzero((GT_VOID*)&cParamsPtr, sizeof(cParamsPtr));

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* get device family */
        st = prvUtfDeviceFamilyGet(dev, &devFamily);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "prvUtfDeviceFamilyGet: %d", dev);

        /* TM initialization */

        st = cpssTmTestTmCtlLibInit(dev);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        st = cpssTmTestPeriodicSchemeConfiguration(dev);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* Configure parameters for node A */
        aParamsPtr.shapingProfilePtr = 0;
        aParamsPtr.quantum = 0x40;
        for (ii=0; ii<8; ii++)
            aParamsPtr.schdModeArr[ii] = CPSS_TM_SCHD_MODE_RR_E; /* No DWRR configured*/
        aParamsPtr.dropProfileInd = 0;
        aParamsPtr.eligiblePrioFuncId = 8;

        /* Configure parameters for node B */
        bParamsPtr.shapingProfilePtr = 0;
        bParamsPtr.quantum = 0x40;
        for (ii=0; ii<8; ii++)
            bParamsPtr.schdModeArr[ii] = CPSS_TM_SCHD_MODE_RR_E; /* No DWRR configured*/
        bParamsPtr.dropProfileInd = 0;
        bParamsPtr.eligiblePrioFuncId = 9;

        /* Configure parameters for node C */
        cParamsPtr.shapingProfilePtr = 0;
        cParamsPtr.quantum = 0x40;
        cParamsPtr.dropCosMap = 1;
        cParamsPtr.eligiblePrioFuncId = 9;
        for (ii=0; ii<8; ii++){
            cParamsPtr.schdModeArr[ii] = CPSS_TM_SCHD_MODE_RR_E;
            cParamsPtr.dropProfileIndArr[ii] = 0;
        }

        /* Configure parameters for port */
        paramsPtr.cirBw = 1000000;
        paramsPtr.eirBw = 0;
        paramsPtr.cbs = 8; /*KBytes*/
        paramsPtr.ebs = 0x1FFFF;
        paramsPtr.eligiblePrioFuncId = CPSS_TM_ELIG_N_MIN_SHP_E;
        for(ii=0; ii<8; ii++){
            paramsPtr.schdModeArr[ii] = CPSS_TM_SCHD_MODE_RR_E;
            paramsPtr.quantumArr[ii] = UT_TM_MIN_PORT_QUANTUM_CNS;
        }
        paramsPtr.dropProfileInd = CPSS_TM_NO_DROP_PROFILE_CNS;

        portInd=1;
        cNodesNum=1;
        bNodesNum=1;
        aNodesNum=1;
        queuesNum=1;

        st = cpssTmPortCreate(dev, portInd, &paramsPtr, cNodesNum, bNodesNum, aNodesNum,queuesNum );
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        st = cpssTmAnodeToPortCreate(dev, portInd, &aParamsPtr, &bParamsPtr, &cParamsPtr, &aNodeIndPtr, &bNodeIndPtr, &cNodeIndPtr );
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*1.1 Check with legal parameters.
                Expected: GT_OK */

        st = cpssTmAnodeUpdate(dev, aNodeIndPtr, &aParamsPtr);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

         /*
            1.2 check out of range aParamsPtr.shapingProfilePtr.
            Call with aParamsPtr.shapingProfilePtr [85580], other parameters same as in 1.1.
            Expected: NON GT_OK.
        */

        aParamsPtr.shapingProfilePtr = 85580;

        st = cpssTmAnodeUpdate(dev, aNodeIndPtr, &aParamsPtr);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, aParamsPtr.shapingProfilePtr);

        aParamsPtr.shapingProfilePtr= 0;


         /*
            1.3 check out of range aParamsPtr.quantum.
            Call with aParamsPtr.quantum [0x4fff], other parameters same as in 1.1.
            Expected: NON GT_OK.
        */

        aParamsPtr.quantum=0x4fff;

        st = cpssTmAnodeUpdate(dev, aNodeIndPtr, &aParamsPtr);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, aParamsPtr.quantum);

        aParamsPtr.quantum=0x5;

        st = cpssTmAnodeUpdate(dev, aNodeIndPtr, &aParamsPtr);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, aParamsPtr.quantum);

        /* verify min and max quantum */
        st = cpssTmNodeQuantumLimitsGet(dev, &cpssQuantumLimits);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        aParamsPtr.quantum = (cpssQuantumLimits.maxQuantum / cpssQuantumLimits.resolution);

        st = cpssTmAnodeUpdate(dev, aNodeIndPtr, &aParamsPtr);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, aParamsPtr.quantum);

        aParamsPtr.quantum = (cpssQuantumLimits.minQuantum / cpssQuantumLimits.resolution);
        st = cpssTmAnodeUpdate(dev, aNodeIndPtr, &aParamsPtr);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, aParamsPtr.quantum);


        /*
            1.4 check out of range aParamsPtr.dropProfileInd.
            Call with aParamsPtr.dropProfileInd [300], other parameters same as in 1.1.
            Expected: NON GT_OK.
        */

        aParamsPtr.dropProfileInd=300;

        st = cpssTmAnodeUpdate(dev, aNodeIndPtr, &aParamsPtr);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev,aParamsPtr.dropProfileInd);

        aParamsPtr.dropProfileInd=0;

         /*
            1.5 check out of range aParamsPtr.eligiblePrioFuncId.
            Call with aParamsPtr.eligiblePrioFuncId [70], other parameters same as in 1.1.
            Expected: NON GT_OK.
        */

        aParamsPtr.eligiblePrioFuncId=70;

        st = cpssTmAnodeUpdate(dev, aNodeIndPtr, &aParamsPtr);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, aParamsPtr.eligiblePrioFuncId);

        aParamsPtr.eligiblePrioFuncId=1;

         /*
            1.6 check out of range aParamsPtr.schdModeArr[0].
            Call with aParamsPtr.schdModeArr[0] [2], other parameters same as in 1.1.
            Expected: NON GT_OK.
        */

        aParamsPtr.schdModeArr[0] = 2;

        st = cpssTmAnodeUpdate(dev, aNodeIndPtr, &aParamsPtr);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, aParamsPtr.schdModeArr[0]);

        aParamsPtr.schdModeArr[0] = CPSS_TM_SCHD_MODE_RR_E;

        /*
            1.7 Call with aParamsPtr [NULL], other parameters same as in 1.1.
            Expected: GT_BAD_PARAM.
        */

        st = cpssTmAnodeUpdate(dev, aNodeIndPtr, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, aParamsPtr = NULL", dev);

        st = cpssTmClose(dev);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
    }


    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_NOT_APPLICABLE_DEVICE.            */
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssTmAnodeUpdate(dev, aNodeIndPtr, &aParamsPtr);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssTmAnodeUpdate(dev, aNodeIndPtr, &aParamsPtr);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*******************************************************************************
GT_STATUS cpssTmBnodeUpdate
(
    IN GT_U8                            devNum,
    IN GT_U32                           index,
    IN CPSS_TM_B_NODE_PARAMS_STC        *paramsPtr
)*/
UTF_TEST_CASE_MAC(cpssTmBnodeUpdate)
{
    GT_STATUS               st = GT_OK;

    GT_U8                           dev;
    GT_U32                          portInd;
/*    CPSS_TM_QUEUE_PARAMS_STC        qParamsPtr;
    CPSS_TM_A_NODE_PARAMS_STC       aParamsPtr; */
    CPSS_TM_B_NODE_PARAMS_STC       bParamsPtr;
    CPSS_TM_C_NODE_PARAMS_STC       cParamsPtr;
    GT_U32                          bNodeIndPtr = 1;
    GT_U32                          cNodeIndPtr;
    CPSS_TM_PORT_PARAMS_STC      paramsPtr;
    GT_U32                       cNodesNum;
    GT_U32                       bNodesNum;
    GT_U32                       aNodesNum;
    GT_U32                       queuesNum;
    CPSS_PP_FAMILY_TYPE_ENT devFamily;
    GT_U32                       ii;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev,
                                 UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_LION_E |
                                     UTF_PUMA2_E | UTF_PUMA3_E | UTF_XCAT2_E | UTF_LION2_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E | UTF_ALDRIN_E | UTF_AC3X_E);

    cpssOsBzero((GT_VOID*)&bParamsPtr, sizeof(bParamsPtr));
    cpssOsBzero((GT_VOID*)&cParamsPtr, sizeof(cParamsPtr));
    cpssOsBzero((GT_VOID*)&paramsPtr,  sizeof(paramsPtr));

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* get device family */
        st = prvUtfDeviceFamilyGet(dev, &devFamily);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "prvUtfDeviceFamilyGet: %d", dev);

        /* TM initialization */

        st = cpssTmTestTmCtlLibInit(dev);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        st = cpssTmTestPeriodicSchemeConfiguration(dev);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* Configure parameters for node A */
/*
        aParamsPtr.shapingProfilePtr=0;
        aParamsPtr.quantum= 0x40;
        for (ii=0; ii<8; ii++)
            aParamsPtr.schdModeArr[ii] = 0;
        aParamsPtr.dropProfileInd =0;
        aParamsPtr.eligiblePrioFuncId =8;
*/
        /* Configure parameters for node B */
        bParamsPtr.shapingProfilePtr = 0;
        bParamsPtr.quantum = 0x40;
        for (ii=0; ii<8; ii++)
            bParamsPtr.schdModeArr[ii] = CPSS_TM_SCHD_MODE_RR_E; /* No DWRR configured*/
        bParamsPtr.dropProfileInd = 0;
        bParamsPtr.eligiblePrioFuncId = 9;

        /* Configure parameters for node C */
        cParamsPtr.shapingProfilePtr = 0;
        cParamsPtr.quantum = 0x40;
        cParamsPtr.dropCosMap = 1;
        cParamsPtr.eligiblePrioFuncId = 9;
        for (ii=0; ii<8; ii++){
            cParamsPtr.schdModeArr[ii] = CPSS_TM_SCHD_MODE_RR_E;
            cParamsPtr.dropProfileIndArr[ii] = 0;
        }

        /* profile for Q level */
/*
        qParamsPtr.shapingProfilePtr=0;
        qParamsPtr.quantum=0x40;
        qParamsPtr.dropProfileInd=0;
        qParamsPtr.eligiblePrioFuncId=1;
*/
        /* Configure parameters for port */
        paramsPtr.cirBw = 1000000;
        paramsPtr.eirBw = 0;
        paramsPtr.cbs = 8; /*KBytes*/
        paramsPtr.ebs = 0x1FFFF;
        paramsPtr.eligiblePrioFuncId = CPSS_TM_ELIG_N_MIN_SHP_E;
        for(ii=0; ii<8; ii++){
            paramsPtr.schdModeArr[ii] = CPSS_TM_SCHD_MODE_RR_E;
            paramsPtr.quantumArr[ii] = UT_TM_MIN_PORT_QUANTUM_CNS;
        }
        paramsPtr.dropProfileInd = CPSS_TM_NO_DROP_PROFILE_CNS;

        portInd=1;
        cNodesNum=1;
        bNodesNum=1;
        aNodesNum=1;
        queuesNum=1;

        st = cpssTmPortCreate(dev, portInd, &paramsPtr, cNodesNum, bNodesNum, aNodesNum,queuesNum );
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        st = cpssTmBnodeToPortCreate(dev, portInd, &bParamsPtr, &cParamsPtr,&bNodeIndPtr, &cNodeIndPtr );
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*1.1 Check with legal parameters.
                Expected: GT_OK */

        st = cpssTmBnodeUpdate(dev, bNodeIndPtr, &bParamsPtr);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.2 check out of range bParamsPtr.shapingProfilePtr.
            Call with bParamsPtr.shapingProfilePtr [85580], other parameters same as in 1.1.
            Expected: NON GT_OK.
        */

        bParamsPtr.shapingProfilePtr = 85580;

        st = cpssTmBnodeUpdate(dev, bNodeIndPtr, &bParamsPtr);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, bParamsPtr.shapingProfilePtr);

        bParamsPtr.shapingProfilePtr= 0;

         /*
            1.3 check out of range bParamsPtr.quantum.
            Call with bParamsPtr.quantum [0x4fff], other parameters same as in 1.1.
            Expected: NON GT_OK.
        */

        bParamsPtr.quantum=0x4fff;

        st = cpssTmBnodeUpdate(dev, bNodeIndPtr, &bParamsPtr);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, bParamsPtr.quantum);

        bParamsPtr.quantum=0x5;

        st = cpssTmBnodeUpdate(dev, bNodeIndPtr, &bParamsPtr);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, bParamsPtr.quantum);

        bParamsPtr.quantum=0x40;

         /*
            1.4 check out of range bParamsPtr.dropProfileInd.
            Call with bParamsPtr.dropProfileInd [300], other parameters same as in 1.1.
            Expected: NON GT_OK.
        */

        bParamsPtr.dropProfileInd=300;

        st = cpssTmBnodeUpdate(dev, bNodeIndPtr, &bParamsPtr);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev,bParamsPtr.dropProfileInd);

        bParamsPtr.dropProfileInd=0;

         /*
            1.5 check out of range bParamsPtr.eligiblePrioFuncId.
            Call with bParamsPtr.eligiblePrioFuncId [70], other parameters same as in 1.1.
            Expected: NON GT_OK.
        */

        bParamsPtr.eligiblePrioFuncId=70;

        st = cpssTmBnodeUpdate(dev, bNodeIndPtr, &bParamsPtr);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, bParamsPtr.eligiblePrioFuncId);

        bParamsPtr.eligiblePrioFuncId=1;

         /*
            1.6 check out of range bParamsPtr.schdModeArr[0].
            Call with bParamsPtr.schdModeArr[0] [2], other parameters same as in 1.1.
            Expected: NON GT_OK.
        */

        bParamsPtr.schdModeArr[0] = 2;

        st = cpssTmBnodeUpdate(dev, bNodeIndPtr, &bParamsPtr);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, bParamsPtr.schdModeArr[0]);

        bParamsPtr.schdModeArr[0] = CPSS_TM_SCHD_MODE_RR_E;

        /*
            1.7 Call with bParamsPtr [NULL], other parameters same as in 1.1.
            Expected: GT_BAD_PTR.
        */

        st = cpssTmBnodeUpdate(dev, bNodeIndPtr, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, bParamsPtr = NULL", dev);

        st = cpssTmClose(dev);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
    }


    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_NOT_APPLICABLE_DEVICE.            */
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssTmBnodeUpdate(dev, bNodeIndPtr, &bParamsPtr);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssTmBnodeUpdate(dev, bNodeIndPtr, &bParamsPtr);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}


/*******************************************************************************
GT_STATUS cpssTmCnodeUpdate
(
    IN GT_U8                            devNum,
    IN GT_U32                           index,
    IN CPSS_TM_C_NODE_PARAMS_STC        *paramsPtr
)
*/
UTF_TEST_CASE_MAC(cpssTmCnodeUpdate)
{
    GT_STATUS               st = GT_OK;

    GT_U8                        dev;
    GT_U32                       portInd;
    CPSS_TM_PORT_PARAMS_STC      paramsPtr;
    GT_U32                       cNodesNum;
    GT_U32                       bNodesNum;
    GT_U32                       aNodesNum;
    GT_U32                       queuesNum;
    CPSS_TM_C_NODE_PARAMS_STC    cParamsPtr;
    GT_U32                       cNodeIndPtr = 1;
    CPSS_PP_FAMILY_TYPE_ENT devFamily;
    GT_U32                       ii;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev,
                                 UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_LION_E |
                                     UTF_PUMA2_E | UTF_PUMA3_E | UTF_XCAT2_E | UTF_LION2_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E | UTF_ALDRIN_E | UTF_AC3X_E);
    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* get device family */
        st = prvUtfDeviceFamilyGet(dev, &devFamily);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "prvUtfDeviceFamilyGet: %d", dev);

        /* TM initialization */

        st = cpssTmTestTmCtlLibInit(dev);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        st = cpssTmTestPeriodicSchemeConfiguration(dev);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

         /* Configuration of port params */
        portInd=1;
        cNodesNum=1;
        bNodesNum=1;
        aNodesNum=1;
        queuesNum=1;


        paramsPtr.cirBw = 1000000;
        paramsPtr.eirBw = 0;
        paramsPtr.cbs = 8; /*KBytes*/
        paramsPtr.ebs = 0x1FFFF;
        paramsPtr.eligiblePrioFuncId = CPSS_TM_ELIG_N_MIN_SHP_E;
        for(ii=0; ii<8; ii++){
            paramsPtr.schdModeArr[ii] = CPSS_TM_SCHD_MODE_RR_E;
            paramsPtr.quantumArr[ii] = UT_TM_MIN_PORT_QUANTUM_CNS;
        }
        paramsPtr.dropProfileInd = CPSS_TM_NO_DROP_PROFILE_CNS;

        st = cpssTmPortCreate(dev, portInd, &paramsPtr, cNodesNum, bNodesNum, aNodesNum,queuesNum );
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* Configure parameters for node C */
        cParamsPtr.shapingProfilePtr = 0;
        cParamsPtr.quantum = 0x40;
        cParamsPtr.dropCosMap = 1;
        cParamsPtr.eligiblePrioFuncId = 9;
        for (ii=0; ii<8; ii++){
            cParamsPtr.schdModeArr[ii] = CPSS_TM_SCHD_MODE_RR_E;
            cParamsPtr.dropProfileIndArr[ii] = 0;
        }

        st = cpssTmCnodeToPortCreate(dev, portInd, &cParamsPtr, &cNodeIndPtr);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);


        /*1.1 Check with legal parameters.
                Expected: GT_OK */

        st = cpssTmCnodeUpdate(dev, cNodeIndPtr, &cParamsPtr);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.2 check out of range cParamsPtr.shapingProfilePtr.
            Call with cParamsPtr.shapingProfilePtr [85580], other parameters same as in 1.1.
            Expected: NON GT_OK.
        */

        cParamsPtr.shapingProfilePtr = 85580;

        st = cpssTmCnodeUpdate(dev, cNodeIndPtr, &cParamsPtr);

        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, cParamsPtr.shapingProfilePtr);

        cParamsPtr.shapingProfilePtr= 0;

         /*
            1.3 check out of range cParamsPtr.quantum.
            Call with cParamsPtr.quantum [0x4fff], other parameters same as in 1.1.
            Expected: NON GT_OK.
        */

        cParamsPtr.quantum=0x4fff;

        st = cpssTmCnodeUpdate(dev, cNodeIndPtr, &cParamsPtr);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, cParamsPtr.quantum);

        cParamsPtr.quantum=0x5;

        st = cpssTmCnodeUpdate(dev, cNodeIndPtr, &cParamsPtr);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, cParamsPtr.quantum);

        cParamsPtr.quantum=0x40;

         /*
            1.4 check out of range cParamsPtr.dropProfileInd.
            Call with cParamsPtr.dropProfileIndArr[0][300], other parameters same as in 1.1.
            Expected: NON GT_OK.
        */

        cParamsPtr.dropProfileIndArr[0]=300;

        st = cpssTmCnodeUpdate(dev, cNodeIndPtr, &cParamsPtr);

        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev,cParamsPtr.dropProfileIndArr[0]);

        cParamsPtr.dropProfileIndArr[0]=0;

         /*
            1.5 check out of range cParamsPtr.eligiblePrioFuncId.
            Call with cParamsPtr.eligiblePrioFuncId [70], other parameters same as in 1.1.
            Expected: NON GT_OK.
        */

        cParamsPtr.eligiblePrioFuncId=70;

        st = cpssTmCnodeUpdate(dev, cNodeIndPtr, &cParamsPtr);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, cParamsPtr.eligiblePrioFuncId);

        cParamsPtr.eligiblePrioFuncId=1;

         /*
            1.7 check out of range cParamsPtr.schdModeArr[0].
            Call with cParamsPtr.schdModeArr[0] [2], other parameters same as in 1.1.
            Expected: NON GT_OK.
        */

        cParamsPtr.schdModeArr[0] = 2;

        st = cpssTmCnodeUpdate(dev, cNodeIndPtr, &cParamsPtr);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, cParamsPtr.schdModeArr[0]);

        cParamsPtr.schdModeArr[0] = CPSS_TM_SCHD_MODE_RR_E;


       /*
            1.8 Call with cNodeIndPtr [NULL], other parameters same as in 1.1.
            Expected: GT_BAD_PTR.
        */

        st = cpssTmCnodeUpdate(dev, cNodeIndPtr, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, cNodeIndPtr = NULL", dev);


        /*
            1.9 Call with cParamsPtr [NULL], other parameters same as in 1.1.
            Expected: GT_BAD_PTR.
        */

        st = cpssTmCnodeUpdate(dev, cNodeIndPtr, NULL);

        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, cParamsPtr = NULL", dev);



        st = cpssTmClose(dev);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
    }


    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_NOT_APPLICABLE_DEVICE.            */
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssTmCnodeUpdate(dev, cNodeIndPtr, &cParamsPtr);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssTmCnodeUpdate(dev, cNodeIndPtr, &cParamsPtr);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}


/*******************************************************************************
GT_STATUS cpssTmPortDropUpdate
(
    IN GT_U8    devNum,
    IN GT_U32   index,
    IN GT_U32   wredProfileRef
)
*/
UTF_TEST_CASE_MAC(cpssTmPortDropUpdate)
{
    GT_STATUS               st = GT_OK;

    GT_U8                   dev;
    CPSS_TM_LEVEL_ENT       level;
    GT_U32                  cos = 0;
    CPSS_TM_DROP_PROFILE_PARAMS_STC profileStrPtr;
    GT_U32                          profilePtr = 1;
    CPSS_PP_FAMILY_TYPE_ENT devFamily;
    GT_U32                  ii;
    GT_U32                       portInd;
    CPSS_TM_PORT_PARAMS_STC      paramsPtr;
    GT_U32                       cNodesNum;
    GT_U32                       bNodesNum;
    GT_U32                       aNodesNum;
    GT_U32                       queuesNum;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev,
                                 UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_LION_E |
                                     UTF_PUMA2_E | UTF_PUMA3_E | UTF_XCAT2_E | UTF_LION2_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E | UTF_ALDRIN_E | UTF_AC3X_E);

    cpssOsBzero((GT_VOID*)&profileStrPtr, sizeof(profileStrPtr));

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* get device family */
        st = prvUtfDeviceFamilyGet(dev, &devFamily);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "prvUtfDeviceFamilyGet: %d", dev);

        /* TM initialization */

        st = cpssTmTestTmCtlLibInit(dev);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        st = cpssTmTestPeriodicSchemeConfiguration(dev);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);


        level = CPSS_TM_LEVEL_P_E;
        cos = CPSS_TM_INVAL_CNS;

        profileStrPtr.dropMode = CPSS_TM_DROP_MODE_CB_TD_ONLY_E;
        profileStrPtr.cbTdThresholdBytes = cpssTmDropConvertBWToBytes(200000);

        st = cpssTmDropProfileCreate(dev, level, cos, &profileStrPtr, &profilePtr);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

          /* Configuration of port params */
        paramsPtr.cirBw = 1000000;
        paramsPtr.eirBw = 0;
        paramsPtr.cbs = 8; /*KBytes*/
        paramsPtr.ebs = 0x1FFFF;
        paramsPtr.eligiblePrioFuncId = CPSS_TM_ELIG_N_MIN_SHP_E;
        for(ii=0; ii<8; ii++){
            paramsPtr.schdModeArr[ii] = CPSS_TM_SCHD_MODE_RR_E;
            paramsPtr.quantumArr[ii] = UT_TM_MIN_PORT_QUANTUM_CNS;
        }
        paramsPtr.dropProfileInd = CPSS_TM_NO_DROP_PROFILE_CNS;

        portInd = 1;
        cNodesNum = 1;
        bNodesNum = 1;
        aNodesNum = 1;
        queuesNum = 1;

        st = cpssTmPortCreate(dev, portInd, &paramsPtr, cNodesNum, bNodesNum, aNodesNum, queuesNum);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*1.1 Check with legal parameters.
                Expected: GT_OK */

        st = cpssTmPortDropUpdate(dev, portInd, profilePtr);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

         /*
            1.2 check out of range portInd.
            Call with portInd [70], other parameters same as in 1.1.
            Expected: NON GT_OK.
        */

        portInd = 70;

        st = cpssTmPortDropUpdate(dev, portInd, profilePtr);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, portInd);

        portInd = 1;

        /*
            1.3 check out of range profilePtr.
            Call with profilePtr [profilePtr+256], other parameters same as in 1.1.
            Expected: NON GT_OK.
        */

        profilePtr = profilePtr+400;

        st = cpssTmPortDropUpdate(dev, portInd, profilePtr);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, profilePtr);

        profilePtr = profilePtr-400;

        st = cpssTmClose(dev);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
    }


    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_NOT_APPLICABLE_DEVICE.            */
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    portInd=1;

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssTmPortDropUpdate(dev, portInd, profilePtr);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssTmPortDropUpdate(dev, portInd, profilePtr);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}


/*******************************************************************************
GT_STATUS cpssTmPortDropCosUpdate
(
    IN GT_U8    devNum,
    IN GT_U32   index,
    IN CPSS_TM_PORT_DROP_PER_COS_STC   *paramsPtr
)
*/
UTF_TEST_CASE_MAC(cpssTmPortDropCosUpdate)
{
    GT_STATUS               st = GT_OK;

    GT_U8                   dev;
    CPSS_TM_LEVEL_ENT       level;
    GT_U32                  cos=0;
    CPSS_TM_DROP_PROFILE_PARAMS_STC profileStrPtr;
    GT_U32                          profilePtr = 1;
    CPSS_PP_FAMILY_TYPE_ENT devFamily;
    GT_U32                  ii;
    GT_U32                       portInd;
    CPSS_TM_PORT_PARAMS_STC      paramsPtr;
    CPSS_TM_PORT_DROP_PER_COS_STC cosParams;
    GT_U32                       cNodesNum;
    GT_U32                       bNodesNum;
    GT_U32                       aNodesNum;
    GT_U32                       queuesNum;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev,
                                 UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_LION_E |
                                     UTF_PUMA2_E | UTF_PUMA3_E | UTF_XCAT2_E | UTF_LION2_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E | UTF_ALDRIN_E | UTF_AC3X_E);

    cpssOsBzero((GT_VOID*)&profileStrPtr, sizeof(profileStrPtr));

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* get device family */
        st = prvUtfDeviceFamilyGet(dev, &devFamily);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "prvUtfDeviceFamilyGet: %d", dev);

        /* TM initialization */

        st = cpssTmTestTmCtlLibInit(dev);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        st = cpssTmTestPeriodicSchemeConfiguration(dev);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        level = CPSS_TM_LEVEL_P_E;
        cos = 2;

        profileStrPtr.dropMode = CPSS_TM_DROP_MODE_CB_TD_ONLY_E;
        profileStrPtr.cbTdThresholdBytes = cpssTmDropConvertBWToBytes(200000);


        st = cpssTmDropProfileCreate(dev, level, cos, &profileStrPtr, &profilePtr);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* Configuration of port params */
        paramsPtr.cirBw = 1000000;
        paramsPtr.eirBw = 0;
        paramsPtr.cbs = 8; /*KBytes*/
        paramsPtr.ebs = 0x1FFFF;
        paramsPtr.eligiblePrioFuncId = CPSS_TM_ELIG_N_MIN_SHP_E;
        for(ii=0; ii<8; ii++){
            paramsPtr.schdModeArr[ii] = CPSS_TM_SCHD_MODE_RR_E;
            paramsPtr.quantumArr[ii] = UT_TM_MIN_PORT_QUANTUM_CNS;
        }

        paramsPtr.dropProfileInd = CPSS_TM_NO_DROP_PROFILE_CNS;

        portInd = 1;
        cNodesNum = 1;
        bNodesNum = 1;
        aNodesNum = 1;
        queuesNum = 1;

        st = cpssTmPortCreate(dev, portInd, &paramsPtr, cNodesNum, bNodesNum, aNodesNum, queuesNum);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        cosParams.dropCosMap = 0x1;
        for(ii=0; ii<8; ii++)
            cosParams.dropProfileIndArr[ii] = CPSS_TM_NO_DROP_PROFILE_CNS;

        st = cpssTmPortDropPerCosConfig(dev, portInd, &cosParams);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);


        /*1.1 Check with legal parameters.
                Expected: GT_OK */

        cosParams.dropCosMap = 0x4; /* Cos = 2 */
        cosParams.dropProfileIndArr[2] = profilePtr;

        st = cpssTmPortDropCosUpdate(dev, portInd, &cosParams);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

         /*
            1.2 check out of range portInd.
            Call with portInd [70], other parameters same as in 1.1.
            Expected: NON GT_OK.
        */

        portInd = 70;

        st = cpssTmPortDropCosUpdate(dev, portInd, &cosParams);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, portInd);

        portInd = 1;

        /*
            1.3 check out of range profilePtr.
            Call with cosParams.dropProfileIndArr[2] [profilePtr+400], other parameters same as in 1.1.
            Expected: NON GT_OK.
        */

        cosParams.dropProfileIndArr[2] = profilePtr + 400;

        st = cpssTmPortDropCosUpdate(dev, portInd, &cosParams);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, cosParams.dropProfileIndArr[2]);

        cosParams.dropProfileIndArr[2] = profilePtr;

        st = cpssTmClose(dev);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
    }


    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_NOT_APPLICABLE_DEVICE.            */
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    portInd=1;

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssTmPortDropCosUpdate(dev, portInd, &cosParams);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssTmPortDropCosUpdate(dev, portInd, &cosParams);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}


/*******************************************************************************
GT_STATUS cpssTmPortSchedulingUpdate
(
    IN GT_U8    devNum,
    IN GT_U32   index,
    IN GT_U32   eligPrioFuncId,
    IN GT_U32   quantumArrPtr[8],
    IN CPSS_TM_RR_MODE_TYPE_ENT  *schdModeArr
)
*/

UTF_TEST_CASE_MAC(cpssTmPortSchedulingUpdate)
{
    GT_STATUS               st = GT_OK;

    GT_U8                   dev;
    CPSS_PP_FAMILY_TYPE_ENT devFamily;
    GT_U32                  ii;
    GT_U32                  portInd;
    CPSS_TM_PORT_PARAMS_STC paramsPtr;
    GT_U32                  cNodesNum;
    GT_U32                  bNodesNum;
    GT_U32                  aNodesNum;
    GT_U32                  queuesNum;
    GT_U32                  eligPrioFuncId;
    GT_U32                  quantumArrPtr[8];
    CPSS_TM_SCHD_MODE_ENT   schdModeArr[8];

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev,
                                 UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_LION_E |
                                     UTF_PUMA2_E | UTF_PUMA3_E | UTF_XCAT2_E | UTF_LION2_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E | UTF_ALDRIN_E | UTF_AC3X_E);
    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* get device family */
        st = prvUtfDeviceFamilyGet(dev, &devFamily);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "prvUtfDeviceFamilyGet: %d", dev);

        /* TM initialization */

        st = cpssTmTestTmCtlLibInit(dev);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        st = cpssTmTestPeriodicSchemeConfiguration(dev);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* Configuration of port params */
        paramsPtr.cirBw = 1000000;
        paramsPtr.eirBw = 0;
        paramsPtr.cbs = 8; /*KBytes*/
        paramsPtr.ebs = 0x1FFFF;
        paramsPtr.eligiblePrioFuncId = CPSS_TM_ELIG_N_MIN_SHP_E;
        for(ii=0; ii<8; ii++){
            paramsPtr.schdModeArr[ii] = CPSS_TM_SCHD_MODE_RR_E;
            paramsPtr.quantumArr[ii] = UT_TM_MIN_PORT_QUANTUM_CNS;
        }
        paramsPtr.dropProfileInd = CPSS_TM_NO_DROP_PROFILE_CNS;

        portInd=1;
        cNodesNum=1;
        bNodesNum=1;
        aNodesNum=1;
        queuesNum=1;

        st = cpssTmPortCreate(dev, portInd, &paramsPtr, cNodesNum, bNodesNum, aNodesNum,queuesNum );
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*1.1 Check with legal parameters.
               Call with schdModeArr [CPSS_TM_SCHD_MODE_RR_E]
                Expected: GT_OK */

        for(ii=0; ii<8; ii++)
        {
            quantumArrPtr[ii] = 0x30;
            schdModeArr[ii] = CPSS_TM_SCHD_MODE_RR_E;
        }
        eligPrioFuncId = CPSS_TM_ELIG_N_PRIO1_E;

        st = cpssTmPortSchedulingUpdate(dev, portInd, eligPrioFuncId, quantumArrPtr, schdModeArr);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*1.2 Check with legal parameters.
               Call with schdModeArr [CPSS_TM_DWRR_MODE_E]
                Expected: GT_OK */

        for(ii=0; ii<8; ii++)
        {
            schdModeArr[ii] = CPSS_TM_SCHD_MODE_DWRR_E;
        }

        st = cpssTmPortSchedulingUpdate(dev, portInd, eligPrioFuncId, quantumArrPtr, schdModeArr);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        for(ii=0; ii<8; ii++)
        {
            schdModeArr[ii] = CPSS_TM_SCHD_MODE_RR_E;
        }

         /*
            1.3 check out of range portInd.
            Call with portInd [70], other parameters same as in 1.1.
            Expected: NON GT_OK.
        */

        portInd = 70;

        st = cpssTmPortSchedulingUpdate(dev, portInd, eligPrioFuncId, quantumArrPtr, schdModeArr);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, portInd);

        portInd = 1;

        /*
            1.4 check out of range eligPrioFuncId.
            Call with eligPrioFuncId [70], other parameters same as in 1.1.
            Expected: NON GT_OK.
        */

        eligPrioFuncId = 70;

        st = cpssTmPortSchedulingUpdate(dev, portInd, eligPrioFuncId, quantumArrPtr, schdModeArr);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, eligPrioFuncId);

        eligPrioFuncId = CPSS_TM_ELIG_N_PRIO1_E;

         /*
            1.5 check out of range quantumArrPtr.
            Call with quantumArrPtr [0x204], other parameters same as in 1.1.
            Expected: NON GT_OK.
        */

        quantumArrPtr[0]= 0x204;

        st = cpssTmPortSchedulingUpdate(dev, portInd, eligPrioFuncId, quantumArrPtr, schdModeArr);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, quantumArrPtr);


         /*
            1.6 check out of range quantumArrPtr.
            Call with quantumArrPtr [1], other parameters same as in 1.1.
            Expected: NON GT_OK.
        */
         quantumArrPtr[0] = 1;

        st = cpssTmPortSchedulingUpdate(dev, portInd, eligPrioFuncId, quantumArrPtr, schdModeArr);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, quantumArrPtr);


        quantumArrPtr[0] = 0;

        st = cpssTmClose(dev);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
    }


    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_NOT_APPLICABLE_DEVICE.            */
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    eligPrioFuncId = CPSS_TM_ELIG_N_PRIO1_E;
    portInd=1;

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssTmPortSchedulingUpdate(dev, portInd, eligPrioFuncId, quantumArrPtr, schdModeArr);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssTmPortSchedulingUpdate(dev, portInd, eligPrioFuncId, quantumArrPtr, schdModeArr);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*******************************************************************************
GT_STATUS cpssTmPortShapingUpdate
(
    IN GT_U8    devNum,
    IN GT_U32   index,
    IN GT_BOOL  shaping,
    IN GT_U32   cirBw,
    IN GT_U32   eirBw,
    IN GT_U32   cbs,
    IN GT_U32   ebs
)
*/

UTF_TEST_CASE_MAC(cpssTmPortShapingUpdate)
{
    GT_STATUS               st = GT_OK;

    GT_U8                   dev;
    CPSS_PP_FAMILY_TYPE_ENT devFamily;
    GT_U32                  ii;
    GT_U32                       portInd;
    CPSS_TM_PORT_PARAMS_STC      paramsPtr;
    GT_U32                       cNodesNum;
    GT_U32                       bNodesNum;
    GT_U32                       aNodesNum;
    GT_U32                       queuesNum;
/*    GT_BOOL                      shaping; */
    GT_U32                       cirBw;
    GT_U32                       eirBw;
    GT_U32                       cbs;
    GT_U32                       ebs;

    CPSS_TM_PORT_PARAMS_STC       outPortParams;
    CPSS_TM_PORT_DROP_PER_COS_STC outPortDropCos;


    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev,
                                 UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_LION_E |
                                     UTF_PUMA2_E | UTF_PUMA3_E | UTF_XCAT2_E | UTF_LION2_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E | UTF_ALDRIN_E | UTF_AC3X_E);
    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* get device family */
        st = prvUtfDeviceFamilyGet(dev, &devFamily);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "prvUtfDeviceFamilyGet: %d", dev);

        /* TM initialization */


        st = cpssTmTestTmCtlLibInit(dev);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        st = cpssTmTestPeriodicSchemeConfiguration(dev);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* Configuration of port params */
        paramsPtr.cirBw = 1000000;
        paramsPtr.eirBw = 0;
        paramsPtr.cbs = 8; /*KBytes*/
        paramsPtr.ebs = 0x1FFFF;
        paramsPtr.eligiblePrioFuncId = CPSS_TM_ELIG_N_MIN_SHP_E;
        for(ii=0; ii<8; ii++){
            paramsPtr.schdModeArr[ii] = CPSS_TM_SCHD_MODE_RR_E;
            paramsPtr.quantumArr[ii] = UT_TM_MIN_PORT_QUANTUM_CNS;
        }
        paramsPtr.dropProfileInd = CPSS_TM_NO_DROP_PROFILE_CNS;

        portInd=1;
        cNodesNum=1;
        bNodesNum=1;
        aNodesNum=1;
        queuesNum=1;
/*        shaping= GT_TRUE; */

        cirBw=1000000;
        eirBw=0;
        cbs=8; /*KBytes*/
        ebs=0x1FFFF;

        st = cpssTmPortCreate(dev, portInd, &paramsPtr, cNodesNum, bNodesNum, aNodesNum,queuesNum );
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*1.1 Check with legal parameters.
                Expected: GT_OK */

        st = cpssTmPortShapingUpdate(dev, portInd, cirBw, eirBw, cbs, ebs);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*1.1 Check with max legal parameters MAX_BW.
                Expected: GT_OK */

        st = cpssTmPortShapingUpdate(dev,
                                     portInd,
                                     CPSS_TM_SHAPING_INFINITE_BW_CNS,
                                     CPSS_TM_SHAPING_INFINITE_BW_CNS,
                                     CPSS_TM_SHAPING_MAX_BURST_CNS,
                                     CPSS_TM_SHAPING_MAX_BURST_CNS);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);


         /*
            1.2 check out of range cirBw.
            Call with cirBw [200000000], other parameters same as in 1.1.
            Expected: NON GT_OK.
        */

        cirBw = 200000000;

        st = cpssTmPortShapingUpdate(dev, portInd, cirBw,eirBw, cbs, ebs);
        UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        cirBw= 1000000;

         /*
            1.3 check out of range eirBw.
            Call with eirBw [200000000], other parameters same as in 1.1.
            Expected: NON GT_OK.
        */

        eirBw = 200000000;

        st = cpssTmPortShapingUpdate(dev, portInd,cirBw,eirBw, cbs, ebs);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, paramsPtr.eirBw);

        eirBw= 0;

        portInd=2;
        paramsPtr.cirBw=CPSS_TM_SHAPING_INFINITE_BW_CNS;
        paramsPtr.eirBw=0;
        paramsPtr.cbs=CPSS_TM_SHAPING_MAX_BURST_CNS;
        paramsPtr.ebs=0x1FFFF;
        st = cpssTmPortCreate(dev, portInd, &paramsPtr, cNodesNum, bNodesNum, aNodesNum,queuesNum);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        portInd=3;
        paramsPtr.cirBw=0;
        paramsPtr.eirBw=CPSS_TM_SHAPING_INFINITE_BW_CNS;
        paramsPtr.cbs=0;
        paramsPtr.ebs=CPSS_TM_SHAPING_MAX_BURST_CNS;
        st = cpssTmPortCreate(dev, portInd, &paramsPtr, cNodesNum, bNodesNum, aNodesNum,queuesNum);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        st = cpssTmPortConfigurationRead(dev, portInd, &outPortParams, &outPortDropCos);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* verify port DB params */
        UTF_VERIFY_EQUAL1_PARAM_MAC(0, outPortParams.cirBw, portInd);
        UTF_VERIFY_EQUAL1_PARAM_MAC(CPSS_TM_SHAPING_INFINITE_BW_CNS, outPortParams.eirBw, portInd);
        UTF_VERIFY_EQUAL1_PARAM_MAC(0, outPortParams.cbs, portInd);
        UTF_VERIFY_EQUAL1_PARAM_MAC(CPSS_TM_SHAPING_MAX_BURST_CNS, outPortParams.ebs, portInd);

        st = cpssTmClose(dev);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
    }


    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_NOT_APPLICABLE_DEVICE.            */
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    cirBw=1000000;
    eirBw=0;
    cbs=8; /*KBytes*/
    ebs=0x1FFFF;
    portInd=1;

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssTmPortShapingUpdate(dev, portInd, cirBw,eirBw, cbs, ebs);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssTmPortShapingUpdate(dev, portInd, cirBw,eirBw, cbs, ebs);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*******************************************************************************
GT_STATUS cpssTmQueueUpdate
(
    IN GT_U8                            devNum,
    IN GT_U32                           index,
    IN CPSS_TM_QUEUE_PARAMS_STC         *paramsPtr
)
*/
UTF_TEST_CASE_MAC(cpssTmQueueUpdate)
{
    GT_STATUS               st = GT_OK;

    GT_U8                        dev;
    GT_U32                       portInd;
    CPSS_TM_QUEUE_PARAMS_STC     qParamsPtr;
    GT_U32                       queueIndPtr =1;
    CPSS_TM_PORT_PARAMS_STC      paramsPtr;
    GT_U32                       cNodesNum;
    GT_U32                       bNodesNum;
    GT_U32                       aNodesNum;
    GT_U32                       queuesNum;
    CPSS_PP_FAMILY_TYPE_ENT devFamily;
    GT_U32                       ii;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev,
                                 UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_LION_E |
                                     UTF_PUMA2_E | UTF_PUMA3_E | UTF_XCAT2_E | UTF_LION2_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E | UTF_ALDRIN_E | UTF_AC3X_E);
    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* get device family */
        st = prvUtfDeviceFamilyGet(dev, &devFamily);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "prvUtfDeviceFamilyGet: %d", dev);

        /* TM initialization */

        st = cpssTmTestTmCtlLibInit(dev);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        st = cpssTmTestPeriodicSchemeConfiguration(dev);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

         /* Configuration of port params */
        portInd=1;
        cNodesNum=1;
        bNodesNum=1;
        aNodesNum=1;
        queuesNum=1;


        paramsPtr.cirBw = 1000000;
        paramsPtr.eirBw = 0;
        paramsPtr.cbs = 8; /*KBytes*/
        paramsPtr.ebs = 0x1FFFF;
        paramsPtr.eligiblePrioFuncId = CPSS_TM_ELIG_N_MIN_SHP_E;
        for(ii=0; ii<8; ii++){
            paramsPtr.schdModeArr[ii] = CPSS_TM_SCHD_MODE_RR_E;
            paramsPtr.quantumArr[ii] = UT_TM_MIN_PORT_QUANTUM_CNS;
        }
        paramsPtr.dropProfileInd = CPSS_TM_NO_DROP_PROFILE_CNS;

        st = cpssTmPortCreate(dev, portInd, &paramsPtr, cNodesNum, bNodesNum, aNodesNum,queuesNum );
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);


        /* profile for Q level */
        qParamsPtr.shapingProfilePtr=0;
        qParamsPtr.quantum=0x40;
        qParamsPtr.dropProfileInd=0;
        qParamsPtr.eligiblePrioFuncId=1;

        st = cpssTmTransQueueToPortCreate(dev, portInd, &qParamsPtr, &queueIndPtr);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*1.1 Check with legal parameters.
                Expected: GT_OK */

        st = cpssTmQueueUpdate(dev, queueIndPtr, &qParamsPtr);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.2 check out of range qParamsPtr.shapingProfilePtr.
            Call with qParamsPtr.shapingProfilePtr [85580], other parameters same as in 1.1.
            Expected: NON GT_OK.
        */

        qParamsPtr.shapingProfilePtr = 85580;

        st = cpssTmQueueUpdate(dev, queueIndPtr, &qParamsPtr);

        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, qParamsPtr.shapingProfilePtr);

        qParamsPtr.shapingProfilePtr= 0;

         /*
            1.3 check out of range qParamsPtr.quantum.
            Call with qParamsPtr.quantum [0x4fff], other parameters same as in 1.1.
            Expected: NON GT_OK.
        */

        qParamsPtr.quantum=0x4fff;

        st = cpssTmQueueUpdate(dev, queueIndPtr, &qParamsPtr);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, qParamsPtr.quantum);

        qParamsPtr.quantum=0x7;

        st = cpssTmQueueUpdate(dev, queueIndPtr, &qParamsPtr);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, qParamsPtr.quantum);

        qParamsPtr.quantum=0x40;

         /*
            1.4 check out of range qParamsPtr.dropProfileInd.
            Call with qParamsPtr.dropProfileInd [300], other parameters same as in 1.1.
            Expected: NON GT_OK.
        */

        qParamsPtr.dropProfileInd=300;

        st = cpssTmQueueUpdate(dev, queueIndPtr, &qParamsPtr);

        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev,qParamsPtr.dropProfileInd);

        qParamsPtr.dropProfileInd=0;

         /*
            1.5 check out of range qParamsPtr.eligiblePrioFuncId.
            Call with qParamsPtr.eligiblePrioFuncId [70], other parameters same as in 1.1.
            Expected: NON GT_OK.
        */

        qParamsPtr.eligiblePrioFuncId=70;

        st = cpssTmQueueUpdate(dev, queueIndPtr, &qParamsPtr);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, qParamsPtr.eligiblePrioFuncId);

        qParamsPtr.eligiblePrioFuncId=1;

       /*
            1.6 Call with qParamsPtr [NULL], other parameters same as in 1.1.
            Expected: GT_BAD_PTR.
        */

        st = cpssTmQueueUpdate(dev, queueIndPtr, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, qParamsPtr = NULL", dev);

        /*
            1.7 check out of range portInd.
            Call with portInd [70], other parameters same as in 1.1.
            Expected: NON GT_OK.
        */

        portInd=70;

        st = cpssTmTransQueueToPortCreate(dev, portInd, &qParamsPtr, &queueIndPtr);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, portInd);

        portInd=1;

        st = cpssTmClose(dev);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
    }


    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_NOT_APPLICABLE_DEVICE.            */
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssTmQueueUpdate(dev, queueIndPtr, &qParamsPtr);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssTmQueueUpdate(dev, queueIndPtr, &qParamsPtr);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}


/*----------------------------------------------------------------------------*/
/*
 * Configuration of cpssTmDrop suit
 */
UTF_SUIT_BEGIN_TESTS_MAC(cpssTmNodesUpdate)
    UTF_SUIT_DECLARE_TEST_MAC(cpssTmAnodeUpdate)
    UTF_SUIT_DECLARE_TEST_MAC(cpssTmBnodeUpdate)
    UTF_SUIT_DECLARE_TEST_MAC(cpssTmCnodeUpdate)
    UTF_SUIT_DECLARE_TEST_MAC(cpssTmPortDropUpdate)
    UTF_SUIT_DECLARE_TEST_MAC(cpssTmPortDropCosUpdate)
    UTF_SUIT_DECLARE_TEST_MAC(cpssTmPortSchedulingUpdate)
    UTF_SUIT_DECLARE_TEST_MAC(cpssTmPortShapingUpdate)
    UTF_SUIT_DECLARE_TEST_MAC(cpssTmQueueUpdate)
UTF_SUIT_END_TESTS_MAC(cpssTmNodesUpdate)

