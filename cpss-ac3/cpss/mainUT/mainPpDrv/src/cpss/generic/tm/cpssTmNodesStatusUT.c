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
* @file cpssTmNodesStatusUT.c
*
* @brief Unit tests for cpssTmNodesStatus.
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
#include <cpss/generic/tm/cpssTmNodesStatus.h>
#include <cpssCommon/private/prvCpssMath.h>
#include <cpss/generic/tm/cpssTmNodesRead.h>
#include <cpss/generic/tm/cpssTmUtDefs.h>
#include <utf/utfMain.h>
#include <utf/private/prvUtfExtras.h>



/*******************************************************************************
GT_STATUS cpssTmQueueStatusRead
(
    IN  GT_U8                       devNum,
    IN  GT_U32                      index,
    OUT CPSS_TM_NODE_STATUS_STC     *statusPtr
)

*/
UTF_TEST_CASE_MAC(cpssTmQueueStatusRead)
{
    GT_STATUS               st = GT_OK;

    GT_U8                        dev;
    GT_U32                       portInd;
    CPSS_TM_QUEUE_PARAMS_STC     qParamsPtr;
    GT_U32                       queueIndPtr = 1;
    CPSS_TM_PORT_PARAMS_STC      paramsPtr;
    GT_U32                       cNodesNum;
    GT_U32                       bNodesNum;
    GT_U32                       aNodesNum;
    GT_U32                       queuesNum;
    CPSS_TM_NODE_STATUS_STC      statusPtr;
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
        paramsPtr.cbs = 8;/*KBytes*/
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

        st = cpssTmQueueStatusRead(dev, queueIndPtr, &statusPtr);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

                /*
            1.2 check out of range queueIndPtr.
            Call with queueIndPtr [queueIndPtr+100], other parameters same as in 1.1.
            Expected: NON GT_OK.
        */

        queueIndPtr= queueIndPtr+100;

        st = cpssTmQueueStatusRead(dev, queueIndPtr, &statusPtr);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, queueIndPtr);

        queueIndPtr= queueIndPtr-100;


        /*
            1.3 Call with statusPtr [NULL], other parameters same as in 1.1.
            Expected: GT_BAD_PTR.
        */

        st = cpssTmQueueStatusRead(dev, queueIndPtr, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, statusPtr = NULL", dev);


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
        st = cpssTmQueueStatusRead(dev, queueIndPtr, NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssTmQueueStatusRead(dev, queueIndPtr, NULL);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}


/*******************************************************************************
GT_STATUS cpssTmPortStatusRead
(
    IN  GT_U8                       devNum,
    IN  GT_U32                      index,
    OUT CPSS_TM_PORT_STATUS_STC     *statusPtr
)
*/
UTF_TEST_CASE_MAC(cpssTmPortStatusRead)
{
    GT_STATUS               st = GT_OK;

    GT_U8                        dev;
    GT_U32                       portInd;
    CPSS_TM_PORT_PARAMS_STC      paramsPtr;
    GT_U32                       cNodesNum;
    GT_U32                       bNodesNum;
    GT_U32                       aNodesNum;
    GT_U32                       queuesNum;
    CPSS_TM_PORT_STATUS_STC      statusPtr;
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

        portInd=1;
        cNodesNum=1;
        bNodesNum=1;
        aNodesNum=1;
        queuesNum=1;

        st = cpssTmPortCreate(dev, portInd, &paramsPtr, cNodesNum, bNodesNum, aNodesNum,queuesNum );
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*1.1 Check with legal parameters.
                Expected: GT_OK */

        st =cpssTmPortStatusRead(dev, portInd, &statusPtr);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

         /*
            1.2 check out of range portInd.
            Call with portInd [70], other parameters same as in 1.1.
            Expected: NON GT_OK.
        */

        portInd = portInd+70;

        st =cpssTmPortStatusRead(dev, portInd, &statusPtr);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, portInd);

        portInd= portInd-70;


        /*
            1.3 Call with statusPtr [NULL], other parameters same as in 1.1.
            Expected: GT_BAD_PTR.
        */

        st =cpssTmPortStatusRead(dev, portInd, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, statusPtr = NULL", dev);

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
        st =cpssTmPortStatusRead(dev, portInd, &statusPtr);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st =cpssTmPortStatusRead(dev, portInd, &statusPtr);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}


/*******************************************************************************
GT_STATUS cpssTmCnodeStatusRead
(
    IN  GT_U8                       devNum,
    IN  GT_U32                      index,
    OUT CPSS_TM_NODE_STATUS_STC     *statusPtr
)
*/
UTF_TEST_CASE_MAC(cpssTmCnodeStatusRead)
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
    CPSS_TM_NODE_STATUS_STC      statusPtr;
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

        st = cpssTmCnodeStatusRead(dev, cNodeIndPtr, &statusPtr);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.2 check out of range cNodeIndPtr.
            Call with cNodeIndPtr [cNodeIndPtr+20], other parameters same as in 1.1.
            Expected: NON GT_OK.
        */

        cNodeIndPtr= cNodeIndPtr+20;

        st = cpssTmCnodeStatusRead(dev, cNodeIndPtr, &statusPtr);

        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, cNodeIndPtr);

        cNodeIndPtr= cNodeIndPtr-20;

        /*
            1.3 Call with statusPtr [NULL], other parameters same as in 1.1.
            Expected: GT_BAD_PTR.
        */

        st = cpssTmCnodeStatusRead(dev, cNodeIndPtr, NULL);

        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, statusPtr = NULL", dev);


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
        st = cpssTmCnodeStatusRead(dev, cNodeIndPtr, &statusPtr);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssTmCnodeStatusRead(dev, cNodeIndPtr, &statusPtr);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}


/*******************************************************************************
GT_STATUS cpssTmBnodeStatusRead
(
    IN  GT_U8                       devNum,
    IN  GT_U32                      index,
    OUT CPSS_TM_NODE_STATUS_STC     *statusPtr
)*/
UTF_TEST_CASE_MAC(cpssTmBnodeStatusRead)
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
    CPSS_TM_NODE_STATUS_STC      statusPtr;
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

        st = cpssTmBnodeStatusRead(dev, bNodeIndPtr, &statusPtr);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);


        /*
            1.2 check out of range bNodeIndPtr.
            Call with bNodeIndPtr [bNodeIndPtr+20], other parameters same as in 1.1.
            Expected: NON GT_OK.
        */

        bNodeIndPtr=bNodeIndPtr+20;

        st = cpssTmBnodeStatusRead(dev, bNodeIndPtr, &statusPtr);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, bNodeIndPtr);

        bNodeIndPtr=bNodeIndPtr-20;

        /*
            1.3 Call with statusPtr [NULL], other parameters same as in 1.1.
            Expected: GT_BAD_PTR.
        */

        st = cpssTmBnodeStatusRead(dev, bNodeIndPtr, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, statusPtr = NULL", dev);

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
        st = cpssTmBnodeStatusRead(dev, bNodeIndPtr, NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssTmBnodeStatusRead(dev, bNodeIndPtr, NULL);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}


/*******************************************************************************
GT_STATUS cpssTmAnodeStatusRead
(
    IN  GT_U8                       devNum,
    IN  GT_U32                      index,
    OUT CPSS_TM_NODE_STATUS_STC     *statusPtr
)
*/
UTF_TEST_CASE_MAC(cpssTmAnodeStatusRead)
{
    GT_STATUS               st = GT_OK;

    GT_U8                           dev;
    GT_U32                          portInd;
    CPSS_TM_A_NODE_PARAMS_STC       aParamsPtr;
    CPSS_TM_B_NODE_PARAMS_STC       bParamsPtr;
    CPSS_TM_C_NODE_PARAMS_STC       cParamsPtr;
    GT_U32                          aNodeIndPtr = 1;
    GT_U32                          bNodeIndPtr;
    GT_U32                          cNodeIndPtr;
    CPSS_TM_PORT_PARAMS_STC      paramsPtr;
    GT_U32                       cNodesNum;
    GT_U32                       bNodesNum;
    GT_U32                       aNodesNum;
    GT_U32                       queuesNum;
    CPSS_TM_NODE_STATUS_STC      statusPtr;
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
    cpssOsBzero((GT_VOID*)&statusPtr, sizeof(statusPtr));

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

        st = cpssTmAnodeStatusRead(dev, aNodeIndPtr, &statusPtr);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);


        /*
            1.2 check out of range aNodeIndPtr.
            Call with aNodeIndPtr [aNodeIndPtr+20], other parameters same as in 1.1.
            Expected: NON GT_OK.
        */

        aNodeIndPtr=aNodeIndPtr+20;

        st = cpssTmAnodeStatusRead(dev, aNodeIndPtr, &statusPtr);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, aNodeIndPtr);

        aNodeIndPtr=aNodeIndPtr-20;

        /*
            1.3 Call with statusPtr [NULL], other parameters same as in 1.1.
            Expected: GT_BAD_PTR.
        */

        st = cpssTmAnodeStatusRead(dev, aNodeIndPtr, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, statusPtr = NULL", dev);

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
        st = cpssTmAnodeStatusRead(dev, aNodeIndPtr, &statusPtr);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssTmAnodeStatusRead(dev, aNodeIndPtr, &statusPtr);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}
/*----------------------------------------------------------------------------*/
/*
 * Configuration of cpssTmSched suit
 */
UTF_SUIT_BEGIN_TESTS_MAC(cpssTmNodesStatus)
    UTF_SUIT_DECLARE_TEST_MAC(cpssTmQueueStatusRead)
    UTF_SUIT_DECLARE_TEST_MAC(cpssTmPortStatusRead)
    UTF_SUIT_DECLARE_TEST_MAC(cpssTmCnodeStatusRead)
    UTF_SUIT_DECLARE_TEST_MAC(cpssTmBnodeStatusRead)
    UTF_SUIT_DECLARE_TEST_MAC(cpssTmAnodeStatusRead)
UTF_SUIT_END_TESTS_MAC(cpssTmNodesStatus)

