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
* @file cpssTmNodesReorderUT.c
*
* @brief Unit tests for cpssTmNodesReorder.
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
#include <cpss/generic/tm/cpssTmNodesReorder.h>
#include <cpssCommon/private/prvCpssMath.h>
#include <cpss/generic/tm/cpssTmUtDefs.h>
#include <utf/utfMain.h>
#include <utf/private/prvUtfExtras.h>




/*******************************************************************************
GT_STATUS cpssTmNodesSwitch
(
    IN GT_U8                        devNum,
    IN CPSS_TM_LEVEL_ENT            level,
    IN GT_U32                       nodeAIndex,
    IN GT_U32                       nodeBIndex
)
*/
UTF_TEST_CASE_MAC(cpssTmNodesSwitch)
{
    GT_STATUS               st = GT_OK;

    GT_U8                        dev;
    GT_U32                       portInd;
    CPSS_TM_LEVEL_ENT            level;
    CPSS_TM_PORT_PARAMS_STC      paramsPtr;
    GT_U32                       cNodesNum;
    GT_U32                       bNodesNum;
    GT_U32                       aNodesNum;
    GT_U32                       queuesNum;
    CPSS_TM_C_NODE_PARAMS_STC    cParamsPtr;
    GT_U32                       cNodeIndPtr;
    GT_U32                       cNodeIndPtr1 = 1;
    GT_U32                       cNodeIndPtr2 = 2;
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
        cNodesNum=2;
        bNodesNum=2;
        aNodesNum=2;
        queuesNum=2;

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

        /* Configure parameters for node C */
        cParamsPtr.shapingProfilePtr = 0;
        cParamsPtr.quantum = 0x40;
        cParamsPtr.dropCosMap = 1;
        cParamsPtr.eligiblePrioFuncId = 9;
        for (ii=0; ii<8; ii++){
            cParamsPtr.schdModeArr[ii] = CPSS_TM_SCHD_MODE_RR_E;
            cParamsPtr.dropProfileIndArr[ii] = 0;
        }


        st = cpssTmCnodeToPortCreate(dev, portInd, &cParamsPtr, &cNodeIndPtr1);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        st = cpssTmCnodeToPortCreate(dev, portInd, &cParamsPtr, &cNodeIndPtr2);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);


        /*1.1 Check with legal parameters.
                Expected: GT_OK */

        level= CPSS_TM_LEVEL_C_E;

        st = cpssTmNodesSwitch(dev, level, cNodeIndPtr1, cNodeIndPtr2);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.2 check out of range cNodeIndPtr (from).
            Call with cNodeIndPtr [85580], other parameters same as in 1.1.
            Expected: NON GT_OK.
        */

        cNodeIndPtr = 514;

        st = cpssTmNodesSwitch(dev, level, cNodeIndPtr, cNodeIndPtr2);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, cNodeIndPtr);

        /*
            1.3 check out of range cNodeIndPtr (10).
            Call with cNodeIndPtr [85580], other parameters same as in 1.1.
            Expected: NON GT_OK.
        */


        st = cpssTmNodesSwitch(dev, level, cNodeIndPtr1, cNodeIndPtr);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, cNodeIndPtr);


        /*
            1.4 Check for wrong enum values level and other params same as in 1.1
            Expected: GT_BAD_PARAM.
        */
        UTF_ENUMS_CHECK_MAC(cpssTmNodesSwitch
                            (dev, level, cNodeIndPtr, cNodeIndPtr2) ,level);


        st = cpssTmClose(dev);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
    }


    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_NOT_APPLICABLE_DEVICE.            */
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    level= CPSS_TM_LEVEL_C_E;

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssTmNodesSwitch(dev, level, cNodeIndPtr1, cNodeIndPtr2);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssTmNodesSwitch(dev, level, cNodeIndPtr1, cNodeIndPtr2);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}




/*----------------------------------------------------------------------------*/
/*
 * Configuration of cpssTmDrop suit
 */
UTF_SUIT_BEGIN_TESTS_MAC(cpssTmNodesReorder)
    UTF_SUIT_DECLARE_TEST_MAC(cpssTmNodesSwitch)
UTF_SUIT_END_TESTS_MAC(cpssTmNodesReorder)

