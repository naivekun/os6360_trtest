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
* @file cpssTmInternalUT.c
*
* @brief Unit tests for internal TM/RM
*
* @version   1
********************************************************************************
*/

#include <cpss/common/cpssTypes.h>
#include <cpss/generic/tm/cpssTmCtl.h>
#include <cpss/generic/tm/prvCpssTmCtl.h>
#include <cpss/generic/tm/cpssTmUtDefs.h>
#include <utf/utfMain.h>
#include <utf/private/prvUtfExtras.h>
#include <appDemo/sysHwConfig/gtAppDemoTmConfig.h>
#include <platform/tm_regs.h>

int tm_rm_ut_chunk_list(GT_U8 devNum, int num_of_iterations);
int tm_drop_profiles_ut(GT_U8 devNum, int num_of_iterations);
int tm_aging_drop_profiles_ut(GT_U8 devNum, int num_of_iterations);
int tm_mixed_drop_profiles_ut(GT_U8 devNum, int num_of_iterations);

/*******************************************************************************
int rm_chunk_list_ut
(
    IN GT_U8    devNum,
    IN uint32_t num_of_iterations
)
*/
UTF_TEST_CASE_MAC(rm_chunk_list)
{
    GT_STATUS               st = GT_OK;
    GT_U8                   dev;
    CPSS_PP_FAMILY_TYPE_ENT devFamily;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev,
                                 UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_LION_E |
                                 UTF_PUMA2_E | UTF_PUMA3_E | UTF_XCAT2_E | UTF_LION2_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E | UTF_ALDRIN_E | UTF_AC3X_E);

    /* test stuck CPU - need to fix it in future
    UTF_HW_CRASHED_TEST_BYPASS_MAC(UTF_CAELUM_E);*/

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* get device family */
        st = prvUtfDeviceFamilyGet(dev, &devFamily);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "prvUtfDeviceFamilyGet: %d", dev);

        st = tm_rm_ut_chunk_list(dev, 6);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
    }
}

/*
int tm_drop_profiles_ut
(
    IN GT_U8    devNum,
    IN uint32_t num_of_iterations
)
*/
UTF_TEST_CASE_MAC(tm_drop_profiles)
{
    GT_STATUS               st = GT_OK;
    GT_U8                   dev;
    CPSS_PP_FAMILY_TYPE_ENT devFamily;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev,
                                 UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_LION_E |
                                 UTF_PUMA2_E | UTF_PUMA3_E | UTF_XCAT2_E | UTF_LION2_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E | UTF_ALDRIN_E | UTF_AC3X_E);

    /* test stuck CPU - need to fix it in future
    UTF_HW_CRASHED_TEST_BYPASS_MAC(UTF_CAELUM_E); */

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* get device family */
        st = prvUtfDeviceFamilyGet(dev, &devFamily);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "prvUtfDeviceFamilyGet: %d", dev);

        st = tm_drop_profiles_ut(dev, 2);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
    }
}

/*
int tm_aging_drop_profiles_ut
(
    IN GT_U8    devNum,
    IN uint32_t num_of_iterations
)
*/
UTF_TEST_CASE_MAC(tm_aging_drop_profiles)
{
    GT_STATUS               st = GT_OK;
    GT_U8                   dev;
    CPSS_PP_FAMILY_TYPE_ENT devFamily;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev,
                                 UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_LION_E |
                                 UTF_PUMA2_E | UTF_PUMA3_E | UTF_XCAT2_E | UTF_LION2_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E | UTF_ALDRIN_E | UTF_AC3X_E);

    /* test stuck CPU - need to fix it in future
    UTF_HW_CRASHED_TEST_BYPASS_MAC(UTF_CAELUM_E); */

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* get device family */
        st = prvUtfDeviceFamilyGet(dev, &devFamily);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "prvUtfDeviceFamilyGet: %d", dev);

        st = tm_aging_drop_profiles_ut(dev, 2);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
    }
}

 /*
int tm_mixed_drop_profiles_ut
(
    IN GT_U8    devNum,
    IN uint32_t num_of_iterations
)
*/
UTF_TEST_CASE_MAC(tm_mixed_drop_profiles)
{
    GT_STATUS               st = GT_OK;
    GT_U8                   dev;
    CPSS_PP_FAMILY_TYPE_ENT devFamily;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev,
                                 UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_LION_E |
                                 UTF_PUMA2_E | UTF_PUMA3_E | UTF_XCAT2_E | UTF_LION2_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E | UTF_ALDRIN_E | UTF_AC3X_E);

    /* test stuck CPU - need to fix it in future
    UTF_HW_CRASHED_TEST_BYPASS_MAC(UTF_CAELUM_E); */

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* get device family */
        st = prvUtfDeviceFamilyGet(dev, &devFamily);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "prvUtfDeviceFamilyGet: %d", dev);

        st = tm_mixed_drop_profiles_ut(dev, 2);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
    }
}

typedef GT_STATUS (appDemoTmInitFunc)(GT_U8 dev);


UTF_TEST_CASE_MAC(appDemo_tm_scenarios)
{
    GT_STATUS               st = GT_OK;
    GT_U8                   dev;
    CPSS_PP_FAMILY_TYPE_ENT devFamily;
    GT_U32                  i;

    appDemoTmInitFunc *appDemoTmInitFuncArr[] =
    {
        appDemoTmGeneral48PortsInit,
        appDemoTmGeneral4PortsInit,
        appDemoTmScenario2Init,
        appDemoTmScenario3Init,
        appDemoTmScenario4Init,
        appDemoTmScenario5Init,
        appDemoTmScenario6Init,
        appDemoTmScenario7Init,
        appDemoTmScenario8Init,
        appDemoTmScenario9Init,
        appDemoTmScenario20Init
    };

    GT_U32 appDemoTmInitFuncArrSize =
        sizeof(appDemoTmInitFuncArr) / sizeof(appDemoTmInitFuncArr[0]);

    char *appDemoTmInitFuncNamesArr[] =
    {
        "appDemoTmGeneral48PortsInit",
        "appDemoTmGeneral4PortsInit",
        "appDemoTmScenario2Init",
        "appDemoTmScenario3Init",
        "appDemoTmScenario4Init",
        "appDemoTmScenario5Init",
        "appDemoTmScenario6Init",
        "appDemoTmScenario7Init",
        "appDemoTmScenario8Init",
        "appDemoTmScenario9Init",
        "appDemoTmScenario20Init"
    };

    GT_U8 firstStressMode = 0;
    GT_U8 lastStressMode = 4;


    PRV_TGF_SKIP_TO_REDUCE_LOG_SIZE_MAC;

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

        for (i = 0; i < appDemoTmInitFuncArrSize; i++)
        {
            /* verify tm lib is closed before reopen */
            cpssTmClose(dev);

            st = appDemoTmInitFuncArr[i](dev);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK,
                                         st,
                                         "tmScenario: %s, dev: %d",
                                         appDemoTmInitFuncNamesArr[i],
                                         dev);
        }
        /*
        if (IS_BOBK_DEV_MAC(dev))
            continue;
        */
        /* tmStressInit */
        for (i = firstStressMode; i <= lastStressMode; i++)
        {
            /* verify tm lib is closed before reopen */
            cpssTmClose(dev);
            st = appDemoTmStressScenarioInit(dev, (GT_U8)i);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK,
                                         st,
                                         "tmScenario: appDemoTmStressScenarioInit, mode: %d, dev: %d",
                                         i,
                                         dev);
        }
        /* clean after the test */
        cpssTmTestTmClose(dev);
    }
}


/*----------------------------------------------------------------------------*/
/*
 * tm internal UT
 */
UTF_SUIT_BEGIN_TESTS_MAC(cpssTmInternal)
    UTF_SUIT_DECLARE_TEST_MAC(rm_chunk_list)
    UTF_SUIT_DECLARE_TEST_MAC(tm_drop_profiles)
    UTF_SUIT_DECLARE_TEST_MAC(tm_aging_drop_profiles)
    UTF_SUIT_DECLARE_TEST_MAC(tm_mixed_drop_profiles)
    UTF_SUIT_DECLARE_TEST_MAC(appDemo_tm_scenarios)
UTF_SUIT_END_TESTS_MAC(cpssTmInternal)

