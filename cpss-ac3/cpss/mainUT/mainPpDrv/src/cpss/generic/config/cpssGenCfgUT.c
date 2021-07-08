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
* @file cpssGenCfgUT.c
*
* @brief Unit tests for cpssGenCfg, that provides
* CPSS generic configuration functions.
*
* @version   2
********************************************************************************
*/

/* includes */
#include <cpss/common/config/cpssGenCfg.h>

#include <utf/utfMain.h>
#include <utf/private/prvUtfExtras.h>

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssPpCfgNextDevGet
(
    IN  GT_U8 devNum,
    OUT GT_U8 *nextDevNumPtr
)
*/
UTF_TEST_CASE_MAC(cpssPpCfgNextDevGet)
{
/*
    ITERATE_DEVICE (all)
    1.1. Call with valid nextDevNumPtr.
    Expected: GT_OK.
    1.2. Call with out of range nextDevNumPtr[NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st          = GT_OK;
    GT_U8       dev         = 0;
    GT_U8       nextDevNum  = 0;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with valid nextDevNumPtr.
            Expected: GT_OK.
        */
        st = cpssPpCfgNextDevGet(dev, &nextDevNum);
        st = GT_NO_MORE == st ? GT_OK : st;
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.2. Call with out of range nextDevNumPtr[NULL].
            Expected: GT_BAD_PTR.
        */
        st = cpssPpCfgNextDevGet(dev, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, nextDevNumPtr = NULL", dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPpCfgNextDevGet(dev, &nextDevNum);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssPpCfgNextDevGet(dev, &nextDevNum);
    st = GT_NO_MORE == st ? GT_BAD_PARAM : st;
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

UTF_TEST_CASE_MAC(prvTgfResetAndInitSystem)
{
    PRV_TGF_SKIP_TO_REDUCE_LOG_SIZE_MAC;

    if(prvTgfResetModeGet() == GT_FALSE)
    {
        prvUtfSkipTestsSet();
        return;
    }

    prvTgfResetAndInitSystem();
}

UTF_TEST_CASE_MAC(prvTgfResetAndInitSystem_gtShutdownAndCoreRestart)
{
    GT_STATUS   st;
    PRV_TGF_SKIP_TO_REDUCE_LOG_SIZE_MAC;

    PRV_TGF_SKIP_SIMULATION_FAILED_TEST_MAC(UTF_BOBCAT2_E | UTF_AC3X_E | UTF_LION2_E, "JIRA-6784,JIRA-6873");

    if(prvTgfResetModeGet_gtShutdownAndCoreRestart() == GT_FALSE)
    {
        /* not applicable device */
        prvUtfSkipTestsSet();
        return;
    }
    st = prvTgfResetAndInitSystem_gtShutdownAndCoreRestart();
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,"prvTgfResetAndInitSystem_gtShutdownAndCoreRestart : FAILED st=[%d]",st);

    /* Linux needed extra sleep */
    cpssOsTimerWkAfter(1000);

}

/* use mainUt test to allow the LUA do 'gtShutdownAndCoreRestart' */
void fromLua_gtShutdownAndCoreRestart(void)
{
    if(prvTgfResetModeGet_gtShutdownAndCoreRestart() == GT_FALSE)
    {
        /* not applicable device */
        prvUtfSkipTestsSet();
        return;
    }
    prvTgfResetAndInitSystem_gtShutdownAndCoreRestart();
}

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/

/*
 * Configuration of cpssGenCfg suit
 */
UTF_SUIT_BEGIN_TESTS_MAC(cpssGenCfg)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPpCfgNextDevGet)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfResetAndInitSystem_gtShutdownAndCoreRestart)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfResetAndInitSystem)
UTF_SUIT_END_TESTS_MAC(cpssGenCfg)

