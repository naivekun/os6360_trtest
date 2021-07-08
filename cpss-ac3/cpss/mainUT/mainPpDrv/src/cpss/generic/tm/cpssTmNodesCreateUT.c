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
* @file cpssTmNodesCreateUT.c
*
* @brief Unit tests for cpssTmNodesCreate.
*
* @version   3
********************************************************************************
*/

#include <cpss/generic/tm/cpssTmCtl.h>
#include <cpss/generic/tm/cpssTmDrop.h>
#include <cpss/generic/tm/cpssTmSched.h>
#include <cpss/generic/tm/cpssTmShaping.h>
#include <cpss/generic/tm/cpssTmNodesCreate.h>
#include <cpss/generic/tm/cpssTmNodesTree.h>
#include <cpssCommon/private/prvCpssMath.h>
#include <cpss/generic/tm/cpssTmUtDefs.h>
#include <utf/utfMain.h>
#include <utf/private/prvUtfExtras.h>





/*******************************************************************************
GT_STATUS cpssTmPortCreate
(
    IN GT_U8                        devNum,
    IN GT_U32                       portInd,
    IN CPSS_TM_PORT_PARAMS_STC      *paramsPtr,
    IN GT_U32                       cNodesNum,
    IN GT_U32                       bNodesNum,
    IN GT_U32                       aNodesNum,
    IN GT_U32                       queuesNum
)
*/
UTF_TEST_CASE_MAC(cpssTmPortCreate)
{
    GT_STATUS               st = GT_OK;

    GT_U8                        dev;
    GT_U32                       portInd = 1;
    CPSS_TM_PORT_PARAMS_STC      paramsPtr;
    GT_U32                       cNodesNum = 1;
    GT_U32                       bNodesNum = 1;
    GT_U32                       aNodesNum = 1;
    GT_U32                       queuesNum = 1;
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
        paramsPtr.cirBw = 1000000;
        paramsPtr.eirBw = 0;
        paramsPtr.cbs = 8; /*KBytes*/
        paramsPtr.ebs = 0x1FFFF;
        paramsPtr.eligiblePrioFuncId = CPSS_TM_ELIG_N_MIN_SHP_E;
        paramsPtr.dropProfileInd = CPSS_TM_NO_DROP_PROFILE_CNS;
        for(ii=0; ii<8; ii++){
            paramsPtr.schdModeArr[ii] = CPSS_TM_SCHD_MODE_RR_E;
            paramsPtr.quantumArr[ii] = UT_TM_MIN_PORT_QUANTUM_CNS;
        }

        portInd = 1;

        /*1.1 Check with legal parameters.
                Expected: GT_OK */

        st = cpssTmPortCreate(dev, portInd, &paramsPtr, cNodesNum, bNodesNum, aNodesNum, queuesNum);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);


        portInd = 1;
        /*
            1.3 check out of range paramsPtr.cirBw.
            Call with paramsPtr.cirBw [200000000], other parameters same as in 1.1.
            Expected: NON GT_OK.
        */

        paramsPtr.cirBw = 200000000;

        st = cpssTmPortCreate(dev, portInd, &paramsPtr, cNodesNum, bNodesNum, aNodesNum, queuesNum);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, paramsPtr.cirBw);

        paramsPtr.cirBw = 1000000;

         /*
            1.4 check out of range paramsPtr.eirBw.
            Call with paramsPtr.eirBw [200000000], other parameters same as in 1.1.
            Expected: NON GT_OK.
        */

        paramsPtr.eirBw = 200000000;

        st = cpssTmPortCreate(dev, portInd, &paramsPtr, cNodesNum, bNodesNum, aNodesNum, queuesNum);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, paramsPtr.eirBw);

        paramsPtr.eirBw = 0;

         /*
            1.5 check out of range paramsPtr.quantumArr.
            Call with paramsPtr.quantumArr[0] [4], other parameters same as in 1.1.
            Expected: NON GT_OK.
        */

        paramsPtr.quantumArr[0] = 512;

        st = cpssTmPortCreate(dev, portInd, &paramsPtr, cNodesNum, bNodesNum, aNodesNum, queuesNum);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, paramsPtr.quantumArr[0]);

        paramsPtr.quantumArr[0] = UT_TM_MIN_PORT_QUANTUM_CNS;

         /*
            1.6 check out of range paramsPtr.paramsPtr.quantumArr.
            Call with paramsPtr.quantumArr[1] [1], other parameters same as in 1.1.
            Expected: NON GT_OK.
        */

        paramsPtr.quantumArr[1] = 1;

        st = cpssTmPortCreate(dev, portInd, &paramsPtr, cNodesNum, bNodesNum, aNodesNum, queuesNum);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, paramsPtr.quantumArr[0]);

        paramsPtr.quantumArr[1]= UT_TM_MIN_PORT_QUANTUM_CNS;

         /*
            1.7 check out of range paramsPtr.eirBw.
            Call with paramsPtr.schdModeArr[0] [2], other parameters same as in 1.1.
            Expected: NON GT_OK.
        */

        paramsPtr.schdModeArr[0] = 2;

        st = cpssTmPortCreate(dev, portInd, &paramsPtr, cNodesNum, bNodesNum, aNodesNum, queuesNum);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, paramsPtr.schdModeArr[0]);

        paramsPtr.schdModeArr[0] = CPSS_TM_SCHD_MODE_RR_E;

         /*
            1.8 check out of range paramsPtr.eirBw.
            Call with paramsPtr.dropProfileInd [300], other parameters same as in 1.1.
            Expected: NON GT_OK.
        */

        paramsPtr.dropProfileInd = 300;

        st = cpssTmPortCreate(dev, portInd, &paramsPtr, cNodesNum, bNodesNum, aNodesNum, queuesNum);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, paramsPtr.dropProfileInd);

        paramsPtr.dropProfileInd = CPSS_TM_NO_DROP_PROFILE_CNS;

         /*
            1.9 check out of range paramsPtr.eirBw.
            Call with paramsPtr.eligiblePrioFuncId [70], other parameters same as in 1.1.
            Expected: NON GT_OK.
        */

        paramsPtr.eligiblePrioFuncId = 70;

        st = cpssTmPortCreate(dev, portInd, &paramsPtr, cNodesNum, bNodesNum, aNodesNum, queuesNum);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, paramsPtr.eligiblePrioFuncId);

        paramsPtr.eligiblePrioFuncId = CPSS_TM_ELIG_N_MIN_SHP_E;


        /*
            1.10 Call with paramsPtr [NULL], other parameters same as in 1.1.
            Expected: GT_BAD_PTR.
        */

        st = cpssTmPortCreate(dev, portInd, NULL, cNodesNum, bNodesNum, aNodesNum, queuesNum);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, paramsPtr = NULL", dev);


        /*
            1.11 Call with infinit shaper - https://jirail.marvell.com/browse/CPSS-4814
            Expected: GT_OK
        */

        paramsPtr.cirBw = CPSS_TM_SHAPING_INFINITE_BW_CNS;
        paramsPtr.eirBw = 0;
        paramsPtr.cbs = 8; /*KBytes*/
        paramsPtr.ebs = 0;

        /* verify port shaping params */
        st =  cpssTmPortShapingValidate(dev, CPSS_TM_SHAPING_INFINITE_BW_CNS, 0, &paramsPtr.cbs, &paramsPtr.ebs);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* verify create infinit shaping with non shaping elig function*/
        paramsPtr.eligiblePrioFuncId = CPSS_TM_ELIG_N_PRIO1_E;

        portInd = 3;
        st = cpssTmPortCreate(dev, portInd, &paramsPtr, cNodesNum, bNodesNum, aNodesNum, queuesNum);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* verify create infinit shaping with shaping elig function*/
        paramsPtr.cirBw = 100000;
        paramsPtr.eirBw = CPSS_TM_SHAPING_INFINITE_BW_CNS;
        paramsPtr.cbs = 8; /*KBytes*/
        paramsPtr.ebs = 0;

        /* verify create infinit shaping with non shaping elig function*/
        paramsPtr.eligiblePrioFuncId = CPSS_TM_ELIG_N_MIN_SHP_E;

        portInd = 4;
        st = cpssTmPortCreate(dev, portInd, &paramsPtr, cNodesNum, bNodesNum, aNodesNum, queuesNum);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        st = cpssTmClose(dev);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* TM LIB not initialized */

        /*1.11 Check when TM LIB not initialized.
               Expected: GT_NOT_INITIALIZED */

        st = cpssTmPortCreate(dev, portInd, &paramsPtr, cNodesNum, bNodesNum, aNodesNum, queuesNum);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_INITIALIZED, st, dev);
    }


    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_NOT_APPLICABLE_DEVICE.            */
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);


    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssTmPortCreate(dev, portInd, &paramsPtr, cNodesNum, bNodesNum, aNodesNum, queuesNum);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssTmPortCreate(dev, portInd, &paramsPtr, cNodesNum, bNodesNum, aNodesNum, queuesNum);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}


/*******************************************************************************
GT_STATUS cpssTmAsymPortCreate
(
    IN GT_U8                        devNum,
    IN GT_U32                       portInd,
    IN CPSS_TM_PORT_PARAMS_STC      *paramsPtr
)
*/
UTF_TEST_CASE_MAC(cpssTmAsymPortCreate)
{
    GT_STATUS               st = GT_OK;

    GT_U8                        dev;
    GT_U32                       portInd = 1;
    CPSS_TM_PORT_PARAMS_STC      paramsPtr;
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
        paramsPtr.cirBw = 1000000;
        paramsPtr.eirBw = 0;
        paramsPtr.cbs = 8; /*KBytes*/
        paramsPtr.ebs = 0x1FFFF;
        paramsPtr.eligiblePrioFuncId = CPSS_TM_ELIG_N_MIN_SHP_E;
        paramsPtr.dropProfileInd = CPSS_TM_NO_DROP_PROFILE_CNS;
        for(ii=0; ii<8; ii++){
            paramsPtr.schdModeArr[ii] = CPSS_TM_SCHD_MODE_RR_E;
            paramsPtr.quantumArr[ii] = UT_TM_MIN_PORT_QUANTUM_CNS;
        }
        paramsPtr.numOfChildren = 1;


        /*1.1 Check with legal parameters.
                Expected: GT_OK */

        st = cpssTmAsymPortCreate(dev, portInd, &paramsPtr);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);



        portInd = 1;

        /*
            1.3 check out of range paramsPtr.cirBw.
            Call with paramsPtr.cirBw [200000000], other parameters same as in 1.1.
            Expected: NON GT_OK.
        */

        paramsPtr.cirBw = 200000000;

        st = cpssTmAsymPortCreate(dev, portInd, &paramsPtr);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, paramsPtr.cirBw);

        paramsPtr.cirBw= 1000000;

         /*
            1.4 check out of range paramsPtr.eirBw.
            Call with paramsPtr.eirBw [200000000], other parameters same as in 1.1.
            Expected: NON GT_OK.
        */

        paramsPtr.eirBw = 200000000;

        st = cpssTmAsymPortCreate(dev, portInd, &paramsPtr);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, paramsPtr.eirBw);

        paramsPtr.eirBw= 0;

         /*
            1.5 check out of range paramsPtr.eirBw.
            Call with paramsPtr.quantumArr[0] [1], other parameters same as in 1.1.
            Expected: NON GT_OK.
        */

        paramsPtr.quantumArr[0] = 1;

        st = cpssTmAsymPortCreate(dev, portInd, &paramsPtr);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, paramsPtr.quantumArr[0]);

        paramsPtr.quantumArr[0]= UT_TM_MIN_PORT_QUANTUM_CNS;

         /*
            1.6 check out of range paramsPtr.eirBw.
            Call with paramsPtr.quantumArr[0] [0x2ff], other parameters same as in 1.1.
            Expected: NON GT_OK.
        */

        paramsPtr.quantumArr[0] = 0x2ff;

        st = cpssTmAsymPortCreate(dev, portInd, &paramsPtr);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, paramsPtr.quantumArr[0]);

        paramsPtr.quantumArr[0] = UT_TM_MIN_PORT_QUANTUM_CNS;

         /*
            1.7 check out of range paramsPtr.eirBw.
            Call with paramsPtr.schdModeArr[0] [2], other parameters same as in 1.1.
            Expected: NON GT_OK.
        */

        paramsPtr.schdModeArr[0] = 2;

        st = cpssTmAsymPortCreate(dev, portInd, &paramsPtr);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, paramsPtr.schdModeArr[0]);

        paramsPtr.schdModeArr[0] = CPSS_TM_SCHD_MODE_RR_E;

         /*
            1.8 check out of range paramsPtr.dropProfileIndArr.
            Call with paramsPtr.dropProfileInd [300], other parameters same as in 1.1.
            Expected: NON GT_OK.
        */

        paramsPtr.dropProfileInd = 300;

        st = cpssTmAsymPortCreate(dev, portInd, &paramsPtr);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, paramsPtr.dropProfileInd);

        paramsPtr.dropProfileInd = CPSS_TM_NO_DROP_PROFILE_CNS;

         /*
            1.9 check out of range paramsPtr.eirBw.
            Call with paramsPtr.eligiblePrioFuncId [70], other parameters same as in 1.1.
            Expected: NON GT_OK.
        */

        paramsPtr.eligiblePrioFuncId = 70;

        st = cpssTmAsymPortCreate(dev, portInd, &paramsPtr);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, paramsPtr.eligiblePrioFuncId);

        paramsPtr.eligiblePrioFuncId = CPSS_TM_ELIG_N_MIN_SHP_E;


        /*
            1.10 Call with paramsPtr [NULL], other parameters same as in 1.1.
            Expected: GT_BAD_PTR.
        */

        st = cpssTmAsymPortCreate(dev, portInd, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, paramsPtr = NULL", dev);



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
        st = cpssTmAsymPortCreate(dev, portInd, &paramsPtr);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssTmAsymPortCreate(dev, portInd, &paramsPtr);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}


/*******************************************************************************
GT_STATUS cpssTmPortDropPerCosConfig
(
    IN GT_U8                         devNum,
    IN GT_U32                        portInd,
    IN CPSS_TM_PORT_DROP_PER_COS_STC *paramsPtr
)
*/
UTF_TEST_CASE_MAC(cpssTmPortDropPerCosConfig)
{
    GT_STATUS               st = GT_OK;

    GT_U8                           dev;
    GT_U32                          portInd = 1;
    CPSS_TM_PORT_PARAMS_STC         paramsPtr;
    CPSS_TM_PORT_DROP_PER_COS_STC   params;
    CPSS_PP_FAMILY_TYPE_ENT         devFamily;
    GT_U32                          ii;

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
        paramsPtr.dropProfileInd = CPSS_TM_NO_DROP_PROFILE_CNS;
        for(ii=0; ii<8; ii++){
            paramsPtr.schdModeArr[ii] = CPSS_TM_SCHD_MODE_RR_E;
            paramsPtr.quantumArr[ii] = UT_TM_MIN_PORT_QUANTUM_CNS;
        }
        paramsPtr.numOfChildren = 1;

        st = cpssTmAsymPortCreate(dev, portInd, &paramsPtr);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);


        /* Configuration of port Drop per Cos params */
        params.dropCosMap = 1;
        for(ii=0; ii<8; ii++)
            params.dropProfileIndArr[ii] = CPSS_TM_NO_DROP_PROFILE_CNS;

        /*1.1 Check with legal parameters.
                Expected: GT_OK */

        st = cpssTmPortDropPerCosConfig(dev, portInd, &params);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.2 Check for wrong values portInd and other params same as in 1.1
            Expected: GT_BAD_PARAM.
        */

        portInd = 2;
        st = cpssTmPortDropPerCosConfig(dev, portInd, &params);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, portInd);

        portInd = 1;

        /*
            1.3 check out of range paramsPtr.cirBw.
            Call with paramsPtr.cirBw [200000000], other parameters same as in 1.1.
            Expected: NON GT_OK.
        */

        params.dropProfileIndArr[0] = 10;

        st = cpssTmPortDropPerCosConfig(dev, portInd, &params);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, params.dropProfileIndArr[0]);

        params.dropProfileIndArr[0] = CPSS_TM_NO_DROP_PROFILE_CNS;

        /*
            1.4 Call with params [NULL], other parameters same as in 1.1.
            Expected: GT_BAD_PTR.
        */

        st = cpssTmPortDropPerCosConfig(dev, portInd, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, params = NULL", dev);


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
        st = cpssTmPortDropPerCosConfig(dev, portInd, &params);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssTmPortDropPerCosConfig(dev, portInd, &params);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}


/*******************************************************************************
GT_STATUS cpssTmQueueToPortCreate
(
    IN  GT_U8                           devNum,
    IN  GT_U32                          portInd,
    IN  CPSS_TM_QUEUE_PARAMS_STC        *qParamsPtr,
    IN  CPSS_TM_A_NODE_PARAMS_STC       *aParamsPtr,
    IN  CPSS_TM_B_NODE_PARAMS_STC       *bParamsPtr,
    IN  CPSS_TM_C_NODE_PARAMS_STC       *cParamsPtr,
    OUT GT_U32                          *queueIndPtr,
    OUT GT_U32                          *aNodeIndPtr,
    OUT GT_U32                          *bNodeIndPtr,
    OUT GT_U32                          *cNodeIndPtr
)
*/
UTF_TEST_CASE_MAC(cpssTmQueueToPortCreate)
{
    GT_STATUS               st = GT_OK;

    GT_U8                           dev;
    GT_U32                          portInd = 1;
    CPSS_TM_QUEUE_PARAMS_STC        qParamsPtr;
    CPSS_TM_A_NODE_PARAMS_STC       aParamsPtr;
    CPSS_TM_B_NODE_PARAMS_STC       bParamsPtr;
    CPSS_TM_C_NODE_PARAMS_STC       cParamsPtr;
    GT_U32                          queueIndPtr;
    GT_U32                          aNodeIndPtr;
    GT_U32                          bNodeIndPtr;
    GT_U32                          cNodeIndPtr;
    CPSS_TM_PORT_PARAMS_STC      paramsPtr;
    GT_U32                       cNodesNum = 1;
    GT_U32                       bNodesNum = 1;
    GT_U32                       aNodesNum = 1;
    GT_U32                       queuesNum = 1;
    CPSS_PP_FAMILY_TYPE_ENT devFamily;
    GT_U32                       ii;

     /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev,
                                 UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_LION_E |
                                     UTF_PUMA2_E | UTF_PUMA3_E | UTF_XCAT2_E | UTF_LION2_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E | UTF_ALDRIN_E | UTF_AC3X_E);

    cpssOsBzero((GT_VOID*)&qParamsPtr, sizeof(qParamsPtr));
    cpssOsBzero((GT_VOID*)&aParamsPtr, sizeof(aParamsPtr));
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
        aParamsPtr.shapingProfilePtr = 0;
        aParamsPtr.quantum = 0x40;
        for (ii=0; ii<8; ii++)
            aParamsPtr.schdModeArr[ii] = CPSS_TM_SCHD_MODE_RR_E; /* No DWRR configured*/
        aParamsPtr.dropProfileInd = 0;
        aParamsPtr.eligiblePrioFuncId = 8;

        /* Configure parameters for node B */
        bParamsPtr.shapingProfilePtr=0;
        bParamsPtr.quantum= 0x40;
        for (ii=0; ii<8; ii++)
            bParamsPtr.schdModeArr[ii] = CPSS_TM_SCHD_MODE_RR_E; /* No DWRR configured*/
        bParamsPtr.dropProfileInd =0;
        bParamsPtr.eligiblePrioFuncId =9;

        /* Configure parameters for node C */
        cParamsPtr.shapingProfilePtr= 0;
        cParamsPtr.quantum =0x40;
        cParamsPtr.dropCosMap =1;
        cParamsPtr.eligiblePrioFuncId =9;
        for (ii=0; ii<8; ii++){
            cParamsPtr.schdModeArr[ii] = CPSS_TM_SCHD_MODE_RR_E;
            cParamsPtr.dropProfileIndArr[ii] = 0;
        }

        /* profile for Q level */
        qParamsPtr.shapingProfilePtr=0;
        qParamsPtr.quantum=0x40;
        qParamsPtr.dropProfileInd=0;
        qParamsPtr.eligiblePrioFuncId=1;

        /* Configure parameters for port */
        paramsPtr.cirBw=1000000;
        paramsPtr.eirBw=0;
        paramsPtr.cbs=8; /*KBytes*/
        paramsPtr.ebs=0x1FFFF;
        paramsPtr.eligiblePrioFuncId= CPSS_TM_ELIG_N_MIN_SHP_E;
        for(ii=0; ii<8; ii++){
            paramsPtr.schdModeArr[ii] = CPSS_TM_SCHD_MODE_RR_E;
            paramsPtr.quantumArr[ii] = UT_TM_MIN_PORT_QUANTUM_CNS;
        }
        paramsPtr.dropProfileInd = CPSS_TM_NO_DROP_PROFILE_CNS;

        st = cpssTmPortCreate(dev, portInd, &paramsPtr, cNodesNum, bNodesNum, aNodesNum,queuesNum );
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*1.1 Check with legal parameters.
                Expected: GT_OK */

        st = cpssTmQueueToPortCreate(dev, portInd, &qParamsPtr, &aParamsPtr, &bParamsPtr, &cParamsPtr, &queueIndPtr,&aNodeIndPtr, &bNodeIndPtr, &cNodeIndPtr );
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.2 check out of range qParamsPtr.shapingProfilePtr.
            Call with qParamsPtr.shapingProfilePtr [85580], other parameters same as in 1.1.
            Expected: NON GT_OK.
        */

        qParamsPtr.shapingProfilePtr = 85580;

        st = cpssTmQueueToPortCreate(dev, portInd, &qParamsPtr, &aParamsPtr, &bParamsPtr, &cParamsPtr, &queueIndPtr,&aNodeIndPtr, &bNodeIndPtr, &cNodeIndPtr );

        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, qParamsPtr.shapingProfilePtr);

        qParamsPtr.shapingProfilePtr= 0;

         /*
            1.3 check out of range qParamsPtr.quantum.
            Call with qParamsPtr.quantum [0x30], other parameters same as in 1.1.
            Expected: NON GT_OK.
        */

        qParamsPtr.quantum=0x30;

        st = cpssTmQueueToPortCreate(dev, portInd, &qParamsPtr, &aParamsPtr, &bParamsPtr, &cParamsPtr, &queueIndPtr,&aNodeIndPtr, &bNodeIndPtr, &cNodeIndPtr );

        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, qParamsPtr.quantum);

        qParamsPtr.quantum=0x40;

         /*
            1.4 check out of range qParamsPtr.quantum.
            Call with qParamsPtr.quantum [0x4fff], other parameters same as in 1.1.
            Expected: NON GT_OK.
        */

        qParamsPtr.quantum=0x4fff;

        st = cpssTmQueueToPortCreate(dev, portInd, &qParamsPtr, &aParamsPtr, &bParamsPtr, &cParamsPtr, &queueIndPtr,&aNodeIndPtr, &bNodeIndPtr, &cNodeIndPtr );

        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, qParamsPtr.quantum);

        qParamsPtr.quantum=0x40;

         /*
            1.5 check out of range qParamsPtr.dropProfileInd.
            Call with qParamsPtr.dropProfileInd [300], other parameters same as in 1.1.
            Expected: NON GT_OK.
        */

        qParamsPtr.dropProfileInd=300;

        st = cpssTmQueueToPortCreate(dev, portInd, &qParamsPtr, &aParamsPtr, &bParamsPtr, &cParamsPtr, &queueIndPtr,&aNodeIndPtr, &bNodeIndPtr, &cNodeIndPtr );

        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev,qParamsPtr.dropProfileInd);

        qParamsPtr.dropProfileInd=0;

         /*
            1.6 check out of range qParamsPtr.eligiblePrioFuncId.
            Call with qParamsPtr.eligiblePrioFuncId [70], other parameters same as in 1.1.
            Expected: NON GT_OK.
        */

        qParamsPtr.eligiblePrioFuncId=70;

         st = cpssTmQueueToPortCreate(dev, portInd, &qParamsPtr, &aParamsPtr, &bParamsPtr, &cParamsPtr, &queueIndPtr,&aNodeIndPtr, &bNodeIndPtr, &cNodeIndPtr );
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, qParamsPtr.eligiblePrioFuncId);

        qParamsPtr.eligiblePrioFuncId=1;

        /*
            1.7 check out of range aParamsPtr.shapingProfilePtr.
            Call with aParamsPtr.shapingProfilePtr [85580], other parameters same as in 1.1.
            Expected: NON GT_OK.
        */

        aParamsPtr.shapingProfilePtr = 85580;

        st = cpssTmQueueToPortCreate(dev, portInd, &qParamsPtr, &aParamsPtr, &bParamsPtr, &cParamsPtr, &queueIndPtr,&aNodeIndPtr, &bNodeIndPtr, &cNodeIndPtr );

        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, aParamsPtr.shapingProfilePtr);

        aParamsPtr.shapingProfilePtr= 0;

         /*
            1.8 check out of range aParamsPtr.quantum.
            Call with aParamsPtr.quantum [0x30], other parameters same as in 1.1.
            Expected: NON GT_OK.
        */

        aParamsPtr.quantum=0x30;

        st = cpssTmQueueToPortCreate(dev, portInd, &qParamsPtr, &aParamsPtr, &bParamsPtr, &cParamsPtr, &queueIndPtr,&aNodeIndPtr, &bNodeIndPtr, &cNodeIndPtr );

        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, aParamsPtr.quantum);

        aParamsPtr.quantum=0x40;

         /*
            1.9 check out of range aParamsPtr.quantum.
            Call with aParamsPtr.quantum [0x4fff], other parameters same as in 1.1.
            Expected: NON GT_OK.
        */

        aParamsPtr.quantum=0x4fff;

        st = cpssTmQueueToPortCreate(dev, portInd, &qParamsPtr, &aParamsPtr, &bParamsPtr, &cParamsPtr, &queueIndPtr,&aNodeIndPtr, &bNodeIndPtr, &cNodeIndPtr );

        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, aParamsPtr.quantum);

        aParamsPtr.quantum=0x40;

         /*
            1.10 check out of range aParamsPtr.dropProfileInd.
            Call with aParamsPtr.dropProfileInd [300], other parameters same as in 1.1.
            Expected: NON GT_OK.
        */

        aParamsPtr.dropProfileInd=300;

        st = cpssTmQueueToPortCreate(dev, portInd, &qParamsPtr, &aParamsPtr, &bParamsPtr, &cParamsPtr, &queueIndPtr,&aNodeIndPtr, &bNodeIndPtr, &cNodeIndPtr );

        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev,aParamsPtr.dropProfileInd);

        aParamsPtr.dropProfileInd=0;

         /*
            1.11 check out of range aParamsPtr.eligiblePrioFuncId.
            Call with aParamsPtr.eligiblePrioFuncId [70], other parameters same as in 1.1.
            Expected: NON GT_OK.
        */

        aParamsPtr.eligiblePrioFuncId=70;

        st = cpssTmQueueToPortCreate(dev, portInd, &qParamsPtr, &aParamsPtr, &bParamsPtr, &cParamsPtr, &queueIndPtr,&aNodeIndPtr, &bNodeIndPtr, &cNodeIndPtr );
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, aParamsPtr.eligiblePrioFuncId);

        aParamsPtr.eligiblePrioFuncId=1;

         /*
            1.12 check out of range aParamsPtr.schdModeArr[0].
            Call with aParamsPtr.schdModeArr[0] [2], other parameters same as in 1.1.
            Expected: NON GT_OK.
        */

        aParamsPtr.schdModeArr[0] = 2;

        st = cpssTmQueueToPortCreate(dev, portInd, &qParamsPtr, &aParamsPtr, &bParamsPtr, &cParamsPtr, &queueIndPtr,&aNodeIndPtr, &bNodeIndPtr, &cNodeIndPtr );
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, aParamsPtr.schdModeArr[0]);

        aParamsPtr.schdModeArr[0] = CPSS_TM_SCHD_MODE_RR_E;

        /*
            1.13 check out of range bParamsPtr.shapingProfilePtr.
            Call with bParamsPtr.shapingProfilePtr [85580], other parameters same as in 1.1.
            Expected: NON GT_OK.
        */

        bParamsPtr.shapingProfilePtr = 85580;

        st = cpssTmQueueToPortCreate(dev, portInd, &qParamsPtr, &aParamsPtr, &bParamsPtr, &cParamsPtr, &queueIndPtr,&aNodeIndPtr, &bNodeIndPtr, &cNodeIndPtr );

        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, bParamsPtr.shapingProfilePtr);

        bParamsPtr.shapingProfilePtr= 0;

         /*
            1.14 check out of range bParamsPtr.quantum.
            Call with bParamsPtr.quantum [0x30], other parameters same as in 1.1.
            Expected: NON GT_OK.
        */

        bParamsPtr.quantum=0x30;

        st = cpssTmQueueToPortCreate(dev, portInd, &qParamsPtr, &aParamsPtr, &bParamsPtr, &cParamsPtr, &queueIndPtr,&aNodeIndPtr, &bNodeIndPtr, &cNodeIndPtr );

        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, bParamsPtr.quantum);

        bParamsPtr.quantum=0x40;

         /*
            1.15 check out of range bParamsPtr.quantum.
            Call with bParamsPtr.quantum [0x4fff], other parameters same as in 1.1.
            Expected: NON GT_OK.
        */

        bParamsPtr.quantum=0x4fff;

        st = cpssTmQueueToPortCreate(dev, portInd, &qParamsPtr, &aParamsPtr, &bParamsPtr, &cParamsPtr, &queueIndPtr,&aNodeIndPtr, &bNodeIndPtr, &cNodeIndPtr );

        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, bParamsPtr.quantum);

        bParamsPtr.quantum=0x40;

         /*
            1.16 check out of range bParamsPtr.dropProfileInd.
            Call with bParamsPtr.dropProfileInd [300], other parameters same as in 1.1.
            Expected: NON GT_OK.
        */

        bParamsPtr.dropProfileInd=300;

        st = cpssTmQueueToPortCreate(dev, portInd, &qParamsPtr, &aParamsPtr, &bParamsPtr, &cParamsPtr, &queueIndPtr,&aNodeIndPtr, &bNodeIndPtr, &cNodeIndPtr );

        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev,bParamsPtr.dropProfileInd);

        bParamsPtr.dropProfileInd=0;

         /*
            1.17 check out of range bParamsPtr.eligiblePrioFuncId.
            Call with bParamsPtr.eligiblePrioFuncId [70], other parameters same as in 1.1.
            Expected: NON GT_OK.
        */

        bParamsPtr.eligiblePrioFuncId=70;

        st = cpssTmQueueToPortCreate(dev, portInd, &qParamsPtr, &aParamsPtr, &bParamsPtr, &cParamsPtr, &queueIndPtr,&aNodeIndPtr, &bNodeIndPtr, &cNodeIndPtr );
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, bParamsPtr.eligiblePrioFuncId);

        bParamsPtr.eligiblePrioFuncId=1;

         /*
            1.18 check out of range bParamsPtr.schdModeArr[0].
            Call with bParamsPtr.schdModeArr[0] [2], other parameters same as in 1.1.
            Expected: NON GT_OK.
        */

        bParamsPtr.schdModeArr[0] = 2;

        st = cpssTmQueueToPortCreate(dev, portInd, &qParamsPtr, &aParamsPtr, &bParamsPtr, &cParamsPtr, &queueIndPtr,&aNodeIndPtr, &bNodeIndPtr, &cNodeIndPtr );
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, bParamsPtr.schdModeArr[0]);

        bParamsPtr.schdModeArr[0] = CPSS_TM_SCHD_MODE_RR_E;

        /*
            1.19 check out of range cParamsPtr.shapingProfilePtr.
            Call with cParamsPtr.shapingProfilePtr [85580], other parameters same as in 1.1.
            Expected: NON GT_OK.
        */

        cParamsPtr.shapingProfilePtr = 85580;

        st = cpssTmQueueToPortCreate(dev, portInd, &qParamsPtr, &aParamsPtr, &bParamsPtr, &cParamsPtr, &queueIndPtr,&aNodeIndPtr, &bNodeIndPtr, &cNodeIndPtr );

        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, cParamsPtr.shapingProfilePtr);

        cParamsPtr.shapingProfilePtr= 0;

         /*
            1.20 check out of range cParamsPtr.quantum.
            Call with cParamsPtr.quantum [0x30], other parameters same as in 1.1.
            Expected: NON GT_OK.
        */

        cParamsPtr.quantum=0x30;

        st = cpssTmQueueToPortCreate(dev, portInd, &qParamsPtr, &aParamsPtr, &bParamsPtr, &cParamsPtr, &queueIndPtr,&aNodeIndPtr, &bNodeIndPtr, &cNodeIndPtr );

        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, cParamsPtr.quantum);

        cParamsPtr.quantum=0x40;

         /*
            1.21 check out of range cParamsPtr.quantum.
            Call with cParamsPtr.quantum [0x4fff], other parameters same as in 1.1.
            Expected: NON GT_OK.
        */

        cParamsPtr.quantum=0x4fff;

        st = cpssTmQueueToPortCreate(dev, portInd, &qParamsPtr, &aParamsPtr, &bParamsPtr, &cParamsPtr, &queueIndPtr,&aNodeIndPtr, &bNodeIndPtr, &cNodeIndPtr );

        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, cParamsPtr.quantum);

        cParamsPtr.quantum=0x40;

         /*
            1.22 check out of range cParamsPtr.dropProfileInd.
            Call with cParamsPtr.dropProfileIndArr[0][300], other parameters same as in 1.1.
            Expected: NON GT_OK.
        */

        cParamsPtr.dropProfileIndArr[0]=300;

        st = cpssTmQueueToPortCreate(dev, portInd, &qParamsPtr, &aParamsPtr, &bParamsPtr, &cParamsPtr, &queueIndPtr,&aNodeIndPtr, &bNodeIndPtr, &cNodeIndPtr );

        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev,cParamsPtr.dropProfileIndArr[0]);

        cParamsPtr.dropProfileIndArr[0]=0;

         /*
            1.23 check out of range cParamsPtr.eligiblePrioFuncId.
            Call with cParamsPtr.eligiblePrioFuncId [70], other parameters same as in 1.1.
            Expected: NON GT_OK.
        */

        cParamsPtr.eligiblePrioFuncId=70;

        st = cpssTmQueueToPortCreate(dev, portInd, &qParamsPtr, &aParamsPtr, &bParamsPtr, &cParamsPtr, &queueIndPtr,&aNodeIndPtr, &bNodeIndPtr, &cNodeIndPtr );
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, cParamsPtr.eligiblePrioFuncId);

        cParamsPtr.eligiblePrioFuncId=1;

         /*
            1.24 check out of range cParamsPtr.schdModeArr[0].
            Call with cParamsPtr.schdModeArr[0] [2], other parameters same as in 1.1.
            Expected: NON GT_OK.
        */

        cParamsPtr.schdModeArr[0] = 2;

        st = cpssTmQueueToPortCreate(dev, portInd, &qParamsPtr, &aParamsPtr, &bParamsPtr, &cParamsPtr, &queueIndPtr,&aNodeIndPtr, &bNodeIndPtr, &cNodeIndPtr );
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, cParamsPtr.schdModeArr[0]);

        cParamsPtr.schdModeArr[0] = CPSS_TM_SCHD_MODE_RR_E;

         /*
            1.25 check out of range cParamsPtr.dropCosMap.
            Call with cParamsPtr.dropCosMap [9], other parameters same as in 1.1.
            Expected: NON GT_OK.
        */

        cParamsPtr.dropCosMap=9;

        st = cpssTmQueueToPortCreate(dev, portInd, &qParamsPtr, &aParamsPtr, &bParamsPtr, &cParamsPtr, &queueIndPtr,&aNodeIndPtr, &bNodeIndPtr, &cNodeIndPtr );
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, cParamsPtr.dropCosMap);

        cParamsPtr.dropCosMap=1;

        /*
            1.26 Call with qParamsPtr [NULL], other parameters same as in 1.1.
            Expected: GT_BAD_PTR.
        */

        st = cpssTmQueueToPortCreate(dev, portInd, NULL, &aParamsPtr, &bParamsPtr, &cParamsPtr, &queueIndPtr,&aNodeIndPtr, &bNodeIndPtr, &cNodeIndPtr );
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, qParamsPtr = NULL", dev);

        /*
            1.27 Call with aParamsPtr [NULL], other parameters same as in 1.1.
            Expected: GT_BAD_PTR.
        */

        st = cpssTmQueueToPortCreate(dev, portInd, &qParamsPtr, NULL, &bParamsPtr, &cParamsPtr, &queueIndPtr,&aNodeIndPtr, &bNodeIndPtr, &cNodeIndPtr );
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, aParamsPtr = NULL", dev);

        /*
            1.28 Call with bParamsPtr [NULL], other parameters same as in 1.1.
            Expected: GT_BAD_PTR.
        */

        st = cpssTmQueueToPortCreate(dev, portInd, &qParamsPtr, &aParamsPtr, NULL, &cParamsPtr, &queueIndPtr,&aNodeIndPtr, &bNodeIndPtr, &cNodeIndPtr );
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, bParamsPtr = NULL", dev);

        /*
            1.29 Call with cParamsPtr [NULL], other parameters same as in 1.1.
            Expected: GT_BAD_PTR.
        */

        st = cpssTmQueueToPortCreate(dev, portInd, &qParamsPtr, &aParamsPtr, &bParamsPtr, NULL, &queueIndPtr,&aNodeIndPtr, &bNodeIndPtr, &cNodeIndPtr );
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, cParamsPtr = NULL", dev);

        /*
            1.30 Call with queueIndPtr [NULL], other parameters same as in 1.1.
            Expected: GT_BAD_PTR.
        */

        st = cpssTmQueueToPortCreate(dev, portInd, &qParamsPtr, &aParamsPtr, &bParamsPtr, &cParamsPtr, NULL,&aNodeIndPtr, &bNodeIndPtr, &cNodeIndPtr );
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, queueIndPtr = NULL", dev);

        /*
            1.31 Call with aNodeIndPtr [NULL], other parameters same as in 1.1.
            Expected: GT_BAD_PTR.
        */

        st = cpssTmQueueToPortCreate(dev, portInd, &qParamsPtr, &aParamsPtr, &bParamsPtr, &cParamsPtr, &queueIndPtr,NULL, &bNodeIndPtr, &cNodeIndPtr );
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, aNodeIndPtr = NULL", dev);

        /*
            1.32 Call with bNodeIndPtr [NULL], other parameters same as in 1.1.
            Expected: GT_BAD_PTR.
        */

        st = cpssTmQueueToPortCreate(dev, portInd, &qParamsPtr, &aParamsPtr, &bParamsPtr, &cParamsPtr, &queueIndPtr,&aNodeIndPtr, NULL, &cNodeIndPtr );
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, bNodeIndPtr = NULL", dev);

        /*
            1.33 Call with cNodeIndPtr [NULL], other parameters same as in 1.1.
            Expected: GT_BAD_PTR.
        */

        st = cpssTmQueueToPortCreate(dev, portInd, &qParamsPtr, &aParamsPtr, &bParamsPtr, &cParamsPtr, &queueIndPtr,&aNodeIndPtr, &bNodeIndPtr, NULL );
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, cNodeIndPtr = NULL", dev);

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
        st = cpssTmQueueToPortCreate(dev, portInd, &qParamsPtr, &aParamsPtr, &bParamsPtr, &cParamsPtr, &queueIndPtr,&aNodeIndPtr, &bNodeIndPtr, &cNodeIndPtr );
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssTmQueueToPortCreate(dev, portInd, &qParamsPtr, &aParamsPtr, &bParamsPtr, &cParamsPtr, &queueIndPtr,&aNodeIndPtr, &bNodeIndPtr, &cNodeIndPtr );
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}


/*******************************************************************************
GT_STATUS cpssTmTransQueueToPortCreate
(
    IN  GT_U8                           devNum,
    IN  GT_U32                          portInd,
    IN  CPSS_TM_QUEUE_PARAMS_STC        *qParamsPtr,
    OUT GT_U32                          *queueIndPtr
)
*/
UTF_TEST_CASE_MAC(cpssTmTransQueueToPortCreate)
{
    GT_STATUS               st = GT_OK;

    GT_U8                        dev;
    GT_U32                       portInd=1;
    CPSS_TM_QUEUE_PARAMS_STC     qParamsPtr;
    GT_U32                       queueIndPtr;
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


        paramsPtr.cirBw=1000000;
        paramsPtr.eirBw=0;
        paramsPtr.cbs=8; /*KBytes*/
        paramsPtr.ebs=0x1FFFF;
        paramsPtr.eligiblePrioFuncId= CPSS_TM_ELIG_N_MIN_SHP_E;
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

        /*1.1 Check with legal parameters.
                Expected: GT_OK */

        st = cpssTmTransQueueToPortCreate(dev, portInd, &qParamsPtr, &queueIndPtr);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

                /*
            1.2 check out of range qParamsPtr.shapingProfilePtr.
            Call with qParamsPtr.shapingProfilePtr [85580], other parameters same as in 1.1.
            Expected: NON GT_OK.
        */

        qParamsPtr.shapingProfilePtr = 85580;

        st = cpssTmTransQueueToPortCreate(dev, portInd, &qParamsPtr, &queueIndPtr);

        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, qParamsPtr.shapingProfilePtr);

        qParamsPtr.shapingProfilePtr= 0;

         /*
            1.3 check out of range qParamsPtr.quantum.
            Call with qParamsPtr.quantum [0x30], other parameters same as in 1.1.
            Expected: NON GT_OK.
        */

        qParamsPtr.quantum=0x30;

        st = cpssTmTransQueueToPortCreate(dev, portInd, &qParamsPtr, &queueIndPtr);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, qParamsPtr.quantum);

        qParamsPtr.quantum=0x40;

         /*
            1.4 check out of range qParamsPtr.quantum.
            Call with qParamsPtr.quantum [0x4fff], other parameters same as in 1.1.
            Expected: NON GT_OK.
        */

        qParamsPtr.quantum=0x4fff;

        st = cpssTmTransQueueToPortCreate(dev, portInd, &qParamsPtr, &queueIndPtr);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, qParamsPtr.quantum);

        qParamsPtr.quantum=0x40;

         /*
            1.5 check out of range qParamsPtr.dropProfileInd.
            Call with qParamsPtr.dropProfileInd [300], other parameters same as in 1.1.
            Expected: NON GT_OK.
        */

        qParamsPtr.dropProfileInd=300;

        st = cpssTmTransQueueToPortCreate(dev, portInd, &qParamsPtr, &queueIndPtr);

        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev,qParamsPtr.dropProfileInd);

        qParamsPtr.dropProfileInd=0;

         /*
            1.6 check out of range qParamsPtr.eligiblePrioFuncId.
            Call with qParamsPtr.eligiblePrioFuncId [70], other parameters same as in 1.1.
            Expected: NON GT_OK.
        */

        qParamsPtr.eligiblePrioFuncId=70;

        st = cpssTmTransQueueToPortCreate(dev, portInd, &qParamsPtr, &queueIndPtr);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, qParamsPtr.eligiblePrioFuncId);

        qParamsPtr.eligiblePrioFuncId=1;

       /*
            1.7 Call with qParamsPtr [NULL], other parameters same as in 1.1.
            Expected: GT_BAD_PTR.
        */

        st = cpssTmTransQueueToPortCreate(dev, portInd, NULL, &queueIndPtr);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, qParamsPtr = NULL", dev);


        /*
            1.8 Call with queueIndPtr [NULL], other parameters same as in 1.1.
            Expected: GT_BAD_PTR.
        */

        st = cpssTmTransQueueToPortCreate(dev, portInd, &qParamsPtr, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, queueIndPtr = NULL", dev);

        /*
            1.9 check out of range portInd.
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
        st = cpssTmTransQueueToPortCreate(dev, portInd, &qParamsPtr, &queueIndPtr);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssTmTransQueueToPortCreate(dev, portInd, &qParamsPtr, &queueIndPtr);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}


/*******************************************************************************
GT_STATUS cpssTmQueueToCnodeCreate
(
    IN  GT_U8                           devNum,
    IN  GT_U32                          cNodeInd,
    IN  CPSS_TM_QUEUE_PARAMS_STC        *qParamsPtr,
    IN  CPSS_TM_A_NODE_PARAMS_STC       *aParamsPtr,
    IN  CPSS_TM_B_NODE_PARAMS_STC       *bParamsPtr,
    OUT GT_U32                          *queueIndPtr,
    OUT GT_U32                          *aNodeIndPtr,
    OUT GT_U32                          *bNodeIndPtr
)
*/
UTF_TEST_CASE_MAC(cpssTmQueueToCnodeCreate)
{
    GT_STATUS               st = GT_OK;

    GT_U8                           dev;
    GT_U32                          portInd = 1;
    CPSS_TM_QUEUE_PARAMS_STC        qParamsPtr;
    CPSS_TM_A_NODE_PARAMS_STC       aParamsPtr;
    CPSS_TM_B_NODE_PARAMS_STC       bParamsPtr;
    CPSS_TM_C_NODE_PARAMS_STC       cParamsPtr;
    GT_U32                          queueIndPtr;
    GT_U32                          aNodeIndPtr;
    GT_U32                          bNodeIndPtr;
    GT_U32                          cNodeIndPtr = 1;
    CPSS_TM_PORT_PARAMS_STC         paramsPtr;
    GT_U32                          cNodesNum;
    GT_U32                          bNodesNum;
    GT_U32                          aNodesNum;
    GT_U32                          queuesNum;
    CPSS_PP_FAMILY_TYPE_ENT devFamily;
    GT_U32                       ii;

     /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev,
                                 UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_LION_E |
                                     UTF_PUMA2_E | UTF_PUMA3_E | UTF_XCAT2_E | UTF_LION2_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E | UTF_ALDRIN_E | UTF_AC3X_E);

    cpssOsBzero((GT_VOID*)&aParamsPtr, sizeof(aParamsPtr));
    cpssOsBzero((GT_VOID*)&bParamsPtr, sizeof(bParamsPtr));
    cpssOsBzero((GT_VOID*)&cParamsPtr, sizeof(cParamsPtr));
    cpssOsBzero((GT_VOID*)&qParamsPtr, sizeof(qParamsPtr));
    cpssOsBzero((GT_VOID*)&paramsPtr, sizeof(paramsPtr));

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

        /* profile for Q level */
        qParamsPtr.shapingProfilePtr=0;
        qParamsPtr.quantum=0x40;
        qParamsPtr.dropProfileInd=0;
        qParamsPtr.eligiblePrioFuncId=1;

                /* profile for Q level */
        qParamsPtr.shapingProfilePtr=0;
        qParamsPtr.quantum=0x40;
        qParamsPtr.dropProfileInd=0;
        qParamsPtr.eligiblePrioFuncId=1;

        /* Configure parameters for port */
        paramsPtr.cirBw = 1000000;
        paramsPtr.eirBw = 0;
        paramsPtr.cbs = 8;/*KBytes*/
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

        st = cpssTmCnodeToPortCreate(dev, portInd, &cParamsPtr, &cNodeIndPtr);



        /*1.1 Check with legal parameters.
                Expected: GT_OK */

        st = cpssTmQueueToCnodeCreate(dev, cNodeIndPtr, &qParamsPtr, &aParamsPtr, &bParamsPtr,&queueIndPtr,&aNodeIndPtr, &bNodeIndPtr );
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.2 check out of range qParamsPtr.shapingProfilePtr.
            Call with qParamsPtr.shapingProfilePtr [85580], other parameters same as in 1.1.
            Expected: NON GT_OK.
        */

        qParamsPtr.shapingProfilePtr = 85580;

        st = cpssTmQueueToCnodeCreate(dev, cNodeIndPtr, &qParamsPtr, &aParamsPtr, &bParamsPtr,&queueIndPtr,&aNodeIndPtr, &bNodeIndPtr );

        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, qParamsPtr.shapingProfilePtr);

        qParamsPtr.shapingProfilePtr= 0;

         /*
            1.3 check out of range qParamsPtr.quantum.
            Call with qParamsPtr.quantum [0x30], other parameters same as in 1.1.
            Expected: NON GT_OK.
        */

        qParamsPtr.quantum=0x30;

        st = cpssTmQueueToCnodeCreate(dev, cNodeIndPtr, &qParamsPtr, &aParamsPtr, &bParamsPtr,&queueIndPtr,&aNodeIndPtr, &bNodeIndPtr );

        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, qParamsPtr.quantum);

        qParamsPtr.quantum=0x40;

         /*
            1.4 check out of range qParamsPtr.quantum.
            Call with qParamsPtr.quantum [0x4fff], other parameters same as in 1.1.
            Expected: NON GT_OK.
        */

        qParamsPtr.quantum=0x4fff;

        st = cpssTmQueueToCnodeCreate(dev, cNodeIndPtr, &qParamsPtr, &aParamsPtr, &bParamsPtr,&queueIndPtr,&aNodeIndPtr, &bNodeIndPtr );

        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, qParamsPtr.quantum);

        qParamsPtr.quantum=0x40;

         /*
            1.5 check out of range qParamsPtr.dropProfileInd.
            Call with qParamsPtr.dropProfileInd [300], other parameters same as in 1.1.
            Expected: NON GT_OK.
        */

        qParamsPtr.dropProfileInd=300;

        st = cpssTmQueueToCnodeCreate(dev, cNodeIndPtr, &qParamsPtr, &aParamsPtr, &bParamsPtr,&queueIndPtr,&aNodeIndPtr, &bNodeIndPtr );

        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev,qParamsPtr.dropProfileInd);

        qParamsPtr.dropProfileInd=0;

         /*
            1.6 check out of range qParamsPtr.eligiblePrioFuncId.
            Call with qParamsPtr.eligiblePrioFuncId [70], other parameters same as in 1.1.
            Expected: NON GT_OK.
        */

        qParamsPtr.eligiblePrioFuncId=70;

        st = cpssTmQueueToCnodeCreate(dev, cNodeIndPtr, &qParamsPtr, &aParamsPtr, &bParamsPtr,&queueIndPtr,&aNodeIndPtr, &bNodeIndPtr );
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, qParamsPtr.eligiblePrioFuncId);

        qParamsPtr.eligiblePrioFuncId=1;

        /*
            1.7 check out of range aParamsPtr.shapingProfilePtr.
            Call with aParamsPtr.shapingProfilePtr [85580], other parameters same as in 1.1.
            Expected: NON GT_OK.
        */

        aParamsPtr.shapingProfilePtr = 85580;

        st = cpssTmQueueToCnodeCreate(dev, cNodeIndPtr, &qParamsPtr, &aParamsPtr, &bParamsPtr,&queueIndPtr,&aNodeIndPtr, &bNodeIndPtr );
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, aParamsPtr.shapingProfilePtr);

        aParamsPtr.shapingProfilePtr= 0;

         /*
            1.8 check out of range aParamsPtr.quantum.
            Call with aParamsPtr.quantum [0x30], other parameters same as in 1.1.
            Expected: NON GT_OK.
        */

        aParamsPtr.quantum=0x30;

        st = cpssTmQueueToCnodeCreate(dev, cNodeIndPtr, &qParamsPtr, &aParamsPtr, &bParamsPtr,&queueIndPtr,&aNodeIndPtr, &bNodeIndPtr );
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, aParamsPtr.quantum);

        aParamsPtr.quantum=0x40;

         /*
            1.9 check out of range aParamsPtr.quantum.
            Call with aParamsPtr.quantum [0x4fff], other parameters same as in 1.1.
            Expected: NON GT_OK.
        */

        aParamsPtr.quantum=0x4fff;

        st = cpssTmQueueToCnodeCreate(dev, cNodeIndPtr, &qParamsPtr, &aParamsPtr, &bParamsPtr,&queueIndPtr,&aNodeIndPtr, &bNodeIndPtr );
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, aParamsPtr.quantum);

        aParamsPtr.quantum=0x40;

         /*
            1.10 check out of range aParamsPtr.dropProfileInd.
            Call with aParamsPtr.dropProfileInd [300], other parameters same as in 1.1.
            Expected: NON GT_OK.
        */

        aParamsPtr.dropProfileInd=300;

        st = cpssTmQueueToCnodeCreate(dev, cNodeIndPtr, &qParamsPtr, &aParamsPtr, &bParamsPtr,&queueIndPtr,&aNodeIndPtr, &bNodeIndPtr );
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev,aParamsPtr.dropProfileInd);

        aParamsPtr.dropProfileInd=0;

         /*
            1.11 check out of range aParamsPtr.eligiblePrioFuncId.
            Call with aParamsPtr.eligiblePrioFuncId [70], other parameters same as in 1.1.
            Expected: NON GT_OK.
        */

        aParamsPtr.eligiblePrioFuncId=70;

        st = cpssTmQueueToCnodeCreate(dev, cNodeIndPtr, &qParamsPtr, &aParamsPtr, &bParamsPtr,&queueIndPtr,&aNodeIndPtr, &bNodeIndPtr );
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, aParamsPtr.eligiblePrioFuncId);

        aParamsPtr.eligiblePrioFuncId=1;

         /*
            1.12 check out of range aParamsPtr.schdModeArr[0].
            Call with aParamsPtr.schdModeArr[0] [2], other parameters same as in 1.1.
            Expected: NON GT_OK.
        */

        aParamsPtr.schdModeArr[0] = 2;

        st = cpssTmQueueToCnodeCreate(dev, cNodeIndPtr, &qParamsPtr, &aParamsPtr, &bParamsPtr,&queueIndPtr,&aNodeIndPtr, &bNodeIndPtr );
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, aParamsPtr.schdModeArr[0]);

        aParamsPtr.schdModeArr[0] = CPSS_TM_SCHD_MODE_RR_E;

        /*
            1.13 check out of range bParamsPtr.shapingProfilePtr.
            Call with bParamsPtr.shapingProfilePtr [85580], other parameters same as in 1.1.
            Expected: NON GT_OK.
        */

        bParamsPtr.shapingProfilePtr = 85580;

        st = cpssTmQueueToCnodeCreate(dev, cNodeIndPtr, &qParamsPtr, &aParamsPtr, &bParamsPtr,&queueIndPtr,&aNodeIndPtr, &bNodeIndPtr );
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, bParamsPtr.shapingProfilePtr);

        bParamsPtr.shapingProfilePtr= 0;

         /*
            1.14 check out of range bParamsPtr.quantum.
            Call with bParamsPtr.quantum [0x30], other parameters same as in 1.1.
            Expected: NON GT_OK.
        */

        bParamsPtr.quantum=0x30;

        st = cpssTmQueueToCnodeCreate(dev, cNodeIndPtr, &qParamsPtr, &aParamsPtr, &bParamsPtr,&queueIndPtr,&aNodeIndPtr, &bNodeIndPtr );
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, bParamsPtr.quantum);

        bParamsPtr.quantum=0x40;

         /*
            1.15 check out of range bParamsPtr.quantum.
            Call with bParamsPtr.quantum [0x4fff], other parameters same as in 1.1.
            Expected: NON GT_OK.
        */

        bParamsPtr.quantum=0x4fff;

        st = cpssTmQueueToCnodeCreate(dev, cNodeIndPtr, &qParamsPtr, &aParamsPtr, &bParamsPtr,&queueIndPtr,&aNodeIndPtr, &bNodeIndPtr );
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, bParamsPtr.quantum);

        bParamsPtr.quantum=0x40;

         /*
            1.16 check out of range bParamsPtr.dropProfileInd.
            Call with bParamsPtr.dropProfileInd [300], other parameters same as in 1.1.
            Expected: NON GT_OK.
        */

        bParamsPtr.dropProfileInd=300;

        st = cpssTmQueueToCnodeCreate(dev, cNodeIndPtr, &qParamsPtr, &aParamsPtr, &bParamsPtr,&queueIndPtr,&aNodeIndPtr, &bNodeIndPtr );
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev,bParamsPtr.dropProfileInd);

        bParamsPtr.dropProfileInd=0;

         /*
            1.17 check out of range bParamsPtr.eligiblePrioFuncId.
            Call with bParamsPtr.eligiblePrioFuncId [70], other parameters same as in 1.1.
            Expected: NON GT_OK.
        */

        bParamsPtr.eligiblePrioFuncId=70;

        st = cpssTmQueueToCnodeCreate(dev, cNodeIndPtr, &qParamsPtr, &aParamsPtr, &bParamsPtr,&queueIndPtr,&aNodeIndPtr, &bNodeIndPtr );
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, bParamsPtr.eligiblePrioFuncId);

        bParamsPtr.eligiblePrioFuncId=1;

         /*
            1.18 check out of range bParamsPtr.schdModeArr[0].
            Call with bParamsPtr.schdModeArr[0] [2], other parameters same as in 1.1.
            Expected: NON GT_OK.
        */

        bParamsPtr.schdModeArr[0] = 2;

        st = cpssTmQueueToCnodeCreate(dev, cNodeIndPtr, &qParamsPtr, &aParamsPtr, &bParamsPtr,&queueIndPtr,&aNodeIndPtr, &bNodeIndPtr );
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, bParamsPtr.schdModeArr[0]);

        bParamsPtr.schdModeArr[0] = CPSS_TM_SCHD_MODE_RR_E;

        /*
            1.19 Call with qParamsPtr [NULL], other parameters same as in 1.1.
            Expected: GT_BAD_PTR.
        */

        st = cpssTmQueueToCnodeCreate(dev, cNodeIndPtr, NULL, &aParamsPtr, &bParamsPtr,&queueIndPtr,&aNodeIndPtr, &bNodeIndPtr );
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, qParamsPtr = NULL", dev);

        /*
            1.20 Call with aParamsPtr [NULL], other parameters same as in 1.1.
            Expected: GT_BAD_PTR.
        */

        st = cpssTmQueueToCnodeCreate(dev, cNodeIndPtr, &qParamsPtr,NULL, &bParamsPtr,&queueIndPtr,&aNodeIndPtr, &bNodeIndPtr );
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, aParamsPtr = NULL", dev);

        /*
            1.21 Call with bParamsPtr [NULL], other parameters same as in 1.1.
            Expected: GT_BAD_PTR.
        */

        st = cpssTmQueueToCnodeCreate(dev, cNodeIndPtr, &qParamsPtr, &aParamsPtr, NULL,&queueIndPtr,&aNodeIndPtr, &bNodeIndPtr );
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, bParamsPtr = NULL", dev);


        /*
            1.22 Call with queueIndPtr [NULL], other parameters same as in 1.1.
            Expected: GT_BAD_PTR.
        */

        st = cpssTmQueueToCnodeCreate(dev, cNodeIndPtr, &qParamsPtr, &aParamsPtr, &bParamsPtr,NULL,&aNodeIndPtr, &bNodeIndPtr );
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, queueIndPtr = NULL", dev);

        /*
            1.23 Call with aNodeIndPtr [NULL], other parameters same as in 1.1.
            Expected: GT_BAD_PTR.
        */

        st = cpssTmQueueToCnodeCreate(dev, cNodeIndPtr, &qParamsPtr, &aParamsPtr, &bParamsPtr,&queueIndPtr,NULL, &bNodeIndPtr );
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, aNodeIndPtr = NULL", dev);

        /*
            1.24 Call with bNodeIndPtr [NULL], other parameters same as in 1.1.
            Expected: GT_BAD_PTR.
        */

        st = cpssTmQueueToCnodeCreate(dev, cNodeIndPtr, &qParamsPtr, &aParamsPtr, &bParamsPtr,&queueIndPtr,&aNodeIndPtr, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, bNodeIndPtr = NULL", dev);


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
        st = cpssTmQueueToCnodeCreate(dev, cNodeIndPtr, &qParamsPtr, &aParamsPtr, &bParamsPtr,&queueIndPtr,&aNodeIndPtr, &bNodeIndPtr );
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssTmQueueToCnodeCreate(dev, cNodeIndPtr, &qParamsPtr, &aParamsPtr, &bParamsPtr,&queueIndPtr,&aNodeIndPtr, &bNodeIndPtr );
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}




/*******************************************************************************
GT_STATUS cpssTmCnodeToPortCreate
(
    IN  GT_U8                           devNum,
    IN  GT_U32                          portInd,
    IN  CPSS_TM_C_NODE_PARAMS_STC       *cParamsPtr,
    OUT GT_U32                          *cNodeIndPtr
)
*/
UTF_TEST_CASE_MAC(cpssTmCnodeToPortCreate)
{
    GT_STATUS               st = GT_OK;

    GT_U8                        dev;
    GT_U32                       portInd = 1;
    CPSS_TM_PORT_PARAMS_STC      paramsPtr;
    GT_U32                       cNodesNum;
    GT_U32                       bNodesNum;
    GT_U32                       aNodesNum;
    GT_U32                       queuesNum;
    CPSS_TM_C_NODE_PARAMS_STC    cParamsPtr;
    GT_U32                       cNodeIndPtr;
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

        /*1.1 Check with legal parameters.
                Expected: GT_OK */

        st = cpssTmCnodeToPortCreate(dev, portInd, &cParamsPtr, &cNodeIndPtr);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.2 check out of range cParamsPtr.shapingProfilePtr.
            Call with cParamsPtr.shapingProfilePtr [85580], other parameters same as in 1.1.
            Expected: NON GT_OK.
        */

        cParamsPtr.shapingProfilePtr = 85580;

        st = cpssTmCnodeToPortCreate(dev, portInd, &cParamsPtr, &cNodeIndPtr);

        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, cParamsPtr.shapingProfilePtr);

        cParamsPtr.shapingProfilePtr= 0;

         /*
            1.3 check out of range cParamsPtr.quantum.
            Call with cParamsPtr.quantum [0x30], other parameters same as in 1.1.
            Expected: NON GT_OK.
        */

        cParamsPtr.quantum=0x30;

        st = cpssTmCnodeToPortCreate(dev, portInd, &cParamsPtr, &cNodeIndPtr);

        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, cParamsPtr.quantum);

        cParamsPtr.quantum=0x40;

         /*
            1.4 check out of range cParamsPtr.quantum.
            Call with cParamsPtr.quantum [0x4fff], other parameters same as in 1.1.
            Expected: NON GT_OK.
        */

        cParamsPtr.quantum=0x4fff;

        st = cpssTmCnodeToPortCreate(dev, portInd, &cParamsPtr, &cNodeIndPtr);

        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, cParamsPtr.quantum);

        cParamsPtr.quantum=0x40;

         /*
            1.5 check out of range cParamsPtr.dropProfileInd.
            Call with cParamsPtr.dropProfileIndArr[0][300], other parameters same as in 1.1.
            Expected: NON GT_OK.
        */

        cParamsPtr.dropProfileIndArr[0]=300;

        st = cpssTmCnodeToPortCreate(dev, portInd, &cParamsPtr, &cNodeIndPtr);

        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev,cParamsPtr.dropProfileIndArr[0]);

        cParamsPtr.dropProfileIndArr[0]=0;

         /*
            1.6 check out of range cParamsPtr.eligiblePrioFuncId.
            Call with cParamsPtr.eligiblePrioFuncId [70], other parameters same as in 1.1.
            Expected: NON GT_OK.
        */

        cParamsPtr.eligiblePrioFuncId=70;

        st = cpssTmCnodeToPortCreate(dev, portInd, &cParamsPtr, &cNodeIndPtr);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, cParamsPtr.eligiblePrioFuncId);

        cParamsPtr.eligiblePrioFuncId=1;

         /*
            1.7 check out of range cParamsPtr.schdModeArr[0].
            Call with cParamsPtr.schdModeArr[0] [2], other parameters same as in 1.1.
            Expected: NON GT_OK.
        */

        cParamsPtr.schdModeArr[0] = 2;

        st = cpssTmCnodeToPortCreate(dev, portInd, &cParamsPtr, &cNodeIndPtr);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, cParamsPtr.schdModeArr[0]);

        cParamsPtr.schdModeArr[0] = CPSS_TM_SCHD_MODE_RR_E;

         /*
            1.8 check out of range cParamsPtr.dropCosMap.
            Call with cParamsPtr.dropCosMap [9], other parameters same as in 1.1.
            Expected: NON GT_OK.
        */

        cParamsPtr.dropCosMap=9;

        st = cpssTmCnodeToPortCreate(dev, portInd, &cParamsPtr, &cNodeIndPtr);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, cParamsPtr.dropCosMap);

        cParamsPtr.dropCosMap=1;


       /*
            1.9 Call with cNodeIndPtr [NULL], other parameters same as in 1.1.
            Expected: GT_BAD_PARAM.
        */

        st = cpssTmCnodeToPortCreate(dev, portInd, &cParamsPtr, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, cNodeIndPtr = NULL", dev);


        /*
            1.10 Call with cParamsPtr [NULL], other parameters same as in 1.1.
            Expected: GT_BAD_PTR.
        */

        st = cpssTmCnodeToPortCreate(dev, portInd, NULL, &cNodeIndPtr);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, cParamsPtr = NULL", dev);

        /*
            1.11 check out of range portInd.
            Call with portInd [70], other parameters same as in 1.1.
            Expected: NON GT_OK.
        */

        portInd=70;

        st = cpssTmCnodeToPortCreate(dev, portInd, &cParamsPtr, &cNodeIndPtr);
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
        st = cpssTmCnodeToPortCreate(dev, portInd, &cParamsPtr, &cNodeIndPtr);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssTmCnodeToPortCreate(dev, portInd, &cParamsPtr, &cNodeIndPtr);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}


/*******************************************************************************
GT_STATUS cpssTmBnodeToPortCreate
(
    IN  GT_U8                           devNum,
    IN  GT_U32                          portInd,
    IN  CPSS_TM_B_NODE_PARAMS_STC       *bParamsPtr,
    IN  CPSS_TM_C_NODE_PARAMS_STC       *cParamsPtr,
    OUT GT_U32                          *bNodeIndPtr,
    OUT GT_U32                          *cNodeIndPtr
)*/
UTF_TEST_CASE_MAC(cpssTmBnodeToPortCreate)
{
    GT_STATUS               st = GT_OK;

    GT_U8                           dev;
    GT_U32                          portInd =1;
/*    CPSS_TM_A_NODE_PARAMS_STC       aParamsPtr; */
    CPSS_TM_B_NODE_PARAMS_STC       bParamsPtr;
    CPSS_TM_C_NODE_PARAMS_STC       cParamsPtr;
    GT_U32                          bNodeIndPtr;
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

        /* Configure parameters for port */
        paramsPtr.cirBw = 1000000;
        paramsPtr.eirBw = 0;
        paramsPtr.cbs = 8;/*KBytes*/
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
                Expected: GT_OK */

        st = cpssTmBnodeToPortCreate(dev, portInd, &bParamsPtr, &cParamsPtr,&bNodeIndPtr, &cNodeIndPtr );
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);


        /*
            1.2 check out of range bParamsPtr.shapingProfilePtr.
            Call with bParamsPtr.shapingProfilePtr [85580], other parameters same as in 1.1.
            Expected: NON GT_OK.
        */

        bParamsPtr.shapingProfilePtr = 85580;

        st = cpssTmBnodeToPortCreate(dev, portInd, &bParamsPtr, &cParamsPtr,&bNodeIndPtr, &cNodeIndPtr );
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, bParamsPtr.shapingProfilePtr);

        bParamsPtr.shapingProfilePtr= 0;

         /*
            1.3 check out of range bParamsPtr.quantum.
            Call with bParamsPtr.quantum [0x30], other parameters same as in 1.1.
            Expected: NON GT_OK.
        */

        bParamsPtr.quantum=0x30;

        st = cpssTmBnodeToPortCreate(dev, portInd, &bParamsPtr, &cParamsPtr,&bNodeIndPtr, &cNodeIndPtr );
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, bParamsPtr.quantum);

        bParamsPtr.quantum=0x40;

         /*
            1.4 check out of range bParamsPtr.quantum.
            Call with bParamsPtr.quantum [0x4fff], other parameters same as in 1.1.
            Expected: NON GT_OK.
        */

        bParamsPtr.quantum=0x4fff;

        st = cpssTmBnodeToPortCreate(dev, portInd, &bParamsPtr, &cParamsPtr,&bNodeIndPtr, &cNodeIndPtr );
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, bParamsPtr.quantum);

        bParamsPtr.quantum=0x40;

         /*
            1.5 check out of range bParamsPtr.dropProfileInd.
            Call with bParamsPtr.dropProfileInd [300], other parameters same as in 1.1.
            Expected: NON GT_OK.
        */

        bParamsPtr.dropProfileInd=300;

        st = cpssTmBnodeToPortCreate(dev, portInd, &bParamsPtr, &cParamsPtr,&bNodeIndPtr, &cNodeIndPtr );
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev,bParamsPtr.dropProfileInd);

        bParamsPtr.dropProfileInd=0;

         /*
            1.6 check out of range bParamsPtr.eligiblePrioFuncId.
            Call with bParamsPtr.eligiblePrioFuncId [70], other parameters same as in 1.1.
            Expected: NON GT_OK.
        */

        bParamsPtr.eligiblePrioFuncId=70;

        st = cpssTmBnodeToPortCreate(dev, portInd, &bParamsPtr, &cParamsPtr,&bNodeIndPtr, &cNodeIndPtr );
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, bParamsPtr.eligiblePrioFuncId);

        bParamsPtr.eligiblePrioFuncId=1;

         /*
            1.7 check out of range bParamsPtr.schdModeArr[0].
            Call with bParamsPtr.schdModeArr[0] [2], other parameters same as in 1.1.
            Expected: NON GT_OK.
        */

        bParamsPtr.schdModeArr[0] = 2;

        st = cpssTmBnodeToPortCreate(dev, portInd, &bParamsPtr, &cParamsPtr,&bNodeIndPtr, &cNodeIndPtr );
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, bParamsPtr.schdModeArr[0]);

        bParamsPtr.schdModeArr[0] = CPSS_TM_SCHD_MODE_RR_E;

        /*
            1.8 check out of range cParamsPtr.shapingProfilePtr.
            Call with cParamsPtr.shapingProfilePtr [85580], other parameters same as in 1.1.
            Expected: NON GT_OK.
        */

        cParamsPtr.shapingProfilePtr = 85580;

        st = cpssTmBnodeToPortCreate(dev, portInd, &bParamsPtr, &cParamsPtr,&bNodeIndPtr, &cNodeIndPtr );
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, cParamsPtr.shapingProfilePtr);

        cParamsPtr.shapingProfilePtr= 0;

         /*
            1.9 check out of range cParamsPtr.quantum.
            Call with cParamsPtr.quantum [0x30], other parameters same as in 1.1.
            Expected: NON GT_OK.
        */

        cParamsPtr.quantum=0x30;

        st = cpssTmBnodeToPortCreate(dev, portInd, &bParamsPtr, &cParamsPtr,&bNodeIndPtr, &cNodeIndPtr );
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, cParamsPtr.quantum);

        cParamsPtr.quantum=0x40;

         /*
            1.10 check out of range cParamsPtr.quantum.
            Call with cParamsPtr.quantum [0x4fff], other parameters same as in 1.1.
            Expected: NON GT_OK.
        */

        cParamsPtr.quantum=0x4fff;

        st = cpssTmBnodeToPortCreate(dev, portInd, &bParamsPtr, &cParamsPtr,&bNodeIndPtr, &cNodeIndPtr );
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, cParamsPtr.quantum);

        cParamsPtr.quantum=0x40;

         /*
            1.11 check out of range cParamsPtr.dropProfileInd.
            Call with cParamsPtr.dropProfileIndArr[0][300], other parameters same as in 1.1.
            Expected: NON GT_OK.
        */

        cParamsPtr.dropProfileIndArr[0]=300;

        st = cpssTmBnodeToPortCreate(dev, portInd, &bParamsPtr, &cParamsPtr,&bNodeIndPtr, &cNodeIndPtr );
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev,cParamsPtr.dropProfileIndArr[0]);

        cParamsPtr.dropProfileIndArr[0]=0;

         /*
            1.12 check out of range cParamsPtr.eligiblePrioFuncId.
            Call with cParamsPtr.eligiblePrioFuncId [70], other parameters same as in 1.1.
            Expected: NON GT_OK.
        */

        cParamsPtr.eligiblePrioFuncId=70;

        st = cpssTmBnodeToPortCreate(dev, portInd, &bParamsPtr, &cParamsPtr,&bNodeIndPtr, &cNodeIndPtr );
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, cParamsPtr.eligiblePrioFuncId);

        cParamsPtr.eligiblePrioFuncId=1;

         /*
            1.13 check out of range cParamsPtr.schdModeArr[0].
            Call with cParamsPtr.schdModeArr[0] [2], other parameters same as in 1.1.
            Expected: NON GT_OK.
        */

        cParamsPtr.schdModeArr[0] = 2;

        st = cpssTmBnodeToPortCreate(dev, portInd, &bParamsPtr, &cParamsPtr,&bNodeIndPtr, &cNodeIndPtr );
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, cParamsPtr.schdModeArr[0]);

        cParamsPtr.schdModeArr[0] = CPSS_TM_SCHD_MODE_RR_E;

         /*
            1.14 check out of range cParamsPtr.dropCosMap.
            Call with cParamsPtr.dropCosMap [9], other parameters same as in 1.1.
            Expected: NON GT_OK.
        */

        cParamsPtr.dropCosMap=9;

        st = cpssTmBnodeToPortCreate(dev, portInd, &bParamsPtr, &cParamsPtr,&bNodeIndPtr, &cNodeIndPtr );
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, cParamsPtr.dropCosMap);

        cParamsPtr.dropCosMap=1;

        /*
            1.15 Call with bParamsPtr [NULL], other parameters same as in 1.1.
            Expected: GT_BAD_PTR.
        */

        st = cpssTmBnodeToPortCreate(dev, portInd, NULL, &cParamsPtr,&bNodeIndPtr, &cNodeIndPtr );
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, bParamsPtr = NULL", dev);

        /*
            1.16 Call with cParamsPtr [NULL], other parameters same as in 1.1.
            Expected: GT_BAD_PTR.
        */

        st = cpssTmBnodeToPortCreate(dev, portInd, &bParamsPtr, NULL,&bNodeIndPtr, &cNodeIndPtr );
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, cParamsPtr = NULL", dev);

        /*
            1.17 Call with bNodeIndPtr [NULL], other parameters same as in 1.1.
            Expected: GT_BAD_PTR.
        */

        st = cpssTmBnodeToPortCreate(dev, portInd, &bParamsPtr, &cParamsPtr,NULL, &cNodeIndPtr );
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, bNodeIndPtr = NULL", dev);

        /*
            1.18 Call with cNodeIndPtr [NULL], other parameters same as in 1.1.
            Expected: GT_BAD_PTR.
        */

        st = cpssTmBnodeToPortCreate(dev, portInd, &bParamsPtr, &cParamsPtr,&bNodeIndPtr, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, cNodeIndPtr = NULL", dev);

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
        st = cpssTmBnodeToPortCreate(dev, portInd, &bParamsPtr, &cParamsPtr,&bNodeIndPtr, &cNodeIndPtr );
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssTmBnodeToPortCreate(dev, portInd, &bParamsPtr, &cParamsPtr,&bNodeIndPtr, &cNodeIndPtr );
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}



/*******************************************************************************
GT_STATUS cpssTmQueueToBnodeCreate
(
    IN  GT_U8                           devNum,
    IN  GT_U32                          bNodeInd,
    IN  CPSS_TM_QUEUE_PARAMS_STC        *qParamsPtr,
    IN  CPSS_TM_A_NODE_PARAMS_STC       *aParamsPtr,
    OUT GT_U32                          *queueIndPtr,
    OUT GT_U32                          *aNodeIndPtr
)
*/
UTF_TEST_CASE_MAC(cpssTmQueueToBnodeCreate)
{
    GT_STATUS               st = GT_OK;

    GT_U8                           dev;
    GT_U32                          portInd =1;
    CPSS_TM_QUEUE_PARAMS_STC        qParamsPtr;
    CPSS_TM_A_NODE_PARAMS_STC       aParamsPtr;
    CPSS_TM_B_NODE_PARAMS_STC       bParamsPtr;
    CPSS_TM_C_NODE_PARAMS_STC       cParamsPtr;
    GT_U32                          queueIndPtr;
    GT_U32                          aNodeIndPtr;
    GT_U32                          bNodeIndPtr;
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

    cpssOsBzero((GT_VOID*)&qParamsPtr, sizeof(qParamsPtr));
    cpssOsBzero((GT_VOID*)&aParamsPtr, sizeof(aParamsPtr));
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

        /* profile for Q level */
        qParamsPtr.shapingProfilePtr = 0;
        qParamsPtr.quantum = 0x40;
        qParamsPtr.dropProfileInd = 0;
        qParamsPtr.eligiblePrioFuncId = 1;

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

        /*1.1 Check with legal parameters.
                Expected: GT_OK */

        st = cpssTmQueueToBnodeCreate(dev, bNodeIndPtr, &qParamsPtr, &aParamsPtr, &queueIndPtr,&aNodeIndPtr);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.2 check out of range qParamsPtr.shapingProfilePtr.
            Call with qParamsPtr.shapingProfilePtr [85580], other parameters same as in 1.1.
            Expected: NON GT_OK.
        */

        qParamsPtr.shapingProfilePtr = 85580;

        st = cpssTmQueueToBnodeCreate(dev, bNodeIndPtr, &qParamsPtr, &aParamsPtr, &queueIndPtr,&aNodeIndPtr);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, qParamsPtr.shapingProfilePtr);

        qParamsPtr.shapingProfilePtr= 0;

         /*
            1.3 check out of range qParamsPtr.quantum.
            Call with qParamsPtr.quantum [0x30], other parameters same as in 1.1.
            Expected: NON GT_OK.
        */

        qParamsPtr.quantum=0x30;

        st = cpssTmQueueToBnodeCreate(dev, bNodeIndPtr, &qParamsPtr, &aParamsPtr, &queueIndPtr,&aNodeIndPtr);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, qParamsPtr.quantum);

        qParamsPtr.quantum=0x40;

         /*
            1.4 check out of range qParamsPtr.quantum.
            Call with qParamsPtr.quantum [0x4fff], other parameters same as in 1.1.
            Expected: NON GT_OK.
        */

        qParamsPtr.quantum=0x4fff;

        st = cpssTmQueueToBnodeCreate(dev, bNodeIndPtr, &qParamsPtr, &aParamsPtr, &queueIndPtr,&aNodeIndPtr);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, qParamsPtr.quantum);

        qParamsPtr.quantum=0x40;

         /*
            1.5 check out of range qParamsPtr.dropProfileInd.
            Call with qParamsPtr.dropProfileInd [300], other parameters same as in 1.1.
            Expected: NON GT_OK.
        */

        qParamsPtr.dropProfileInd=300;

        st = cpssTmQueueToBnodeCreate(dev, bNodeIndPtr, &qParamsPtr, &aParamsPtr, &queueIndPtr,&aNodeIndPtr);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev,qParamsPtr.dropProfileInd);

        qParamsPtr.dropProfileInd=0;

         /*
            1.6 check out of range qParamsPtr.eligiblePrioFuncId.
            Call with qParamsPtr.eligiblePrioFuncId [70], other parameters same as in 1.1.
            Expected: NON GT_OK.
        */

        qParamsPtr.eligiblePrioFuncId=70;

         st = cpssTmQueueToBnodeCreate(dev, bNodeIndPtr, &qParamsPtr, &aParamsPtr, &queueIndPtr,&aNodeIndPtr);
         UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, qParamsPtr.eligiblePrioFuncId);

        qParamsPtr.eligiblePrioFuncId=1;

        /*
            1.7 check out of range aParamsPtr.shapingProfilePtr.
            Call with aParamsPtr.shapingProfilePtr [85580], other parameters same as in 1.1.
            Expected: NON GT_OK.
        */

        aParamsPtr.shapingProfilePtr = 85580;

        st = cpssTmQueueToBnodeCreate(dev, bNodeIndPtr, &qParamsPtr, &aParamsPtr, &queueIndPtr,&aNodeIndPtr);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, aParamsPtr.shapingProfilePtr);

        aParamsPtr.shapingProfilePtr= 0;

         /*
            1.8 check out of range aParamsPtr.quantum.
            Call with aParamsPtr.quantum [0x30], other parameters same as in 1.1.
            Expected: NON GT_OK.
        */

        aParamsPtr.quantum=0x30;

        st = cpssTmQueueToBnodeCreate(dev, bNodeIndPtr, &qParamsPtr, &aParamsPtr, &queueIndPtr,&aNodeIndPtr);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, aParamsPtr.quantum);

        aParamsPtr.quantum=0x40;

         /*
            1.9 check out of range aParamsPtr.quantum.
            Call with aParamsPtr.quantum [0x4fff], other parameters same as in 1.1.
            Expected: NON GT_OK.
        */

        aParamsPtr.quantum=0x4fff;

        st = cpssTmQueueToBnodeCreate(dev, bNodeIndPtr, &qParamsPtr, &aParamsPtr, &queueIndPtr,&aNodeIndPtr);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, aParamsPtr.quantum);

        aParamsPtr.quantum=0x40;

         /*
            1.10 check out of range aParamsPtr.dropProfileInd.
            Call with aParamsPtr.dropProfileInd [300], other parameters same as in 1.1.
            Expected: NON GT_OK.
        */

        aParamsPtr.dropProfileInd=300;

        st = cpssTmQueueToBnodeCreate(dev, bNodeIndPtr, &qParamsPtr, &aParamsPtr, &queueIndPtr,&aNodeIndPtr);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev,aParamsPtr.dropProfileInd);

        aParamsPtr.dropProfileInd=0;

         /*
            1.11 check out of range aParamsPtr.eligiblePrioFuncId.
            Call with aParamsPtr.eligiblePrioFuncId [70], other parameters same as in 1.1.
            Expected: NON GT_OK.
        */

        aParamsPtr.eligiblePrioFuncId=70;

        st = cpssTmQueueToBnodeCreate(dev, bNodeIndPtr, &qParamsPtr, &aParamsPtr, &queueIndPtr,&aNodeIndPtr);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, aParamsPtr.eligiblePrioFuncId);

        aParamsPtr.eligiblePrioFuncId=1;

         /*
            1.12 check out of range aParamsPtr.schdModeArr[0].
            Call with aParamsPtr.schdModeArr[0] [2], other parameters same as in 1.1.
            Expected: NON GT_OK.
        */

        aParamsPtr.schdModeArr[0] = 2;

        st = cpssTmQueueToBnodeCreate(dev, bNodeIndPtr, &qParamsPtr, &aParamsPtr, &queueIndPtr,&aNodeIndPtr);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, aParamsPtr.schdModeArr[0]);

        aParamsPtr.schdModeArr[0] = CPSS_TM_SCHD_MODE_RR_E;


        /*
            1.26 Call with qParamsPtr [NULL], other parameters same as in 1.1.
            Expected: GT_BAD_PTR.
        */

        st = cpssTmQueueToBnodeCreate(dev, bNodeIndPtr, NULL, &aParamsPtr, &queueIndPtr,&aNodeIndPtr);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, qParamsPtr = NULL", dev);

        /*
            1.27 Call with aParamsPtr [NULL], other parameters same as in 1.1.
            Expected: GT_BAD_PTR.
        */

        st = cpssTmQueueToBnodeCreate(dev, bNodeIndPtr, &qParamsPtr, NULL, &queueIndPtr,&aNodeIndPtr);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, aParamsPtr = NULL", dev);


        /*
            1.30 Call with queueIndPtr [NULL], other parameters same as in 1.1.
            Expected: GT_BAD_PTR.
        */

        st = cpssTmQueueToBnodeCreate(dev, bNodeIndPtr, &qParamsPtr, &aParamsPtr, NULL,&aNodeIndPtr);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, queueIndPtr = NULL", dev);

        /*
            1.31 Call with aNodeIndPtr [NULL], other parameters same as in 1.1.
            Expected: GT_BAD_PTR.
        */

        st = cpssTmQueueToBnodeCreate(dev, bNodeIndPtr, &qParamsPtr, &aParamsPtr, &queueIndPtr,NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, aNodeIndPtr = NULL", dev);


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
        st = cpssTmQueueToBnodeCreate(dev, bNodeIndPtr, &qParamsPtr, &aParamsPtr, &queueIndPtr,&aNodeIndPtr);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssTmQueueToBnodeCreate(dev, bNodeIndPtr, &qParamsPtr, &aParamsPtr, &queueIndPtr,&aNodeIndPtr);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*******************************************************************************
GT_STATUS cpssTmAnodeToPortCreate
(
    IN  GT_U8                           devNum,
    IN  GT_U32                          portInd,
    IN  CPSS_TM_A_NODE_PARAMS_STC       *aParamsPtr,
    IN  CPSS_TM_B_NODE_PARAMS_STC       *bParamsPtr,
    IN  CPSS_TM_C_NODE_PARAMS_STC       *cParamsPtr,
    OUT GT_U32                          *aNodeIndPtr,
    OUT GT_U32                          *bNodeIndPtr,
    OUT GT_U32                          *cNodeIndPtr
)
*/
UTF_TEST_CASE_MAC(cpssTmAnodeToPortCreate)
{
    GT_STATUS               st = GT_OK;

    GT_U8                           dev;
    GT_U32                          portInd=1;
    CPSS_TM_A_NODE_PARAMS_STC       aParamsPtr;
    CPSS_TM_B_NODE_PARAMS_STC       bParamsPtr;
    CPSS_TM_C_NODE_PARAMS_STC       cParamsPtr;
    GT_U32                          aNodeIndPtr;
    GT_U32                          bNodeIndPtr;
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

    cpssOsBzero((GT_VOID*)&aParamsPtr, sizeof(aParamsPtr));
    cpssOsBzero((GT_VOID*)&bParamsPtr, sizeof(bParamsPtr));
    cpssOsBzero((GT_VOID*)&cParamsPtr, sizeof(cParamsPtr));
    cpssOsBzero((GT_VOID*)&paramsPtr, sizeof(paramsPtr));

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

        /*1.1 Check with legal parameters.
                Expected: GT_OK */

        st = cpssTmAnodeToPortCreate(dev, portInd, &aParamsPtr, &bParamsPtr, &cParamsPtr, &aNodeIndPtr, &bNodeIndPtr, &cNodeIndPtr );
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);


        /*
            1.2 check out of range aParamsPtr.shapingProfilePtr.
            Call with aParamsPtr.shapingProfilePtr [85580], other parameters same as in 1.1.
            Expected: NON GT_OK.
        */

        aParamsPtr.shapingProfilePtr = 85580;

        st = cpssTmAnodeToPortCreate(dev, portInd, &aParamsPtr, &bParamsPtr, &cParamsPtr, &aNodeIndPtr, &bNodeIndPtr, &cNodeIndPtr );
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, aParamsPtr.shapingProfilePtr);

        aParamsPtr.shapingProfilePtr= 0;

         /*
            1.3 check out of range aParamsPtr.quantum.
            Call with aParamsPtr.quantum [0x30], other parameters same as in 1.1.
            Expected: NON GT_OK.
        */

        aParamsPtr.quantum=0x30;

        st = cpssTmAnodeToPortCreate(dev, portInd, &aParamsPtr, &bParamsPtr, &cParamsPtr, &aNodeIndPtr, &bNodeIndPtr, &cNodeIndPtr );
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, aParamsPtr.quantum);

        aParamsPtr.quantum=0x40;

         /*
            1.4 check out of range aParamsPtr.quantum.
            Call with aParamsPtr.quantum [0x4fff], other parameters same as in 1.1.
            Expected: NON GT_OK.
        */

        aParamsPtr.quantum=0x4fff;

        st = cpssTmAnodeToPortCreate(dev, portInd, &aParamsPtr, &bParamsPtr, &cParamsPtr, &aNodeIndPtr, &bNodeIndPtr, &cNodeIndPtr );
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, aParamsPtr.quantum);

        aParamsPtr.quantum=0x40;

         /*
            1.5 check out of range aParamsPtr.dropProfileInd.
            Call with aParamsPtr.dropProfileInd [300], other parameters same as in 1.1.
            Expected: NON GT_OK.
        */

        aParamsPtr.dropProfileInd=300;

        st = cpssTmAnodeToPortCreate(dev, portInd, &aParamsPtr, &bParamsPtr, &cParamsPtr, &aNodeIndPtr, &bNodeIndPtr, &cNodeIndPtr );
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev,aParamsPtr.dropProfileInd);

        aParamsPtr.dropProfileInd=0;

         /*
            1.6 check out of range aParamsPtr.eligiblePrioFuncId.
            Call with aParamsPtr.eligiblePrioFuncId [70], other parameters same as in 1.1.
            Expected: NON GT_OK.
        */

        aParamsPtr.eligiblePrioFuncId=70;

        st = cpssTmAnodeToPortCreate(dev, portInd, &aParamsPtr, &bParamsPtr, &cParamsPtr, &aNodeIndPtr, &bNodeIndPtr, &cNodeIndPtr );
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, aParamsPtr.eligiblePrioFuncId);

        aParamsPtr.eligiblePrioFuncId=1;

         /*
            1.7 check out of range aParamsPtr.schdModeArr[0].
            Call with aParamsPtr.schdModeArr[0] [2], other parameters same as in 1.1.
            Expected: NON GT_OK.
        */

        aParamsPtr.schdModeArr[0] = 2;

        st = cpssTmAnodeToPortCreate(dev, portInd, &aParamsPtr, &bParamsPtr, &cParamsPtr, &aNodeIndPtr, &bNodeIndPtr, &cNodeIndPtr );
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, aParamsPtr.schdModeArr[0]);

        aParamsPtr.schdModeArr[0] = CPSS_TM_SCHD_MODE_RR_E;

        /*
            1.8 check out of range bParamsPtr.shapingProfilePtr.
            Call with bParamsPtr.shapingProfilePtr [85580], other parameters same as in 1.1.
            Expected: NON GT_OK.
        */

        bParamsPtr.shapingProfilePtr = 85580;

        st = cpssTmAnodeToPortCreate(dev, portInd, &aParamsPtr, &bParamsPtr, &cParamsPtr, &aNodeIndPtr, &bNodeIndPtr, &cNodeIndPtr );
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, bParamsPtr.shapingProfilePtr);

        bParamsPtr.shapingProfilePtr= 0;

         /*
            1.9 check out of range bParamsPtr.quantum.
            Call with bParamsPtr.quantum [0x30], other parameters same as in 1.1.
            Expected: NON GT_OK.
        */

        bParamsPtr.quantum=0x30;

        st = cpssTmAnodeToPortCreate(dev, portInd, &aParamsPtr, &bParamsPtr, &cParamsPtr, &aNodeIndPtr, &bNodeIndPtr, &cNodeIndPtr );
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, bParamsPtr.quantum);

        bParamsPtr.quantum=0x40;

         /*
            1.10 check out of range bParamsPtr.quantum.
            Call with bParamsPtr.quantum [0x4fff], other parameters same as in 1.1.
            Expected: NON GT_OK.
        */

        bParamsPtr.quantum=0x4fff;

        st = cpssTmAnodeToPortCreate(dev, portInd, &aParamsPtr, &bParamsPtr, &cParamsPtr, &aNodeIndPtr, &bNodeIndPtr, &cNodeIndPtr );
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, bParamsPtr.quantum);

        bParamsPtr.quantum=0x40;

         /*
            1.11 check out of range bParamsPtr.dropProfileInd.
            Call with bParamsPtr.dropProfileInd [300], other parameters same as in 1.1.
            Expected: NON GT_OK.
        */

        bParamsPtr.dropProfileInd=300;

        st = cpssTmAnodeToPortCreate(dev, portInd, &aParamsPtr, &bParamsPtr, &cParamsPtr, &aNodeIndPtr, &bNodeIndPtr, &cNodeIndPtr );
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev,bParamsPtr.dropProfileInd);

        bParamsPtr.dropProfileInd=0;

         /*
            1.12 check out of range bParamsPtr.eligiblePrioFuncId.
            Call with bParamsPtr.eligiblePrioFuncId [70], other parameters same as in 1.1.
            Expected: NON GT_OK.
        */

        bParamsPtr.eligiblePrioFuncId=70;

        st = cpssTmAnodeToPortCreate(dev, portInd, &aParamsPtr, &bParamsPtr, &cParamsPtr, &aNodeIndPtr, &bNodeIndPtr, &cNodeIndPtr );
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, bParamsPtr.eligiblePrioFuncId);

        bParamsPtr.eligiblePrioFuncId=1;

         /*
            1.13 check out of range bParamsPtr.schdModeArr[0].
            Call with bParamsPtr.schdModeArr[0] [2], other parameters same as in 1.1.
            Expected: NON GT_OK.
        */

        bParamsPtr.schdModeArr[0] = 2;

        st = cpssTmAnodeToPortCreate(dev, portInd, &aParamsPtr, &bParamsPtr, &cParamsPtr, &aNodeIndPtr, &bNodeIndPtr, &cNodeIndPtr );
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, bParamsPtr.schdModeArr[0]);

        bParamsPtr.schdModeArr[0] = CPSS_TM_SCHD_MODE_RR_E;

        /*
            1.14 check out of range cParamsPtr.shapingProfilePtr.
            Call with cParamsPtr.shapingProfilePtr [85580], other parameters same as in 1.1.
            Expected: NON GT_OK.
        */

        cParamsPtr.shapingProfilePtr = 85580;

        st = cpssTmAnodeToPortCreate(dev, portInd, &aParamsPtr, &bParamsPtr, &cParamsPtr, &aNodeIndPtr, &bNodeIndPtr, &cNodeIndPtr );
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, cParamsPtr.shapingProfilePtr);

        cParamsPtr.shapingProfilePtr= 0;

         /*
            1.15 check out of range cParamsPtr.quantum.
            Call with cParamsPtr.quantum [0x30], other parameters same as in 1.1.
            Expected: NON GT_OK.
        */

        cParamsPtr.quantum=0x30;

        st = cpssTmAnodeToPortCreate(dev, portInd, &aParamsPtr, &bParamsPtr, &cParamsPtr, &aNodeIndPtr, &bNodeIndPtr, &cNodeIndPtr );
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, cParamsPtr.quantum);

        cParamsPtr.quantum=0x40;

         /*
            1.16 check out of range cParamsPtr.quantum.
            Call with cParamsPtr.quantum [0x4fff], other parameters same as in 1.1.
            Expected: NON GT_OK.
        */

        cParamsPtr.quantum=0x4fff;

        st = cpssTmAnodeToPortCreate(dev, portInd, &aParamsPtr, &bParamsPtr, &cParamsPtr, &aNodeIndPtr, &bNodeIndPtr, &cNodeIndPtr );
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, cParamsPtr.quantum);

        cParamsPtr.quantum=0x40;

         /*
            1.17 check out of range cParamsPtr.dropProfileInd.
            Call with cParamsPtr.dropProfileIndArr[0][300], other parameters same as in 1.1.
            Expected: NON GT_OK.
        */

        cParamsPtr.dropProfileIndArr[0]=300;

        st = cpssTmAnodeToPortCreate(dev, portInd, &aParamsPtr, &bParamsPtr, &cParamsPtr, &aNodeIndPtr, &bNodeIndPtr, &cNodeIndPtr );
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev,cParamsPtr.dropProfileIndArr[0]);

        cParamsPtr.dropProfileIndArr[0]=0;

         /*
            1.18 check out of range cParamsPtr.eligiblePrioFuncId.
            Call with cParamsPtr.eligiblePrioFuncId [70], other parameters same as in 1.1.
            Expected: NON GT_OK.
        */

        cParamsPtr.eligiblePrioFuncId=70;

        st = cpssTmAnodeToPortCreate(dev, portInd, &aParamsPtr, &bParamsPtr, &cParamsPtr, &aNodeIndPtr, &bNodeIndPtr, &cNodeIndPtr );
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, cParamsPtr.eligiblePrioFuncId);

        cParamsPtr.eligiblePrioFuncId=1;

         /*
            1.19 check out of range cParamsPtr.schdModeArr[0].
            Call with cParamsPtr.schdModeArr[0] [2], other parameters same as in 1.1.
            Expected: NON GT_OK.
        */

        cParamsPtr.schdModeArr[0]=2;

        st = cpssTmAnodeToPortCreate(dev, portInd, &aParamsPtr, &bParamsPtr, &cParamsPtr, &aNodeIndPtr, &bNodeIndPtr, &cNodeIndPtr );
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, cParamsPtr.schdModeArr[0]);

        cParamsPtr.schdModeArr[0]=0;

         /*
            1.20 check out of range cParamsPtr.dropCosMap.
            Call with cParamsPtr.dropCosMap [9], other parameters same as in 1.1.
            Expected: NON GT_OK.
        */

        cParamsPtr.dropCosMap=9;

        st = cpssTmAnodeToPortCreate(dev, portInd, &aParamsPtr, &bParamsPtr, &cParamsPtr, &aNodeIndPtr, &bNodeIndPtr, &cNodeIndPtr );
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, cParamsPtr.dropCosMap);

        cParamsPtr.dropCosMap=1;


        /*
            1.21 Call with aParamsPtr [NULL], other parameters same as in 1.1.
            Expected: GT_BAD_PTR.
        */

        st = cpssTmAnodeToPortCreate(dev, portInd, NULL, &bParamsPtr, &cParamsPtr, &aNodeIndPtr, &bNodeIndPtr, &cNodeIndPtr );
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, aParamsPtr = NULL", dev);

        /*
            1.22 Call with bParamsPtr [NULL], other parameters same as in 1.1.
            Expected: GT_BAD_PTR.
        */

        st = cpssTmAnodeToPortCreate(dev, portInd, &aParamsPtr, NULL, &cParamsPtr, &aNodeIndPtr, &bNodeIndPtr, &cNodeIndPtr );
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, bParamsPtr = NULL", dev);

        /*
            1.23 Call with cParamsPtr [NULL], other parameters same as in 1.1.
            Expected: GT_BAD_PTR.
        */

        st = cpssTmAnodeToPortCreate(dev, portInd, &aParamsPtr, &bParamsPtr, NULL, &aNodeIndPtr, &bNodeIndPtr, &cNodeIndPtr );
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, cParamsPtr = NULL", dev);


        /*
            1.24 Call with aNodeIndPtr [NULL], other parameters same as in 1.1.
            Expected: GT_BAD_PTR.
        */

        st = cpssTmAnodeToPortCreate(dev, portInd, &aParamsPtr, &bParamsPtr, NULL, &aNodeIndPtr, &bNodeIndPtr, &cNodeIndPtr );
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, aNodeIndPtr = NULL", dev);

        /*
            1.25 Call with bNodeIndPtr [NULL], other parameters same as in 1.1.
            Expected: GT_BAD_PTR.
        */

        st = cpssTmAnodeToPortCreate(dev, portInd, &aParamsPtr, &bParamsPtr, &cParamsPtr, &aNodeIndPtr, NULL, &cNodeIndPtr );
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, bNodeIndPtr = NULL", dev);

        /*
            1.26 Call with cNodeIndPtr [NULL], other parameters same as in 1.1.
            Expected: GT_BAD_PTR.
        */

        st = cpssTmAnodeToPortCreate(dev, portInd, &aParamsPtr, &bParamsPtr, &cParamsPtr, &aNodeIndPtr, &bNodeIndPtr, NULL );
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, cNodeIndPtr = NULL", dev);

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
        st = cpssTmAnodeToPortCreate(dev, portInd, &aParamsPtr, &bParamsPtr, &cParamsPtr, &aNodeIndPtr, &bNodeIndPtr, &cNodeIndPtr );
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssTmAnodeToPortCreate(dev, portInd, &aParamsPtr, &bParamsPtr, &cParamsPtr, &aNodeIndPtr, &bNodeIndPtr, &cNodeIndPtr );
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}



/*******************************************************************************
GT_STATUS cpssTmQueueToAnodeCreate
(
    IN  GT_U8                           devNum,
    IN  GT_U32                          aNodeInd,
    IN  CPSS_TM_QUEUE_PARAMS_STC        *qParamsPtr,
    OUT GT_U32                          *queueIndPtr
)
*/
UTF_TEST_CASE_MAC(cpssTmQueueToAnodeCreate)
{
    GT_STATUS               st = GT_OK;

    GT_U8                           dev;
    GT_U32                          portInd= 1;
    CPSS_TM_QUEUE_PARAMS_STC        qParamsPtr;
    CPSS_TM_A_NODE_PARAMS_STC       aParamsPtr;
    CPSS_TM_B_NODE_PARAMS_STC       bParamsPtr;
    CPSS_TM_C_NODE_PARAMS_STC       cParamsPtr;
    GT_U32                          queueIndPtr;
    GT_U32                          aNodeIndPtr;
    GT_U32                          bNodeIndPtr;
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

    cpssOsBzero((GT_VOID*)&aParamsPtr, sizeof(aParamsPtr));
    cpssOsBzero((GT_VOID*)&bParamsPtr, sizeof(bParamsPtr));
    cpssOsBzero((GT_VOID*)&cParamsPtr, sizeof(cParamsPtr));
    cpssOsBzero((GT_VOID*)&qParamsPtr, sizeof(qParamsPtr));
    cpssOsBzero((GT_VOID*)&paramsPtr, sizeof(paramsPtr));

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

        /* profile for Q level */
        qParamsPtr.shapingProfilePtr = 0;
        qParamsPtr.quantum = 0x40;
        qParamsPtr.dropProfileInd = 0;
        qParamsPtr.eligiblePrioFuncId = 1;

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

        st = cpssTmQueueToAnodeCreate(dev, aNodeIndPtr, &qParamsPtr, &queueIndPtr);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.2 check out of range qParamsPtr.shapingProfilePtr.
            Call with qParamsPtr.shapingProfilePtr [85580], other parameters same as in 1.1.
            Expected: NON GT_OK.
        */

        qParamsPtr.shapingProfilePtr = 85580;

        st = cpssTmQueueToAnodeCreate(dev, aNodeIndPtr, &qParamsPtr, &queueIndPtr);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, qParamsPtr.shapingProfilePtr);

        qParamsPtr.shapingProfilePtr= 0;

         /*
            1.3 check out of range qParamsPtr.quantum.
            Call with qParamsPtr.quantum [0x30], other parameters same as in 1.1.
            Expected: NON GT_OK.
        */

        qParamsPtr.quantum=0x30;

        st = cpssTmQueueToAnodeCreate(dev, aNodeIndPtr, &qParamsPtr, &queueIndPtr);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, qParamsPtr.quantum);

        qParamsPtr.quantum=0x40;

         /*
            1.4 check out of range qParamsPtr.quantum.
            Call with qParamsPtr.quantum [0x4fff], other parameters same as in 1.1.
            Expected: NON GT_OK.
        */

        qParamsPtr.quantum=0x4fff;

        st = cpssTmQueueToAnodeCreate(dev, aNodeIndPtr, &qParamsPtr, &queueIndPtr);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, qParamsPtr.quantum);

        qParamsPtr.quantum=0x40;

         /*
            1.5 check out of range qParamsPtr.dropProfileInd.
            Call with qParamsPtr.dropProfileInd [300], other parameters same as in 1.1.
            Expected: NON GT_OK.
        */

        qParamsPtr.dropProfileInd=300;

        st = cpssTmQueueToAnodeCreate(dev, aNodeIndPtr, &qParamsPtr, &queueIndPtr);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev,qParamsPtr.dropProfileInd);

        qParamsPtr.dropProfileInd=0;

         /*
            1.6 check out of range qParamsPtr.eligiblePrioFuncId.
            Call with qParamsPtr.eligiblePrioFuncId [70], other parameters same as in 1.1.
            Expected: NON GT_OK.
        */

        qParamsPtr.eligiblePrioFuncId=70;

        st = cpssTmQueueToAnodeCreate(dev, aNodeIndPtr, &qParamsPtr, &queueIndPtr);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, qParamsPtr.eligiblePrioFuncId);

        qParamsPtr.eligiblePrioFuncId=1;


        /*
            1.7 Call with qParamsPtr [NULL], other parameters same as in 1.1.
            Expected: GT_BAD_PTR.
        */

        st = cpssTmQueueToAnodeCreate(dev, aNodeIndPtr, NULL, &queueIndPtr);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, qParamsPtr = NULL", dev);


        /*
            1.8 Call with queueIndPtr [NULL], other parameters same as in 1.1.
            Expected: GT_BAD_PTR.
        */

        st = cpssTmQueueToAnodeCreate(dev, aNodeIndPtr, &qParamsPtr, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, queueIndPtr = NULL", dev);

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
        st = cpssTmQueueToAnodeCreate(dev, aNodeIndPtr, &qParamsPtr, &queueIndPtr);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssTmQueueToAnodeCreate(dev, aNodeIndPtr, &qParamsPtr, &queueIndPtr);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}



/*******************************************************************************
GT_STATUS cpssTmBnodeToCnodeCreate
(
    IN  GT_U8                           devNum,
    IN  GT_U32                          cNodeInd,
    IN  CPSS_TM_B_NODE_PARAMS_STC       *bParamsPtr,
    OUT GT_U32                          *bNodeIndPtr
)
*/
UTF_TEST_CASE_MAC(cpssTmBnodeToCnodeCreate)
{
    GT_STATUS               st = GT_OK;

    GT_U8                           dev;
    GT_U32                          portInd =1;
    CPSS_TM_B_NODE_PARAMS_STC       bParamsPtr;
    CPSS_TM_C_NODE_PARAMS_STC       cParamsPtr;
    GT_U32                          bNodeIndPtr;
    GT_U32                          cNodeIndPtr = 1;
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
    cpssOsBzero((GT_VOID*)&paramsPtr, sizeof(paramsPtr));

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

        st = cpssTmCnodeToPortCreate(dev, portInd, &cParamsPtr, &cNodeIndPtr);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*1.1 Check with legal parameters.
                Expected: GT_OK */

        st = cpssTmBnodeToCnodeCreate(dev,cNodeIndPtr, &bParamsPtr, &bNodeIndPtr);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);


        /*
            1.2 check out of range bParamsPtr.shapingProfilePtr.
            Call with bParamsPtr.shapingProfilePtr [85580], other parameters same as in 1.1.
            Expected: NON GT_OK.
        */

        bParamsPtr.shapingProfilePtr = 85580;

        st = cpssTmBnodeToCnodeCreate(dev,cNodeIndPtr, &bParamsPtr, &bNodeIndPtr);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, bParamsPtr.shapingProfilePtr);

        bParamsPtr.shapingProfilePtr= 0;

         /*
            1.3 check out of range bParamsPtr.quantum.
            Call with bParamsPtr.quantum [0x30], other parameters same as in 1.1.
            Expected: NON GT_OK.
        */

        bParamsPtr.quantum=0x30;

        st = cpssTmBnodeToCnodeCreate(dev,cNodeIndPtr, &bParamsPtr, &bNodeIndPtr);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, bParamsPtr.quantum);


         /*
            1.4 check out of range bParamsPtr.quantum.
            Call with bParamsPtr.quantum [0x4fff], other parameters same as in 1.1.
            Expected: NON GT_OK.
        */

        bParamsPtr.quantum=0x4fff;

        st = cpssTmBnodeToCnodeCreate(dev,cNodeIndPtr, &bParamsPtr, &bNodeIndPtr);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, bParamsPtr.quantum);

        bParamsPtr.quantum=0x40;

         /*
            1.5 check out of range bParamsPtr.dropProfileInd.
            Call with bParamsPtr.dropProfileInd [300], other parameters same as in 1.1.
            Expected: NON GT_OK.
        */

        bParamsPtr.dropProfileInd=300;

        st = cpssTmBnodeToCnodeCreate(dev,cNodeIndPtr, &bParamsPtr, &bNodeIndPtr);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev,bParamsPtr.dropProfileInd);

        bParamsPtr.dropProfileInd=0;

         /*
            1.6 check out of range bParamsPtr.eligiblePrioFuncId.
            Call with bParamsPtr.eligiblePrioFuncId [70], other parameters same as in 1.1.
            Expected: NON GT_OK.
        */

        bParamsPtr.eligiblePrioFuncId=70;

        st = cpssTmBnodeToCnodeCreate(dev,cNodeIndPtr, &bParamsPtr, &bNodeIndPtr);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, bParamsPtr.eligiblePrioFuncId);

        bParamsPtr.eligiblePrioFuncId=1;

         /*
            1.7 check out of range bParamsPtr.schdModeArr[0].
            Call with bParamsPtr.schdModeArr[0] [2], other parameters same as in 1.1.
            Expected: NON GT_OK.
        */

        bParamsPtr.schdModeArr[0] = 2;

        st = cpssTmBnodeToCnodeCreate(dev,cNodeIndPtr, &bParamsPtr, &bNodeIndPtr);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, bParamsPtr.schdModeArr[0]);

        bParamsPtr.schdModeArr[0] = CPSS_TM_SCHD_MODE_RR_E;

        /*
            1.8 Call with bParamsPtr [NULL], other parameters same as in 1.1.
            Expected: GT_BAD_PTR.
        */

        st = cpssTmBnodeToCnodeCreate(dev,cNodeIndPtr, NULL, &bNodeIndPtr);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, bParamsPtr = NULL", dev);

        /*
            1.9 Call with bNodeIndPtr [NULL], other parameters same as in 1.1.
            Expected: GT_BAD_PTR.
        */

        st = cpssTmBnodeToCnodeCreate(dev,cNodeIndPtr, &bParamsPtr, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, bNodeIndPtr = NULL", dev);


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
        st = cpssTmBnodeToCnodeCreate(dev,cNodeIndPtr, &bParamsPtr, &bNodeIndPtr);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssTmBnodeToCnodeCreate(dev,cNodeIndPtr, &bParamsPtr, &bNodeIndPtr);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*******************************************************************************
GT_STATUS cpssTmAnodeToCnodeCreate
(
    IN  GT_U8                           devNum,
    IN  GT_U32                          cNodeInd,
    IN  CPSS_TM_A_NODE_PARAMS_STC       *aParamsPtr,
    IN  CPSS_TM_B_NODE_PARAMS_STC       *bParamsPtr,
    OUT GT_U32                          *aNodeIndPtr,
    OUT GT_U32                          *bNodeIndPtr
)
*/
UTF_TEST_CASE_MAC(cpssTmAnodeToCnodeCreate)
{
    GT_STATUS               st = GT_OK;

    GT_U8                           dev;
    GT_U32                          portInd =1;
    CPSS_TM_A_NODE_PARAMS_STC       aParamsPtr;
    CPSS_TM_B_NODE_PARAMS_STC       bParamsPtr;
    CPSS_TM_C_NODE_PARAMS_STC       cParamsPtr;
    GT_U32                          aNodeIndPtr;
    GT_U32                          bNodeIndPtr;
    GT_U32                          cNodeIndPtr = 1;
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

    cpssOsBzero((GT_VOID*)&aParamsPtr, sizeof(aParamsPtr));
    cpssOsBzero((GT_VOID*)&bParamsPtr, sizeof(bParamsPtr));
    cpssOsBzero((GT_VOID*)&cParamsPtr, sizeof(cParamsPtr));
    cpssOsBzero((GT_VOID*)&paramsPtr, sizeof(paramsPtr));

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
        paramsPtr.cbs = 8;/*KBytes*/
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

        st = cpssTmCnodeToPortCreate(dev, portInd, &cParamsPtr, &cNodeIndPtr);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*1.1 Check with legal parameters.
                Expected: GT_OK */

        st = cpssTmAnodeToCnodeCreate(dev, cNodeIndPtr, &aParamsPtr, &bParamsPtr, &aNodeIndPtr, &bNodeIndPtr);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.2 check out of range aParamsPtr.shapingProfilePtr.
            Call with aParamsPtr.shapingProfilePtr [85580], other parameters same as in 1.1.
            Expected: NON GT_OK.
        */

        aParamsPtr.shapingProfilePtr = 85580;

        st = cpssTmAnodeToCnodeCreate(dev, cNodeIndPtr, &aParamsPtr, &bParamsPtr, &aNodeIndPtr, &bNodeIndPtr);

        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, aParamsPtr.shapingProfilePtr);

        aParamsPtr.shapingProfilePtr= 0;

         /*
            1.3 check out of range aParamsPtr.quantum.
            Call with aParamsPtr.quantum [0x30], other parameters same as in 1.1.
            Expected: NON GT_OK.
        */

        aParamsPtr.quantum=0x30;

        st = cpssTmAnodeToCnodeCreate(dev, cNodeIndPtr, &aParamsPtr, &bParamsPtr, &aNodeIndPtr, &bNodeIndPtr);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, aParamsPtr.quantum);

        aParamsPtr.quantum=0x40;

         /*
            1.4 check out of range aParamsPtr.quantum.
            Call with aParamsPtr.quantum [0x4fff], other parameters same as in 1.1.
            Expected: NON GT_OK.
        */

        aParamsPtr.quantum=0x4fff;

        st = cpssTmAnodeToCnodeCreate(dev, cNodeIndPtr, &aParamsPtr, &bParamsPtr, &aNodeIndPtr, &bNodeIndPtr);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, aParamsPtr.quantum);

        aParamsPtr.quantum=0x40;

         /*
            1.5 check out of range aParamsPtr.dropProfileInd.
            Call with aParamsPtr.dropProfileInd [300], other parameters same as in 1.1.
            Expected: NON GT_OK.
        */

        aParamsPtr.dropProfileInd=300;

        st = cpssTmAnodeToCnodeCreate(dev, cNodeIndPtr, &aParamsPtr, &bParamsPtr, &aNodeIndPtr, &bNodeIndPtr);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev,aParamsPtr.dropProfileInd);

        aParamsPtr.dropProfileInd=0;

         /*
            1.6 check out of range aParamsPtr.eligiblePrioFuncId.
            Call with aParamsPtr.eligiblePrioFuncId [70], other parameters same as in 1.1.
            Expected: NON GT_OK.
        */

        aParamsPtr.eligiblePrioFuncId=70;

        st = cpssTmAnodeToCnodeCreate(dev, cNodeIndPtr, &aParamsPtr, &bParamsPtr, &aNodeIndPtr, &bNodeIndPtr);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, aParamsPtr.eligiblePrioFuncId);

        aParamsPtr.eligiblePrioFuncId=1;

         /*
            1.7 check out of range aParamsPtr.schdModeArr[0].
            Call with aParamsPtr.schdModeArr[0] [2], other parameters same as in 1.1.
            Expected: NON GT_OK.
        */

        aParamsPtr.schdModeArr[0] = 2;

        st = cpssTmAnodeToCnodeCreate(dev, cNodeIndPtr, &aParamsPtr, &bParamsPtr, &aNodeIndPtr, &bNodeIndPtr);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, aParamsPtr.schdModeArr[0]);

        aParamsPtr.schdModeArr[0] = CPSS_TM_SCHD_MODE_RR_E;

        /*
            1.8 check out of range bParamsPtr.shapingProfilePtr.
            Call with bParamsPtr.shapingProfilePtr [85580], other parameters same as in 1.1.
            Expected: NON GT_OK.
        */

        bParamsPtr.shapingProfilePtr = 85580;

        st = cpssTmAnodeToCnodeCreate(dev, cNodeIndPtr, &aParamsPtr, &bParamsPtr, &aNodeIndPtr, &bNodeIndPtr);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, bParamsPtr.shapingProfilePtr);

        bParamsPtr.shapingProfilePtr= 0;

         /*
            1.9 check out of range bParamsPtr.quantum.
            Call with bParamsPtr.quantum [0x30], other parameters same as in 1.1.
            Expected: NON GT_OK.
        */

        bParamsPtr.quantum=0x30;

        st = cpssTmAnodeToCnodeCreate(dev, cNodeIndPtr, &aParamsPtr, &bParamsPtr, &aNodeIndPtr, &bNodeIndPtr);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, bParamsPtr.quantum);

        bParamsPtr.quantum=0x40;

         /*
            1.10 check out of range bParamsPtr.quantum.
            Call with bParamsPtr.quantum [0x4fff], other parameters same as in 1.1.
            Expected: NON GT_OK.
        */

        bParamsPtr.quantum=0x4fff;

        st = cpssTmAnodeToCnodeCreate(dev, cNodeIndPtr, &aParamsPtr, &bParamsPtr, &aNodeIndPtr, &bNodeIndPtr);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, bParamsPtr.quantum);

        bParamsPtr.quantum=0x40;

         /*
            1.11 check out of range bParamsPtr.dropProfileInd.
            Call with bParamsPtr.dropProfileInd [300], other parameters same as in 1.1.
            Expected: NON GT_OK.
        */

        bParamsPtr.dropProfileInd=300;

        st = cpssTmAnodeToCnodeCreate(dev, cNodeIndPtr, &aParamsPtr, &bParamsPtr, &aNodeIndPtr, &bNodeIndPtr);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev,bParamsPtr.dropProfileInd);

        bParamsPtr.dropProfileInd=0;

         /*
            1.12 check out of range bParamsPtr.eligiblePrioFuncId.
            Call with bParamsPtr.eligiblePrioFuncId [70], other parameters same as in 1.1.
            Expected: NON GT_OK.
        */

        bParamsPtr.eligiblePrioFuncId=70;

        st = cpssTmAnodeToCnodeCreate(dev, cNodeIndPtr, &aParamsPtr, &bParamsPtr, &aNodeIndPtr, &bNodeIndPtr);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, bParamsPtr.eligiblePrioFuncId);

        bParamsPtr.eligiblePrioFuncId=1;

         /*
            1.13 check out of range bParamsPtr.schdModeArr[0].
            Call with bParamsPtr.schdModeArr[0] [2], other parameters same as in 1.1.
            Expected: NON GT_OK.
        */

        bParamsPtr.schdModeArr[0] = 2;

        st = cpssTmAnodeToCnodeCreate(dev, cNodeIndPtr, &aParamsPtr, &bParamsPtr, &aNodeIndPtr, &bNodeIndPtr);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, bParamsPtr.schdModeArr[0]);

        bParamsPtr.schdModeArr[0] = CPSS_TM_SCHD_MODE_RR_E;

        /*
            1.14 Call with aParamsPtr [NULL], other parameters same as in 1.1.
            Expected: GT_BAD_PTR.
        */

        st = cpssTmAnodeToCnodeCreate(dev, cNodeIndPtr, NULL, &bParamsPtr, &aNodeIndPtr, &bNodeIndPtr);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, aParamsPtr = NULL", dev);

        /*
            1.15 Call with bParamsPtr [NULL], other parameters same as in 1.1.
            Expected: GT_BAD_PTR.
        */

        st = cpssTmAnodeToCnodeCreate(dev, cNodeIndPtr, &aParamsPtr, NULL, &aNodeIndPtr, &bNodeIndPtr);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, bParamsPtr = NULL", dev);

        /*
            1.16 Call with aNodeIndPtr [NULL], other parameters same as in 1.1.
            Expected: GT_BAD_PTR.
        */

        st = cpssTmAnodeToCnodeCreate(dev, cNodeIndPtr, &aParamsPtr, &bParamsPtr, NULL, &bNodeIndPtr);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, aNodeIndPtr = NULL", dev);

        /*
            1.17 Call with bNodeIndPtr [NULL], other parameters same as in 1.1.
            Expected: GT_BAD_PTR.
        */

        st = cpssTmAnodeToCnodeCreate(dev, cNodeIndPtr, &aParamsPtr, &bParamsPtr, &aNodeIndPtr, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, bNodeIndPtr = NULL", dev);

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
        st = cpssTmAnodeToCnodeCreate(dev, cNodeIndPtr, &aParamsPtr, &bParamsPtr, &aNodeIndPtr, &bNodeIndPtr);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssTmAnodeToCnodeCreate(dev, cNodeIndPtr, &aParamsPtr, &bParamsPtr, &aNodeIndPtr, &bNodeIndPtr);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}



/*******************************************************************************
GT_STATUS cpssTmAnodeToBnodeCreate
(
    IN  GT_U8                           devNum,
    IN  GT_U32                          bNodeInd,
    IN  CPSS_TM_A_NODE_PARAMS_STC       *aParamsPtr,
    OUT GT_U32                          *aNodeIndPtr
)
*/
UTF_TEST_CASE_MAC(cpssTmAnodeToBnodeCreate)
{
    GT_STATUS               st = GT_OK;

    GT_U8                           dev;
    GT_U32                          portInd =1;
    CPSS_TM_A_NODE_PARAMS_STC       aParamsPtr;
    CPSS_TM_B_NODE_PARAMS_STC       bParamsPtr;
    CPSS_TM_C_NODE_PARAMS_STC       cParamsPtr;
    GT_U32                          aNodeIndPtr;
    GT_U32                          bNodeIndPtr;
    GT_U32                          cNodeIndPtr;
    GT_U32                         ii;
    CPSS_TM_PORT_PARAMS_STC      paramsPtr;
    GT_U32                       cNodesNum;
    GT_U32                       bNodesNum;
    GT_U32                       aNodesNum;
    GT_U32                       queuesNum;
    CPSS_PP_FAMILY_TYPE_ENT devFamily;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev,
                                 UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_LION_E |
                                     UTF_PUMA2_E | UTF_PUMA3_E | UTF_XCAT2_E | UTF_LION2_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E | UTF_ALDRIN_E | UTF_AC3X_E);

    cpssOsBzero((GT_VOID*)&aParamsPtr, sizeof(aParamsPtr));
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
        aNodesNum=2;
        queuesNum=2;

        st = cpssTmPortCreate(dev, portInd, &paramsPtr, cNodesNum, bNodesNum, aNodesNum,queuesNum );
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

         st = cpssTmBnodeToPortCreate(dev, portInd, &bParamsPtr, &cParamsPtr,&bNodeIndPtr, &cNodeIndPtr );
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);



        /*1.1 Check with legal parameters.
                Expected: GT_OK */

        st = cpssTmAnodeToBnodeCreate(dev,bNodeIndPtr, &aParamsPtr,&aNodeIndPtr);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.2 check out of range aParamsPtr.shapingProfilePtr.
            Call with aParamsPtr.shapingProfilePtr [85580], other parameters same as in 1.1.
            Expected: NON GT_OK.
        */

        aParamsPtr.shapingProfilePtr = 85580;

        st = cpssTmAnodeToBnodeCreate(dev,bNodeIndPtr, &aParamsPtr,&aNodeIndPtr);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, aParamsPtr.shapingProfilePtr);

        aParamsPtr.shapingProfilePtr= 0;

         /*
            1.3 check out of range aParamsPtr.quantum.
            Call with aParamsPtr.quantum [0x30], other parameters same as in 1.1.
            Expected: NON GT_OK.
        */

        aParamsPtr.quantum=0x7;

        st = cpssTmAnodeToBnodeCreate(dev,bNodeIndPtr, &aParamsPtr,&aNodeIndPtr);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, aParamsPtr.quantum);

        aParamsPtr.quantum=0x40;

         /*
            1.4 check out of range aParamsPtr.quantum.
            Call with aParamsPtr.quantum [0x4fff], other parameters same as in 1.1.
            Expected: NON GT_OK.
        */

        aParamsPtr.quantum=0x4fff;

        st = cpssTmAnodeToBnodeCreate(dev,bNodeIndPtr, &aParamsPtr,&aNodeIndPtr);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, aParamsPtr.quantum);

        aParamsPtr.quantum=0x7;

        st = cpssTmAnodeToBnodeCreate(dev,bNodeIndPtr, &aParamsPtr,&aNodeIndPtr);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, aParamsPtr.quantum);

        aParamsPtr.quantum=0x40;
         /*
            1.5 check out of range aParamsPtr.dropProfileInd.
            Call with aParamsPtr.dropProfileInd [300], other parameters same as in 1.1.
            Expected: NON GT_OK.
        */

        aParamsPtr.dropProfileInd=300;

        st = cpssTmAnodeToBnodeCreate(dev,bNodeIndPtr, &aParamsPtr,&aNodeIndPtr);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev,aParamsPtr.dropProfileInd);

        aParamsPtr.dropProfileInd=0;

         /*
            1.6 check out of range aParamsPtr.eligiblePrioFuncId.
            Call with aParamsPtr.eligiblePrioFuncId [70], other parameters same as in 1.1.
            Expected: NON GT_OK.
        */

        aParamsPtr.eligiblePrioFuncId=70;

        st = cpssTmAnodeToBnodeCreate(dev,bNodeIndPtr, &aParamsPtr,&aNodeIndPtr);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, aParamsPtr.eligiblePrioFuncId);

        aParamsPtr.eligiblePrioFuncId=1;

         /*
            1.7 check out of range aParamsPtr.schdModeArr[0].
            Call with aParamsPtr.schdModeArr[0] [2], other parameters same as in 1.1.
            Expected: NON GT_OK.
        */

        aParamsPtr.schdModeArr[0] = 2;

        st = cpssTmAnodeToBnodeCreate(dev,bNodeIndPtr, &aParamsPtr,&aNodeIndPtr);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, aParamsPtr.schdModeArr[0]);

        aParamsPtr.schdModeArr[0] = CPSS_TM_SCHD_MODE_RR_E;

        /*
            1.8 Call with aParamsPtr [NULL], other parameters same as in 1.1.
            Expected: GT_BAD_PTR.
        */

        st = cpssTmAnodeToBnodeCreate(dev,bNodeIndPtr, NULL,&aNodeIndPtr);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, aParamsPtr = NULL", dev);

        /*
            1.9 Call with aNodeIndPtr [NULL], other parameters same as in 1.1.
            Expected: GT_BAD_PTR.
        */

        st = cpssTmAnodeToBnodeCreate(dev,bNodeIndPtr, &aParamsPtr,NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, aNodeIndPtr = NULL", dev);


        /*
            2.0  valid call - with parameters same as in 1.1.
            Expected: GT_OK.
        */
       st = cpssTmAnodeToBnodeCreate(dev,bNodeIndPtr, &aParamsPtr,&aNodeIndPtr);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);



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
        st = cpssTmAnodeToBnodeCreate(dev,bNodeIndPtr, &aParamsPtr,&aNodeIndPtr);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssTmAnodeToBnodeCreate(dev,bNodeIndPtr, &aParamsPtr,&aNodeIndPtr);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}


/*******************************************************************************
*/
UTF_TEST_CASE_MAC(cpssRangeRelatedCreations)
{
    GT_STATUS               st = GT_OK;

    GT_U8                           dev;
    GT_U32                          portInd= 1;
    CPSS_TM_QUEUE_PARAMS_STC        qParamsPtr;
    CPSS_TM_A_NODE_PARAMS_STC       aParamsPtr;
    CPSS_TM_B_NODE_PARAMS_STC       bParamsPtr;
    CPSS_TM_C_NODE_PARAMS_STC       cParamsPtr;
    GT_U32                          bNodeIndPtr;
    GT_U32                          cNodeIndPtr;
    CPSS_TM_PORT_PARAMS_STC         paramsPtr;
    CPSS_PP_FAMILY_TYPE_ENT         devFamily;
    GT_U32                          ii;
    GT_U32                          aNodeInd0;
    GT_U32                          aNodeInd1;
    GT_U32                          aNodeInd2;
    GT_U32                          aNodeInd3;
    GT_U32                          queueInd0;
    GT_U32                          queueInd1;
    GT_U32                          queueInd2;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev,
                                 UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_LION_E |
                                     UTF_PUMA2_E | UTF_PUMA3_E | UTF_XCAT2_E | UTF_LION2_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E | UTF_ALDRIN_E | UTF_AC3X_E);

    cpssOsBzero((GT_VOID*)&aParamsPtr, sizeof(aParamsPtr));
    cpssOsBzero((GT_VOID*)&bParamsPtr, sizeof(bParamsPtr));
    cpssOsBzero((GT_VOID*)&cParamsPtr, sizeof(cParamsPtr));
    cpssOsBzero((GT_VOID*)&qParamsPtr, sizeof(qParamsPtr));
    cpssOsBzero((GT_VOID*)&paramsPtr, sizeof(paramsPtr));

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
        bParamsPtr.dropProfileInd =0;
        bParamsPtr.eligiblePrioFuncId =9;

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
        qParamsPtr.shapingProfilePtr = 0;
        qParamsPtr.quantum = 0x40;
        qParamsPtr.dropProfileInd = 0;
        qParamsPtr.eligiblePrioFuncId = 1;

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
        paramsPtr.numOfChildren = 1;

        portInd = 1;

        st = cpssTmAsymPortCreate(dev, portInd, &paramsPtr);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        cParamsPtr.numOfChildren = 1;
        st = cpssTmCnodeToPortCreate(dev, portInd, &cParamsPtr,  &cNodeIndPtr );
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        bParamsPtr.numOfChildren = 5;
        st = cpssTmBnodeToCnodeCreate(dev, cNodeIndPtr, &bParamsPtr,  &bNodeIndPtr );
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);



        aParamsPtr.numOfChildren = 4;
        st = cpssTmAnodeToBnodeCreate(dev, bNodeIndPtr, &aParamsPtr,  &aNodeInd0);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
        /* Here Anode allocated by default queues 1,2,3,4  */

        /* overlap with previously allocated queues - should fail */
        st = cpssTmAnodeToBnodeWithQueuePoolCreate(dev, bNodeIndPtr, &aParamsPtr,  /*first queue */1, /*queue range */2, &aNodeInd1);
        UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, dev);
        /* overhead with previously allocated queues - should fail */
        st = cpssTmAnodeToBnodeWithQueuePoolCreate(dev, bNodeIndPtr, &aParamsPtr,  /*first queue */3, /*queue range */3, &aNodeInd1);
        UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* required range is too big - should fail */
        st = cpssTmAnodeToBnodeWithQueuePoolCreate(dev, bNodeIndPtr, &aParamsPtr,  /*first queue */5, /*queue range */20000, &aNodeInd1);
        UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* allocate queues in free pool - should success */
        st = cpssTmAnodeToBnodeWithQueuePoolCreate(dev, bNodeIndPtr, &aParamsPtr,  /*first queue */17, /*queue range */4,  &aNodeInd1);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        st = cpssTmQueueToAnodeCreate(dev, aNodeInd0, &qParamsPtr, &queueInd0);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
        UTF_VERIFY_EQUAL1_PARAM_MAC(1, queueInd0, dev);
        /* should fail - already initialized by previous call */
        st = cpssTmQueueToAnodeByIndexCreate(dev, aNodeInd0, &qParamsPtr, queueInd0);
        UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* should success  */
        st = cpssTmQueueToAnodeByIndexCreate(dev, aNodeInd0, &qParamsPtr, 2);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
        st = cpssTmQueueToAnodeByIndexCreate(dev, aNodeInd0, &qParamsPtr, 3);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
        st = cpssTmQueueToAnodeByIndexCreate(dev, aNodeInd0, &qParamsPtr, 4);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* should fail -not in range */
        st = cpssTmQueueToAnodeByIndexCreate(dev, aNodeInd0, &qParamsPtr, 5);
        UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* should success  */
        st = cpssTmQueueToAnodeCreate(dev, aNodeInd1, &qParamsPtr, &queueInd1);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
        UTF_VERIFY_EQUAL1_PARAM_MAC(17, queueInd1, dev);
        st = cpssTmQueueToAnodeByIndexCreate(dev, aNodeInd1, &qParamsPtr, 18);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
        st = cpssTmQueueToAnodeCreate(dev, aNodeInd1, &qParamsPtr, &queueInd2);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
        UTF_VERIFY_EQUAL1_PARAM_MAC(19, queueInd2, dev);
        st = cpssTmQueueToAnodeByIndexCreate(dev, aNodeInd1, &qParamsPtr, 20);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
        /* should fail -not in range */
        st = cpssTmQueueToAnodeByIndexCreate(dev, aNodeInd1, &qParamsPtr, 21);
        UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, dev);


        /* allocate queues in free pool - should fail , head overlap*/
        st = cpssTmAnodeToBnodeWithQueuePoolCreate(dev, bNodeIndPtr, &aParamsPtr,  /*first queue */3, /*queue range */4,  &aNodeInd1);
        UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* allocate queues in free pool - should fail , tail overlap*/
        st = cpssTmAnodeToBnodeWithQueuePoolCreate(dev, bNodeIndPtr, &aParamsPtr,  /*first queue */7, /*queue range */11,  &aNodeInd1);
        UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, dev);


        st = cpssTmAnodeToBnodeCreate(dev, bNodeIndPtr, &aParamsPtr,  &aNodeInd2);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
        /* according to allocation rules  next queue chunk should be after index 21  - should fail */
        st = cpssTmQueueToAnodeByIndexCreate(dev, aNodeInd2, &qParamsPtr, 5);
        UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        aParamsPtr.numOfChildren = 12; /*17 -4 -1 */
        /* now  required size is equial to free space between  aNodeInd0 & aNodeInd1 child queues */
        /* so it should be allocated during next call */
        st = cpssTmAnodeToBnodeCreate(dev, bNodeIndPtr, &aParamsPtr,  &aNodeInd3);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
        /* now  it should success */
        st = cpssTmQueueToAnodeByIndexCreate(dev, aNodeInd3, &qParamsPtr, 5);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);


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
        st = cpssTmAnodeToBnodeWithQueuePoolCreate(dev, bNodeIndPtr, &aParamsPtr, 1, 2, &queueInd0);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
        st = cpssTmQueueToAnodeByIndexCreate(dev, aNodeInd0, &qParamsPtr, 1);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssTmAnodeToBnodeWithQueuePoolCreate(dev, bNodeIndPtr, &aParamsPtr, 1, 2, &queueInd0);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
    st = cpssTmQueueToAnodeByIndexCreate(dev, aNodeInd0, &qParamsPtr, 1);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}


/*----------------------------------------------------------------------------*/
/*
 * Configuration of cpssTmDrop suit
 */
UTF_SUIT_BEGIN_TESTS_MAC(cpssTmNodesCreate)
    UTF_SUIT_DECLARE_TEST_MAC(cpssTmPortCreate)
    UTF_SUIT_DECLARE_TEST_MAC(cpssTmAsymPortCreate)
    UTF_SUIT_DECLARE_TEST_MAC(cpssTmPortDropPerCosConfig)
    UTF_SUIT_DECLARE_TEST_MAC(cpssTmQueueToPortCreate)
    UTF_SUIT_DECLARE_TEST_MAC(cpssTmTransQueueToPortCreate)
    UTF_SUIT_DECLARE_TEST_MAC(cpssTmQueueToCnodeCreate)
    UTF_SUIT_DECLARE_TEST_MAC(cpssTmCnodeToPortCreate)
    UTF_SUIT_DECLARE_TEST_MAC(cpssTmBnodeToPortCreate)
    UTF_SUIT_DECLARE_TEST_MAC(cpssTmQueueToBnodeCreate)
    UTF_SUIT_DECLARE_TEST_MAC(cpssTmAnodeToPortCreate)
    UTF_SUIT_DECLARE_TEST_MAC(cpssTmQueueToAnodeCreate)
    UTF_SUIT_DECLARE_TEST_MAC(cpssTmBnodeToCnodeCreate)
    UTF_SUIT_DECLARE_TEST_MAC(cpssTmAnodeToCnodeCreate)
    UTF_SUIT_DECLARE_TEST_MAC(cpssTmAnodeToBnodeCreate)
    UTF_SUIT_DECLARE_TEST_MAC(cpssRangeRelatedCreations)
UTF_SUIT_END_TESTS_MAC(cpssTmNodesCreate)

