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
* @file prvTgfBrgSrcId.h
*
* @brief Bridge Source-Id UT.
*
* @version   2.
********************************************************************************
*/
#ifndef __prvTgfBrgSrcId
#define __prvTgfBrgSrcId

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


/**
* @internal prvTgfBrgSrcIdPortForceConfigurationSet function
* @endinternal
*
* @brief   Set test configuration:
*         Create VLAN 2 on all ports.
*         Set MAC table with two entries:
*         - MAC = 00:00:00:00:00:01, VLAN = 2, intType = PORT,
*         port = 0, sourceId = 1
*         - MAC = 00:00:00:00:00:02, VLAN = 2, intType = PORT,
*         port = 8, sourceId = 0
*         Source ID configuration:
*         Disable port force source-ID assigment
*         Configure Global Assigment mode to FDB SA-based.
*         Configure default port source-ID to 2.
*         Delete port 8 from source-ID group 1.
*         Enable Unicast Egreess filter.
*         Generate Traffic:
*         Send to device's port 0 packet:
*         daMAC = 00:00:00:00:00:02, saMAC = 00:00:00:00:00:01,
*         etherType = 0x8100 vlanID = 0x2.
*         Success Criteria:
*         Packet is not captured in all ports.
*         Additional Configuration:
*         Enable Port Force Src-ID assigment.
*         Generate Traffic:
*         Send to device's port 0 packet:
*         daMAC = 00:00:00:00:00:02, saMAC = 00:00:00:00:00:01,
*         etherType = 0x8100 vlanID = 0x2.
*         Success Criteria:
*         Packet is captured in port: 8.
*/
GT_VOID prvTgfBrgSrcIdPortForceConfigurationSet
(
    GT_VOID
);

/**
* @internal prvTgfBrgSrcIdPortForceTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic:
*/
GT_VOID prvTgfBrgSrcIdPortForceTrafficGenerate
(
    GT_VOID
);

/**
* @internal prvTgfBrgSrcIdPortForceConfigurationRestore function
* @endinternal
*
* @brief   Restore test configuration
*/
GT_VOID prvTgfBrgSrcIdPortForceConfigurationRestore
(
    GT_VOID
);

/**
* @internal prvTgfBrgSrcIdPortPclRedirect function
* @endinternal
*
* @brief   Set parameters for SRC-ID tests. this only save values that later used
*         during run of test
* @param[in] enablePclRedirect        - enable PCL action 'redirect'
* @param[in] enableBypassIngressPipe  - enable PCL action 'BypassIngressPipe'
*                                       None
*/
GT_VOID prvTgfBrgSrcIdPortPclRedirect
(
    IN GT_BOOL  enablePclRedirect,
    IN GT_BOOL  enableBypassIngressPipe
);

/**
* @internal prvTgfBrgSrcIdFdbSaAssigmentConfigurationSet function
* @endinternal
*
* @brief   Set test configuration:
*         Create VLAN 2 on all ports.
*         Set MAC table with two entries:
*         - MAC = 00:00:00:00:00:01, VLAN = 2, intType = PORT,
*         port = 0, sourceId = 1
*         - MAC = 00:00:00:00:00:02, VLAN = 2, intType = PORT,
*         port = 8, sourceId = 0
*         Source ID configuration:
*         Disable port force source-ID assigment
*         Configure Global Assigment mode to FDB SA-based.
*         Configure default port source-ID to 2.
*         Delete port 8 from source-ID group 1.
*         Enable Unicast Egreess filter.
*         Generate Traffic:
*         Send to device's port 0 packet:
*         daMAC = 00:00:00:00:00:02, saMAC = 00:00:00:00:00:01,
*         etherType = 0x8100 vlanID = 0x2.
*         Success Criteria:
*         Packet is not captured in port 8.
*         Additional Configuration:
*         Add to Source-ID group 1 port 8.
*         Delete from Source-ID group 2 port 8.
*         Generate Traffic:
*         Send to device's port 0 packet:
*         daMAC = 00:00:00:00:00:02, saMAC = 00:00:00:00:00:01,
*         etherType = 0x8100 vlanID = 0x2.
*         Success Criteria:
*         Packet is captured on port: 8.
*/
GT_VOID prvTgfBrgSrcIdFdbSaAssigmentConfigurationSet
(
    GT_VOID
);

/**
* @internal prvTgfBrgSrcIdFdbSaAssigmentTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic:
*/
GT_VOID prvTgfBrgSrcIdFdbSaAssigmentTrafficGenerate
(
    GT_VOID
);

/**
* @internal prvTgfBrgSrcIdFdbSaAssigmentConfigurationRestore function
* @endinternal
*
* @brief   Restore test configuration
*/
GT_VOID prvTgfBrgSrcIdFdbSaAssigmentConfigurationRestore
(
    GT_VOID
);

/**
* @internal prvTgfBrgSrcIdFdbDaAssigmentConfigurationSet function
* @endinternal
*
* @brief   Set test configuration:
*         Create VLAN 2 on all ports.
*         Set MAC table with two entries:
*         - MAC = 00:00:00:00:00:01, VLAN = 2, intType = PORT,
*         port = 0, sourceId = 1
*         - MAC = 00:00:00:00:00:02, VLAN = 2, intType = PORT,
*         port = 8, sourceId = 0
*         Source ID configuration:
*         Disable port force source-ID assigment
*         Configure Global Assigment mode to FDB SA-based.
*         Configure default port source-ID to 2.
*         Delete port 8 from source-ID group 1.
*         Enable Unicast Egreess filter.
*         Generate Traffic:
*         Send to device's port 0 packet:
*         daMAC = 00:00:00:00:00:02, saMAC = 00:00:00:00:00:01,
*         etherType = 0x8100 vlanID = 0x2.
*         Success Criteria:
*         Packet is not captured in port 8.
*         Additional Configuration:
*         Add to Source-ID group 1 port 8.
*         Delete from Source-ID group 2 port 8.
*         Generate Traffic:
*         Send to device's port 0 packet:
*         daMAC = 00:00:00:00:00:02, saMAC = 00:00:00:00:00:01,
*         etherType = 0x8100 vlanID = 0x2.
*         Success Criteria:
*         Packet is captured on port: 8.
*/
GT_VOID prvTgfBrgSrcIdFdbDaAssigmentConfigurationSet
(
    GT_VOID
);

/**
* @internal prvTgfBrgSrcIdFdbDaAssigmentTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic:
*         Send to device's port 0 packet:
*         macDa = 00:00:00:00:00:22,
*         macSa = 00:00:00:00:00:11,
*/
GT_VOID prvTgfBrgSrcIdFdbDaAssigmentTrafficGenerate
(
    GT_VOID
);

/**
* @internal prvTgfBrgSrcIdFdbDaAssigmentConfigurationRestore function
* @endinternal
*
* @brief   Restore test configuration
*/
GT_VOID prvTgfBrgSrcIdFdbDaAssigmentConfigurationRestore
(
    GT_VOID
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvTgfFdbAging */



