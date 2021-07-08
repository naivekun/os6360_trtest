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
* @file tgfCommonIpfixUT.c
*
* @brief Enhanced UTs for CPSS IPFIX
*
* @version   15
********************************************************************************
*/

#include <cpssCommon/cpssPresteraDefs.h>
#include <utf/utfMain.h>
#include <utf/private/prvUtfExtras.h>
#include <common/tgfCommon.h>

#include <ipfix/prvTgfBasicIngressPclKeys.h>
#include <ipfix/prvTgfBasicTests.h>
#include <ipfix/prvTgfAging.h>
#include <ipfix/prvTgfWraparound.h>
#include <ipfix/prvTgfDropMode.h>
#include <ipfix/prvTgfSampling.h>
#include <ipfix/prvTgfTimersUpload.h>
#include <ipfix/prvTgfPortGroupEntry.h>
#include <ipfix/prvTgfPortGroupAging.h>
#include <ipfix/prvTgfPortGroupWraparound.h>
#include <ipfix/prvTgfPortGroupAlarms.h>

/*----------------------------------------------------------------------------*/
/*
    tgfIpfixHelloTest
*/
UTF_TEST_CASE_MAC(tgfIpfixHelloTest)
{
/********************************************************************
    1. Set configuration
    2. Generate traffic
    3. Restore configuration
********************************************************************/

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E |
                                        UTF_XCAT2_E);
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_PUMA_E);

    if(GT_FALSE == prvTgfPclPolicerCheck())
    {
        /* the device may support policer but not from the PCL/TTI */
        return;
    }

    /* test is not ready for GM */
    UTF_SIP5_NOT_SIP6_GM_NOT_READY_FAIL_MAC("IPFIX is not alligned with GM yet\n")

    prvTgfIpfixTrafficGenManager(prvTgfIpfixHelloTestInit,              /* Set configuration */
                                 prvTgfIpfixHelloTestTrafficGenerate,   /* Generate traffic  */
                                 prvTgfIpfixHelloTestRestore);          /* Restore configuration */
}

/*----------------------------------------------------------------------------*/
/*
    tgfIpfixFirstIngressTest
*/
UTF_TEST_CASE_MAC(tgfIpfixFirstIngressTest)
{
/********************************************************************
    1. Set configuration
    2. Generate traffic
    3. Restore configuration
********************************************************************/

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E |
                                        UTF_XCAT2_E);
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_PUMA_E);

    if(GT_FALSE == prvTgfPclPolicerCheck())
    {
        /* the device may support policer but not from the PCL/TTI */
        return;
    }
    /* test is not ready for GM */
    UTF_SIP5_NOT_SIP6_GM_NOT_READY_FAIL_MAC("IPFIX is not alligned with GM yet\n")

    prvTgfIpfixTrafficGenManager(prvTgfIpfixFirstIngressTestInit,         /* Set configuration */
                                 prvTgfIpfixIngressTestTrafficGenerate,   /* Generate traffic  */
                                 prvTgfIpfixIngressTestRestore);          /* Restore configuration */
}

/*----------------------------------------------------------------------------*/
/*
    tgfIpfixFirstUseAllIngressTest
*/
UTF_TEST_CASE_MAC(tgfIpfixFirstUseAllIngressTest)
{
/********************************************************************
    1. Set configuration
    2. Generate traffic
    3. Restore configuration
********************************************************************/

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E |
                                        UTF_XCAT2_E);
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_PUMA_E);

    if(GT_FALSE == prvTgfPclPolicerCheck())
    {
        /* the device may support policer but not from the PCL/TTI */
        return;
    }
    /* test is not ready for GM */
    UTF_SIP5_NOT_SIP6_GM_NOT_READY_FAIL_MAC("IPFIX is not alligned with GM yet\n")

    prvTgfIpfixTrafficGenManager(prvTgfIpfixFirstUseAllIngressTestInit, /* Set configuration */
                                 prvTgfIpfixIngressTestTrafficGenerate, /* Generate traffic  */
                                 prvTgfIpfixIngressTestRestore);        /* Restore configuration */
}

/*----------------------------------------------------------------------------*/
/*
    tgfIpfixSecondUseAllIngressTest
*/
UTF_TEST_CASE_MAC(tgfIpfixSecondUseAllIngressTest)
{
/********************************************************************
    1. Set configuration
    2. Generate traffic
    3. Restore configuration
********************************************************************/

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E |
                                        UTF_XCAT2_E);
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_PUMA_E);

    if(GT_FALSE == prvTgfPclPolicerCheck())
    {
        /* the device may support policer but not from the PCL/TTI */
        return;
    }
    /* test is not ready for GM */
    UTF_SIP5_NOT_SIP6_GM_NOT_READY_FAIL_MAC("IPFIX is not alligned with GM yet\n")

    prvTgfIpfixTrafficGenManager(prvTgfIpfixSecondUseAllIngressTestInit,    /* Set configuration */
                                 prvTgfIpfixIngressTestTrafficGenerate,     /* Generate traffic  */
                                 prvTgfIpfixIngressTestRestore);            /* Restore configuration */
}

/*----------------------------------------------------------------------------*/
/*
    tgfIpfixTimestampVerificationTest
*/
UTF_TEST_CASE_MAC(tgfIpfixTimestampVerificationTest)
{
/********************************************************************
    1. Set configuration
    2. Generate traffic
    3. Restore configuration
********************************************************************/

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E |
                                        UTF_XCAT2_E);
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_PUMA_E);

    PRV_TGF_SKIP_LONG_TEST_MAC(UTF_DXCH_E);

    if(GT_FALSE == prvTgfPclPolicerCheck())
    {
        /* the device may support policer but not from the PCL/TTI */
        return;
    }
    /* test is not ready for GM */
    UTF_SIP5_NOT_SIP6_GM_NOT_READY_FAIL_MAC("IPFIX is not alligned with GM yet\n")

    prvTgfIpfixTrafficGenManager(prvTgfIpfixTimestampVerificationTestInit,              /* Set configuration */
                                 prvTgfIpfixTimestampVerificationTestTrafficGenerate,   /* Generate traffic  */
                                 prvTgfIpfixIngressTestRestore);                        /* Restore configuration */
}

/*----------------------------------------------------------------------------*/
/*
    tgfIpfixAgingTest
*/
UTF_TEST_CASE_MAC(tgfIpfixAgingTest)
{
/********************************************************************
    1. Set configuration
    2. Generate traffic
    3. Restore configuration
********************************************************************/

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E |
                                        UTF_XCAT2_E);
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_PUMA_E);

    if(GT_FALSE == prvTgfPclPolicerCheck())
    {
        /* the device may support policer but not from the PCL/TTI */
        return;
    }
    /* test is not ready for GM */
    UTF_SIP5_GM_NOT_READY_FAIL_MAC("IPFIX is not alligned with GM yet\n")

    prvTgfIpfixTrafficGenManager(prvTgfIpfixAgingTestInit,              /* Set configuration */
                                 prvTgfIpfixAgingTestTrafficGenerate,   /* Generate traffic  */
                                 prvTgfIpfixIngressTestRestore);        /* Restore configuration */
}

/*----------------------------------------------------------------------------*/
/*
    tgfIpfixWraparoundFreezeTest
*/
UTF_TEST_CASE_MAC(tgfIpfixWraparoundFreezeTest)
{
/********************************************************************
    1. Set configuration
    2. Generate traffic
    3. Restore configuration
********************************************************************/

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E |
                                        UTF_XCAT2_E);
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_PUMA_E);

    if(GT_FALSE == prvTgfPclPolicerCheck())
    {
        /* the device may support policer but not from the PCL/TTI */
        return;
    }
    /* test is not ready for GM */
    UTF_SIP5_NOT_SIP6_GM_NOT_READY_FAIL_MAC("IPFIX is not alligned with GM yet\n")

    /* Set configuration */
    prvTgfIpfixWraparoundTestInit();

    /* Generate traffic */
    prvTgfIpfixWraparoundFreezeTestTrafficGenerate();

    /* Restore configuration */
    prvTgfIpfixWraparoundTestRestore();
}

/*----------------------------------------------------------------------------*/
/*
    tgfIpfixWraparoundClearTest
*/
UTF_TEST_CASE_MAC(tgfIpfixWraparoundClearTest)
{
/********************************************************************
    1. Set configuration
    2. Generate traffic
    3. Restore configuration
********************************************************************/

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E |
                                        UTF_XCAT2_E);
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_PUMA_E);

    if(GT_FALSE == prvTgfPclPolicerCheck())
    {
        /* the device may support policer but not from the PCL/TTI */
        return;
    }
    /* test is not ready for GM */
    UTF_SIP5_GM_NOT_READY_FAIL_MAC("IPFIX is not alligned with GM yet\n")

    /* Set configuration */
    prvTgfIpfixWraparoundTestInit();

    /* Generate traffic */
    prvTgfIpfixWraparoundClearTestTrafficGenerate();

    /* Restore configuration */
    prvTgfIpfixWraparoundTestRestore();
}

/*----------------------------------------------------------------------------*/
/*
    tgfIpfixWraparoundMaxBytesTest
*/
UTF_TEST_CASE_MAC(tgfIpfixWraparoundMaxBytesTest)
{
/********************************************************************
    1. Set configuration
    2. Generate traffic
    3. Restore configuration
********************************************************************/

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E |
                                        UTF_XCAT2_E);
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_PUMA_E);

    if(GT_FALSE == prvTgfPclPolicerCheck())
    {
        /* the device may support policer but not from the PCL/TTI */
        return;
    }
    /* test is not ready for GM */
    UTF_SIP5_GM_NOT_READY_FAIL_MAC("IPFIX is not alligned with GM yet\n")

    /* Set configuration */
    prvTgfIpfixWraparoundTestInit();

    /* Generate traffic */
    prvTgfIpfixWraparoundMaxBytesTestTrafficGenerate();

#ifdef ASIC_SIMULATION
    /* Restore configuration */
    prvTgfIpfixWraparoundTestRestore();
#endif
}

/*----------------------------------------------------------------------------*/
/*
    tgfIpfixAgingTest
*/
UTF_TEST_CASE_MAC(tgfIpfixDropModeSimpleTest)
{
/********************************************************************
    1. Set configuration
    2. Generate traffic
    3. Restore configuration
********************************************************************/

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E |
                                        UTF_XCAT2_E);
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_PUMA_E);

    if(GT_FALSE == prvTgfPclPolicerCheck())
    {
        /* the device may support policer but not from the PCL/TTI */
        return;
    }
    /* test is not ready for GM */
    UTF_SIP5_NOT_SIP6_GM_NOT_READY_FAIL_MAC("IPFIX is not alligned with GM yet\n")

    prvTgfIpfixTrafficGenManager(prvTgfIpfixDropModeSimpleTestInit,             /* Set configuration */
                                 prvTgfIpfixDropModeSimpleTestTrafficGenerate,  /* Generate traffic  */
                                 prvTgfIpfixIngressTestRestore);                /* Restore configuration */
}

/*----------------------------------------------------------------------------*/
/*
    tgfIpfixSamplingSimpleTest
*/
UTF_TEST_CASE_MAC(tgfIpfixSamplingSimpleTest)
{
/********************************************************************
    1. Set configuration
    2. Generate traffic
    3. Restore configuration
********************************************************************/

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E |
                                        UTF_XCAT2_E);
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_PUMA_E);

    if(GT_FALSE == prvTgfPclPolicerCheck())
    {
        /* the device may support policer but not from the PCL/TTI */
        return;
    }
    /* test is not ready for GM */
    UTF_SIP5_NOT_SIP6_GM_NOT_READY_FAIL_MAC("IPFIX is not alligned with GM yet\n")

    prvTgfIpfixTrafficGenManager(prvTgfIpfixSamplingSimpleTestInit,             /* Set configuration */
                                 prvTgfIpfixSamplingSimpleTestTrafficGenerate,  /* Generate traffic  */
                                 prvTgfIpfixSamplingTestRestore);               /* Restore configuration */
}

/*----------------------------------------------------------------------------*/
/*
    tgfIpfixSamplingToCpuTest
*/
UTF_TEST_CASE_MAC(tgfIpfixSamplingToCpuTest)
{
/********************************************************************
    1. Set configuration
    2. Generate traffic
    3. Restore configuration
********************************************************************/

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E |
                                        UTF_XCAT2_E);
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_PUMA_E);

    if(GT_FALSE == prvTgfPclPolicerCheck())
    {
        /* the device may support policer but not from the PCL/TTI */
        return;
    }
    /* test is not ready for GM */
    UTF_SIP5_GM_NOT_READY_FAIL_MAC("IPFIX is not alligned with GM yet\n")

    prvTgfIpfixTrafficGenManager(prvTgfIpfixSamplingToCpuTestInit,              /* Set configuration */
                                 prvTgfIpfixSamplingToCpuTestTrafficGenerate,   /* Generate traffic  */
                                 prvTgfIpfixSamplingTestRestore);               /* Restore configuration */
}

/*----------------------------------------------------------------------------*/
/*
    tgfIpfixAlarmEventsTest
*/
UTF_TEST_CASE_MAC(tgfIpfixAlarmEventsTest)
{
/********************************************************************
    1. Set configuration
    2. Generate traffic
    3. Restore configuration
********************************************************************/

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E |
                                        UTF_XCAT2_E);
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_PUMA_E);

    if(GT_FALSE == prvTgfPclPolicerCheck())
    {
        /* the device may support policer but not from the PCL/TTI */
        return;
    }
    /* test is not ready for GM */
    UTF_SIP5_GM_NOT_READY_FAIL_MAC("IPFIX is not alligned with GM yet\n")

    prvTgfIpfixTrafficGenManager(prvTgfIpfixAlarmEventsTestInit,            /* Set configuration */
                                 prvTgfIpfixAlarmEventsTestTrafficGenerate, /* Generate traffic  */
                                 prvTgfIpfixSamplingTestRestore);           /* Restore configuration */

}

/*----------------------------------------------------------------------------*/
/*
    tgfIpfixTimersUploadAbsoluteTest
*/
UTF_TEST_CASE_MAC(tgfIpfixTimersUploadAbsoluteTest)
{
/********************************************************************
    1. Upload timers
    2. Restore
********************************************************************/

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E |
                                        UTF_XCAT2_E);
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_PUMA_E);
    /* test is not ready for GM */
    UTF_SIP5_GM_NOT_READY_FAIL_MAC("IPFIX is not alligned with GM yet\n")

    /* Upload timers */
    prvTgfIpfixTimersUploadAbsoluteTest();

    /* Restore */
    prvTgfIpfixTimersUploadRestoreByClear();
}

/*----------------------------------------------------------------------------*/
/*
    tgfIpfixTimersUploadIncrementalTest
*/
UTF_TEST_CASE_MAC(tgfIpfixTimersUploadIncrementalTest)
{
/********************************************************************
    1. Upload timers
    2. Restore
********************************************************************/

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E |
                                        UTF_XCAT2_E);
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_PUMA_E);
    /* test is not ready for GM */
    UTF_SIP5_GM_NOT_READY_FAIL_MAC("IPFIX is not alligned with GM yet\n")

    /* Upload timers */
    prvTgfIpfixTimersUploadIncrementalTest();

    /* Restore */
    prvTgfIpfixTimersUploadRestoreByClear();
}

/*----------------------------------------------------------------------------*/
/*
    tgfIpfixPortGroupEntryTest
*/
UTF_TEST_CASE_MAC(tgfIpfixPortGroupEntryTest)
{
/********************************************************************
    1. Set configuration
    2. Generate traffic
    3. Restore configuration
********************************************************************/

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E |
                                        UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_PUMA_E);
    /* test is not ready for GM */
    UTF_SIP5_GM_NOT_READY_FAIL_MAC("IPFIX is not alligned with GM yet\n")


    prvTgfIpfixPortGroupEntryTestInit();            /* Set configuration */
    prvTgfIpfixPortGroupEntryTestTrafficGenerate(); /* Generate traffic  */
    prvTgfIpfixPortGroupEntryTestRestore();         /* Restore configuration */
}

/*----------------------------------------------------------------------------*/
/*
    tgfIpfixPortGroupAgingTest
*/
UTF_TEST_CASE_MAC(tgfIpfixPortGroupAgingTest)
{
/********************************************************************
    1. Set configuration
    2. Generate traffic
    3. Restore configuration
********************************************************************/

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E |
                                        UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_PUMA_E);
    /* test is not ready for GM */
    UTF_SIP5_GM_NOT_READY_FAIL_MAC("IPFIX is not alligned with GM yet\n")

    prvTgfIpfixPortGroupEntryTestInit();            /* Set configuration */
    prvTgfIpfixPortGroupAgingTestTrafficGenerate(); /* Generate traffic  */
    prvTgfIpfixPortGroupEntryTestRestore();         /* Restore configuration */
}

/*----------------------------------------------------------------------------*/
/*
    tgfIpfixPortGroupWraparoundTest
*/
UTF_TEST_CASE_MAC(tgfIpfixPortGroupWraparoundTest)
{
/********************************************************************
    1. Set configuration
    2. Generate traffic
    3. Restore configuration
********************************************************************/

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E |
                                        UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_PUMA_E);
    /* test is not ready for GM */
    UTF_SIP5_GM_NOT_READY_FAIL_MAC("IPFIX is not alligned with GM yet\n")

    prvTgfIpfixPortGroupEntryTestInit();                    /* Set configuration */
    prvTgfIpfixPortGroupWraparoundTestTrafficGenerate();    /* Generate traffic  */
    prvTgfIpfixPortGroupEntryTestRestore();                 /* Restore configuration */
}

/*----------------------------------------------------------------------------*/
/*
    tgfIpfixPortGroupAlarmsTest
*/
UTF_TEST_CASE_MAC(tgfIpfixPortGroupAlarmsTest)
{
/********************************************************************
    1. Set configuration
    2. Generate traffic
    3. Restore configuration
********************************************************************/

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E |
                                        UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_PUMA_E);
    /* test is not ready for GM */
    UTF_SIP5_GM_NOT_READY_FAIL_MAC("IPFIX is not alligned with GM yet\n")

    PRV_TGF_SKIP_LONG_TEST_MAC(UTF_ALL_FAMILY_E);

    prvTgfIpfixPortGroupAlarmsTestInit();               /* Set configuration */
    prvTgfIpfixPortGroupAlarmsTestTrafficGenerate();    /* Generate traffic  */
    prvTgfIpfixPortGroupAlarmsTestRestore();            /* Restore configuration */
}

/*----------------------------------------------------------------------------*/
/*
    tgfIpfixTimersUploadIncrementalTest
*/
UTF_TEST_CASE_MAC(tgfIpfixTimestampToCpuTest)
{
/********************************************************************
    1. Upload timers
    2. Restore
********************************************************************/

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E |
                                        UTF_XCAT2_E);
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_PUMA_E);
    /* test is not ready for GM */
    UTF_SIP5_GM_NOT_READY_FAIL_MAC("IPFIX is not alligned with GM yet\n")

    prvTgfIpfixTimestampToCpuTestInit();            /* Set configuration */
    prvTgfIpfixTimestampToCpuTestTrafficGenerate(); /* Generate traffic  */
    prvTgfIpfixTimestampToCpuTestRestore();         /* Restore configuration */
}

UTF_TEST_CASE_MAC(tgfIpfixEgressSamplingToCpuTest)
{
/********************************************************************
    1. Set configuration
    2. Generate traffic
    3. Restore configuration
********************************************************************/
    PRV_TGF_SUPPORTED_DEV_DECLARE_MAC( UTF_FALCON_E )

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(
        prvTgfDevNum, (~ UTF_CPSS_PP_ALL_SIP6_CNS))

    if(GT_FALSE == prvTgfPclPolicerCheck())
    {
        /* the device may support policer but not from the PCL/TTI */
        return;
    }

    prvTgfIpfixEgressSamplingToCpuTestInit();          /* Set configuration */
    prvTgfIpfixSamplingSimpleTestTrafficGenerate();   /* Generate traffic  */
    prvTgfIpfixEgressSamplingTestRestore();            /* Restore configuration */
}
/*
 * Configuration of tgfIpfix suit
 */
UTF_SUIT_BEGIN_TESTS_MAC(tgfIpfix)
    UTF_SUIT_DECLARE_TEST_MAC(tgfIpfixHelloTest)
    UTF_SUIT_DECLARE_TEST_MAC(tgfIpfixFirstIngressTest)
    UTF_SUIT_DECLARE_TEST_MAC(tgfIpfixFirstUseAllIngressTest)
    UTF_SUIT_DECLARE_TEST_MAC(tgfIpfixSecondUseAllIngressTest)
    UTF_SUIT_DECLARE_TEST_MAC(tgfIpfixTimestampVerificationTest)
    UTF_SUIT_DECLARE_TEST_MAC(tgfIpfixAgingTest)
    UTF_SUIT_DECLARE_TEST_MAC(tgfIpfixWraparoundFreezeTest)
    UTF_SUIT_DECLARE_TEST_MAC(tgfIpfixWraparoundClearTest)
    UTF_SUIT_DECLARE_TEST_MAC(tgfIpfixWraparoundMaxBytesTest)
    UTF_SUIT_DECLARE_TEST_MAC(tgfIpfixDropModeSimpleTest)
    UTF_SUIT_DECLARE_TEST_MAC(tgfIpfixSamplingSimpleTest)
    UTF_SUIT_DECLARE_TEST_MAC(tgfIpfixSamplingToCpuTest)
    UTF_SUIT_DECLARE_TEST_MAC(tgfIpfixAlarmEventsTest)
    UTF_SUIT_DECLARE_TEST_MAC(tgfIpfixTimersUploadAbsoluteTest)
    UTF_SUIT_DECLARE_TEST_MAC(tgfIpfixTimersUploadIncrementalTest)
    UTF_SUIT_DECLARE_TEST_MAC(tgfIpfixPortGroupEntryTest)
    UTF_SUIT_DECLARE_TEST_MAC(tgfIpfixPortGroupAgingTest)
    UTF_SUIT_DECLARE_TEST_MAC(tgfIpfixPortGroupWraparoundTest)
    UTF_SUIT_DECLARE_TEST_MAC(tgfIpfixPortGroupAlarmsTest)
    UTF_SUIT_DECLARE_TEST_MAC(tgfIpfixTimestampToCpuTest)
    UTF_SUIT_DECLARE_TEST_MAC(tgfIpfixEgressSamplingToCpuTest)
UTF_SUIT_END_TESTS_MAC(tgfIpfix)

















