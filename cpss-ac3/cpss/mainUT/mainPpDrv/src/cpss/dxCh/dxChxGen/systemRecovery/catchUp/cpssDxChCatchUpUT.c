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
* @file cpssDxChCatchUpUT.c
*
* @brief Unit tests for CatchUp feature
*
* @version   1
********************************************************************************
*/

/* includes */
#include <cpss/dxCh/dxChxGen/systemRecovery/catchUp/private/prvCpssDxChCatchUpDbg.h>
#include <cpss/dxCh/dxChxGen/systemRecovery/catchUp/cpssDxChCatchUpDbg.h>
#include <cpss/dxCh/dxChxGen/systemRecovery/catchUp/private/prvCpssDxChCatchUp.h>
#include <cpss/dxCh/dxChxGen/systemRecovery/catchUp/cpssDxChCatchUp.h>
#include <cpss/generic/systemRecovery/cpssGenSystemRecovery.h>
#include <utf/utfMain.h>
#include <utf/private/prvUtfExtras.h>

/* defines */

/* Default valid value for port id */
#define PORT_CTRL_VALID_PHY_PORT_CNS  0


UTF_TEST_CASE_MAC(cpssDxChCatchUpValidityCheckTest)
{
    GT_STATUS   st = GT_OK;
    GT_U8       dev;
    CPSS_SYSTEM_RECOVERY_INFO_STC tmpRecovery;
    GT_BOOL     enable;
    GT_U32      seed;

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_PUMA_E);
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E | UTF_ALDRIN_E | UTF_AC3X_E);

    utfRandomRunModeGet(&enable, &seed);
    if (enable == GT_TRUE)
    {
        /* Pre condition for running the test is that SW database is synchronized with HW info.
           Since some CPSS APIs don't synchronize the SW according to HW the test may fail if
           it doesn't run prior to some other tests. */
        SKIP_TEST_MAC;
    }
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E);

    /* Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* 1. Call cpssDxChCatchUpValidityCheckEnable for all active devices.
           Expected: GT_OK.
        */
        if (UTF_CPSS_SIP5_0_ONLY_CHECK_MAC(dev))
        {
            /* not supported in Bobcat2 A0 */
            SKIP_TEST_MAC;
        }

        st = cpssDxChCatchUpValidityCheckEnable(dev);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
    }
    /* 2. Call prvCpssDxChSystemRecoveryCatchUpHandle.
       Expected: GT_OK.
    */
    tmpRecovery.systemRecoveryProcess = CPSS_SYSTEM_RECOVERY_PROCESS_FAST_BOOT_E;
    tmpRecovery.systemRecoveryState = CPSS_SYSTEM_RECOVERY_HW_CATCH_UP_STATE_E;
    tmpRecovery.systemRecoveryMode.continuousAuMessages = GT_FALSE;
    tmpRecovery.systemRecoveryMode.continuousFuMessages = GT_FALSE;
    tmpRecovery.systemRecoveryMode.continuousRx = GT_FALSE;
    tmpRecovery.systemRecoveryMode.continuousTx = GT_FALSE;
    tmpRecovery.systemRecoveryMode.haCpuMemoryAccessBlocked = GT_FALSE;
    st = cpssSystemRecoveryStateSet(&tmpRecovery);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E);

    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
       /*  3. Call   cpssDxChCatchUpValidityCheck for all active devices.
           Expected: GT_OK.
       */
        st = cpssDxChCatchUpValidityCheck(dev);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
    }
    tmpRecovery.systemRecoveryState = CPSS_SYSTEM_RECOVERY_COMPLETION_STATE_E;
    st = cpssSystemRecoveryStateSet(&tmpRecovery);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

}
/*
GT_STATUS cpssDxChCatchUpPortModeDbSet
(
    IN GT_U8                        devNum,
    IN GT_PHYSICAL_PORT_NUM         port,
    IN CPSS_PORT_INTERFACE_MODE_ENT ifMode
)
*/
UTF_TEST_CASE_MAC(cpssDxChCatchUpPortModeDbSet)
{
/*
    ITERATE_DEVICES_PHY_CPU_PORTS (DxCh)
    1.1.1. Call with ifMode[CPSS_PORT_INTERFACE_MODE_KR_E /
                            CPSS_PORT_INTERFACE_MODE_SR_LR_E].
    Expected: GT_OK.
    1.1.2. Call with ifMode [CPSS_PORT_INTERFACE_MODE_SGMII_E].
    Expected: NOT GT_OK.
    1.1.3. Call with wrong enum values ifMode.
    Expected: GT_BAD_PARAM.
    1.1.4. Call not during recovery process.
    Expected: GT_BAD_STATE.
*/
    GT_STATUS                    st     = GT_OK;
    GT_U8                        dev;
    GT_PHYSICAL_PORT_NUM         port   = PORT_CTRL_VALID_PHY_PORT_CNS;
    CPSS_PORT_INTERFACE_MODE_ENT ifMode = CPSS_PORT_INTERFACE_MODE_NA_E;
    PRV_CPSS_PORT_TYPE_ENT       portType;

    CPSS_PORT_INTERFACE_MODE_ENT ifMode_save   = CPSS_PORT_INTERFACE_MODE_NA_E;
    PRV_CPSS_PORT_TYPE_ENT       portType_save = PRV_CPSS_PORT_NOT_EXISTS_E;
    CPSS_SYSTEM_RECOVERY_INFO_STC saved_system_recovery;
    CPSS_SYSTEM_RECOVERY_INFO_STC system_recovery;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E);

    /* Set system recovery state to HA */
    st = cpssSystemRecoveryStateGet(&system_recovery);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);
    saved_system_recovery = system_recovery;

    system_recovery.systemRecoveryProcess = CPSS_SYSTEM_RECOVERY_PROCESS_HA_E;
    system_recovery.systemRecoveryState = CPSS_SYSTEM_RECOVERY_INIT_STATE_E;
    st =  cpssSystemRecoveryStateSet(&system_recovery);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_TRUE))
        {
            /* save values */
            ifMode_save   = PRV_CPSS_PP_MAC(dev)->phyPortInfoArray[port].portIfMode;
            portType_save = PRV_CPSS_PP_MAC(dev)->phyPortInfoArray[port].portType;

            if(prvCpssDxChPortRemotePortCheck(dev,port))
            {
                /* Skip Remote ports */
                continue;
            }

            st = prvUtfPortTypeGet(dev, port, &portType);
            if (st != GT_OK)
            {
                continue;
            }
            /*UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);*/

            /*
                1.1.1. Call with ifMode[CPSS_PORT_INTERFACE_MODE_KR_E /
                                        CPSS_PORT_INTERFACE_MODE_SR_LR_E].
                Expected: GT_OK.
            */

            /* call with ifMode = CPSS_PORT_INTERFACE_MODE_KR_E */
            ifMode = CPSS_PORT_INTERFACE_MODE_KR_E;

            st = cpssDxChCatchUpPortModeDbSet(dev, port, ifMode);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, ifMode);

            /* call with ifMode = CPSS_PORT_INTERFACE_MODE_SR_LR_E */
            ifMode = CPSS_PORT_INTERFACE_MODE_SR_LR_E;

            st = cpssDxChCatchUpPortModeDbSet(dev, port, ifMode);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, ifMode);

            /*
                1.1.2. Call with ifMode [CPSS_PORT_INTERFACE_MODE_SGMII_E].
                Expected: GT_BAD_PARAM.
            */
            ifMode = CPSS_PORT_INTERFACE_MODE_SGMII_E;

            st = cpssDxChCatchUpPortModeDbSet(dev, port, ifMode);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PARAM, st, dev, port, ifMode);

            /*
                1.1.3. Call with wrong enum values ifMode.
                Expected: GT_BAD_PARAM.
            */
            UTF_ENUMS_CHECK_MAC(cpssDxChCatchUpPortModeDbSet
                                (dev, port, ifMode),
                                ifMode);

            /*
                1.1.4. Call not during recovery process.
                Expected: GT_BAD_STATE.
            */

            /* Restore system recovery state to normal */
            st =  cpssSystemRecoveryStateSet(&saved_system_recovery);
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

            /* call with ifMode = CPSS_PORT_INTERFACE_MODE_KR_E */
            ifMode = CPSS_PORT_INTERFACE_MODE_KR_E;

            st = cpssDxChCatchUpPortModeDbSet(dev, port, ifMode);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_STATE, st, dev, port, ifMode);

            /* Set system recovery state to HA */
            system_recovery.systemRecoveryProcess = CPSS_SYSTEM_RECOVERY_PROCESS_HA_E;
            system_recovery.systemRecoveryState = CPSS_SYSTEM_RECOVERY_INIT_STATE_E;
            st =  cpssSystemRecoveryStateSet(&system_recovery);
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

            /* restore values */
            PRV_CPSS_PP_MAC(dev)->phyPortInfoArray[port].portIfMode = ifMode_save;
            PRV_CPSS_PP_MAC(dev)->phyPortInfoArray[port].portType   = portType_save;
        }

        ifMode = CPSS_PORT_INTERFACE_MODE_KR_E;

        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available
           physical ports.
        */
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_FALSE))
        {
            st = cpssDxChCatchUpPortModeDbSet(dev, port, ifMode);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssDxChCatchUpPortModeDbSet(dev, port, ifMode);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

    }

    ifMode = CPSS_PORT_INTERFACE_MODE_KR_E;

    /* 2. For not active devices check that function returns non GT_OK.*/

    port = PORT_CTRL_VALID_PHY_PORT_CNS;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChCatchUpPortModeDbSet(dev, port, ifMode);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChCatchUpPortModeDbSet(dev, port, ifMode);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);

    /* Restore system recovery state to normal */
    st =  cpssSystemRecoveryStateSet(&saved_system_recovery);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

}

/*
 * Configuration of cpssDxChCatchUp suit
 */
UTF_SUIT_BEGIN_TESTS_MAC(cpssDxChCatchUp)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChCatchUpValidityCheckTest)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChCatchUpPortModeDbSet)
UTF_SUIT_END_TESTS_MAC(cpssDxChCatchUp)


