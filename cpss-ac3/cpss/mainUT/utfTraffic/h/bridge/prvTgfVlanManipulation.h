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
* @file prvTgfVlanManipulation.h
*
* @brief VLAN Manipulation
*
* @version   9
********************************************************************************
*/
#ifndef __prvTgfVlanManipulation
#define __prvTgfVlanManipulation

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <trafficEngine/tgfTrafficEngine.h>

/**
* @internal prvTgfVlanManipulationTestAsDsa function
* @endinternal
*
* @brief   set the tests to be with egress DSA tag (extended DSA/eDSA) on the egress ports.
*
* @param[in] egressDsaType            - type of DSA tag on egress port
*                                       None
*/
GT_STATUS prvTgfVlanManipulationTestAsDsa
(
    TGF_DSA_TYPE_ENT    egressDsaType
);

/**
* @internal prvTgfBrgVlanManipulationUntaggedCmdDoubleTagConfigurationSet function
* @endinternal
*
* @brief   Set test configuration:
*         - Set ingress TPID table to: Ethertype0 = 0x8100, Ethertype1 = 0x88a8
*         - Set ingress TPID select to: TAG0 is Ethertype0, TAG1 is Ethertype1.
*         - Build VLAN Entry with VID = 5 and portsMember = 0, 8, 18, 23
*         and "Untagged" tagging command for all ports in VLAN.
*         - Build VLAN Entry with VID = 10 and portsMember = 0, 8, 18, 23 and
*         tagging command "TAG0" for all ports in VLAN to be sure that
*         decision is according to VID = 5.
*         - Set egress TPID table to: Ethertype0 = 0x9100, Ethertype1 = 0x98a8
*         - Set egress TPID select to: TAG0 is Ethertype0, TAG1 is Ethertype1.
* @param[in] isWithErrorInjection     - indication that the test need to run with 'error
*                                      injection' to vlan memories
*                                       None
*/
GT_VOID prvTgfBrgVlanManipulationUntaggedCmdDoubleTagConfigurationSet
(
    GT_BOOL isWithErrorInjection
);

/**
* @internal prvTgfBrgVlanManipulationUntaggedCmdDoubleTagTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic:
*         Send to device's port 0 packet:
*         macDa = 00:00:00:00:00:02,
*         macSa = 00:00:00:00:00:12,
*         TAG0: ethertype = 0x8100, vlan = 5
*         TAG1: ethertype = 0x88a8, vlan = 10
*         Success Criteria:
*         Output packet has no any tags
*/
GT_VOID prvTgfBrgVlanManipulationUntaggedCmdDoubleTagTrafficGenerate
(
    GT_VOID
);

/**
* @internal prvTgfBrgVlanManipulationUntaggedCmdDoubleTagConfigurationRestore function
* @endinternal
*
* @brief   Restore test configuration
*/
GT_VOID prvTgfBrgVlanManipulationUntaggedCmdDoubleTagConfigurationRestore
(
    GT_VOID
);

/**
* @internal prvTgfBrgVlanManipulationUntaggedCmdSingleTagConfigurationSet function
* @endinternal
*
* @brief   Set test configuration:
*         - Set ingress TPID table to: Ethertype0 = 0x8100, Ethertype1 = 0x88a8
*         - Set ingress TPID select to: TAG0 is Ethertype0, TAG1 is Ethertype1.
*         - Build VLAN Entry with VID = 5 and portsMember = 0, 8, 18, 23
*         and "Untagged" tagging command for all ports in VLAN.
*         - Set egress TPID table to: Ethertype0 = 0x9100, Ethertype1 = 0x98a8
*         - Set egress TPID select to: TAG0 is Ethertype0, TAG1 is Ethertype1.
*/
GT_VOID prvTgfBrgVlanManipulationUntaggedCmdSingleTagConfigurationSet
(
    GT_VOID
);

/**
* @internal prvTgfBrgVlanManipulationUntaggedCmdSingleTagTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic:
*         Send to device's port 0 packet:
*         macDa = 00:00:00:00:00:02,
*         macSa = 00:00:00:00:00:12,
*         TAG0: ethertype = 0x8100, vlan = 5
*         Success Criteria:
*         Output packet has no any tags
*/
GT_VOID prvTgfBrgVlanManipulationUntaggedCmdSingleTagTrafficGenerate
(
    GT_VOID
);

/**
* @internal prvTgfBrgVlanManipulationUntaggedCmdSingleTagConfigurationRestore function
* @endinternal
*
* @brief   Restore test configuration
*/
GT_VOID prvTgfBrgVlanManipulationUntaggedCmdSingleTagConfigurationRestore
(
    GT_VOID
);

/**
* @internal prvTgfBrgVlanManipulationUntaggedCmdUntagConfigurationSet function
* @endinternal
*
* @brief   Set test configuration:
*         - Set ingress TPID table to: Ethertype0 = 0x8100, Ethertype1 = 0x88a8
*         - Set ingress TPID select to: TAG0 is Ethertype0, TAG1 is Ethertype1.
*         - Build VLAN Entry with VID = 5 and portsMember = 0, 8, 18, 23
*         and "Untagged" tagging command for all ports in VLAN.
*         - Set egress TPID table to: Ethertype0 = 0x9100, Ethertype1 = 0x98a8
*         - Set egress TPID select to: TAG0 is Ethertype0, TAG1 is Ethertype1.
*/
GT_VOID prvTgfBrgVlanManipulationUntaggedCmdUntagConfigurationSet
(
    GT_VOID
);

/**
* @internal prvTgfBrgVlanManipulationUntaggedCmdUntagTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic:
*         Send to device's port 0 packet:
*         macDa = 00:00:00:00:00:02,
*         macSa = 00:00:00:00:00:12
*         Success Criteria:
*         Output packet has no any tags
*/
GT_VOID prvTgfBrgVlanManipulationUntaggedCmdUntagTrafficGenerate
(
    GT_VOID
);

/**
* @internal prvTgfBrgVlanManipulationUntaggedCmdUntagConfigurationRestore function
* @endinternal
*
* @brief   Restore test configuration
*/
GT_VOID prvTgfBrgVlanManipulationUntaggedCmdUntagConfigurationRestore
(
    GT_VOID
);

/**
* @internal prvTgfBrgVlanManipulationTag0CmdDoubleTagConfigurationSet function
* @endinternal
*
* @brief   Set test configuration:
*         - Set ingress TPID table to: Ethertype0 = 0x8100, Ethertype1 = 0x88a8
*         - Set ingress TPID select to: TAG0 is Ethertype0, TAG1 is Ethertype1.
*         - Build VLAN Entry with VID = 25 and portsMember = 0, 8, 18, 23
*         and "TAG0" tagging command for all ports in VLAN.
*         - Set egress TPID table to: Ethertype0 = 0x9100, Ethertype1 = 0x98a8
*         - Set egress TPID select to: TAG0 is Ethertype0, TAG1 is Ethertype1.
*/
GT_VOID prvTgfBrgVlanManipulationTag0CmdDoubleTagConfigurationSet
(
    GT_VOID
);

/**
* @internal prvTgfBrgVlanManipulationTag0CmdDoubleTagTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic:
*         Send to device's port 0 packet:
*         macDa = 00:00:00:00:00:02,
*         macSa = 00:00:00:00:00:12,
*         TAG0: ethertype = 0x8100, vlan = 25
*         TAG1: ethertype = 0x88a8, vlan = 4095
*         Success Criteria:
*         Output packet has one tag TAG0 with
*         ethertype = 0x9100, vlan = 5
*/
GT_VOID prvTgfBrgVlanManipulationTag0CmdDoubleTagTrafficGenerate
(
    GT_VOID
);

/**
* @internal prvTgfBrgVlanManipulationTag0CmdDoubleTagConfigurationRestore function
* @endinternal
*
* @brief   Restore test configuration
*/
GT_VOID prvTgfBrgVlanManipulationTag0CmdDoubleTagConfigurationRestore
(
    GT_VOID
);

/**
* @internal prvTgfBrgVlanManipulationTag0CmdSingleTagConfigurationSet function
* @endinternal
*
* @brief   Set test configuration:
*         - Set ingress TPID table to: Ethertype0 = 0x8100, Ethertype1 = 0x88a8
*         - Set ingress TPID select to: TAG0 is Ethertype0, TAG1 is Ethertype1.
*         - Build VLAN Entry with VID = 5 and portsMember = 0, 8, 18, 23
*         and "TAG0" tagging command for all ports in VLAN.
*         - Set egress TPID table to: Ethertype0 = 0x9100, Ethertype1 = 0x98a8
*         - Set egress TPID select to: TAG0 is Ethertype0, TAG1 is Ethertype1.
*/
GT_VOID prvTgfBrgVlanManipulationTag0CmdSingleTagConfigurationSet
(
    GT_VOID
);

/**
* @internal prvTgfBrgVlanManipulationTag0CmdSingleTagTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic:
*         Send to device's port 0 packet:
*         macDa = 00:00:00:00:00:02,
*         macSa = 00:00:00:00:00:12,
*         TAG0: ethertype = 0x8100, vlan = 5
*         Success Criteria:
*         Output packet has one tag TAG0 with
*         ethertype = 0x9100, vlan = 5
*/
GT_VOID prvTgfBrgVlanManipulationTag0CmdSingleTagTrafficGenerate
(
    GT_VOID
);

/**
* @internal prvTgfBrgVlanManipulationTag0CmdSingleTagConfigurationRestore function
* @endinternal
*
* @brief   Restore test configuration
*/
GT_VOID prvTgfBrgVlanManipulationTag0CmdSingleTagConfigurationRestore
(
    GT_VOID
);

/**
* @internal prvTgfBrgVlanManipulationTag0CmdUntagConfigurationSet function
* @endinternal
*
* @brief   Set test configuration:
*         - Set ingress TPID table to: Ethertype0 = 0x8100, Ethertype1 = 0x88a8
*         - Set ingress TPID select to: TAG0 is Ethertype0, TAG1 is Ethertype1.
*         - Build VLAN Entry with VID = 5 and portsMember = 0, 8, 18, 23
*         and "TAG0" tagging command for all ports in VLAN.
*         - Set egress TPID table to: Ethertype0 = 0x9100, Ethertype1 = 0x98a8
*         - Set egress TPID select to: TAG0 is Ethertype0, TAG1 is Ethertype1.
*/
GT_VOID prvTgfBrgVlanManipulationTag0CmdUntagConfigurationSet
(
    GT_VOID
);

/**
* @internal prvTgfBrgVlanManipulationTag0CmdUntagTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic:
*         Send to device's port 0 packet:
*         macDa = 00:00:00:00:00:02,
*         macSa = 00:00:00:00:00:12
*         Success Criteria:
*         Output packet has one tag TAG0 with
*         ethertype = 0x9100, vlan = 5
*/
GT_VOID prvTgfBrgVlanManipulationTag0CmdUntagTrafficGenerate
(
    GT_VOID
);

/**
* @internal prvTgfBrgVlanManipulationTag0CmdUntagConfigurationRestore function
* @endinternal
*
* @brief   Restore test configuration
*/
GT_VOID prvTgfBrgVlanManipulationTag0CmdUntagConfigurationRestore
(
    GT_VOID
);

/**
* @internal prvTgfBrgVlanManipulationTag1CmdDoubleTagConfigurationSet function
* @endinternal
*
* @brief   Set test configuration:
*         - Set ingress TPID table to: Ethertype0 = 0x8100, Ethertype1 = 0x88a8
*         - Set ingress TPID select to: TAG0 is Ethertype0, TAG1 is Ethertype1.
*         - Build VLAN Entry with VID = 5 and portsMember = 0, 8, 18, 23
*         and "TAG1" tagging command for all ports in VLAN.
*         - Set egress TPID table to: Ethertype0 = 0x9100, Ethertype1 = 0x98a8
*         - Set egress TPID select to: TAG0 is Ethertype0, TAG1 is Ethertype1.
*/
GT_VOID prvTgfBrgVlanManipulationTag1CmdDoubleTagConfigurationSet
(
    GT_VOID
);

/**
* @internal prvTgfBrgVlanManipulationTag1CmdDoubleTagTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic:
*         Send to device's port 0 packet:
*         macDa = 00:00:00:00:00:02,
*         macSa = 00:00:00:00:00:12,
*         TAG0: ethertype = 0x8100, vlan = 5
*         TAG1: ethertype = 0x88a8, vlan = 10
*         Success Criteria:
*         Output packet has one tag TAG1 with
*         ethertype = 0x98a8, vlan = 10
*/
GT_VOID prvTgfBrgVlanManipulationTag1CmdDoubleTagTrafficGenerate
(
    GT_VOID
);

/**
* @internal prvTgfBrgVlanManipulationTag1CmdDoubleTagConfigurationRestore function
* @endinternal
*
* @brief   Restore test configuration
*/
GT_VOID prvTgfBrgVlanManipulationTag1CmdDoubleTagConfigurationRestore
(
    GT_VOID
);

/**
* @internal prvTgfBrgVlanManipulationTag1CmdSingleTagConfigurationSet function
* @endinternal
*
* @brief   Set test configuration:
*         - Set ingress TPID table to: Ethertype0 = 0x8100, Ethertype1 = 0x88a8
*         - Set ingress TPID select to: TAG0 is Ethertype0, TAG1 is Ethertype1.
*         - Build VLAN Entry with VID = 5 and portsMember = 0, 8, 18, 23
*         and "TAG1" tagging command for all ports in VLAN.
*         - Set egress TPID table to: Ethertype0 = 0x9100, Ethertype1 = 0x98a8
*         - Set egress TPID select to: TAG0 is Ethertype0, TAG1 is Ethertype1.
*/
GT_VOID prvTgfBrgVlanManipulationTag1CmdSingleTagConfigurationSet
(
    GT_VOID
);

/**
* @internal prvTgfBrgVlanManipulationTag1CmdSingleTagTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic:
*         Send to device's port 0 packet:
*         macDa = 00:00:00:00:00:02,
*         macSa = 00:00:00:00:00:12,
*         TAG0: ethertype = 0x8100, vlan = 5
*         Success Criteria:
*         Output packet has one tag TAG1 with
*         ethertype = 0x98a8, vlan = 10
*/
GT_VOID prvTgfBrgVlanManipulationTag1CmdSingleTagTrafficGenerate
(
    GT_VOID
);

/**
* @internal prvTgfBrgVlanManipulationTag1CmdSingleTagConfigurationRestore function
* @endinternal
*
* @brief   Restore test configuration
*/
GT_VOID prvTgfBrgVlanManipulationTag1CmdSingleTagConfigurationRestore
(
    GT_VOID
);

/**
* @internal prvTgfBrgVlanManipulationTag1CmdUntagConfigurationSet function
* @endinternal
*
* @brief   Set test configuration:
*         - Set ingress TPID table to: Ethertype0 = 0x8100, Ethertype1 = 0x88a8
*         - Set ingress TPID select to: TAG0 is Ethertype0, TAG1 is Ethertype1.
*         - Build VLAN Entry with VID = 5 and portsMember = 0, 8, 18, 23
*         and "TAG1" tagging command for all ports in VLAN.
*         - Set egress TPID table to: Ethertype0 = 0x9100, Ethertype1 = 0x98a8
*         - Set egress TPID select to: TAG0 is Ethertype0, TAG1 is Ethertype1.
*/
GT_VOID prvTgfBrgVlanManipulationTag1CmdUntagConfigurationSet
(
    GT_VOID
);

/**
* @internal prvTgfBrgVlanManipulationTag1CmdUntagTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic:
*         Send to device's port 0 packet:
*         macDa = 00:00:00:00:00:02,
*         macSa = 00:00:00:00:00:12
*         Success Criteria:
*         Output packet has one tag TAG1 with
*         ethertype = 0x98a8, vlan = 10
*/
GT_VOID prvTgfBrgVlanManipulationTag1CmdUntagTrafficGenerate
(
    GT_VOID
);

/**
* @internal prvTgfBrgVlanManipulationTag1CmdUntagConfigurationRestore function
* @endinternal
*
* @brief   Restore test configuration
*/
GT_VOID prvTgfBrgVlanManipulationTag1CmdUntagConfigurationRestore
(
    GT_VOID
);

/**
* @internal prvTgfBrgVlanManipulationOuterTag0InnerTag1CmdDoubleTagConfigurationSet function
* @endinternal
*
* @brief   Set test configuration:
*         - Set ingress TPID table to: Ethertype0 = 0x8100, Ethertype1 = 0x88a8
*         - Set ingress TPID select to: TAG0 is Ethertype0, TAG1 - is Ethertype1.
*         - Build VLAN Entry with VID = 5 and portsMember = 0, 8, 18, 23
*         and tagging command "Outer Tag0, Inner Tag1" for all ports in VLAN.
*         - Set egress TPID table to: Ethertype0 = 0x9100, Ethertype1 = 0x98a8
*         - Set egress TPID select to: TAG0 is Ethertype0, TAG1 - is Ethertype1.
*/
GT_VOID prvTgfBrgVlanManipulationOuterTag0InnerTag1CmdDoubleTagConfigurationSet
(
    GT_VOID
);

/**
* @internal prvTgfBrgVlanManipulationOuterTag0InnerTag1CmdDoubleTagTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic:
*         Send to device's port 0 packet:
*         macDa = 00:00:00:00:00:02,
*         macSa = 00:00:00:00:00:12,
*         TAG1: ethertype = 0x88a8, vlan = 10
*         TAG0: ethertype = 0x8100, vlan = 5
*         Success Criteria:
*         Output packet has two tags in the following order: TAG0 with
*         ethertype = 0x9100, vlan = 5 and TAG1 with ethertype = 0x98a8,
*         vlan = 10
*/
GT_VOID prvTgfBrgVlanManipulationOuterTag0InnerTag1CmdDoubleTagTrafficGenerate
(
    GT_VOID
);

/**
* @internal prvTgfBrgVlanManipulationOuterTag0InnerTag1CmdDoubleTagConfigurationRestore function
* @endinternal
*
* @brief   Restore test configuration
*/
GT_VOID prvTgfBrgVlanManipulationOuterTag0InnerTag1CmdDoubleTagConfigurationRestore
(
    GT_VOID
);

/**
* @internal prvTgfBrgVlanManipulationOuterTag0InnerTag1CmdSingleTagConfigurationSet function
* @endinternal
*
* @brief   Set test configuration:
*         - Set ingress TPID table to: Ethertype0 = 0x8100, Ethertype1 = 0x88a8
*         - Set ingress TPID select to: TAG0 is Ethertype0, TAG1 - is Ethertype1.
*         - Build VLAN Entry with VID = 5 and portsMember = 0, 8, 18, 23
*         and tagging command "Outer Tag0, Inner Tag1" for all ports in VLAN.
*         - Set egress TPID table to: Ethertype0 = 0x9100, Ethertype1 = 0x98a8
*         - Set egress TPID select to: TAG0 is Ethertype0, TAG1 - is Ethertype1.
*/
GT_VOID prvTgfBrgVlanManipulationOuterTag0InnerTag1CmdSingleTagConfigurationSet
(
    GT_VOID
);

/**
* @internal prvTgfBrgVlanManipulationOuterTag0InnerTag1CmdSingleTagTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic:
*         Send to device's port 0 packet:
*         macDa = 00:00:00:00:00:02,
*         macSa = 00:00:00:00:00:12,
*         TAG0: ethertype = 0x8100, vlan = 5
*         Success Criteria:
*         Output packet has two tags in the following order: TAG0 with
*         ethertype = 0x9100, vlan = 5 and TAG1 with ethertype = 0x98a8,
*         vlan = 0
*/
GT_VOID prvTgfBrgVlanManipulationOuterTag0InnerTag1CmdSingleTagTrafficGenerate
(
    GT_VOID
);

/**
* @internal prvTgfBrgVlanManipulationOuterTag0InnerTag1CmdSingleTagConfigurationRestore function
* @endinternal
*
* @brief   Restore test configuration
*/
GT_VOID prvTgfBrgVlanManipulationOuterTag0InnerTag1CmdSingleTagConfigurationRestore
(
    GT_VOID
);

/**
* @internal prvTgfBrgVlanManipulationOuterTag0InnerTag1CmdUntagConfigurationSet function
* @endinternal
*
* @brief   Set test configuration:
*         - Set ingress TPID table to: Ethertype0 = 0x8100, Ethertype1 = 0x88a8
*         - Set ingress TPID select to: TAG0 is Ethertype0, TAG1 - is Ethertype1.
*         - Build VLAN Entry with VID = 5 and portsMember = 0, 8, 18, 23
*         and tagging command "Outer Tag0, Inner Tag1" for all ports in VLAN.
*         - Set PVID to 5 - for untagged packet
*         - Set egress TPID table to: Ethertype0 = 0x9100, Ethertype1 = 0x98a8
*         - Set egress TPID select to: TAG0 is Ethertype0, TAG1 - is Ethertype1.
*/
GT_VOID prvTgfBrgVlanManipulationOuterTag0InnerTag1CmdUntagConfigurationSet
(
    GT_VOID
);

/**
* @internal prvTgfBrgVlanManipulationOuterTag0InnerTag1CmdUntagTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic:
*         Send to device's port 0 packet:
*         macDa = 00:00:00:00:00:02,
*         macSa = 00:00:00:00:00:12,
*         Success Criteria:
*         Output packet has two tags in the following order: TAG0 with
*         ethertype = 0x9100, vlan = 5 and TAG1 with ethertype = 0x98a8,
*         vlan = 0
*/
GT_VOID prvTgfBrgVlanManipulationOuterTag0InnerTag1CmdUntagTrafficGenerate
(
    GT_VOID
);

/**
* @internal prvTgfBrgVlanManipulationOuterTag0InnerTag1CmdUntagConfigurationRestore function
* @endinternal
*
* @brief   Restore test configuration
*/
GT_VOID prvTgfBrgVlanManipulationOuterTag0InnerTag1CmdUntagConfigurationRestore
(
    GT_VOID
);

/**
* @internal prvTgfBrgVlanManipulationOuterTag1InnerTag0CmdDoubleTagConfigurationSet function
* @endinternal
*
* @brief   Set test configuration:
*         - Set ingress TPID table to: Ethertype0 = 0x8100, Ethertype1 = 0x88a8
*         - Set ingress TPID select to: TAG0 is Ethertype0, TAG1 is Ethertype1.
*         - Build VLAN Entry with VID = 5 and portsMember = 0, 8, 18, 23
*         and "Outer Tag1, Inner Tag0" tagging command for all ports in VLAN.
*         - Set egress TPID table to: Ethertype0 = 0x9100, Ethertype1 = 0x98a8
*         - Set egress TPID select to: TAG0 is Ethertype0, TAG1 is Ethertype1.
*/
GT_VOID prvTgfBrgVlanManipulationOuterTag1InnerTag0CmdDoubleTagConfigurationSet
(
    GT_VOID
);

/**
* @internal prvTgfBrgVlanManipulationOuterTag1InnerTag0CmdDoubleTagTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic:
*         Send to device's port 0 packet:
*         macDa = 00:00:00:00:00:02,
*         macSa = 00:00:00:00:00:12,
*         TAG1: ethertype = 0x88a8, vlan = 10
*         TAG0: ethertype = 0x8100, vlan = 5
*         Success Criteria:
*         Output packet has two tags in the following order: TAG1 with
*         ethertype = 0x98a8, vlan = 10 and TAG0 with ethertype = 0x9100,
*         vlan = 5.
*/
GT_VOID prvTgfBrgVlanManipulationOuterTag1InnerTag0CmdDoubleTagTrafficGenerate
(
    GT_VOID
);

/**
* @internal prvTgfBrgVlanManipulationOuterTag1InnerTag0CmdDoubleTagConfigurationRestore function
* @endinternal
*
* @brief   Restore test configuration
*/
GT_VOID prvTgfBrgVlanManipulationOuterTag1InnerTag0CmdDoubleTagConfigurationRestore
(
    GT_VOID
);

/**
* @internal prvTgfBrgVlanManipulationOuterTag1InnerTag0CmdSingleTagConfigurationSet function
* @endinternal
*
* @brief   Set test configuration:
*         - Set ingress TPID table to: Ethertype0 = 0x8100, Ethertype1 = 0x88a8
*         - Set ingress TPID select to: TAG0 is Ethertype0, TAG1 - is Ethertype1.
*         - Build VLAN Entry with VID = 5 and portsMember = 0, 8, 18, 23
*         and tagging command "Outer Tag1, Inner Tag0" for all ports in VLAN.
*         - Set egress TPID table to: Ethertype0 = 0x9100, Ethertype1 = 0x98a8
*         - Set egress TPID select to: TAG0 is Ethertype0, TAG1 - is Ethertype1.
*/
GT_VOID prvTgfBrgVlanManipulationOuterTag1InnerTag0CmdSingleTagConfigurationSet
(
    GT_VOID
);

/**
* @internal prvTgfBrgVlanManipulationOuterTag1InnerTag0CmdSingleTagTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic:
*         Send to device's port 0 packet:
*         macDa = 00:00:00:00:00:02,
*         macSa = 00:00:00:00:00:12,
*         TAG0: ethertype = 0x8100, vlan = 5
*         Success Criteria:
*         Output packet has two tags in the following order: TAG1 with
*         ethertype = 0x98a8, vlan = 0 and TAG0 with ethertype = 0x9100,
*         vlan = 5
*/
GT_VOID prvTgfBrgVlanManipulationOuterTag1InnerTag0CmdSingleTagTrafficGenerate
(
    GT_VOID
);

/**
* @internal prvTgfBrgVlanManipulationOuterTag1InnerTag0CmdSingleTagConfigurationRestore function
* @endinternal
*
* @brief   Restore test configuration
*/
GT_VOID prvTgfBrgVlanManipulationOuterTag1InnerTag0CmdSingleTagConfigurationRestore
(
    GT_VOID
);

/**
* @internal prvTgfBrgVlanManipulationOuterTag1InnerTag0CmdUntagConfigurationSet function
* @endinternal
*
* @brief   Set test configuration:
*         - Set ingress TPID table to: Ethertype0 = 0x8100, Ethertype1 = 0x88a8
*         - Set ingress TPID select to: TAG0 is Ethertype0, TAG1 - is Ethertype1.
*         - Build VLAN Entry with VID = 5 and portsMember = 0, 8, 18, 23
*         and tagging command "Outer Tag1, Inner Tag0" for all ports in VLAN.
*         - Set PVID to 5 - for untagged packet
*         - Set egress TPID table to: Ethertype0 = 0x9100, Ethertype1 = 0x98a8
*         - Set egress TPID select to: TAG0 is Ethertype0, TAG1 - is Ethertype1.
*/
GT_VOID prvTgfBrgVlanManipulationOuterTag1InnerTag0CmdUntagConfigurationSet
(
    GT_VOID
);

/**
* @internal prvTgfBrgVlanManipulationOuterTag1InnerTag0CmdUntagTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic:
*         Send to device's port 0 packet:
*         macDa = 00:00:00:00:00:02,
*         macSa = 00:00:00:00:00:12,
*         Success Criteria:
*         Output packet has two tags in the following order: TAG1 with
*         ethertype = 0x98a8, vlan = 0 and TAG0 with ethertype = 0x9100,
*         vlan = 5
*/
GT_VOID prvTgfBrgVlanManipulationOuterTag1InnerTag0CmdUntagTrafficGenerate
(
    GT_VOID
);

/**
* @internal prvTgfBrgVlanManipulationOuterTag1InnerTag0CmdUntagConfigurationRestore function
* @endinternal
*
* @brief   Restore test configuration
*/
GT_VOID prvTgfBrgVlanManipulationOuterTag1InnerTag0CmdUntagConfigurationRestore
(
    GT_VOID
);

/**
* @internal prvTgfBrgVlanManipulationPushTag0CmdDoubleTagConfigurationSet function
* @endinternal
*
* @brief   Set test configuration:
*         - Set ingress TPID table to: Ethertype0 = 0x8100, Ethertype1 = 0x88a8
*         - Set ingress TPID select to: TAG0 is Ethertype0, TAG1 is Ethertype1
*         - Build VLAN Entry with VID = 5 and portsMember = 0, 8, 18, 23 and
*         tagging command "Push Tag0" for all ports in VLAN.
*         - Set egress TPID table to: Ethertype0 = 0x9100, Ethertype1 = 0x98a8
*         - Set egress TPID select to: TAG0 is Ethertype0, TAG1 is Ethertype1
*/
GT_VOID prvTgfBrgVlanManipulationPushTag0CmdDoubleTagConfigurationSet
(
    GT_VOID
);

/**
* @internal prvTgfBrgVlanManipulationPushTag0CmdDoubleTagTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic:
*         Send to device's port 0 packet:
*         macDa = 00:00:00:00:00:02,
*         macSa = 00:00:00:00:00:12,
*         TAG0: ethertype = 0x8100, vlan = 5
*         TAG1: ethertype = 0x88a8, vlan = 10
*         Success Criteria:
*         Output packet has tree tags in the following order: TAG0 with
*         ethertype = 0x9100, vlan = 5 and TAG0 with ethertype = 0x8100,
*         vlan = 5, TAG1 with ethertype = 0x88a8, vlan = 10.
*/
GT_VOID prvTgfBrgVlanManipulationPushTag0CmdDoubleTagTrafficGenerate
(
    GT_VOID
);

/**
* @internal prvTgfBrgVlanManipulationPushTag0CmdDoubleTagConfigurationRestore function
* @endinternal
*
* @brief   Restore test configuration
*/
GT_VOID prvTgfBrgVlanManipulationPushTag0CmdDoubleTagConfigurationRestore
(
    GT_VOID
);

/**
* @internal prvTgfBrgVlanManipulationPushTag0CmdSingleTagConfigurationSet function
* @endinternal
*
* @brief   Set test configuration:
*         - Set ingress TPID table to: Ethertype0 = 0x8100, Ethertype1 = 0x88a8
*         - Set ingress TPID select to: TAG0 is Ethertype0, TAG1 - is Ethertype1.
*         - Build VLAN Entry with VID = 5 and portsMember = 0, 8, 18, 23
*         and tagging command "Push Tag0" for all ports in VLAN.
*         - Set egress TPID table to: Ethertype0 = 0x9100, Ethertype1 = 0x98a8
*         - Set egress TPID select to: TAG0 is Ethertype0, TAG1 - is Ethertype1.
*/
GT_VOID prvTgfBrgVlanManipulationPushTag0CmdSingleTagConfigurationSet
(
    GT_VOID
);

/**
* @internal prvTgfBrgVlanManipulationPushTag0CmdSingleTagTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic:
*         Send to device's port 0 packet:
*         macDa = 00:00:00:00:00:02,
*         macSa = 00:00:00:00:00:12,
*         TAG0: ethertype = 0x8100, vlan = 5
*         Success Criteria:
*         Output packet has two tags in the following order: TAG0 with
*         ethertype = 0x9100, vlan = 5 and TAG0 with ethertype = 0x8100,
*         vlan = 5
*/
GT_VOID prvTgfBrgVlanManipulationPushTag0CmdSingleTagTrafficGenerate
(
    GT_VOID
);

/**
* @internal prvTgfBrgVlanManipulationPushTag0CmdSingleTagConfigurationRestore function
* @endinternal
*
* @brief   Restore test configuration
*/
GT_VOID prvTgfBrgVlanManipulationPushTag0CmdSingleTagConfigurationRestore
(
    GT_VOID
);

/**
* @internal prvTgfBrgVlanManipulationPushTag0CmdUntagConfigurationSet function
* @endinternal
*
* @brief   Set test configuration:
*         - Set ingress TPID table to: Ethertype0 = 0x8100, Ethertype1 = 0x88a8
*         - Set ingress TPID select to: TAG0 is Ethertype0, TAG1 - is Ethertype1.
*         - Build VLAN Entry with VID = 5 and portsMember = 0, 8, 18, 23
*         and tagging command "Push Tag0" for all ports in VLAN.
*         - Set egress TPID table to: Ethertype0 = 0x9100, Ethertype1 = 0x98a8
*         - Set egress TPID select to: TAG0 is Ethertype0, TAG1 - is Ethertype1.
*/
GT_VOID prvTgfBrgVlanManipulationPushTag0CmdUntagConfigurationSet
(
    GT_VOID
);

/**
* @internal prvTgfBrgVlanManipulationPushTag0CmdUntagTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic:
*         Send to device's port 0 packet:
*         macDa = 00:00:00:00:00:02,
*         macSa = 00:00:00:00:00:12,
*         Success Criteria:
*         Output packet has one tag: TAG0 with ethertype = 0x9100, vlan = 5
*/
GT_VOID prvTgfBrgVlanManipulationPushTag0CmdUntagTrafficGenerate
(
    GT_VOID
);

/**
* @internal prvTgfBrgVlanManipulationPushTag0CmdUntagConfigurationRestore function
* @endinternal
*
* @brief   Restore test configuration
*/
GT_VOID prvTgfBrgVlanManipulationPushTag0CmdUntagConfigurationRestore
(
    GT_VOID
);

/**
* @internal prvTgfBrgVlanManipulationPopOuterTagCmdDoubleTagConfigurationSet function
* @endinternal
*
* @brief   Set test configuration:
*         - Set ingress TPID table to: Ethertype0 = 0x8100, Ethertype1 = 0x88a8
*         - Set ingress TPID select to: TAG0 is Ethertype0, TAG1 is Ethertype1
*         - Build VLAN Entry with VID = 5 and portsMember = 0, 8, 18, 23 and
*         tagging command "Pop Outer Tag" for all ports in VLAN.
*         - Build VLAN Entry with VID = 10 and portsMember = 0, 8, 18, 23 and
*         tagging command "UNTAGGED" for all ports in VLAN to be sure that
*         decision is according to VID = 5.
*         - Set egress TPID table to: Ethertype0 = 0x9100, Ethertype1 = 0x98a8
*         - Set egress TPID select to: TAG0 is Ethertype0, TAG1 is Ethertype1
*/
GT_VOID prvTgfBrgVlanManipulationPopOuterTagCmdDoubleTagConfigurationSet
(
    GT_VOID
);

/**
* @internal prvTgfBrgVlanManipulationPopOuterTagCmdDoubleTagTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic:
*         Send to device's port 0 packet:
*         macDa = 00:00:00:00:00:02,
*         macSa = 00:00:00:00:00:12,
*         TAG0: ethertype = 0x8100, vlan = 5
*         TAG1: ethertype = 0x88a8, vlan = 10
*         Success Criteria:
*         Output packet has one tag: TAG1 with
*         ethertype = 0x98a8,vlan = 10.
*/
GT_VOID prvTgfBrgVlanManipulationPopOuterTagCmdDoubleTagTrafficGenerate
(
    GT_VOID
);

/**
* @internal prvTgfBrgVlanManipulationPopOuterTagCmdDoubleTagConfigurationRestore function
* @endinternal
*
* @brief   Restore test configuration
*/
GT_VOID prvTgfBrgVlanManipulationPopOuterTagCmdDoubleTagConfigurationRestore
(
    GT_VOID
);

/**
* @internal prvTgfBrgVlanManipulationPopOuterTagCmdSingleTagConfigurationSet function
* @endinternal
*
* @brief   Set test configuration:
*         - Set ingress TPID table to: Ethertype0 = 0x8100, Ethertype1 = 0x88a8
*         - Set ingress TPID select to: TAG0 is Ethertype0, TAG1 - is Ethertype1.
*         - Build VLAN Entry with VID = 5 and portsMember = 0, 8, 18, 23
*         and tagging command "Pop Outer Tag" for all ports in VLAN.
*         - Set egress TPID table to: Ethertype0 = 0x9100, Ethertype1 = 0x98a8
*         - Set egress TPID select to: TAG0 is Ethertype0, TAG1 - is Ethertype1.
*/
GT_VOID prvTgfBrgVlanManipulationPopOuterTagCmdSingleTagConfigurationSet
(
    GT_VOID
);

/**
* @internal prvTgfBrgVlanManipulationPopOuterTagCmdSingleTagTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic:
*         Send to device's port 0 packet:
*         macDa = 00:00:00:00:00:02,
*         macSa = 00:00:00:00:00:12,
*         TAG0: ethertype = 0x8100, vlan = 5
*         Success Criteria:
*         Output packet is untagged
*/
GT_VOID prvTgfBrgVlanManipulationPopOuterTagCmdSingleTagTrafficGenerate
(
    GT_VOID
);

/**
* @internal prvTgfBrgVlanManipulationPopOuterTagCmdSingleTagConfigurationRestore function
* @endinternal
*
* @brief   Restore test configuration
*/
GT_VOID prvTgfBrgVlanManipulationPopOuterTagCmdSingleTagConfigurationRestore
(
    GT_VOID
);

/**
* @internal prvTgfBrgVlanManipulationIngressVlanAssignment1ConfigurationSet function
* @endinternal
*
* @brief   Set test configuration:
*         - Set ingress TPID table to:
*         Ethertype0 = 0x8100,
*         Ethertype1 = 0x88a8,
*         Ethertype2 = 0x5000,
*         Ethertype3 = 0xA0A0,
*         Ethertype4 = 0x5050,
*         Ethertype5 = 0x2525,
*         Ethertype6 = 0x5555,
*         Ethertype7 = 0xAAAA,
*         - Set ingress TPID select to: TAG0 is Ethertype0, Ethertype3, Ethertype5,
*         Ethertype6, Ethertype7, TAG1 is Ethertype1, Ethertype2, Ethertype4.
*         - Build VLAN Entry with VID = 5 and portsMember = 0, 8, 18, 23
*         and "TAG0" tagging command for all ports in VLAN.
*         - Build VLAN Entry with VID = 25 and portsMember = 0, 8, 18, 23
*         and "TAG0" tagging command for all ports in VLAN.
*         - Build VLAN Entry with VID = 4000 and portsMember = 0, 8, 18, 23
*         and "TAG0" tagging command for all ports in VLAN.
*         - Set egress TPID table to: Ethertype0 = 0x9100, Ethertype1 = 0x98a8
*         - Set egress TPID select to: TAG0 is Ethertype0, TAG1 is Ethertype1.
*/
GT_VOID prvTgfBrgVlanManipulationIngressVlanAssignment1ConfigurationSet
(
    GT_VOID
);

/**
* @internal prvTgfBrgVlanManipulationIngressVlanAssignment1TrafficGenerate function
* @endinternal
*
* @brief   Generate traffic:
*         Send to device's port 0 packet:
*         macDa = 00:00:00:00:00:02,
*         macSa = 00:00:00:00:00:12,
*         TAG0: ethertype = 0x8100, vlan = 5
*         Send to device's port 0 packet:
*         macDa = 00:00:00:00:00:03,
*         macSa = 00:00:00:00:00:13,
*         TAG0: ethertype = 0x8100, vlan = 25
*         Send to device's port 0 packet:
*         macDa = 00:00:00:00:00:04,
*         macSa = 00:00:00:00:00:14,
*         TAG0: ethertype = 0x8100, vlan = 4000
*         Success Criteria:
*         First packet have TAG0: ethertype =0x9100, vlan = 5
*         Second packet have TAG0: ethertype =0x9100, vlan = 25
*         Third packet have TAG0: ethertype =0x9100, vlan = 4000
*/
GT_VOID prvTgfBrgVlanManipulationIngressVlanAssignment1TrafficGenerate
(
    GT_VOID
);

/**
* @internal prvTgfBrgVlanManipulationIngressVlanAssignment1ConfigurationRestore function
* @endinternal
*
* @brief   Restore test configuration
*/
GT_VOID prvTgfBrgVlanManipulationIngressVlanAssignment1ConfigurationRestore
(
    GT_VOID
);

/**
* @internal prvTgfBrgVlanManipulationIngressVlanAssignment2ConfigurationSet function
* @endinternal
*
* @brief   Set test configuration:
*         - Set ingress TPID table to:
*         Ethertype0 = 0x8100,
*         Ethertype1 = 0x88a8,
*         Ethertype2 = 0x5000,
*         Ethertype3 = 0xA0A0,
*         Ethertype4 = 0x5050,
*         Ethertype5 = 0x2525,
*         Ethertype6 = 0x5555,
*         Ethertype7 = 0xAAAA,
*         - Set ingress TPID select to: TAG0 is Ethertype0, Ethertype3, Ethertype5,
*         Ethertype6, Ethertype7, TAG1 is Ethertype1, Ethertype2, Ethertype4.
*         - Build VLAN Entry with VID = 5 and portsMember = 0, 8, 18, 23
*         and "TAG1" tagging command for all ports in VLAN.
*         - Set egress TPID table to: Ethertype0 = 0x9100, Ethertype1 = 0x98a8
*         - Set egress TPID select to: TAG0 is Ethertype0, TAG1 is Ethertype1.
*/
GT_VOID prvTgfBrgVlanManipulationIngressVlanAssignment2ConfigurationSet
(
    GT_VOID
);

/**
* @internal prvTgfBrgVlanManipulationIngressVlanAssignment2TrafficGenerate function
* @endinternal
*
* @brief   Generate traffic:
*         Send to device's port 0 packet:
*         macDa = 00:00:00:00:00:02,
*         macSa = 00:00:00:00:00:12,
*         TAG0: ethertype = 0x8100, vlan = 5
*         TAG1: ethertype = 0x88a8, vlan = 25
*         Send to device's port 0 packet:
*         macDa = 00:00:00:00:00:03,
*         macSa = 00:00:00:00:00:13,
*         TAG0: ethertype = 0xA0A0, vlan = 25
*         TAG1: ethertype = 0x5050, vlan = 100
*         Success Criteria:
*         First packet have TAG1: ethertype =0x98a8, vlan = 25
*         Second packet have TAG1: ethertype =0x98a8, vlan = 100
*/
GT_VOID prvTgfBrgVlanManipulationIngressVlanAssignment2TrafficGenerate
(
    GT_VOID
);

/**
* @internal prvTgfBrgVlanManipulationIngressVlanAssignment2ConfigurationRestore function
* @endinternal
*
* @brief   Restore test configuration
*/
GT_VOID prvTgfBrgVlanManipulationIngressVlanAssignment2ConfigurationRestore
(
    GT_VOID
);

/**
* @internal prvTgfBrgVlanManipulationChangeEgressTpidConfigurationSet function
* @endinternal
*
* @brief   Set test configuration:
*         - Set ingress TPID table to:
*         Ethertype0 = 0x8100,
*         Ethertype1 = 0x88a8,
*         Ethertype2 = 0x5000,
*         Ethertype3 = 0xA0A0,
*         Ethertype4 = 0x5050,
*         Ethertype5 = 0x2525,
*         Ethertype6 = 0x5555,
*         Ethertype7 = 0xAAAA,
*         - Set ingress TPID select to: TAG0 is Ethertype0, Ethertype3, Ethertype5,
*         Ethertype6, Ethertype7, TAG1 is Ethertype1, Ethertype2, Ethertype4.
*         - Build VLAN Entry with VID = 5 and portsMember = 0, 8, 18, 23
*         and "Outer Tag0, Inner Tag1" tagging command for all ports in VLAN.
*         - Set egress TPID table to: Ethertype0 = 0x9100, Ethertype1 = 0x98a8
*         - Set egress TPID select to: TAG0 is Ethertype0, TAG1 is Ethertype1.
*/
GT_VOID prvTgfBrgVlanManipulationChangeEgressTpidConfigurationSet
(
    GT_VOID
);

/**
* @internal prvTgfBrgVlanManipulationChangeEgressTpidTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic:
*         Send to device's port 0 packet:
*         macDa = 00:00:00:00:00:02,
*         macSa = 00:00:00:00:00:12,
*         TAG0: ethertype = 0x8100, vlan = 5
*         TAG1: ethertype = 0x88a8, vlan = 100
*         Additional configuration:
*         Set egress TPID table to: Ethertype0 = 0x2222 and Ethertype1 = 0x3333
*         Send to device's port 0 packet:
*         macDa = 00:00:00:00:00:02,
*         macSa = 00:00:00:00:00:12,
*         TAG0: ethertype = 0x8100, vlan = 5
*         TAG1: ethertype = 0x88a8, vlan = 100
*         Success Criteria:
*         First packet have TAG0: ethertype =0x9100, vlan = 5;
*         TAG1: ethertype =0x98a8, vlan = 100;
*         Second packet have TAG0: ethertype =0x2222, vlan = 5;
*         TAG1: ethertype =0x3333, vlan = 100;
*/
GT_VOID prvTgfBrgVlanManipulationChangeEgressTpidTrafficGenerate
(
    GT_VOID
);

/**
* @internal prvTgfBrgVlanManipulationChangeEgressTpidConfigurationRestore function
* @endinternal
*
* @brief   Restore test configuration
*/
GT_VOID prvTgfBrgVlanManipulationChangeEgressTpidConfigurationRestore
(
    GT_VOID
);

/**
* @internal prvTgfBrgVlanTableWriteWorkaroundTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic:
*         Send full-wire speed traffic
*         Additional configuration:
*         Write all existing VLAN entries in the loop
*         Success Criteria:
*         There are no discarded egress packets under traffic.
*/
GT_VOID prvTgfBrgVlanTableWriteWorkaroundTrafficGenerate
(
    GT_VOID
);

/**
* @internal prvTgfBrgVlanTableWriteWorkaroundConfigurationRestore function
* @endinternal
*
* @brief   Restore test configuration
*/
GT_VOID prvTgfBrgVlanTableWriteWorkaroundConfigurationRestore
(
    GT_VOID
);

/**
* @internal prvTgfBrgStgTableWriteWorkaroundTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic:
*         Send full-wire speed traffic
*         Write dummy STG entry with groupId = 1 configured with block command
*         Write default STG entry with groupId = 0 without changes
*         Success Criteria:
*         There are no discarded egress packets under traffic.
*/
GT_VOID prvTgfBrgStgTableWriteWorkaroundTrafficGenerate
(
    GT_VOID
);

/**
* @internal prvTgfBrgStgTableWriteWorkaroundConfigurationRestore function
* @endinternal
*
* @brief   Restore test configuration
*/
GT_VOID prvTgfBrgStgTableWriteWorkaroundConfigurationRestore
(
    GT_VOID
);



/**
* @internal prvTgfFdbTableReadUnderWireSpeedTrafficGenerate function
* @endinternal
*
* @brief   Verify that there are no discarded packets during read access by the
*         CPU to the FDB.
*         Run full-wire speed traffic test on 10G ports (4 ports, 4 VLAN entries)
*         static mac addresses that relate to macSA,DA
*         Check bridge drop counters - must be 0
*/
GT_VOID prvTgfFdbTableReadUnderWireSpeedTrafficGenerate
(
    GT_VOID
);


/**
* @internal prvTgfFdbTableReadUnderWireSpeedConfigurationRestore function
* @endinternal
*
* @brief   Restore test configuration
*/
GT_VOID prvTgfFdbTableReadUnderWireSpeedConfigurationRestore
(
    GT_VOID
);



/*  enum for the test of prvTgfBrgVlanManipulationEthernetOverMplsConfigurationSet
    PRV_TGF_BRG_VLAN_ETHERNET_OVER_MPLS_CONFIG_TEST_PASSENGER_SRC_EPORT_REASSIGN_E -
        the packet is TT and have new src eport assignment --> new TPID profile
        applicable devices : Bobcat2; Caelum; Bobcat3

    PRV_TGF_BRG_VLAN_ETHERNET_OVER_MPLS_CONFIG_TEST_PASSENGER_E -
        the packet is TT  --> default TPID profile / bmp
        applicable devices : xcat and above

    PRV_TGF_BRG_VLAN_ETHERNET_OVER_MPLS_CONFIG_TEST_NO_TUNNEL_E -
        the packet is NOT TT  --> default TPID profile / bmp
        applicable devices : xcat and above

*/
typedef enum{
    PRV_TGF_BRG_VLAN_ETHERNET_OVER_MPLS_CONFIG_TEST_PASSENGER_SRC_EPORT_REASSIGN_E,
    PRV_TGF_BRG_VLAN_ETHERNET_OVER_MPLS_CONFIG_TEST_PASSENGER_E,
    PRV_TGF_BRG_VLAN_ETHERNET_OVER_MPLS_CONFIG_TEST_NO_TUNNEL_E ,

    PRV_TGF_BRG_VLAN_ETHERNET_OVER_MPLS_CONFIG_TEST____LAST____E
}PRV_TGF_BRG_VLAN_ETHERNET_OVER_MPLS_CONFIG_TEST_TYPE_ENT;


/**
* @internal prvTgfBrgVlanManipulationEthernetOverMpls_isValid function
* @endinternal
*
* @brief   check if test is valid for the current device
*
* @param[in] testId                   - id of the test
* @param[in] senderPortIndex          - index of the sending port (0..11)
*                                       None
*/
GT_BOOL prvTgfBrgVlanManipulationEthernetOverMpls_isValid
(
    IN PRV_TGF_BRG_VLAN_ETHERNET_OVER_MPLS_CONFIG_TEST_TYPE_ENT       testId ,
    IN GT_U32       senderPortIndex
);

/**
* @internal prvTgfBrgVlanManipulationEthernetOverMplsConfigurationSet function
* @endinternal
*
* @brief   Set test configuration
*
* @param[in] testId                   - id of the test
* @param[in] senderPortIndex          - index of the sending port (0..11)
*                                       None
*/
GT_VOID prvTgfBrgVlanManipulationEthernetOverMplsConfigurationSet
(
    IN PRV_TGF_BRG_VLAN_ETHERNET_OVER_MPLS_CONFIG_TEST_TYPE_ENT       testId ,
    IN GT_U32       senderPortIndex
);

/**
* @internal prvTgfBrgVlanManipulationEthernetOverMplsTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic
*
* @param[in] testId                   - id of the test
* @param[in] senderPortIndex          - index of the sending port (0..11)
*                                       None
*/
GT_VOID prvTgfBrgVlanManipulationEthernetOverMplsTrafficGenerate
(
    IN PRV_TGF_BRG_VLAN_ETHERNET_OVER_MPLS_CONFIG_TEST_TYPE_ENT       testId ,
    IN GT_U32       senderPortIndex
);

/**
* @internal prvTgfBrgVlanManipulationEthernetOverMplsConfigurationRestore function
* @endinternal
*
* @brief   Restore test configuration
*
* @param[in] testId                   - id of the test
* @param[in] senderPortIndex          - index of the sending port (0..11)
*                                       None
*/
GT_VOID prvTgfBrgVlanManipulationEthernetOverMplsConfigurationRestore
(
    IN PRV_TGF_BRG_VLAN_ETHERNET_OVER_MPLS_CONFIG_TEST_TYPE_ENT       testId ,
    IN GT_U32       senderPortIndex
);

/**
* @internal prvTgfEgressPortCascadeSet function
* @endinternal
*
* @brief   set/unset the egress port as cascade with DSA tag .
*
* @param[in] portInterfacePtr         - (pointer to) port interface (port only is legal param)
* @param[in] enable                   - enable/disable the cascade port.
* @param[in] egrDsaType               - type of DSAtag on egress port
*                                       None
*/
GT_VOID prvTgfEgressPortCascadeSet
(
    IN CPSS_INTERFACE_INFO_STC  *portInterfacePtr,
    IN GT_BOOL                  enable,
    IN TGF_DSA_TYPE_ENT         egrDsaType
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvTgfVlanManipulation */


