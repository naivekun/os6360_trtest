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
* @file cpssTmEligPrioFuncUT.c
*
* @brief Unit tests for cpssTmEligPrioFunc.
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
#include <cpss/generic/tm/cpssTmNodesReorder.h>
#include <cpss/generic/tm/cpssTmEligPrioFunc.h>
#include <cpssCommon/private/prvCpssMath.h>
#include <cpss/generic/tm/cpssTmUtDefs.h>
#include <utf/utfMain.h>
#include <utf/private/prvUtfExtras.h>


/*******************************************************************************
GT_STATUS cpssTmEligPrioFuncQueueConfig
(
    IN GT_U8                        devNum,
    IN GT_U32                       eligPrioFunc,
    IN CPSS_TM_ELIG_PRIO_FUNC_STC   *funcOutArr
)
*/
UTF_TEST_CASE_MAC(cpssTmEligPrioFuncQueueConfig)
{
    GT_STATUS               st = GT_OK;

    GT_U8                        dev;
    GT_U32                       eligPrioFunc;
    CPSS_TM_ELIG_PRIO_FUNC_STC   funcOutArr;
    CPSS_PP_FAMILY_TYPE_ENT devFamily;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev,
                                 UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_LION_E |
                                     UTF_PUMA2_E | UTF_PUMA3_E | UTF_XCAT2_E | UTF_LION2_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E | UTF_ALDRIN_E | UTF_AC3X_E);


    cpssOsBzero((GT_VOID*)&funcOutArr, sizeof(funcOutArr));

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

        /*1.1 Check with legal parameters.
                Expected: GT_OK */

        eligPrioFunc= 45;
        funcOutArr.queueEligPrioFunc[0].maxTb=0;
        funcOutArr.queueEligPrioFunc[0].minTb=1;
        funcOutArr.queueEligPrioFunc[0].propPrio=1;
        funcOutArr.queueEligPrioFunc[0].schedPrio=1;
        funcOutArr.queueEligPrioFunc[0].elig=0;
        funcOutArr.queueEligPrioFunc[1].maxTb=0;
        funcOutArr.queueEligPrioFunc[1].minTb=1;
        funcOutArr.queueEligPrioFunc[1].propPrio=1;
        funcOutArr.queueEligPrioFunc[1].schedPrio=1;
        funcOutArr.queueEligPrioFunc[1].elig=1;
        funcOutArr.queueEligPrioFunc[2].maxTb=1;
        funcOutArr.queueEligPrioFunc[2].minTb=1;
        funcOutArr.queueEligPrioFunc[2].propPrio=1;
        funcOutArr.queueEligPrioFunc[2].schedPrio=1;
        funcOutArr.queueEligPrioFunc[2].elig=0;
        funcOutArr.queueEligPrioFunc[3].maxTb=0;
        funcOutArr.queueEligPrioFunc[3].minTb=1;
        funcOutArr.queueEligPrioFunc[3].propPrio=0;
        funcOutArr.queueEligPrioFunc[3].schedPrio=1;
        funcOutArr.queueEligPrioFunc[3].elig=1;

        st = cpssTmEligPrioFuncQueueConfig(dev, eligPrioFunc, &funcOutArr);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.2 check out of range eligPrioFunc .
            Call with eligPrioFunc [80], other parameters same as in 1.1.
            Expected: NON GT_OK.
        */

        eligPrioFunc = 80;

        st = cpssTmEligPrioFuncQueueConfig(dev, eligPrioFunc, &funcOutArr);
        UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        eligPrioFunc = 45;

         /*
            1.3 Call with funcOutArr [NULL], other parameters same as in 1.1.
            Expected: GT_BAD_PTR.
        */

        st = cpssTmEligPrioFuncQueueConfig(dev, eligPrioFunc, NULL);

        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, funcOutArr = NULL", dev);


        st = cpssTmClose(dev);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
    }


    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_NOT_APPLICABLE_DEVICE.            */
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    eligPrioFunc= 45;

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssTmEligPrioFuncQueueConfig(dev, eligPrioFunc, &funcOutArr);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssTmEligPrioFuncQueueConfig(dev, eligPrioFunc, &funcOutArr);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}


/*******************************************************************************
GT_STATUS cpssTmEligPrioFuncNodeConfig
(
    IN GT_U8                        devNum,
    IN GT_U32                       eligPrioFuncPtr,
    IN CPSS_TM_LEVEL_ENT            level,
    IN CPSS_TM_ELIG_PRIO_FUNC_STC   *funcOutArr
)
*/
UTF_TEST_CASE_MAC(cpssTmEligPrioFuncNodeConfig)
{
    GT_STATUS               st = GT_OK;

    GT_U8                        dev;
    GT_U32                       eligPrioFunc;
    CPSS_TM_ELIG_PRIO_FUNC_STC   funcOutArr;
    CPSS_TM_LEVEL_ENT            level = CPSS_TM_LEVEL_C_E;
    CPSS_PP_FAMILY_TYPE_ENT devFamily;
    GT_U32                  ii;

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

        /*1.1 Check with legal parameters.
                Expected: GT_OK */

        eligPrioFunc= 45;
        for (ii=0; ii<8; ii++) {
            funcOutArr.nodeEligPrioFunc[ii][0].maxTb=0;
            funcOutArr.nodeEligPrioFunc[ii][0].minTb=1;
            funcOutArr.nodeEligPrioFunc[ii][0].propPrio=1;
            funcOutArr.nodeEligPrioFunc[ii][0].schedPrio=1;
            funcOutArr.nodeEligPrioFunc[ii][0].elig=0;
            funcOutArr.nodeEligPrioFunc[ii][1].maxTb=0;
            funcOutArr.nodeEligPrioFunc[ii][1].minTb=1;
            funcOutArr.nodeEligPrioFunc[ii][1].propPrio=1;
            funcOutArr.nodeEligPrioFunc[ii][1].schedPrio=1;
            funcOutArr.nodeEligPrioFunc[ii][1].elig=1;
            funcOutArr.nodeEligPrioFunc[ii][2].maxTb=1;
            funcOutArr.nodeEligPrioFunc[ii][2].minTb=1;
            funcOutArr.nodeEligPrioFunc[ii][2].propPrio=1;
            funcOutArr.nodeEligPrioFunc[ii][2].schedPrio=1;
            funcOutArr.nodeEligPrioFunc[ii][2].elig=0;
            funcOutArr.nodeEligPrioFunc[ii][3].maxTb=0;
            funcOutArr.nodeEligPrioFunc[ii][3].minTb=1;
            funcOutArr.nodeEligPrioFunc[ii][3].propPrio=0;
            funcOutArr.nodeEligPrioFunc[ii][3].schedPrio=1;
            funcOutArr.nodeEligPrioFunc[ii][3].elig=1;
        }

        st = cpssTmEligPrioFuncNodeConfig(dev, eligPrioFunc, level, &funcOutArr);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.2 check out of range eligPrioFunc .
            Call with eligPrioFunc [80], other parameters same as in 1.1.
            Expected: NON GT_OK.
        */

        eligPrioFunc = 80;

        st = cpssTmEligPrioFuncNodeConfig(dev, eligPrioFunc, level, &funcOutArr);
        UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        eligPrioFunc = 45;

         /*
            1.3 Call with funcOutArr [NULL], other parameters same as in 1.1.
            Expected: GT_BAD_PTR.
        */

        st = cpssTmEligPrioFuncNodeConfig(dev, eligPrioFunc, level, NULL);

        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, funcOutArr = NULL", dev);

         /*
            1.4 Check for wrong enum values level and other params same as in 1.1
            Expected: GT_BAD_PARAM.
        */
        UTF_ENUMS_CHECK_MAC(cpssTmEligPrioFuncNodeConfig(dev, eligPrioFunc, level, &funcOutArr) ,level);

        st = cpssTmClose(dev);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
    }


    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_NOT_APPLICABLE_DEVICE.            */
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    eligPrioFunc= 45;

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssTmEligPrioFuncNodeConfig(dev, eligPrioFunc, level, &funcOutArr);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssTmEligPrioFuncNodeConfig(dev, eligPrioFunc, level, &funcOutArr);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}


/*******************************************************************************
GT_STATUS cpssTmEligPrioFuncConfigAllLevels
(
    IN GT_U8                        devNum,
    IN GT_U32                       eligPrioFuncPtr,
    IN CPSS_TM_ELIG_PRIO_FUNC_STC   *funcOutArr
)
*/
UTF_TEST_CASE_MAC(cpssTmEligPrioFuncConfigAllLevels)
{
    GT_STATUS               st = GT_OK;

    GT_U8                        dev;
    GT_U32                       eligPrioFunc;
    CPSS_TM_ELIG_PRIO_FUNC_STC   funcOutArr;
    CPSS_PP_FAMILY_TYPE_ENT devFamily;
    GT_U32          ii;

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

        /*1.1 Check with legal parameters.
                Expected: GT_OK */

        eligPrioFunc= 45;
        for (ii=0; ii<8; ii++) {
            funcOutArr.nodeEligPrioFunc[ii][0].maxTb=0;
            funcOutArr.nodeEligPrioFunc[ii][0].minTb=1;
            funcOutArr.nodeEligPrioFunc[ii][0].propPrio=1;
            funcOutArr.nodeEligPrioFunc[ii][0].schedPrio=1;
            funcOutArr.nodeEligPrioFunc[ii][0].elig=0;
            funcOutArr.nodeEligPrioFunc[ii][1].maxTb=0;
            funcOutArr.nodeEligPrioFunc[ii][1].minTb=1;
            funcOutArr.nodeEligPrioFunc[ii][1].propPrio=1;
            funcOutArr.nodeEligPrioFunc[ii][1].schedPrio=1;
            funcOutArr.nodeEligPrioFunc[ii][1].elig=1;
            funcOutArr.nodeEligPrioFunc[ii][2].maxTb=1;
            funcOutArr.nodeEligPrioFunc[ii][2].minTb=1;
            funcOutArr.nodeEligPrioFunc[ii][2].propPrio=1;
            funcOutArr.nodeEligPrioFunc[ii][2].schedPrio=1;
            funcOutArr.nodeEligPrioFunc[ii][2].elig=0;
            funcOutArr.nodeEligPrioFunc[ii][3].maxTb=0;
            funcOutArr.nodeEligPrioFunc[ii][3].minTb=1;
            funcOutArr.nodeEligPrioFunc[ii][3].propPrio=0;
            funcOutArr.nodeEligPrioFunc[ii][3].schedPrio=1;
            funcOutArr.nodeEligPrioFunc[ii][3].elig=1;
        }

        st = cpssTmEligPrioFuncConfigAllLevels(dev, eligPrioFunc, &funcOutArr);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.2 check out of range eligPrioFunc .
            Call with eligPrioFunc [80], other parameters same as in 1.1.
            Expected: NON GT_OK.
        */

        eligPrioFunc = 80;

        st = cpssTmEligPrioFuncConfigAllLevels(dev, eligPrioFunc, &funcOutArr);
        UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        eligPrioFunc = 45;

         /*
            1.3 Call with funcOutArr [NULL], other parameters same as in 1.1.
            Expected: GT_BAD_PTR.
        */

        st = cpssTmEligPrioFuncConfigAllLevels(dev, eligPrioFunc, NULL);

        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, funcOutArr = NULL", dev);


        st = cpssTmClose(dev);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
    }


    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_NOT_APPLICABLE_DEVICE.            */
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    eligPrioFunc= 45;

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssTmEligPrioFuncConfigAllLevels(dev, eligPrioFunc, &funcOutArr);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssTmEligPrioFuncConfigAllLevels(dev, eligPrioFunc, &funcOutArr);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
 * Configuration of cpssTmDrop suit
 */
UTF_SUIT_BEGIN_TESTS_MAC(cpssTmEligPrioFunc)
    UTF_SUIT_DECLARE_TEST_MAC(cpssTmEligPrioFuncQueueConfig)
    UTF_SUIT_DECLARE_TEST_MAC(cpssTmEligPrioFuncNodeConfig)
    UTF_SUIT_DECLARE_TEST_MAC(cpssTmEligPrioFuncConfigAllLevels)
UTF_SUIT_END_TESTS_MAC(cpssTmEligPrioFunc)

