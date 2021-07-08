/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
* cpssTmNodesCreateUT.c
*
* DESCRIPTION:
*       Unit tests for cpssTmNodesCreate.
*
* FILE REVISION NUMBER:
*       $Revision: 2 $
*******************************************************************************/

#include <cpss/generic/tm/cpssTmCtl.h>
#include <cpss/generic/tm/cpssTmDrop.h>
#include <cpss/generic/tm/cpssTmSched.h>
#include <cpss/generic/tm/cpssTmShaping.h>
#include <cpss/generic/tm/cpssTmNodesCreate.h>
#include <cpss/generic/tm/cpssTmNodesTree.h>
#include <cpssCommon/private/prvCpssMath.h>
#include <cpss/generic/tm/cpssTmLogicalLayer.h>
#include <cpss/generic/tm/cpssTmUtDefs.h>
#include <utf/utfMain.h>
#include <utf/private/prvUtfExtras.h>




/*******************************************************************************
GT_STATUS cpssTmNamedPortCreate
(
    IN GT_U8                        devNum,
    IN GT_U32                       portInd,
    IN CPSS_TM_PORT_PARAMS_STC      *paramsPtr,
    IN GT_U32                       cNodesNum,
    IN GT_U32                       bNodesNum,
    IN GT_U32                       aNodesNum,
    IN GT_U32                       queuesNum
    IN GT_8_PTR                     portName
)
*/
UTF_TEST_CASE_MAC(cpssTmNamedPortCreate)
{
    GT_STATUS               st = GT_OK;

    GT_U8                        dev;
    GT_U32                       portInd = 1;
    CPSS_TM_PORT_PARAMS_STC      paramsPtr;
    GT_U32                       cNodesNum = 1;
    GT_U32                       bNodesNum = 1;
    GT_U32                       aNodesNum = 1;
    GT_U32                       queuesNum = 1;
    GT_8_PTR                     portName=(GT_8_PTR)"TestPort";
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
        paramsPtr.cbs = 8000;
        paramsPtr.ebs = 0x1FFFF;
        paramsPtr.eligiblePrioFuncId = CPSS_TM_ELIG_N_MIN_SHP_E;
        paramsPtr.dropProfileInd = CPSS_TM_NO_DROP_PROFILE_CNS;
        for(ii=0; ii<8; ii++){
            paramsPtr.schdModeArr[ii] = CPSS_TM_SCHD_MODE_RR_E;
            paramsPtr.quantumArr[ii]= UT_TM_MIN_PORT_QUANTUM_CNS;
        }

       /*
            1.0 Check with legal parameters and NULL port name.
            Expected: GT_BAD_VALUE.
        */
        st=cpssTmNamedPortCreate(dev, portInd, &paramsPtr, cNodesNum, bNodesNum, aNodesNum, queuesNum,NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_VALUE, st, dev);

        /*1.1
            Check with legal parameters and valid name.
            Expected: GT_OK
        */
        st = cpssTmNamedPortCreate(dev, portInd, &paramsPtr, cNodesNum, bNodesNum, aNodesNum, queuesNum,portName);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
        /*
            1.2 Check for duplicated name  and other params same as in 1.1
            Expected: GT_BAD_VALUE.
        */
        portInd = 2;
        st = cpssTmNamedPortCreate(dev, portInd, &paramsPtr, cNodesNum, bNodesNum, aNodesNum, queuesNum,portName);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_VALUE, st, dev);


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
        st = cpssTmNamedPortCreate(dev, portInd, &paramsPtr, cNodesNum, bNodesNum, aNodesNum, queuesNum, portName);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssTmNamedPortCreate(dev, portInd, &paramsPtr, cNodesNum, bNodesNum, aNodesNum, queuesNum,portName);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}


/*******************************************************************************
GT_STATUS cpssTmNamedAsymPortCreate
(
    IN GT_U8                        devNum,
    IN GT_U32                       portInd,
    IN CPSS_TM_PORT_PARAMS_STC      *paramsPtr
    IN GT_*_PTR                     portName
)
*/
UTF_TEST_CASE_MAC(cpssTmNamedAsymPortCreate)
{
    GT_STATUS               st = GT_OK;

    GT_U8                        dev;
    GT_U32                       portInd = 1;
    CPSS_TM_PORT_PARAMS_STC      paramsPtr;
    GT_8_PTR                     portName=(GT_8_PTR)"TestAsymPort";
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
        paramsPtr.cbs = 8000;
        paramsPtr.ebs = 0x1FFFF;
        paramsPtr.eligiblePrioFuncId = CPSS_TM_ELIG_N_MIN_SHP_E;
        paramsPtr.dropProfileInd = CPSS_TM_NO_DROP_PROFILE_CNS;
        for(ii=0; ii<8; ii++){
            paramsPtr.schdModeArr[ii] = CPSS_TM_SCHD_MODE_RR_E;
            paramsPtr.quantumArr[ii] = UT_TM_MIN_PORT_QUANTUM_CNS;
        }
        paramsPtr.numOfChildren = 1;

       /*1.0
            Check with legal parameters.
            Expected: GT_BAD_VALUE
        */

        st = cpssTmNamedAsymPortCreate(dev, portInd, &paramsPtr,NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_VALUE, st, dev);

        /*1.1
            Check with legal parameters.
            Expected: GT_OK
        */
        st = cpssTmNamedAsymPortCreate(dev, portInd, &paramsPtr,portName);
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
        st = cpssTmNamedAsymPortCreate(dev, portInd, &paramsPtr,portName);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssTmNamedAsymPortCreate(dev, portInd, &paramsPtr,portName);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}




/*******************************************************************************
GT_STATUS cpssTmNamedCnodeToPortCreate
(
    IN  GT_U8                           devNum,
    IN  GT_U32                          portInd,
    IN  CPSS_TM_C_NODE_PARAMS_STC       *cParamsPtr,
    OUT GT_U32                          *cNodeIndPtr
)
*/
UTF_TEST_CASE_MAC(cpssTmNamedCnodeToPortCreate)
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
    GT_8_PTR                     nodeName=(GT_8_PTR)"TestC-node";
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
        paramsPtr.cbs=8000;
        paramsPtr.ebs=0x1FFFF;
        paramsPtr.eligiblePrioFuncId = CPSS_TM_ELIG_N_MIN_SHP_E;
        for(ii=0; ii<8; ii++){
            paramsPtr.schdModeArr[ii] = CPSS_TM_SCHD_MODE_RR_E;
            paramsPtr.quantumArr[ii] = UT_TM_MIN_PORT_QUANTUM_CNS;
        }
        paramsPtr.dropProfileInd = CPSS_TM_NO_DROP_PROFILE_CNS;

        st = cpssTmPortCreate(dev, portInd, &paramsPtr, cNodesNum, bNodesNum, aNodesNum,queuesNum );
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* Configure parameters for node C */
        cParamsPtr.shapingProfilePtr= 0;
        cParamsPtr.quantum =0x40;
        cParamsPtr.dropCosMap =1;
        cParamsPtr.eligiblePrioFuncId =9;
        for (ii=0; ii<8; ii++){
            cParamsPtr.schdModeArr[ii] = CPSS_TM_SCHD_MODE_RR_E;
            cParamsPtr.dropProfileIndArr[ii] = 0;
        }

        /*1.0 Check with legal parameters and NULL node name
                Expected: GT_BAD_PARAM */

        st = cpssTmNamedCnodeToPortCreate(dev, portInd, &cParamsPtr, NULL, &cNodeIndPtr);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_VALUE, st, dev);

        /*1.1 Check with legal parameters.
                Expected: GT_OK */

        st = cpssTmNamedCnodeToPortCreate(dev, portInd, &cParamsPtr,nodeName, &cNodeIndPtr);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.2 check duplicated name - call again.
            Expected: GT_BAD_PARAM.
        */
        st = cpssTmNamedCnodeToPortCreate(dev, portInd, &cParamsPtr,nodeName, &cNodeIndPtr);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_VALUE, st, dev);

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
        st = cpssTmNamedCnodeToPortCreate(dev, portInd, &cParamsPtr,nodeName,  &cNodeIndPtr);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssTmNamedCnodeToPortCreate(dev, portInd, &cParamsPtr,nodeName,  &cNodeIndPtr);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}



/*******************************************************************************
GT_STATUS cpssTmQueueToAnodeCreate
(
    IN  GT_U8                           devNum,
    IN  GT_U32                          aNodeInd,
    IN  CPSS_TM_QUEUE_PARAMS_STC        *qParamsPtr,
    IN  GT_8_PTR                        queueName,
    OUT GT_U32                          *queueIndPtr
)
*/
UTF_TEST_CASE_MAC(cpssTmNamedQueueToAnodeCreate)
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
    GT_8_PTR                     queueName=(GT_8_PTR)"TestQueue";
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
        aParamsPtr.shapingProfilePtr=0;
        aParamsPtr.quantum= 0x40;
        for (ii=0; ii<8; ii++)
            aParamsPtr.schdModeArr[ii] = CPSS_TM_SCHD_MODE_RR_E; /* No DWRR configured*/
        aParamsPtr.dropProfileInd =0;
        aParamsPtr.eligiblePrioFuncId =8;

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
            cParamsPtr.dropProfileIndArr[ii]=0;
        }

        /* profile for Q level */
        qParamsPtr.shapingProfilePtr=0;
        qParamsPtr.quantum=0x40;
        qParamsPtr.dropProfileInd=0;
        qParamsPtr.eligiblePrioFuncId=1;

        /* Configure parameters for port */
        paramsPtr.cirBw=1000000;
        paramsPtr.eirBw=0;
        paramsPtr.cbs=8000;
        paramsPtr.ebs=0x1FFFF;
        paramsPtr.eligiblePrioFuncId= CPSS_TM_ELIG_N_MIN_SHP_E;
        for(ii=0; ii<8; ii++){
            paramsPtr.schdModeArr[ii] = CPSS_TM_SCHD_MODE_RR_E;
            paramsPtr.quantumArr[ii]= UT_TM_MIN_PORT_QUANTUM_CNS;
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

        /*1.0 Check with legal parameters and NULL name
                Expected: GT_BAD_VALUE */
        st = cpssTmNamedQueueToAnodeCreate(dev, aNodeIndPtr, &qParamsPtr, NULL, &queueIndPtr);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_VALUE, st, dev);
       /*1.1 Check with legal parameters.
                Expected: GT_OK */

        st = cpssTmNamedQueueToAnodeCreate(dev, aNodeIndPtr, &qParamsPtr, queueName,  &queueIndPtr);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
        /*
            1.2 check duplicated name - call again.
            Expected: GT_BAD_VALUE.
        */
        st = cpssTmNamedQueueToAnodeCreate(dev, aNodeIndPtr, &qParamsPtr, queueName, &queueIndPtr);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_VALUE, st, dev);

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
        st = cpssTmNamedQueueToAnodeCreate(dev, aNodeIndPtr, &qParamsPtr, queueName, &queueIndPtr);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssTmNamedQueueToAnodeCreate(dev, aNodeIndPtr, &qParamsPtr, queueName, &queueIndPtr);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}



/*******************************************************************************
GT_STATUS cpssTmNamedBnodeToCnodeCreate
(
    IN  GT_U8                           devNum,
    IN  GT_U32                          cNodeInd,
    IN  CPSS_TM_B_NODE_PARAMS_STC       *bParamsPtr,
    IN  GT_8_PTR                        nodeName,
    OUT GT_U32                          *bNodeIndPtr
)
*/
UTF_TEST_CASE_MAC(cpssTmNamedBnodeToCnodeCreate)
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
    GT_8_PTR                     nodeName=(GT_8_PTR)"TestB-Node";
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


        /* Configure parameters for port */
        paramsPtr.cirBw=1000000;
        paramsPtr.eirBw=0;
        paramsPtr.cbs=8000;
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

        st = cpssTmCnodeToPortCreate(dev, portInd, &cParamsPtr, &cNodeIndPtr);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*1.0 Check with legal parameters and NULL name
                Expected: GT_BAD_VALUE */

        st = cpssTmNamedBnodeToCnodeCreate(dev,cNodeIndPtr, &bParamsPtr, NULL, &bNodeIndPtr);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_VALUE, st, dev);
        /*1.1 Check with legal parameters.
                Expected: GT_OK */

        st = cpssTmNamedBnodeToCnodeCreate(dev,cNodeIndPtr, &bParamsPtr, nodeName, &bNodeIndPtr);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);


        /*1.2 check duplicated name
            Expected: GT_BAD_VALUE.
        */

        st = cpssTmNamedBnodeToCnodeCreate(dev,cNodeIndPtr, &bParamsPtr, nodeName, &bNodeIndPtr);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_VALUE, st, dev);

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
        st = cpssTmNamedBnodeToCnodeCreate(dev,cNodeIndPtr, &bParamsPtr, nodeName, &bNodeIndPtr);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssTmNamedBnodeToCnodeCreate(dev,cNodeIndPtr, &bParamsPtr, nodeName, &bNodeIndPtr);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}


/*******************************************************************************
GT_STATUS cpssTmNamedAnodeToBnodeCreate
(
    IN  GT_U8                           devNum,
    IN  GT_U32                          bNodeInd,
    IN  CPSS_TM_A_NODE_PARAMS_STC       *aParamsPtr,
    IN  GT_8_PTR                        nodeName,
    OUT GT_U32                          *aNodeIndPtr
)
*/
UTF_TEST_CASE_MAC(cpssTmNamedAnodeToBnodeCreate)
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
    GT_8_PTR                     nodeName=(GT_8_PTR)"TestA-node";
    CPSS_PP_FAMILY_TYPE_ENT devFamily;

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
        aParamsPtr.shapingProfilePtr=0;
        aParamsPtr.quantum= 0x40;
        for (ii=0; ii<8; ii++)
            aParamsPtr.schdModeArr[ii] = CPSS_TM_SCHD_MODE_RR_E; /* No DWRR configured*/
        aParamsPtr.dropProfileInd =0;
        aParamsPtr.eligiblePrioFuncId =8;

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

        /* Configure parameters for port */
        paramsPtr.cirBw=1000000;
        paramsPtr.eirBw=0;
        paramsPtr.cbs=8000;
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

         st = cpssTmBnodeToPortCreate(dev, portInd, &bParamsPtr, &cParamsPtr,&bNodeIndPtr, &cNodeIndPtr );
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);


        /*1.1 Check with legal parameters and NULL name.
                Expected: GT_BAD_VALUE */

        st = cpssTmNamedAnodeToBnodeCreate(dev,bNodeIndPtr, &aParamsPtr, NULL, &aNodeIndPtr);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_VALUE, st, dev);

        /*1.1 Check with legal parameters.
                Expected: GT_OK */

        st = cpssTmNamedAnodeToBnodeCreate(dev,bNodeIndPtr, &aParamsPtr, nodeName, &aNodeIndPtr);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
        /*1.2 Check with legal parameters - duplicated name
                Expected: GT_OK */
        st = cpssTmNamedAnodeToBnodeCreate(dev,bNodeIndPtr, &aParamsPtr,nodeName, &aNodeIndPtr);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_VALUE, st, dev);


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
        st = cpssTmNamedAnodeToBnodeCreate(dev,bNodeIndPtr, &aParamsPtr,nodeName, &aNodeIndPtr);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssTmNamedAnodeToBnodeCreate(dev,bNodeIndPtr, &aParamsPtr,nodeName, &aNodeIndPtr);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
 * Configuration of cpssTmDrop suit
 */
UTF_SUIT_BEGIN_TESTS_MAC(cpssTmLogicalLayer)
    UTF_SUIT_DECLARE_TEST_MAC(cpssTmNamedPortCreate)
    UTF_SUIT_DECLARE_TEST_MAC(cpssTmNamedAsymPortCreate)
    UTF_SUIT_DECLARE_TEST_MAC(cpssTmNamedCnodeToPortCreate)
    UTF_SUIT_DECLARE_TEST_MAC(cpssTmNamedQueueToAnodeCreate)
    UTF_SUIT_DECLARE_TEST_MAC(cpssTmNamedBnodeToCnodeCreate)
    UTF_SUIT_DECLARE_TEST_MAC(cpssTmNamedAnodeToBnodeCreate)
UTF_SUIT_END_TESTS_MAC(cpssTmLogicalLayer)
