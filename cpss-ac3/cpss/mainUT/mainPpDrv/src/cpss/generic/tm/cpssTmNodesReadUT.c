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
* @file cpssTmNodesReadUT.c
*
* @brief Unit tests for cpssTmNodesRead.
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
#include <cpssCommon/private/prvCpssMath.h>
#include <cpss/generic/tm/cpssTmNodesRead.h>
#include <cpss/generic/tm/cpssTmUtDefs.h>
#include <utf/utfMain.h>
#include <utf/private/prvUtfExtras.h>


/*----------------------------------------------------------------------------
GT_STATUS cpssTmQueueConfigurationRead
(
    IN  GT_U8                           devNum,
    IN  GT_U32                          index,
    OUT CPSS_TM_QUEUE_PARAMS_STC        *paramsPtr
)
*/
UTF_TEST_CASE_MAC(cpssTmQueueConfigurationRead)
{
    GT_STATUS                   st = GT_OK;
    GT_U8                       dev;
    GT_U32                      index;
    GT_U32                      queue_1_Index = 1;
    GT_U32                      queue_2_Index = 1;
    CPSS_TM_PORT_PARAMS_STC     paramsPtr;
    CPSS_TM_QUEUE_PARAMS_STC    QueueparamsPtr;
    CPSS_TM_QUEUE_PARAMS_STC    Queueparams;
    GT_U32                      ii;
    CPSS_PP_FAMILY_TYPE_ENT     devFamily;
    GT_U32                       cNodesPerPort;
    GT_U32                       bNodesPerCnode;
    GT_U32                       aNodesPerBnode;
    GT_U32                       queuesPerAnode;
    GT_U32                      queueDropProfile;
    CPSS_TM_DROP_PROFILE_PARAMS_STC profileStrPtr;
    GT_U32                      queue_3_Index = 1;
    GT_U32                      queue_4_Index = 1;
    GT_U32                      queue_5_Index = 1;
    GT_U32                      profile_1_Ptr;
    GT_U32                      profile_2_Ptr;
    GT_U32                      dummyProfile;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev,
                                 UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_LION_E |
                                     UTF_PUMA2_E | UTF_PUMA3_E | UTF_XCAT2_E | UTF_LION2_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E | UTF_ALDRIN_E | UTF_AC3X_E);

    cpssOsBzero((GT_VOID*)&profileStrPtr, sizeof (profileStrPtr));

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


        /* profile for Q level */
        Queueparams.shapingProfilePtr=0;
        Queueparams.quantum=0x40;
        Queueparams.dropProfileInd=0;
        Queueparams.eligiblePrioFuncId=1;


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

        cNodesPerPort  = 1;
        bNodesPerCnode = 1 ;
        aNodesPerBnode = 1;
        queuesPerAnode = 1;

        st = cpssTmPortCreate(dev, 1, &paramsPtr,
                                                    cNodesPerPort,
                                                    cNodesPerPort*bNodesPerCnode,
                                                    cNodesPerPort*bNodesPerCnode*aNodesPerBnode,
                                                    cNodesPerPort*bNodesPerCnode*aNodesPerBnode*queuesPerAnode );
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);


        queuesPerAnode = 8;
        st = cpssTmPortCreate(dev, 2, &paramsPtr,
                                                    cNodesPerPort,
                                                    cNodesPerPort*bNodesPerCnode,
                                                    cNodesPerPort*bNodesPerCnode*aNodesPerBnode,
                                                    cNodesPerPort*bNodesPerCnode*aNodesPerBnode*queuesPerAnode );
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        st = cpssTmTransQueueToPortCreate(dev,1,&Queueparams, &queue_1_Index);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);








        /*1.1 Check with legal parameters.
                Expected: GT_OK */

        st = cpssTmQueueConfigurationRead(dev, queue_1_Index, &QueueparamsPtr);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*1.1 Check read values.
                Expected: GT_OK */

        st=GT_OK;
        if (Queueparams.shapingProfilePtr!= QueueparamsPtr.shapingProfilePtr ||
            Queueparams.quantum != QueueparamsPtr.quantum ||
            Queueparams.dropProfileInd != QueueparamsPtr.dropProfileInd ||
            Queueparams.eligiblePrioFuncId != QueueparamsPtr.eligiblePrioFuncId)
            st= GT_FAIL;
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.2 check out of range index.
            Call with index [correctIndex+5], other parameters same as in 1.1.
            Expected: NON GT_OK.
        */

        index = queue_1_Index+5;

        st = cpssTmQueueConfigurationRead(dev, index, &QueueparamsPtr);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, index);

        /*
            1.3 Call with paramsPtr [NULL], other parameters same as in 1.1.
            Expected: GT_BAD_PTR.
        */

        st = cpssTmQueueConfigurationRead(dev, queue_1_Index, NULL);

        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, paramsPtr = NULL", dev);


        /* test for cpssTmNodeDropProfileIndexRead */
        /* create queue with non default drop profile */

        profileStrPtr.dropMode = CPSS_TM_DROP_MODE_CB_TD_ONLY_E;
        profileStrPtr.cbTdThresholdBytes = cpssTmDropConvertBWToBytes(200000);

        st = cpssTmDropProfileCreate(dev, CPSS_TM_LEVEL_Q_E, 0, &profileStrPtr, &profile_1_Ptr);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        Queueparams.dropProfileInd=profile_1_Ptr;


        st = cpssTmTransQueueToPortCreate(dev,2,&Queueparams, &queue_2_Index);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        st = cpssTmTransQueueToPortCreate(dev,2,&Queueparams, &queue_3_Index);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);


        st = cpssTmTransQueueToPortCreate(dev,2,&Queueparams, &queue_4_Index);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);



        st = cpssTmDropProfileCreate(dev, CPSS_TM_LEVEL_Q_E, 0, &profileStrPtr, &profile_2_Ptr);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        Queueparams.dropProfileInd=profile_2_Ptr;

        st = cpssTmTransQueueToPortCreate(dev,2,&Queueparams, &queue_5_Index);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        st = cpssTmNodeDropProfileIndexRead(dev, CPSS_TM_LEVEL_Q_E, 0, queue_4_Index, &queueDropProfile);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        st=GT_OK;
        if (queueDropProfile != profile_1_Ptr) st= GT_FAIL;
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        st = cpssTmNodeDropProfileIndexRead(dev, CPSS_TM_LEVEL_Q_E, 7 ,  queue_5_Index, &queueDropProfile);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        st=GT_OK;
        if (queueDropProfile != profile_2_Ptr) st= GT_FAIL;
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        st = cpssTmNodeDropProfileIndexRead(dev, CPSS_TM_LEVEL_A_E, 7 ,  0, &dummyProfile);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        st=GT_OK;
        if (dummyProfile != 0) st= GT_FAIL;
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.2 check out of range index.
            Call with index [correctIndex+5], other parameters same as in 1.1.
            Expected: NON GT_OK.
        */
        st = cpssTmNodeDropProfileIndexRead(dev, CPSS_TM_LEVEL_Q_E, 0, index, &queueDropProfile);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, index);


        /*
            1.3 check out of range level.
            Call with level > PORT level, other parameters same as in 1.1.
            Expected: NON GT_OK.
        */
        st = cpssTmNodeDropProfileIndexRead(dev, CPSS_TM_LEVEL_P_E + 1 , 0, index, &queueDropProfile);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, index);

        /*
            1.4 check out of range cos (for C levels )
            Call with level > PORT level, other parameters same as in 1.1.
            Expected: NON GT_OK.
        */
        st = cpssTmNodeDropProfileIndexRead(dev, CPSS_TM_LEVEL_C_E , 11, index, &queueDropProfile);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, index);

        /*
            1.4 Call with paramsPtr [NULL], other parameters same as in 1.1.
            Expected: GT_BAD_PTR.
        */
        st = cpssTmNodeDropProfileIndexRead(dev,  CPSS_TM_LEVEL_Q_E, 0, index,  NULL);
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
        st = cpssTmQueueConfigurationRead(dev, queue_1_Index, &QueueparamsPtr);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
        st = cpssTmNodeDropProfileIndexRead(dev, CPSS_TM_LEVEL_Q_E, 0, queue_2_Index, &queueDropProfile);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssTmQueueConfigurationRead(dev, queue_1_Index, &QueueparamsPtr);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
    st = cpssTmNodeDropProfileIndexRead(dev,  CPSS_TM_LEVEL_Q_E, 0, queue_2_Index, &queueDropProfile);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------
GT_STATUS cpssTmAnodeConfigurationRead
(
    IN  GT_U8                       devNum,
    IN  GT_U32                      index,
    OUT CPSS_TM_A_NODE_PARAMS_STC   *paramsPtr
)
*/
UTF_TEST_CASE_MAC(cpssTmAnodeConfigurationRead)
{
    GT_STATUS                   st = GT_OK;
    GT_U8                       dev;
    GT_U32                      portInd;
    CPSS_TM_A_NODE_PARAMS_STC   aParamsPtr;
    CPSS_TM_A_NODE_PARAMS_STC   aParams;
    CPSS_TM_B_NODE_PARAMS_STC   bParamsPtr;
    CPSS_TM_C_NODE_PARAMS_STC   cParamsPtr;
    GT_U32                      aNodeIndPtr = 1;
    GT_U32                      bNodeIndPtr;
    GT_U32                      cNodeIndPtr;
    CPSS_TM_PORT_PARAMS_STC     paramsPtr;
    CPSS_PP_FAMILY_TYPE_ENT     devFamily;
    GT_U32                      ii;
    GT_U32                      index;
    GT_U32                       cNodesPerPort;
    GT_U32                       bNodesPerCnode;
    GT_U32                       aNodesPerBnode;
    GT_U32                       queuesPerAnode;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev,
                                 UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_LION_E |
                                     UTF_PUMA2_E | UTF_PUMA3_E | UTF_XCAT2_E | UTF_LION2_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E | UTF_ALDRIN_E | UTF_AC3X_E);

    cpssOsBzero((GT_VOID*)&aParamsPtr, sizeof(aParamsPtr));
    cpssOsBzero((GT_VOID*)&bParamsPtr, sizeof(bParamsPtr));
    cpssOsBzero((GT_VOID*)&cParamsPtr, sizeof(cParamsPtr));
    cpssOsBzero((GT_VOID*)&paramsPtr, sizeof(paramsPtr));
    cpssOsBzero((GT_VOID*)&aParams, sizeof(aParams));

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
        aParams.shapingProfilePtr = 0;
        aParams.quantum = 0x40;
        for (ii=0; ii<8; ii++)
            aParams.schdModeArr[ii] = CPSS_TM_SCHD_MODE_RR_E; /* No DWRR configured*/
        aParams.dropProfileInd = 0;
        aParams.eligiblePrioFuncId = 8;
        aParams.numOfChildren = 5;

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

        portInd = 1;

        cNodesPerPort  = 1;
        bNodesPerCnode = 1 ;
        aNodesPerBnode = 3;
        queuesPerAnode = 7;

        st = cpssTmPortCreate(dev, portInd, &paramsPtr,
                                                    cNodesPerPort,
                                                    cNodesPerPort*bNodesPerCnode,
                                                    cNodesPerPort*bNodesPerCnode*aNodesPerBnode,
                                                    cNodesPerPort*bNodesPerCnode*aNodesPerBnode*queuesPerAnode );
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);


        st = cpssTmAnodeToPortCreate(dev, portInd, &aParams, &bParamsPtr, &cParamsPtr, &aNodeIndPtr, &bNodeIndPtr, &cNodeIndPtr );
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*1.1 Check with legal parameters.
                Expected: GT_OK */


        st = cpssTmAnodeConfigurationRead(dev, aNodeIndPtr, &aParamsPtr);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*1.1.1 Check read values.
                Expected: GT_OK */

        st=GT_OK;
        if (aParams.shapingProfilePtr!= aParamsPtr.shapingProfilePtr ||
            aParams.quantum != aParamsPtr.quantum ||
            aParams.dropProfileInd != aParamsPtr.dropProfileInd ||
            aParams.eligiblePrioFuncId != aParamsPtr.eligiblePrioFuncId  ||
/*            aParams.numOfChildren != aParamsPtr.numOfChildren) */
            /* for symmetric tree  num_of_childeren is overwritten during port creation */
            queuesPerAnode != aParamsPtr.numOfChildren)
            st= GT_FAIL;
        else
        {
            for (ii=0; ii<8; ii++)
            {
                if (aParams.schdModeArr[ii] != aParamsPtr.schdModeArr[ii])
                    st= GT_FAIL;
            }
        }

        /*
            1.2 check out of range index.
            Call with index [aNodeIndPtr+10], other parameters same as in 1.1.
            Expected: NON GT_OK.
        */

        index = aNodeIndPtr+10;

        st = cpssTmAnodeConfigurationRead(dev, index, &aParamsPtr);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, index);

        /*
            1.3 Call with paramsPtr [NULL], other parameters same as in 1.1.
            Expected: GT_BAD_PTR.
        */

        st = cpssTmAnodeConfigurationRead(dev, aNodeIndPtr, NULL);

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
        st = cpssTmAnodeConfigurationRead(dev, aNodeIndPtr, &aParamsPtr);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssTmAnodeConfigurationRead(dev, aNodeIndPtr, &aParamsPtr);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}
/*----------------------------------------------------------------------------
GT_STATUS cpssTmBnodeConfigurationRead
(
    IN  GT_U8                       devNum,
    IN  GT_U32                      index,
    OUT CPSS_TM_B_NODE_PARAMS_STC   *paramsPtr
)
*/
UTF_TEST_CASE_MAC(cpssTmBnodeConfigurationRead)
{
    GT_STATUS               st = GT_OK;
    GT_U8                           dev;
    GT_U32                          portInd;
/*    CPSS_TM_A_NODE_PARAMS_STC       aParamsPtr; */
    CPSS_TM_B_NODE_PARAMS_STC       bParamsPtr;
    CPSS_TM_B_NODE_PARAMS_STC       bParams;
    CPSS_TM_C_NODE_PARAMS_STC       cParamsPtr;
    GT_U32                          bNodeIndPtr = 1;
    GT_U32                          cNodeIndPtr;
    CPSS_TM_PORT_PARAMS_STC      paramsPtr;

    GT_U32                       cNodesPerPort;
    GT_U32                       bNodesPerCnode;
    GT_U32                       aNodesPerBnode;
    GT_U32                       queuesPerAnode;
    CPSS_PP_FAMILY_TYPE_ENT devFamily;
    GT_U32                       ii;
    GT_U32                       index;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev,
                                 UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_LION_E |
                                     UTF_PUMA2_E | UTF_PUMA3_E | UTF_XCAT2_E | UTF_LION2_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E | UTF_ALDRIN_E | UTF_AC3X_E);

    cpssOsBzero((GT_VOID*)&bParams,    sizeof(bParams));
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
        bParams.shapingProfilePtr =0;
        bParams.quantum = 0x40;
        for (ii=0; ii<8; ii++)
            bParams.schdModeArr[ii] = CPSS_TM_SCHD_MODE_RR_E; /* No DWRR configured*/
        bParams.dropProfileInd = 0;
        bParams.eligiblePrioFuncId = 9;
        bParams.numOfChildren = 4;

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
        cNodesPerPort  = 1;
        bNodesPerCnode = 1 ;
        aNodesPerBnode = 3;
        queuesPerAnode = 7;

        st = cpssTmPortCreate(dev, portInd, &paramsPtr,
                                                    cNodesPerPort,
                                                    cNodesPerPort*bNodesPerCnode,
                                                    cNodesPerPort*bNodesPerCnode*aNodesPerBnode,
                                                    cNodesPerPort*bNodesPerCnode*aNodesPerBnode*queuesPerAnode );
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);


        st = cpssTmBnodeToPortCreate(dev, portInd, &bParams, &cParamsPtr,&bNodeIndPtr, &cNodeIndPtr );
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*1.1 Check with legal parameters.
                Expected: GT_OK */

        st = cpssTmBnodeConfigurationRead(dev, bNodeIndPtr, &bParamsPtr);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*1.1.1 Check read values.
                Expected: GT_OK */

        st=GT_OK;
        if (bParams.shapingProfilePtr!= bParamsPtr.shapingProfilePtr ||
            bParams.quantum != bParamsPtr.quantum ||
            bParams.dropProfileInd != bParamsPtr.dropProfileInd ||
            bParams.eligiblePrioFuncId != bParamsPtr.eligiblePrioFuncId  ||
/*            bParams.numOfChildren != bParamsPtr.numOfChildren) */
            /* for symmetric tree  num_of_childeren is overwritten during port creation */
            aNodesPerBnode != bParamsPtr.numOfChildren)
            st= GT_FAIL;
        else
        {
            for (ii=0; ii<8; ii++)
            {
                if (bParams.schdModeArr[ii] != bParamsPtr.schdModeArr[ii])
                    st= GT_FAIL;
            }
        }


        /*
            1.2 check out of range index.
            Call with index [bNodeIndPtr+10], other parameters same as in 1.1.
            Expected: NON GT_OK.
        */

        index = bNodeIndPtr+10;

        st = cpssTmBnodeConfigurationRead(dev, index, &bParamsPtr);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, index);

        /*
            1.3 Call with paramsPtr [NULL], other parameters same as in 1.1.
            Expected: GT_BAD_PTR.
        */

        st = cpssTmBnodeConfigurationRead(dev, bNodeIndPtr, NULL);

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
        st = cpssTmBnodeConfigurationRead(dev, bNodeIndPtr, &bParamsPtr);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssTmBnodeConfigurationRead(dev, bNodeIndPtr, &bParamsPtr);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}
/*----------------------------------------------------------------------------
GT_STATUS cpssTmCnodeConfigurationRead
(
    IN  GT_U8                       devNum,
    IN  GT_U32                      index,
    OUT CPSS_TM_C_NODE_PARAMS_STC   *paramsPtr
)
*/
UTF_TEST_CASE_MAC(cpssTmCnodeConfigurationRead)
{
    GT_STATUS               st = GT_OK;

    GT_U8                        dev;
    GT_U32                       portInd;
    CPSS_TM_PORT_PARAMS_STC      paramsPtr;
    CPSS_TM_C_NODE_PARAMS_STC    cParamsPtr;
    CPSS_TM_C_NODE_PARAMS_STC    cParams;
    GT_U32                       cNodeIndPtr = 1;
    CPSS_PP_FAMILY_TYPE_ENT devFamily;
    GT_U32                       ii;
    GT_U32                       index;
    GT_U32                       cNodesPerPort;
    GT_U32                       bNodesPerCnode;
    GT_U32                       aNodesPerBnode;
    GT_U32                       queuesPerAnode;

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

        portInd = 1;
        cNodesPerPort  = 1;
        bNodesPerCnode = 5 ;
        aNodesPerBnode = 3;
        queuesPerAnode = 1;
        st = cpssTmPortCreate(dev, portInd, &paramsPtr,
                                                    cNodesPerPort,
                                                    cNodesPerPort*bNodesPerCnode,
                                                    cNodesPerPort*bNodesPerCnode*aNodesPerBnode,
                                                    cNodesPerPort*bNodesPerCnode*aNodesPerBnode*queuesPerAnode );
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* Configure parameters for node C */
        cParams.shapingProfilePtr = 0;
        cParams.quantum = 0x40;
        cParams.dropCosMap = 1;
        cParams.eligiblePrioFuncId = 9;
        for (ii=0; ii<8; ii++){
            cParams.schdModeArr[ii] = CPSS_TM_SCHD_MODE_RR_E;
            cParams.dropProfileIndArr[ii] = 0;
        }
        cParams.numOfChildren = 3;

        st = cpssTmCnodeToPortCreate(dev, portInd, &cParams, &cNodeIndPtr);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*1.1 Check with legal parameters.
                Expected: GT_OK */

        st = cpssTmCnodeConfigurationRead(dev, cNodeIndPtr, &cParamsPtr);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*1.1.1 Check read values.
                Expected: GT_OK */

        st=GT_OK;
        if (cParams.shapingProfilePtr!= cParamsPtr.shapingProfilePtr ||
            cParams.quantum != cParamsPtr.quantum ||
            cParams.dropCosMap != cParamsPtr.dropCosMap ||
            cParams.eligiblePrioFuncId != cParamsPtr.eligiblePrioFuncId ||
 /*           cParams.numOfChildren != cParamsPtr.numOfChildren ) */
            /* for symmetric tree  num_of_childeren is overwritten during port creation */
            bNodesPerCnode != cParamsPtr.numOfChildren )
            st= GT_FAIL;
        else
        {
            for (ii=0; ii<8; ii++)
            {
                if (cParams.schdModeArr[ii] != cParamsPtr.schdModeArr[ii] ||
                    cParams.dropProfileIndArr[ii] != cParamsPtr.dropProfileIndArr[ii])
                    st= GT_FAIL;
            }
        }


        /*
            1.2 check out of range index.
            Call with index [cNodeIndPtr+10], other parameters same as in 1.1.
            Expected: NON GT_OK.
        */

        index = cNodeIndPtr+10;

        st = cpssTmCnodeConfigurationRead(dev, index, &cParamsPtr);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, index);

        /*
            1.3 Call with paramsPtr [NULL], other parameters same as in 1.1.
            Expected: GT_BAD_PTR.
        */

        st = cpssTmCnodeConfigurationRead(dev, cNodeIndPtr, NULL);

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
        st = cpssTmCnodeConfigurationRead(dev, cNodeIndPtr, &cParamsPtr);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssTmCnodeConfigurationRead(dev, cNodeIndPtr, &cParamsPtr);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------
GT_STATUS cpssTmPortConfigurationRead
(
    IN  GT_U8                           devNum,
    IN  GT_U32                          index,
    OUT CPSS_TM_PORT_PARAMS_STC         *paramsPtr
)
*/
UTF_TEST_CASE_MAC(cpssTmPortConfigurationRead)
{
    GT_STATUS               st = GT_OK;

    GT_U8                         dev;
    GT_U32                        index = 1;
    CPSS_TM_PORT_PARAMS_STC       paramsPtr;
    CPSS_TM_PORT_PARAMS_STC       params;
    CPSS_TM_PORT_DROP_PER_COS_STC cosParams;
    CPSS_TM_PORT_DROP_PER_COS_STC cosPrms;
    GT_U32                        ii;
    CPSS_PP_FAMILY_TYPE_ENT       devFamily;
    GT_U32                       cNodesPerPort;
    GT_U32                       bNodesPerCnode;
    GT_U32                       aNodesPerBnode;
    GT_U32                       queuesPerAnode;

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
        params.cirBw = 1000000;
        params.eirBw = 0;
        params.cbs = 8; /*KBytes*/
        params.ebs = 0x1FFFF;
        params.eligiblePrioFuncId = CPSS_TM_ELIG_N_MIN_SHP_E;
        for(ii=0; ii<8; ii++){
            params.schdModeArr[ii] = CPSS_TM_SCHD_MODE_RR_E;
            params.quantumArr[ii] = UT_TM_MIN_PORT_QUANTUM_CNS;
        }
        params.dropProfileInd = CPSS_TM_NO_DROP_PROFILE_CNS;
        params.numOfChildren = 1;

        index = 1;
        cNodesPerPort  = 4;
        bNodesPerCnode = 1 ;
        aNodesPerBnode = 1;
        queuesPerAnode = 1;
        st = cpssTmPortCreate(dev, index, &params,
                                                    cNodesPerPort,
                                                    cNodesPerPort*bNodesPerCnode,
                                                    cNodesPerPort*bNodesPerCnode*aNodesPerBnode,
                                                    cNodesPerPort*bNodesPerCnode*aNodesPerBnode*queuesPerAnode );
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        cosParams.dropCosMap = 0x4; /* for Cos=2 */
        for(ii=0; ii<8; ii++)
            cosParams.dropProfileIndArr[ii] = CPSS_TM_NO_DROP_PROFILE_CNS;

        st = cpssTmPortDropPerCosConfig(dev, index, &cosParams);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*1.1 Check with legal parameters.
                Expected: GT_OK */

        index = 1;

        st = cpssTmPortConfigurationRead(dev, index, &paramsPtr, &cosPrms);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*1.1.1 Check read values.
                Expected: GT_OK */

        st = GT_OK;
        if (params.cirBw!= paramsPtr.cirBw ||
            params.eirBw != paramsPtr.eirBw ||
            /* calculated approximately!*/
            params.cbs != paramsPtr.cbs ||
            params.ebs != paramsPtr.ebs||
            params.eligiblePrioFuncId != paramsPtr.eligiblePrioFuncId ||
/*           params.numOfChildren != paramsPtr.numOfChildren || */
            cNodesPerPort != paramsPtr.numOfChildren ||
            params.dropProfileInd != paramsPtr.dropProfileInd ||
            cosParams.dropCosMap != cosPrms.dropCosMap)
            st = GT_FAIL;
        else
        {
            for (ii=0; ii<8; ii++)
            {
                if (params.schdModeArr[ii] != paramsPtr.schdModeArr[ii] ||
                    cosPrms.dropProfileIndArr[ii] != cosParams.dropProfileIndArr[ii]
#ifndef ASIC_SIMULATION
                /* currently simulator have troubles reading HiWord  of register , so testing full register value disabled here */
                ||params.quantumArr[ii] != paramsPtr.quantumArr[ii]
#endif /*ASIC_SIMULATION*/
                )
                    st = GT_FAIL;
            }
        }
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.2 check out of range index.
            Call with index [6], other parameters same as in 1.1.
            Expected: NON GT_OK.
        */

        index = 6;

        st = cpssTmPortConfigurationRead(dev, index, &paramsPtr, &cosPrms);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, index);

        index = 1;
        /*
            1.3 Call with paramsPtr [NULL], other parameters same as in 1.1.
            Expected: GT_BAD_PTR.
        */

        st = cpssTmPortConfigurationRead(dev, index, NULL, &cosPrms);
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
        st = cpssTmPortConfigurationRead(dev, index, &paramsPtr, &cosPrms);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssTmPortConfigurationRead(dev, index, &paramsPtr, &cosPrms);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}


/*----------------------------------------------------------------------------*/
/*
 * Configuration of cpssTmSched suit
 */
UTF_SUIT_BEGIN_TESTS_MAC(cpssTmNodesRead)
    UTF_SUIT_DECLARE_TEST_MAC(cpssTmQueueConfigurationRead)
    UTF_SUIT_DECLARE_TEST_MAC(cpssTmAnodeConfigurationRead)
    UTF_SUIT_DECLARE_TEST_MAC(cpssTmBnodeConfigurationRead)
    UTF_SUIT_DECLARE_TEST_MAC(cpssTmCnodeConfigurationRead)
    UTF_SUIT_DECLARE_TEST_MAC(cpssTmPortConfigurationRead)
UTF_SUIT_END_TESTS_MAC(cpssTmNodesRead)

