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
* @file tgfCommonStcUT.c
*
* @brief Enhanced UTs for CPSS STC (sampling to CPU)
*
* @version   9
********************************************************************************
*/

#include <cpssCommon/cpssPresteraDefs.h>
#include <utf/utfMain.h>
#include <utf/private/prvUtfExtras.h>
#include <common/tgfCommon.h>

#include <mirror/prvTgfStc.h>

/*----------------------------------------------------------------------------*/
/*
    prvTgfStcBasic
*/
static void prvTgfStcBasic
(
    IN PRV_TGF_STC_TYPE_ENT                 stcType,
    IN PRV_TGF_STC_COUNT_RELOAD_MODE_ENT    reloadMode
)
{
/********************************************************************

    1. Set configuration
    2. Generate traffic
    3. Restore configuration
********************************************************************/
    PRV_TGF_SUPPORTED_DEV_DECLARE_MAC(UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E);
    PRV_TGF_SUPPORTED_DEV_DECLARE_MAC(UTF_XCAT_E | UTF_XCAT3_E | UTF_LION_E | UTF_XCAT2_E | UTF_LION2_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E);
    if (stcType == PRV_TGF_STC_INGRESS_E)
    {
    }
    else
    {
        /* Test is not supporting TM behavior */
        PRV_TGF_SKIP_TEST_WHEN_TM_USED_MAC;
    }
    PRV_TGF_SUPPORTED_DEV_DECLARE_MAC(UTF_PUMA_E);

    /* FALCON not supports ingress/egress STC */
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum,UTF_FALCON_E);

    /* Set configuration */
    prvTgfStcBasicConfigurationSet(stcType,reloadMode);

    /* Generate traffic */
    prvTgfStcBasicTrafficGenerate(stcType,reloadMode);

    /* Restore configuration */
    prvTgfStcBasicConfigurationRestore(stcType,reloadMode);
}

/*----------------------------------------------------------------------------*/
/*
    prvTgfIngressStcBasicContinuous
*/
UTF_TEST_CASE_MAC(prvTgfIngressStcBasicContinuous)
{
    /* skip this test for all families under Golden Model */
    PRV_TGF_SKIP_LONG_TEST_GM_MAC(UTF_ALL_FAMILY_E);

    prvTgfStcBasic(PRV_TGF_STC_INGRESS_E , PRV_TGF_STC_COUNT_RELOAD_CONTINUOUS_E);
}
/*----------------------------------------------------------------------------*/
/*
    prvTgfIngressStcBasicTriggered
*/
UTF_TEST_CASE_MAC(prvTgfIngressStcBasicTriggered)
{
    PRV_TGF_SKIP_LONG_TEST_MAC(UTF_DXCH_E | UTF_PUMA_E);

    prvTgfStcBasic(PRV_TGF_STC_INGRESS_E , PRV_TGF_STC_COUNT_RELOAD_TRIGGERED_E);
}
/*----------------------------------------------------------------------------*/
/*
    prvTgfEgressStcBasicContinuous
*/
UTF_TEST_CASE_MAC(prvTgfEgressStcBasicContinuous)
{

    /* skip this test for all families under Golden Model */
    PRV_TGF_SKIP_LONG_TEST_GM_MAC(UTF_ALL_FAMILY_E);

    prvTgfStcBasic(PRV_TGF_STC_EGRESS_E , PRV_TGF_STC_COUNT_RELOAD_CONTINUOUS_E);
}
/*----------------------------------------------------------------------------*/
/*
    prvTgfEgressStcBasicTriggered
*/
UTF_TEST_CASE_MAC(prvTgfEgressStcBasicTriggered)
{
    PRV_TGF_SKIP_LONG_TEST_MAC(UTF_DXCH_E | UTF_PUMA_E);

    prvTgfStcBasic(PRV_TGF_STC_EGRESS_E , PRV_TGF_STC_COUNT_RELOAD_TRIGGERED_E);
}
/*
 * Configuration of tgfStc suit
 */
UTF_SUIT_BEGIN_TESTS_MAC(tgfStc)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfIngressStcBasicContinuous)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfIngressStcBasicTriggered)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfEgressStcBasicContinuous)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfEgressStcBasicTriggered)

UTF_SUIT_END_TESTS_MAC(tgfStc)


