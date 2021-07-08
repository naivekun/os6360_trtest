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
* @file tgfCommonOamUT.c
*
* @brief Enhanced UTs for CPSS OAM
*
* @version   10
********************************************************************************
*/

#include <cpssCommon/cpssPresteraDefs.h>
#include <utf/utfMain.h>
#include <utf/private/prvUtfExtras.h>
#include <common/tgfCommon.h>
#include <oam/prvTgfOamGeneral.h>
#include <oam/prvTgfOamDelayMeasurement.h>
#include <oam/prvTgfOamLossMeasurement.h>
#include <oam/prvTgfOamGenericKeepalive.h>

/*
    Set mode for the test: using TTI/IPCL for the OAM profile assignment.
    in any case the IPCL is used for other parameters setting.


    NOTE: ALL tests can run in this mode !!!
*/
static GT_BOOL useTtiForOam = GT_FALSE;


/*----------------------------------------------------------------------------*/
/*
    AUTODOC: MEG level check

    2.1.1 tgfOamMegLevelCheck

    Create Vlan 5 on ports with indexs 0, 1, 2, 3;
    Create trunk 6 with port members 0, 2;
    Create 4 TCAM rules:
       - PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E -
            trigger Vlan Id = 5, source port 1, ethertype 0x8902; MEL 3
       - PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E -
            trigger Vlan Id = 5, source port 1, ethertype 0x8902; MEL - Don't care
       - PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E -
            trigger Vlan Id = 5, Trunk Id = 6, MEL 5
       - PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E -
            trigger Vlan Id = 5, source port 2, ethertype 0x8902; MEL 3
    Create 4 IOAM, 4 IPLR billing entries binded to IPCL rules

    Create 4 TCAM rules:
      - PRV_TGF_PCL_RULE_FORMAT_EGRESS_STD_NOT_IP_E -
           trigger DSA Command = FROM_CPU0; Opcode = 46
      - PRV_TGF_PCL_RULE_FORMAT_EGRESS_STD_NOT_IP_E -
           trigger DSA Command = TO_CPU1; Opcode = 46
      - PRV_TGF_PCL_RULE_FORMAT_EGRESS_STD_NOT_IP_E -
           trigger DSA Command = FROM_CPU1; Opcode = 47
      - PRV_TGF_PCL_RULE_FORMAT_EGRESS_STD_NOT_IP_E -
           trigger DSA Command = TO_CPU0; Opcode = 47
    Create 4 EOAM, 4 EPLR billing entries binded to EPCL rules

    Send OAM traffic on port 1 with different MEL values.
    Check that packets forwarded or trapped to CPU accordingly.
*/
UTF_TEST_CASE_MAC(tgfOamMegLevelCheck)
{
 /*
    1. Set VLAN and IPCL configuration
    2. Generate traffic
    3. Restore VLAN and IPCL configuration
 */
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum,
                                        UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                        UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E |
                                        UTF_LION_E | UTF_XCAT2_E | UTF_LION2_E);
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_PUMA_E);

    /* skip this test for all families under Golden Model */
    PRV_TGF_SKIP_LONG_TEST_GM_MAC(UTF_ALL_FAMILY_E);

    prvTgfDefOamGeneralConfigurationSet();

    prvTgfDefOamGeneralMelTrafficTest();

    prvTgfDefOamGeneralConfigurationRestore();
}

/*----------------------------------------------------------------------------*/
/*
    AUTODOC: Source interface verification

    2.1.2 tgfOamSourceInterfaceVerification

    Set common bridge configuration (see paragraph 2.1.1);
    Set common PCL configuration (see paragraph 2.1.1);
    Set egress PCL configuration (see paragraph 2.1.1);

    Send OAM traffic on port 1 and trun 6 with different OAM source interface settings.
    Check that packets forwarded or trapped to CPU accordingly.

*/
UTF_TEST_CASE_MAC(tgfOamSourceInterfaceVerification)
{
 /*
    1. Set VLAN and IPCL configuration
    2. Generate traffic
    3. Restore VLAN and IPCL configuration
 */
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum,
                                        UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                        UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E |
                                        UTF_LION_E | UTF_XCAT2_E | UTF_LION2_E);
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_PUMA_E);

    prvTgfDefOamGeneralConfigurationSet();

    prvTgfDefOamGeneralSourceInterfaceTest();

    prvTgfDefOamGeneralConfigurationRestore();
}

/*----------------------------------------------------------------------------*/
/*
    AUTODOC: Dual-Ended Loss Measurement Packets Test

    2.1.3  tgfOamDualEndedLossMeasurementPacketCommand

    Set common bridge configuration (see paragraph 2.1.1);
    Set common PCL configuration (see paragraph 2.1.1);
    Set egress PCL configuration (see paragraph 2.1.1);

    Send OAM traffic on port 1 with dual-ended LM opcode.
    Check that packets trapped to CPU.

*/
UTF_TEST_CASE_MAC(tgfOamDualEndedLossMeasurementPacketCommand)
{
 /*
    1. Set VLAN and IPCL configuration
    2. Generate traffic
    3. Restore VLAN and IPCL configuration
 */
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum,
                                        UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                        UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E |
                                        UTF_LION_E | UTF_XCAT2_E | UTF_LION2_E);
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_PUMA_E);

    /*
        Set mode for the test: using TTI/IPCL for the OAM profile assignment.
        in any case the IPCL is used for other parameters setting.
    */
    prvTgfOamGenericUseTtiForOam(useTtiForOam);

    prvTgfDefOamGeneralConfigurationSet();

    prvTgfDefOamGeneralDualEndedLossMeasurementTest();

    prvTgfDefOamGeneralConfigurationRestore();

    /* restore default values */
    prvTgfOamGenericUseTtiForOam(GT_FALSE);

}
/* AUTODOC : test tgfOamDualEndedLossMeasurementPacketCommand with TTI action that sets oamProcessingEn and oamIndex */
UTF_TEST_CASE_MAC(tgfOamDualEndedLossMeasurementPacketCommand_useTtiOamProfile)
{
    useTtiForOam = GT_TRUE;
    UTF_TEST_CALL_MAC(tgfOamDualEndedLossMeasurementPacketCommand);
    useTtiForOam = GT_FALSE;
}


/*----------------------------------------------------------------------------*/
/*
    AUTODOC: Delay Measurement Verification

    2.2.1  tgfOamDelayMeasurementVerification

    Set common bridge configuration (see paragraph 2.1.1);
    Set common PCL configuration (see paragraph 2.1.1);
    Set egress PCL configuration (see paragraph 2.1.1);

    Send OAM DMM packet on port 1;
    Wait 2 seconds;
    Send OAM DMR packet on port 2;
    Check recieved PDU header's field: MEL, DMM/DMR opcodes and timestamp results.
*/
UTF_TEST_CASE_MAC(tgfOamDelayMeasurementVerification)
{
 /*
    1. Set VLAN and IPCL configuration
    2. Generate traffic
    3. Restore VLAN and IPCL configuration
 */
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum,
                                        UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                        UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E |
                                        UTF_LION_E | UTF_XCAT2_E | UTF_LION2_E);
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_PUMA_E);

    prvTgfDefOamGeneralConfigurationSet();

    tgfOamDelayMeasurementTest();

    prvTgfDefOamGeneralConfigurationRestore();
}

/*----------------------------------------------------------------------------*/
/*
    AUTODOC: Loss Measurement Verification

    2.3.1  tgfOamLossMeasurementVerification

    Set common bridge configuration (see paragraph 2.1.1);
    Set common PCL configuration (see paragraph 2.1.1);
    Set egress PCL configuration (see paragraph 2.1.1);

    Send 2 OAM LMM packet on port 1;
    Send 2 OAM LMR packet on port 2;
    Check recieved PDU header's field: MEL, LMM/LMR opcodes and billing counters.

*/
UTF_TEST_CASE_MAC(tgfOamLossMeasurementVerification)
{
 /*
    1. Set VLAN and IPCL configuration
    2. Generate traffic
    3. Restore VLAN and IPCL configuration
 */
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum,
                                        UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                        UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E |
                                        UTF_LION_E | UTF_XCAT2_E | UTF_LION2_E);
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_PUMA_E);

    /* skip this test for all families under Golden Model */
    PRV_TGF_SKIP_LONG_TEST_GM_MAC(UTF_ALL_FAMILY_E);

    prvTgfDefOamGeneralConfigurationSet();

    tgfOamLossMeasurementTest();

    prvTgfDefOamGeneralConfigurationRestore();
}

/*----------------------------------------------------------------------------*/
/*
    AUTODOC: Loss Measurement Verification (Green Counter Snapshot)

    2.3.2  tgfOamLossMeasurementGreenCntrSnapshotVerification

    Set common bridge configuration (see paragraph 2.1.1);
    Set common PCL configuration (see paragraph 2.1.1);
    Set egress PCL configuration (see paragraph 2.1.1);

    Send 2 OAM LMM packet on port 1;
    Send 2 OAM LMR packet on port 2;
    Check recieved PDU header's field: MEL, LMM/LMR opcodes and billing counters.

*/
UTF_TEST_CASE_MAC(tgfOamLossMeasurementGreenCntrSnapshotVerification)
{
 /*
    1. Set VLAN and IPCL configuration
    2. Generate traffic
    3. Restore VLAN and IPCL configuration
 */
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum,
                                        UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                        UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E |
                                        UTF_LION_E | UTF_XCAT2_E | UTF_LION2_E);
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_PUMA_E);

    prvTgfDefOamGeneralConfigurationSet();

    tgfOamLossMeasurementGreenCounterSnapshotTest();

    prvTgfDefOamGeneralConfigurationRestore();
}

/*----------------------------------------------------------------------------*/
/*
    AUTODOC: Keepalive Age Verification

    2.4.1  tgfOamKeepAliveAgeState

    Set common bridge configuration (see paragraph 2.1.1);
    Set common PCL configuration (see paragraph 2.1.1);
    Set egress PCL configuration (see paragraph 2.1.1);

    Set aging daemon basic configuration

    Phase 1:
    Set IOAM entry age state = 3
    Send 1 OAM CCM packet to port 1
    Check IOAM entry age state = 0

    Phase 2:
    Set IOAM entry age state = 0
    Send 1 OAM CCM packet to port 1
    Delay 5 seconds
    Check IOAM entry age state = 0; exception type - keepalive aging;
    exception counters > 0

    Reset aging daemon basic configuration
*/
UTF_TEST_CASE_MAC(tgfOamKeepAliveAgeState)
{
 /*
    1. Set VLAN and IPCL configuration
    2. Generate traffic
    3. Restore VLAN and IPCL configuration
 */
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum,
                                        UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                        UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E |
                                        UTF_LION_E | UTF_XCAT2_E | UTF_LION2_E);
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_PUMA_E);

    prvTgfDefOamGeneralConfigurationSet();

    tgfOamKeepAliveAgeTest();

    prvTgfDefOamGeneralConfigurationRestore();
}

/*----------------------------------------------------------------------------*/
/*
    AUTODOC: Keepalive Excess Verification

    2.4.2  tgfOamKeepAliveExcessDetection

    Set common bridge configuration (see paragraph 2.1.1);
    Set common PCL configuration (see paragraph 2.1.1);
    Set egress PCL configuration (see paragraph 2.1.1);

    Set aging daemon basic configuration

    Phase 1:
    Send 8 OAM packets to port 1 with delay 1/2 second
    Check keepalive excess message counter = 7 (excess message threshold)
    Delay 5 seconds
    Check keepalive excess exception = 1
    Check keepalive excess message counter = 0 (cleared by daemon)

    Phase 2:
    Send 4 OAM packets to port 1 with delay 1 second
    Check keepalive excess message counter = 4
    Delay 5 seconds
    Check no keepalive excess exception
    Check keepalive excess message counter = 0 (cleared by daemon)

    Reset aging daemon basic configuration
*/
UTF_TEST_CASE_MAC(tgfOamKeepAliveExcessDetection)
{
 /*
    1. Set VLAN and IPCL configuration
    2. Generate traffic
    3. Restore VLAN and IPCL configuration
 */
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum,
                                        UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                        UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E |
                                        UTF_LION_E | UTF_XCAT2_E | UTF_LION2_E);
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_PUMA_E);

    PRV_TGF_SKIP_LONG_TEST_MAC(UTF_ALL_FAMILY_E);

    prvTgfDefOamGeneralConfigurationSet();

    tgfOamKeepAliveExcessDetectionTest();

    prvTgfDefOamGeneralConfigurationRestore();
}

/*----------------------------------------------------------------------------*/
/*
    AUTODOC: Keepalive RDI bit checking

    2.4.3  tgfOamKeepAliveRdiBitChecking

    Set common bridge configuration (see paragraph 2.1.1);
    Set common PCL configuration (see paragraph 2.1.1);
    Set egress PCL configuration (see paragraph 2.1.1);

    Set aging daemon basic configuration
    Enable RDI bit checking
    Phase 1:
    Set OAM packet RDI status == 1
    Send 1 packet
    Check 1 packet trapped to CPU, RDI exception == 1
    Phase 2:
    Set OAM packet RDI status == 0
    Send 1 packet
    Check 1 packet forwarded, no RDI exception
*/
UTF_TEST_CASE_MAC(tgfOamKeepAliveRdiBitChecking)
{
 /*
    1. Set VLAN and IPCL configuration
    2. Generate traffic
    3. Restore VLAN and IPCL configuration
 */
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum,
                                        UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                        UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E |
                                        UTF_LION_E | UTF_XCAT2_E | UTF_LION2_E);
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_PUMA_E);

    prvTgfDefOamGeneralConfigurationSet();

    tgfOamKeepAliveRdiBitCheckingTest();

    prvTgfDefOamGeneralConfigurationRestore();
}

/*----------------------------------------------------------------------------*/
/*
    AUTODOC: Keepalive period field checking

    2.4.4  tgfOamKeepAlivePeriodFieldChecking

    Set common bridge configuration (see paragraph 2.1.1);
    Set common PCL configuration (see paragraph 2.1.1);
    Set egress PCL configuration (see paragraph 2.1.1);

    Set aging daemon basic configuration
    Enable period field checking
    Phase 1:
    Set keepalive period field
    Send 1 packet
    Check 1 packet trapped to CPU, period field exception == 1
    Phase 2:
    Reset keepalive period field
    Check 1 packet forwarded, no period field exception
*/
UTF_TEST_CASE_MAC(tgfOamKeepAlivePeriodFieldChecking)
{
 /*
    1. Set VLAN and IPCL configuration
    2. Generate traffic
    3. Restore VLAN and IPCL configuration
 */
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum,
                                        UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                        UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E |
                                        UTF_LION_E | UTF_XCAT2_E | UTF_LION2_E);
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_PUMA_E);

    prvTgfDefOamGeneralConfigurationSet();

    tgfOamKeepAlivePeriodFieldCheckingTest();

    prvTgfDefOamGeneralConfigurationRestore();
}

/*----------------------------------------------------------------------------*/
/*
    AUTODOC: Keepalive flow hash checking

    2.4.5  tgfOamKeepAliveFlowHashChecking

    Set common bridge configuration (see paragraph 2.1.1);
    Set common PCL configuration (see paragraph 2.1.1);
    Set egress PCL configuration (see paragraph 2.1.1);

    Set aging daemon basic configuration
    Enable period field checking
    Phase 1:
    Set keepalive flow hash field
    Enable lock hash value
    Send 1 packet
    Check 1 packet trapped to CPU, invalid hash exception == 1,
    OAM new flow hash value not locked
    Phase 2:
    Set keepalive flow hash field
    Disable lock hash value
    Check 1 packet trapped to CPU, invalid hash exception == 1,
    OAM new flow hash value locked, OAM flow lock hash enabled
*/
UTF_TEST_CASE_MAC(tgfOamKeepAliveFlowHashChecking)
{
 /*
    1. Set VLAN and IPCL configuration
    2. Generate traffic
    3. Restore VLAN and IPCL configuration
 */
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum,
                                        UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                        UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E |
                                        UTF_LION_E | UTF_XCAT2_E | UTF_LION2_E);
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_PUMA_E);

    /* skip this test for all families under Golden Model */
    PRV_TGF_SKIP_LONG_TEST_GM_MAC(UTF_ALL_FAMILY_E);

    prvTgfDefOamGeneralConfigurationSet();

    tgfOamKeepAliveFlowHashCheckingTest();

    prvTgfDefOamGeneralConfigurationRestore();
}

UTF_TEST_CASE_MAC(tgfIOamKeepAliveFlowHashVerificationBitSelection)
{
    /*
        1. Configure Keepalive engine
        2. Do the test
        2.1 Change src MAC to change Flow ID
        2.2 Set new combination of selected bits
        2.3 Send Packets
        2.4 Check if new Flow ID changed and it as expected

        Changes of src MAC designed to check the width of selected bits in hash
        start start bit
    */
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_CH1_E |
        UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E |
        UTF_LION_E | UTF_XCAT2_E | UTF_LION2_E);
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_PUMA_E);

    /* skip this test for all families under Golden Model */
    PRV_TGF_SKIP_LONG_TEST_GM_MAC(UTF_ALL_FAMILY_E);

    prvTgfDefOamGeneralConfigurationSet();

    tgfIOamKeepAliveFlowHashVerificationBitSelectionTest();

    prvTgfDefOamGeneralConfigurationRestore();
}

UTF_TEST_CASE_MAC(tgfEOamKeepAliveFlowHashVerificationBitSelection)
{
    /*
        1. Configure egress keepalive engine
        2. Do the test
        2.1 Change src MAC to change Flow ID
        2.2 Set expected Flow ID to MAC entry
        2.3 Send Packets
        2.4 Check if new Flow ID is as expected

        Changes of src MAC designed to check the width of selected bits in hash
        start start bit
    */
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_CH1_E |
        UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E |
        UTF_LION_E | UTF_XCAT2_E | UTF_LION2_E | UTF_BOBCAT2_E);
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_PUMA_E);

    /* skip this test for all families under Golden Model */
    PRV_TGF_SKIP_LONG_TEST_GM_MAC(UTF_ALL_FAMILY_E);

    tgfEOamKeepAliveFlowHashVerificationBitSelectionConfigSet();

    tgfEOamKeepAliveFlowHashVerificationBitSelectionTest();

    tgfEOamKeepAliveFlowHashVerificationBitSelectionRestore();
}

/*
 * Configuration of tgfOam suit
 */
UTF_SUIT_BEGIN_TESTS_MAC(tgfOam)
    UTF_SUIT_DECLARE_TEST_MAC(tgfOamMegLevelCheck)
    UTF_SUIT_DECLARE_TEST_MAC(tgfOamSourceInterfaceVerification)
    UTF_SUIT_DECLARE_TEST_MAC(tgfOamDualEndedLossMeasurementPacketCommand)
    UTF_SUIT_DECLARE_TEST_MAC(tgfOamDualEndedLossMeasurementPacketCommand_useTtiOamProfile)
    UTF_SUIT_DECLARE_TEST_MAC(tgfOamDelayMeasurementVerification)
    UTF_SUIT_DECLARE_TEST_MAC(tgfOamLossMeasurementVerification)
    UTF_SUIT_DECLARE_TEST_MAC(tgfOamLossMeasurementGreenCntrSnapshotVerification)
    UTF_SUIT_DECLARE_TEST_MAC(tgfOamKeepAliveAgeState)
    UTF_SUIT_DECLARE_TEST_MAC(tgfOamKeepAliveExcessDetection)
    UTF_SUIT_DECLARE_TEST_MAC(tgfOamKeepAliveRdiBitChecking)
    UTF_SUIT_DECLARE_TEST_MAC(tgfOamKeepAlivePeriodFieldChecking)
    UTF_SUIT_DECLARE_TEST_MAC(tgfOamKeepAliveFlowHashChecking)
    UTF_SUIT_DECLARE_TEST_MAC(tgfIOamKeepAliveFlowHashVerificationBitSelection)
    UTF_SUIT_DECLARE_TEST_MAC(tgfEOamKeepAliveFlowHashVerificationBitSelection)
UTF_SUIT_END_TESTS_MAC(tgfOam)


