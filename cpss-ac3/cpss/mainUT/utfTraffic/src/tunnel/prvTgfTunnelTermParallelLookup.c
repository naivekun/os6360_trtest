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
* @file prvTgfTunnelTermParallelLookup.c
*
* @brief Tunnel Term: Ipv4 over Ipv4 - check parallel lookup
*
* @version   13
********************************************************************************
*/

#include <cpssCommon/cpssPresteraDefs.h>
#include <cpss/extServices/private/prvCpssBindFunc.h>
#include <utf/private/prvUtfExtras.h>

#include <utf/utfMain.h>
#include <utf/private/prvUtfHelpers.h>

#include <trafficEngine/tgfTrafficTable.h>

#include <common/tgfCommon.h>
#include <common/tgfBridgeGen.h>
#include <common/tgfIpGen.h>
#include <common/tgfTunnelGen.h>
#include <common/tgfCncGen.h>
#include <tunnel/prvTgfTunnelTermParallelLookup.h>
#include <tunnel/prvTgfTunnelTermIpv4overIpv4.h>

/******************************************************************************\
 *                       Test configuration section                           *
\******************************************************************************/

/* default VLAN Id */
#define PRV_TGF_SEND_VLANID_CNS           5

/* nextHop VLAN Id */
#define PRV_TGF_NEXTHOPE_VLANID_CNS       6

/* port number to send traffic to */
#define PRV_TGF_SEND_PORT_IDX_CNS         0

/* the TTI Rules indexes */
static GT_U32        prvTgfTtiRuleLookup0Index;
static GT_U32        prvTgfTtiRuleLookup1Index;
static GT_U32        prvTgfTtiRuleLookup2Index;
static GT_U32        prvTgfTtiRuleLookup3Index;

/* the Mask for MAC address */
static TGF_MAC_ADDR  prvTgfFullMaskMac = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

/* the Mask for IP address */
static TGF_IPV4_ADDR prvTgfFullMaskIp = {255, 255, 255, 255};

/* the Virtual Router index */
static GT_U32        prvUtfVrfId                = 0;

/* the Arp Address index of the Router ARP Table
   to write to the UC Route entry Arp nextHopARPPointer field */
static GT_U32        prvTgfRouterArp1Index      = 1;
static GT_U32        prvTgfRouterArp2Index      = 2;
static GT_U32        prvTgfRouterArp3Index      = 3;

/* the ARP MAC address to write to the Router ARP Table */
static TGF_MAC_ADDR  prvTgfArp1Mac = {0x00, 0x00, 0x00, 0x00, 0x00, 0x22};
static TGF_MAC_ADDR  prvTgfArp2Mac = {0x00, 0x00, 0x00, 0x00, 0x00, 0x33};
static TGF_MAC_ADDR  prvTgfArp3Mac = {0x00, 0x00, 0x00, 0x00, 0x00, 0x44};

/* the Route entry index for UC Route entry Table */
static GT_U32  prvTgfRouteEntryBaseIndex = 8;

/* the leaf index in the RAM PBR section */
static GT_U32   prvTgfPbrLeafIndex = 567;

/* default LPM DB Id */
static GT_U32   prvTgfLpmDBId   = 0;

/****************************** parts of packet *******************************/

/* L2 part of unicast packet */
static TGF_PACKET_L2_STC prvTgfPacketL2Part = {
    {0x00, 0x00, 0x00, 0x00, 0x34, 0x02},               /* daMac */
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x11}                /* saMac */
};

/* packet's IPv4 */
static TGF_PACKET_IPV4_STC prvTgfPacketIpv4Part = {
    4,                  /* version */
    5,                  /* headerLen */
    0,                  /* typeOfService */
    0x3f,               /* totalLen */
    0,                  /* id */
    0,                  /* flags */
    0,                  /* offset */
    0x40,               /* timeToLive */
    0x04,               /* protocol */
    TGF_PACKET_AUTO_CALC_CHECKSUM_CNS,             /* csum */ /*0x5EA0*/
    {10, 10, 10, 10},   /* srcAddr */
    { 4,  4,  4,  4}    /* dstAddr */
};

/*************************** Restore config ***********************************/

/* parameters that is needed to be restored */
static struct
{
    PRV_TGF_TTI_MAC_MODE_ENT                macMode;
    PRV_TGF_TTI_KEY_TCAM_SEGMENT_MODE_ENT   segmentMode;
} prvTgfRestoreCfg;

/******************************************************************************/


/******************************************************************************\
 *                            Test implementation                             *
\******************************************************************************/

/**
* @internal prvTgfTunnelTermParallelLookupRuleIndexesCalculate function
* @endinternal
*
* @brief   Calculate rule indexes
*/
static GT_VOID prvTgfTunnelTermParallelLookupRuleIndexesCalculate
(
    GT_VOID
)
{
    prvTgfTtiRuleLookup0Index = prvTgfTtiTcamAbsoluteIndexWithinTheLookupHitGet(0 , 3);
    prvTgfTtiRuleLookup1Index = prvTgfTtiTcamAbsoluteIndexWithinTheLookupHitGet(1 , 0);

    if (PRV_CPSS_SIP_5_10_CHECK_MAC(prvTgfDevNum) == GT_TRUE)
    {
        prvTgfTtiRuleLookup2Index = prvTgfTtiTcamAbsoluteIndexWithinTheLookupHitGet(2 , 0);
        prvTgfTtiRuleLookup3Index = prvTgfTtiTcamAbsoluteIndexWithinTheLookupHitGet(3 , 0);
    }
}

/**
* @internal prvTgfTunnelTermParallelLookupCheckPacketOn function
* @endinternal
*
* @brief   Check whether the packet received by CPU
*
* @param[in] isPacketExpectedOnCpu    - is packet expected on CPU
*                                       None
*/
static GT_VOID prvTgfTunnelTermParallelLookupCheckPacketOn
(
    IN GT_BOOL         isPacketExpectedOnCpu
)
{
    GT_U8           queue    = 0;
    GT_U8           dev      = 0;
    GT_BOOL         getFirst = GT_TRUE;
    GT_STATUS       rc       = GT_OK;
    GT_U32          buffLen  = TGF_RX_BUFFER_MAX_SIZE_CNS;
    GT_U32          packetActualLength = 0;
    GT_U8           packetBuff[TGF_RX_BUFFER_MAX_SIZE_CNS] = {0};
    TGF_NET_DSA_STC rxParam = {0};

    if (isPacketExpectedOnCpu)
    {
        /* AUTODOC: get entry from rxNetworkIf table */
        rc = tgfTrafficGeneratorRxInCpuGet(TGF_PACKET_TYPE_REGULAR_E,
                                           getFirst, GT_TRUE, packetBuff,
                                           &buffLen, &packetActualLength,
                                           &dev, &queue, &rxParam);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficGeneratorRxInCpuGet\n");

        getFirst = GT_FALSE;
    }
    /* AUTODOC: get entry from rxNetworkIf table */
    rc = tgfTrafficGeneratorRxInCpuGet(TGF_PACKET_TYPE_REGULAR_E,
                                       getFirst, GT_TRUE, packetBuff,
                                       &buffLen, &packetActualLength,
                                       &dev, &queue, &rxParam);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_NO_MORE, rc, "tgfTrafficGeneratorRxInCpuGet\n");
}

/**
* @internal prvTgfTunnelTermParallelLookupBaseConfigurationSet function
* @endinternal
*
* @brief   Set Base Configuration
*
* @param[in] vrfId                    - virtual router index
*                                       None
*/
GT_VOID prvTgfTunnelTermParallelLookupBaseConfigurationSet
(
    GT_U32     vrfId
)
{
    GT_STATUS  rc;

    prvTgfTunnelTermIpv4overIpv4BaseConfigurationSet(vrfId, 0, GT_TRUE);

    /* AUTODOC: Add port PRV_TGF_NEXTHOPE2_PORT_IDX_CNS to PRV_TGF_NEXTHOPE_VLANID_CNS */
    rc = prvTgfBrgVlanMemberAdd(prvTgfDevNum, (GT_U16)PRV_TGF_NEXTHOPE_VLANID_CNS,
                                prvTgfPortsArray[PRV_TGF_NEXTHOPE2_PORT_IDX_CNS], GT_FALSE);
    UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanMemberAdd: %d, %d, %d, %d",
                                 prvTgfDevNum, PRV_TGF_NEXTHOPE_VLANID_CNS,
                                 prvTgfPortsArray[PRV_TGF_NEXTHOPE2_PORT_IDX_CNS], GT_FALSE);
}

/**
* @internal prvTgfTunnelTermParallelLookupRouteConfigurationSet function
* @endinternal
*
* @brief   Set Route Configuration
*/
GT_VOID prvTgfTunnelTermParallelLookupRouteConfigurationSet
(
    GT_VOID
)
{
    prvTgfTunnelTermIpv4overIpv4RouteConfigurationSet();
}

/**
* @internal prvTgfTunnelTermParallelLookupTtiBasicRule function
* @endinternal
*
* @brief   build TTI rule
*
* @param[in] pclId                    - pclId
* @param[in] macAddrPtr               - pointer to mac address
* @param[in] sipPtr                   - pointer to SIP
* @param[in] dipPtr                   - pointer to DIP
* @param[in] vid                      - vlan id
*                                      ttiActionPtr  - (pointer to) tti action
* @param[in,out] ttiPatternPtr            - (pointer to) tti pattern
* @param[in,out] ttiMaskPtr               - (pointer to) tti mask
*                                       None
*/
static GT_VOID prvTgfTunnelTermParallelLookupTtiBasicRule
(
    IN GT_U32                       pclId,
    IN TGF_MAC_ADDR                 *macAddrPtr,
    IN TGF_IPV4_ADDR                *sipPtr,
    IN TGF_IPV4_ADDR                *dipPtr,
    IN GT_U16                       vid,
    INOUT PRV_TGF_TTI_RULE_UNT      *ttiPatternPtr,
    INOUT PRV_TGF_TTI_RULE_UNT      *ttiMaskPtr
)
{
    /* AUTODOC: set TTI Pattern */
    cpssOsMemCpy(ttiPatternPtr->ipv4.common.mac.arEther, macAddrPtr, sizeof(TGF_MAC_ADDR));
    if (sipPtr)
    {
        cpssOsMemCpy(ttiPatternPtr->ipv4.srcIp.arIP, sipPtr, sizeof(TGF_IPV4_ADDR));
    }
    if (dipPtr)
    {
        cpssOsMemCpy(ttiPatternPtr->ipv4.destIp.arIP, dipPtr, sizeof(TGF_IPV4_ADDR));
    }

    ttiPatternPtr->ipv4.common.pclId            = pclId;
    ttiPatternPtr->ipv4.common.srcIsTrunk       = 0;
    ttiPatternPtr->ipv4.common.srcPortTrunk     = 0;
    ttiPatternPtr->ipv4.common.vid              = vid;
    ttiPatternPtr->ipv4.common.isTagged         = GT_TRUE;
    ttiPatternPtr->ipv4.common.dsaSrcIsTrunk    = GT_FALSE;
    ttiPatternPtr->ipv4.common.dsaSrcPortTrunk  = 0;
    ttiPatternPtr->ipv4.common.dsaSrcDevice     = prvTgfDevNum;
    ttiPatternPtr->ipv4.tunneltype              = 0;
    ttiPatternPtr->ipv4.isArp                   = GT_FALSE;

    /* AUTODOC: set TTI Mask */
    cpssOsMemCpy(ttiMaskPtr, ttiPatternPtr, sizeof(PRV_TGF_TTI_RULE_UNT));
    cpssOsMemCpy(ttiMaskPtr->ipv4.common.mac.arEther, prvTgfFullMaskMac, sizeof(TGF_MAC_ADDR));
    if (sipPtr)
    {
        cpssOsMemCpy(ttiMaskPtr->ipv4.srcIp.arIP, prvTgfFullMaskIp, sizeof(TGF_IPV4_ADDR));
    }
    else
    {
        cpssOsMemSet(ttiMaskPtr->ipv4.srcIp.arIP, 0, sizeof(TGF_IPV4_ADDR));
    }
    if (dipPtr)
    {
        cpssOsMemCpy(ttiMaskPtr->ipv4.destIp.arIP, prvTgfFullMaskIp, sizeof(TGF_IPV4_ADDR));
    }
    else
    {
        cpssOsMemSet(ttiMaskPtr->ipv4.destIp.arIP, 0, sizeof(TGF_IPV4_ADDR));
    }
    ttiMaskPtr->ipv4.common.srcIsTrunk = 1;
}

/**
* @internal prvTgfTunnelTermParallelLookupTtiBasicAction function
* @endinternal
*
* @brief   build TTI action
*
* @param[in] continueToNextTtiLookup  - whether to continue to next lookup
* @param[in] command                  - the command
* @param[in,out] actionPtr                - (pointer to) the action
* @param[in,out] actionPtr                - (pointer to) the action
*                                       None
*/
static GT_VOID prvTgfTunnelTermParallelLookupTtiBasicAction
(
    IN    GT_BOOL                   continueToNextTtiLookup,
    IN    CPSS_PACKET_CMD_ENT       command,
    INOUT PRV_TGF_TTI_ACTION_2_STC  *actionPtr
)
{
    /* AUTODOC: set TTI Action */
    actionPtr->command                       = command;
    actionPtr->redirectCommand               = PRV_TGF_TTI_NO_REDIRECT_E;
    actionPtr->userDefinedCpuCode            = CPSS_NET_FIRST_USER_DEFINED_E;
    actionPtr->tunnelTerminate               = GT_TRUE;
    actionPtr->ttPassengerPacketType         = PRV_TGF_TTI_PASSENGER_IPV4_E;
    actionPtr->copyTtlExpFromTunnelHeader    = GT_FALSE;
    actionPtr->mirrorToIngressAnalyzerEnable = GT_FALSE;
    actionPtr->policerIndex                  = 0;
    actionPtr->egressInterface.type            = CPSS_INTERFACE_PORT_E;
    actionPtr->egressInterface.devPort.hwDevNum  = prvTgfDevNum;
    actionPtr->egressInterface.devPort.portNum = prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS];
    actionPtr->tag0VlanPrecedence            = CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_HARD_E;
    actionPtr->tag0VlanCmd                   = PRV_TGF_TTI_VLAN_DO_NOT_MODIFY_E;
    actionPtr->tag0VlanId                    = 0;
    actionPtr->nestedVlanEnable              = GT_FALSE;
    actionPtr->tunnelStart                   = GT_FALSE;
    actionPtr->tunnelStartPtr                = 0;
    actionPtr->routerLttPtr                  = 0;
    actionPtr->vrfId                         = prvUtfVrfId;
    actionPtr->sourceIdSetEnable             = GT_FALSE;
    actionPtr->sourceId                      = 0;
    actionPtr->bindToPolicer                 = GT_FALSE;
    actionPtr->qosPrecedence                 = CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_SOFT_E;
    actionPtr->modifyTag0Up                  = PRV_TGF_TTI_QOS_KEEP_PREVIOUS_E;
    actionPtr->qosProfile                    = 0;
    actionPtr->modifyTag0Up                  = PRV_TGF_TTI_DO_NOT_MODIFY_PREV_UP_E;
    actionPtr->modifyDscp                    = PRV_TGF_TTI_DO_NOT_MODIFY_PREV_DSCP_E;
    actionPtr->tag0Up                        = 0;
    actionPtr->remapDSCP                     = GT_FALSE;
    actionPtr->vntl2Echo                     = GT_FALSE;
    actionPtr->bridgeBypass                  = GT_FALSE;
    actionPtr->actionStop                    = GT_FALSE;
    actionPtr->counterEnable                 = GT_FALSE;
    actionPtr->continueToNextTtiLookup       = continueToNextTtiLookup;
}

/**
* @internal prvTgfTunnelTermParallelLookupTtiBasicConfiguration function
* @endinternal
*
* @brief   Set TTI Basic Configuration
*/
static GT_VOID prvTgfTunnelTermParallelLookupTtiBasicConfiguration
(
    GT_VOID
)
{
    GT_STATUS                rc;

    PRV_UTF_LOG0_MAC("======= Setting TTI Configuration =======\n");

    /* AUTODOC: set the TTI Rule Pattern, Mask and Action for IPV4 TCAM location */
    /* AUTODOC: tti rule key is PRV_TGF_TTI_KEY_IPV4_E */

    /* AUTODOC: enable the TTI lookup for IPv4 at the port 0 */
    rc = prvTgfTtiPortLookupEnableSet(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], PRV_TGF_TTI_KEY_IPV4_E, GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTtiPortLookupEnableSet");

    /* AUTODOC: save the current lookup Mac mode for IPv4 and DA */
    rc = prvTgfTtiMacModeGet(prvTgfDevNum, PRV_TGF_TTI_KEY_IPV4_E, &prvTgfRestoreCfg.macMode);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTtiMacModeGet");

    /* AUTODOC: set lookup Mac mode for IPv4 and DA */
    rc = prvTgfTtiMacModeSet(PRV_TGF_TTI_KEY_IPV4_E, PRV_TGF_TTI_MAC_MODE_DA_E);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTtiMacModeSet");

    /* AUTODOC: save the current TCAM segment mode for IPv4 key */
    rc = prvTgfTtiTcamSegmentModeGet(prvTgfDevNum, PRV_TGF_TTI_KEY_IPV4_E, &prvTgfRestoreCfg.segmentMode);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTtiTcamSegmentModeGet");

    /* AUTODOC: set TCAM segment mode for IPv4 key to GT_TRUE */
    if (PRV_CPSS_SIP_5_10_CHECK_MAC(prvTgfDevNum) == GT_TRUE)
    {
        rc = prvTgfTtiTcamSegmentModeSet(prvTgfDevNum, PRV_TGF_TTI_KEY_IPV4_E, PRV_TGF_TTI_KEY_TCAM_SEGMENT_MODE_FOUR_TCAM_E);
    }
    else
    {
        rc = prvTgfTtiTcamSegmentModeSet(prvTgfDevNum, PRV_TGF_TTI_KEY_IPV4_E, PRV_TGF_TTI_KEY_TCAM_SEGMENT_MODE_TWO_TCAM_E);
    }
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTtiTcamSegmentModeSet");

    /* AUTODOC: enable IPv4 TTI lookup for only tunneled packets received on port 0 */
    rc = prvTgfTtiPortIpv4OnlyTunneledEnableSet(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTtiPortIpv4OnlyTunneledEnableSet");

    return;
}
/**
* @internal prvTgfTunnelTermParallelLookupTtiConfigurationSet function
* @endinternal
*
* @brief   Set TTI Configuration
*/
GT_VOID prvTgfTunnelTermParallelLookupTtiConfigurationSet
(
    GT_VOID
)
{
    GT_STATUS                rc;
    PRV_TGF_TTI_ACTION_2_STC ttiAction;
    PRV_TGF_TTI_RULE_UNT     ttiPattern;
    PRV_TGF_TTI_RULE_UNT     ttiMask;

    PRV_UTF_LOG0_MAC("======= Setting TTI Configuration =======\n");

    prvTgfTunnelTermParallelLookupRuleIndexesCalculate();

    cpssOsMemSet(&ttiAction, 0, sizeof(ttiAction));
    cpssOsMemSet(&ttiPattern, 0, sizeof(ttiPattern));
    cpssOsMemSet(&ttiMask, 0, sizeof(ttiMask));

    /* AUTODOC: set the TTI Rule Pattern, Mask and Action for IPV4 TCAM location */
    /* AUTODOC: tti rule key is PRV_TGF_TTI_KEY_IPV4_E */
    /* AUTODOC: set basic configuration for TTI */
    prvTgfTunnelTermParallelLookupTtiBasicConfiguration();

    /* build action */
    prvTgfTunnelTermParallelLookupTtiBasicAction(GT_TRUE, CPSS_PACKET_CMD_FORWARD_E, &ttiAction);

    /* build rule for lookup 0 */
    prvTgfTunnelTermParallelLookupTtiBasicRule(
        1,/*pclId*/
        &prvTgfPacketL2Part.daMac,
        &prvTgfPacketIpv4Part.srcAddr,
        NULL,
        PRV_TGF_SEND_VLANID_CNS,
        &ttiPattern, &ttiMask);

    /* AUTODOC: add TTI rule 1 with IPv4 key on port 0 VLAN 5 with action: */
    /* AUTODOC:   cmd FORWARD, continueToNextTtiLookup = GT_TRUE */
    /* AUTODOC:   IPv4 passenger packet type */
    /* AUTODOC:   DA=00:00:00:00:34:02 */
    /* AUTODOC:   srcIp=10.10.10.10 */
    rc = prvTgfTtiRule2Set(prvTgfTtiRuleLookup0Index, PRV_TGF_TTI_KEY_IPV4_E,
                          &ttiPattern, &ttiMask, &ttiAction);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTtiRule2Set");

    if (PRV_CPSS_SIP_5_10_CHECK_MAC(prvTgfDevNum) == GT_FALSE)
    {
        /* build action */
        prvTgfTunnelTermParallelLookupTtiBasicAction(GT_FALSE, CPSS_PACKET_CMD_DROP_HARD_E, &ttiAction);

        /* build rule for lookup 1 */
        prvTgfTunnelTermParallelLookupTtiBasicRule(
            1,/*pclId*/
            &prvTgfPacketL2Part.daMac,
            NULL,
            &prvTgfPacketIpv4Part.dstAddr,
            PRV_TGF_SEND_VLANID_CNS,
            &ttiPattern, &ttiMask);

        /* AUTODOC: add TTI rule 15360 with IPv4 key on port 0 VLAN 5 with action: */
        /* AUTODOC:   cmd DROP_HARD, continueToNextTtiLookup = GT_FALSE */
        /* AUTODOC:   IPv4 passenger packet type */
        /* AUTODOC:   DA=00:00:00:00:34:02 */
        /* AUTODOC:   dstIp=4.4.4.4 */
        rc = prvTgfTtiRule2Set(prvTgfTtiRuleLookup1Index, PRV_TGF_TTI_KEY_IPV4_E,
                              &ttiPattern, &ttiMask, &ttiAction);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTtiRule2Set");
    }
    else
    {
        /* build action */
        prvTgfTunnelTermParallelLookupTtiBasicAction(GT_TRUE, CPSS_PACKET_CMD_MIRROR_TO_CPU_E, &ttiAction);

        /* build rule for lookup 1 */
        prvTgfTunnelTermParallelLookupTtiBasicRule(
            1,/*pclId*/
            &prvTgfPacketL2Part.daMac,
            NULL,
            &prvTgfPacketIpv4Part.dstAddr,
            PRV_TGF_SEND_VLANID_CNS,
            &ttiPattern, &ttiMask);

        /* AUTODOC: add TTI rule 15360 with IPv4 key on port 0 VLAN 5 with action: */
        /* AUTODOC:   cmd MIRROR_TO_CPU, continueToNextTtiLookup = GT_TRUE */
        /* AUTODOC:   IPv4 passenger packet type */
        /* AUTODOC:   DA=00:00:00:00:34:02 */
        /* AUTODOC:   dstIp=4.4.4.4 */
        rc = prvTgfTtiRule2Set(prvTgfTtiRuleLookup1Index, PRV_TGF_TTI_KEY_IPV4_E,
                              &ttiPattern, &ttiMask, &ttiAction);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTtiRule2Set");

        /* build action */
        prvTgfTunnelTermParallelLookupTtiBasicAction(GT_TRUE, CPSS_PACKET_CMD_TRAP_TO_CPU_E, &ttiAction);

        /* build rule for lookup 2 */
        prvTgfTunnelTermParallelLookupTtiBasicRule(
            1,/*pclId*/
            &prvTgfPacketL2Part.daMac,
            &prvTgfPacketIpv4Part.srcAddr,
            NULL,
            PRV_TGF_SEND_VLANID_CNS,
            &ttiPattern, &ttiMask);

        /* AUTODOC: add TTI rule 1 with IPv4 key on port 0 VLAN 5 with action: */
        /* AUTODOC:   cmd TRAP_TO_CPU, continueToNextTtiLookup = GT_TRUE */
        /* AUTODOC:   IPv4 passenger packet type */
        /* AUTODOC:   DA=00:00:00:00:34:02 */
        /* AUTODOC:   srcIp=10.10.10.10 */
        rc = prvTgfTtiRule2Set(prvTgfTtiRuleLookup2Index, PRV_TGF_TTI_KEY_IPV4_E,
                              &ttiPattern, &ttiMask, &ttiAction);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTtiRule2Set");

        /* build action */
        prvTgfTunnelTermParallelLookupTtiBasicAction(GT_FALSE, CPSS_PACKET_CMD_DROP_HARD_E, &ttiAction);

        /* build rule for lookup 3 */
        prvTgfTunnelTermParallelLookupTtiBasicRule(
            1,/*pclId*/
            &prvTgfPacketL2Part.daMac,
            NULL,
            &prvTgfPacketIpv4Part.dstAddr,
            PRV_TGF_SEND_VLANID_CNS,
            &ttiPattern, &ttiMask);

        /* AUTODOC: add TTI rule 15360 with IPv4 key on port 0 VLAN 5 with action: */
        /* AUTODOC:   cmd DROP_HARD, continueToNextTtiLookup = GT_FALSE */
        /* AUTODOC:   IPv4 passenger packet type */
        /* AUTODOC:   DA=00:00:00:00:34:02 */
        /* AUTODOC:   dstIp=4.4.4.4 */
        rc = prvTgfTtiRule2Set(prvTgfTtiRuleLookup3Index, PRV_TGF_TTI_KEY_IPV4_E,
                              &ttiPattern, &ttiMask, &ttiAction);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTtiRule2Set");
    }
}

/**
* @internal prvTgfTunnelTermParallelLookupRedirectRouteConfigurationSet function
* @endinternal
*
* @brief   Set LTT Route Configuration
*/
GT_VOID prvTgfTunnelTermParallelLookupRedirectRouteConfigurationSet
(
    GT_VOID
)
{
    GT_STATUS                               rc = GT_FAIL;
    GT_ETHERADDR                            arpMacAddr;
    PRV_TGF_IP_UC_ROUTE_ENTRY_STC           routeEntriesArray[1];
    PRV_TGF_IP_UC_ROUTE_ENTRY_STC           *regularEntryPtr;
    PRV_TGF_LPM_LEAF_ENTRY_STC              leafEntry;
    PRV_TGF_IP_TCAM_ROUTE_ENTRY_INFO_UNT    defIpv4UcRouteEntryInfo;
    PRV_TGF_IP_TCAM_ROUTE_ENTRY_INFO_UNT    defIpv6UcRouteEntryInfo;
    PRV_TGF_IP_LTT_ENTRY_STC                *ipLttEntryPtr = NULL;
    GT_U32                                  numOfPaths = 1;
    GT_U32                                  ii;

    PRV_UTF_LOG0_MAC("======= Setting LTT Route Configuration =======\n");


    /* AUTODOC: Enable Routing */

    /* AUTODOC: enable Unicast IPv4 Routing on port 0 */
    rc = prvTgfIpPortRoutingEnable(PRV_TGF_SEND_PORT_IDX_CNS, CPSS_IP_UNICAST_E,
                                   CPSS_IP_PROTOCOL_IPV4_E, GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfIpPortRoutingEnable: %d %d",
                                 prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);

    /* AUTODOC: update VRF Id in vlan[PRV_TGF_SEND_VLANID_CNS] */
    rc = prvTgfBrgVlanVrfIdSet(PRV_TGF_SEND_VLANID_CNS, prvUtfVrfId);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanVrfIdSet: %d %d",
                                 prvTgfDevNum, PRV_TGF_SEND_VLANID_CNS);

    /* AUTODOC: enable IPv4 Unicast Routing on Vlan 5 */
    rc = prvTgfIpVlanRoutingEnable(PRV_TGF_SEND_VLANID_CNS, CPSS_IP_UNICAST_E, CPSS_IP_PROTOCOL_IPV4_E, GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfIpVlanRoutingEnable: %d %d",
                                 prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);


    /* AUTODOC: Create the Route entry (Next hop) in Route table and Router ARP Table */

    /* AUTODOC: write a ARP MAC address to the Router ARP Table */
    cpssOsMemCpy(arpMacAddr.arEther, prvTgfArp3Mac, sizeof(TGF_MAC_ADDR));

    /* AUTODOC: write a ARP MAC 00:00:00:00:00:44 to the Router ARP Table */
    rc = prvTgfIpRouterArpAddrWrite(prvTgfRouterArp3Index, &arpMacAddr);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpRouterArpAddrWrite: %d", prvTgfDevNum);

    /* AUTODOC: write a UC Route entry to the Route Table */
    cpssOsMemSet(routeEntriesArray, 0, sizeof(routeEntriesArray));
    regularEntryPtr = &routeEntriesArray[0];

    regularEntryPtr->cmd                        = CPSS_PACKET_CMD_ROUTE_E;
    regularEntryPtr->nextHopVlanId              = PRV_TGF_NEXTHOPE_VLANID_CNS;
    regularEntryPtr->nextHopARPPointer          = prvTgfRouterArp3Index;

    regularEntryPtr->nextHopInterface.type      = CPSS_INTERFACE_PORT_E;
    regularEntryPtr->nextHopInterface.devPort.hwDevNum = prvTgfDevNum;
    regularEntryPtr->nextHopInterface.devPort.portNum = prvTgfPortsArray[PRV_TGF_NEXTHOPE2_PORT_IDX_CNS];

    /* AUTODOC: add UC route entry with nexthop VLAN 6 and nexthop port 3 */
    rc = prvTgfIpUcRouteEntriesWrite(prvTgfRouteEntryBaseIndex, routeEntriesArray, 1);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpUcRouteEntriesWrite: %d", prvTgfDevNum);

    /* AUTODOC:  Create Virtual Router [prvUtfVrfId] */

    if (0 != prvUtfVrfId)
    {
        /* clear entry */
        cpssOsMemSet(&defIpv4UcRouteEntryInfo, 0,
                     sizeof(PRV_TGF_IP_TCAM_ROUTE_ENTRY_INFO_UNT));
            ipLttEntryPtr = &defIpv4UcRouteEntryInfo.ipLttEntry;

        for (ii = 0; ii < 2; ii++)
        {
            if (ii == 0)
            {
                ipLttEntryPtr = &defIpv4UcRouteEntryInfo.ipLttEntry;
            }
            if (ii == 1)
            {
                ipLttEntryPtr = &defIpv6UcRouteEntryInfo.ipLttEntry;
            }
            /* set defUcRouteEntryInfo */
            ipLttEntryPtr->routeType                = PRV_TGF_IP_REGULAR_ROUTE_ENTRY_GROUP_MAC;
            ipLttEntryPtr->numOfPaths               = numOfPaths;
            ipLttEntryPtr->routeEntryBaseIndex      = prvTgfRouteEntryBaseIndex;
            ipLttEntryPtr->ucRPFCheckEnable         = GT_FALSE;
            ipLttEntryPtr->sipSaCheckMismatchEnable = GT_FALSE;
            ipLttEntryPtr->ipv6MCGroupScopeLevel    = CPSS_IPV6_PREFIX_SCOPE_LINK_LOCAL_E;
        }

        /* AUTODOC: create Virtual Router [prvUtfVrfId] with created default LTT entries */
        rc = prvTgfIpLpmVirtualRouterAdd(prvTgfLpmDBId, prvUtfVrfId,
                                         &defIpv4UcRouteEntryInfo,
                                         &defIpv6UcRouteEntryInfo,
                                         NULL, NULL, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
                                     "prvTgfIpLpmVirtualRouterAdd: %d",
                                     prvTgfDevNum);
    }

    cpssOsMemSet(&leafEntry, 0, sizeof(PRV_TGF_LPM_LEAF_ENTRY_STC));

    /* AUTODOC: write a leaf entry to the PBR area in RAM */
    leafEntry.entryType                = PRV_TGF_LPM_ROUTE_ENTRY_NEXT_PTR_TYPE_E;
    leafEntry.index                    = prvTgfRouteEntryBaseIndex;
    leafEntry.ucRPFCheckEnable         = GT_FALSE;
    leafEntry.sipSaCheckMismatchEnable = GT_FALSE;
    leafEntry.ipv6MCGroupScopeLevel    = 0;

    /* AUTODOC: Write a PBR leaf */
    prvTgfLpmLeafEntryWrite(prvTgfDevNum, prvTgfPbrLeafIndex, &leafEntry);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfLpmLeafEntryWrite: %d", prvTgfDevNum);
}

/**
* @internal prvTgfTunnelTermParallelLookupRedirectTtiRule0ConfigurationSet function
* @endinternal
*
* @brief   Set TTI Parallel lookup redirect Configuration
*
* @param[in] redirectCommand          - redirect command
*                                       None
*/
GT_VOID prvTgfTunnelTermParallelLookupRedirectTtiRule0ConfigurationSet
(
    PRV_TGF_TTI_REDIRECT_COMMAND_ENT    redirectCommand
)
{
    GT_STATUS                rc;
    PRV_TGF_TTI_ACTION_2_STC ttiAction;
    PRV_TGF_TTI_RULE_UNT     ttiPattern;
    PRV_TGF_TTI_RULE_UNT     ttiMask;
    GT_ETHERADDR             arpMacAddr;
    PRV_TGF_CNC_CONFIGURATION_STC cncCfg;
    PRV_TGF_CNC_COUNTER_STC counterValue0;

    PRV_UTF_LOG0_MAC("======= Setting TTI Configuration =======\n");

    prvTgfTunnelTermParallelLookupRuleIndexesCalculate();

    cpssOsMemSet(&ttiAction, 0, sizeof(ttiAction));
    cpssOsMemSet(&ttiPattern, 0, sizeof(ttiPattern));
    cpssOsMemSet(&ttiMask, 0, sizeof(ttiMask));

    /* AUTODOC: set the TTI Rule Pattern, Mask and Action for IPV4 TCAM location */
    /* AUTODOC: tti rule key is PRV_TGF_TTI_KEY_IPV4_E */
    /* AUTODOC: set basic configuration for TTI */
    prvTgfTunnelTermParallelLookupTtiBasicConfiguration();

    /* AUTODOC: set the TTI Rule Action for matched packet: redirect to egress */
    ttiAction.tunnelTerminate                  = GT_TRUE;
    ttiAction.command                          = CPSS_PACKET_CMD_FORWARD_E;
    ttiAction.redirectCommand                  = redirectCommand;
    ttiAction.egressInterface.type             = CPSS_INTERFACE_PORT_E;
    ttiAction.egressInterface.devPort.hwDevNum = prvTgfDevNum;
    ttiAction.egressInterface.devPort.portNum  = prvTgfPortsArray[PRV_TGF_NEXTHOPE_PORT_IDX_CNS];
    ttiAction.tag0VlanCmd                      = PRV_TGF_TTI_VLAN_MODIFY_ALL_E;
    ttiAction.tag0VlanId                       = 0x06;
    ttiAction.ttPassengerPacketType            = PRV_TGF_TTI_PASSENGER_IPV4_E;
    ttiAction.arpPtr                           = prvTgfRouterArp1Index;
    ttiAction.modifyMacDa                      = GT_TRUE;
    ttiAction.continueToNextTtiLookup          = GT_TRUE;
    ttiAction.bindToCentralCounter             = GT_TRUE;
    ttiAction.centralCounterIndex              = PRV_TGF_CNC0_COUNTER_NUM_CNS;

    /* build rule for lookup 0 */
    prvTgfTunnelTermParallelLookupTtiBasicRule(
        1,/*pclId*/
        &prvTgfPacketL2Part.daMac,
        &prvTgfPacketIpv4Part.srcAddr,
        NULL,
        PRV_TGF_SEND_VLANID_CNS,
        &ttiPattern, &ttiMask);

    /* AUTODOC: add TTI rule with IPv4 key on port 0 VLAN 5 with action: */
    /* AUTODOC:   cmd FORWARD, continueToNextTtiLookup = GT_TRUE */
    /* AUTODOC:   IPv4 passenger packet type */
    /* AUTODOC:   DA=00:00:00:00:34:02 */
    /* AUTODOC:   srcIp=10.10.10.10 */
    rc = prvTgfTtiRule2Set(prvTgfTtiRuleLookup0Index, PRV_TGF_TTI_KEY_IPV4_E,
                          &ttiPattern, &ttiMask, &ttiAction);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTtiRule2Set");

    /* AUTODOC: write a ARP MAC 00:00:00:00:00:22 to the Router ARP Table */
    cpssOsMemCpy(arpMacAddr.arEther, prvTgfArp1Mac, sizeof(TGF_MAC_ADDR));

    rc = prvTgfIpRouterArpAddrWrite(prvTgfRouterArp1Index, &arpMacAddr);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpRouterArpAddrWrite: %d", prvTgfDevNum);

    PRV_UTF_LOG0_MAC("======= Setting CNC Configuration =======\n");

    /* AUTODOC: define CNC configuration */
    cpssOsMemSet(&cncCfg, 0, sizeof(cncCfg));
    cpssOsMemSet(&counterValue0, 0, sizeof(counterValue0));

    /* common parameters */
    cncCfg.countingUnit  = PRV_TGF_CNC_COUNTING_ENABLE_UNIT_TTI_E;
    cncCfg.clientType    = PRV_TGF_CNC_CLIENT_TTI_PARALLEL_0_E;
    cncCfg.blockNum      = PRV_TGF_CNC0_BLOCK_NUM_CNS;
    cncCfg.counterNum    = PRV_TGF_CNC0_COUNTER_NUM_CNS;
    cncCfg.counterFormat = PRV_TGF_CNC_COUNTER_FORMAT_MODE_0_E;
    /* prvTgfCncCountingEnableSet */
    cncCfg.configureCountingUnitEnable = GT_TRUE;
    cncCfg.countingUnitEnable          = GT_TRUE;
    /* prvTgfCncBlockClientEnableSet */
    cncCfg.configureClientEnable       = GT_TRUE;
    cncCfg.clientEnable                = GT_TRUE;
    /* prvTgfCncBlockClientRangesBmpArrSet */
    cncCfg.configureIndexRangeBitMap   = GT_TRUE;
    cncCfg.indexRangeBitMap[0]         = 1; /* [1,2,3] - zeros */
    /* prvTgfCncCounterSet */
    cncCfg.configureCounterValue       = GT_TRUE;
    cncCfg.counterValue                = counterValue0;
    /* prvTgfCncClientByteCountModeSet */
    cncCfg.configureByteCountMode      = GT_TRUE;
    cncCfg.byteCountMode               = PRV_TGF_CNC_BYTE_COUNT_MODE_L2_E;
    /* prvTgfCncCounterClearByReadEnableSet */
    cncCfg.configureClearByReadEnable  = GT_TRUE;
    cncCfg.clearByReadEnable           = GT_TRUE;

    rc = prvTgfCncGenConfigure(
        &cncCfg, GT_TRUE /*stopOnErrors*/);
    UTF_VERIFY_EQUAL0_STRING_MAC(
        GT_OK, rc, "prvTgfCncGenConfigure");

}

/**
* @internal prvTgfTunnelTermParallelLookupRedirectTtiRule1ConfigurationSet function
* @endinternal
*
* @brief   Set TTI Parallel lookup redirect Configuration
*
* @param[in] redirectCommand          - redirect command
*                                       None
*/
GT_VOID prvTgfTunnelTermParallelLookupRedirectTtiRule1ConfigurationSet
(
    PRV_TGF_TTI_REDIRECT_COMMAND_ENT    redirectCommand
)
{
    GT_STATUS                rc;
    PRV_TGF_TTI_ACTION_2_STC ttiAction;
    PRV_TGF_TTI_RULE_UNT     ttiPattern;
    PRV_TGF_TTI_RULE_UNT     ttiMask;
    GT_ETHERADDR             arpMacAddr;
    PRV_TGF_CNC_CONFIGURATION_STC cncCfg;
    PRV_TGF_CNC_COUNTER_STC counterValue1;

    PRV_UTF_LOG0_MAC("======= Setting TTI Configuration =======\n");

    prvTgfTunnelTermParallelLookupRuleIndexesCalculate();

    cpssOsMemSet(&ttiAction, 0, sizeof(ttiAction));
    cpssOsMemSet(&ttiPattern, 0, sizeof(ttiPattern));
    cpssOsMemSet(&ttiMask, 0, sizeof(ttiMask));

    /* AUTODOC: set the TTI Rule Action for matched packet: redirect to egress */
    ttiAction.tunnelTerminate                  = GT_TRUE;
    ttiAction.command                          = CPSS_PACKET_CMD_FORWARD_E;
    ttiAction.redirectCommand                  = redirectCommand;
    ttiAction.egressInterface.type             = CPSS_INTERFACE_PORT_E;
    ttiAction.egressInterface.devPort.hwDevNum = prvTgfDevNum;
    ttiAction.egressInterface.devPort.portNum  = prvTgfPortsArray[PRV_TGF_NEXTHOPE1_PORT_IDX_CNS];
    ttiAction.tag0VlanCmd                      = PRV_TGF_TTI_VLAN_MODIFY_ALL_E;
    ttiAction.tag0VlanId                       = 0x06;
    ttiAction.ttPassengerPacketType            = PRV_TGF_TTI_PASSENGER_IPV4_E;
    if (PRV_CPSS_SIP_5_10_CHECK_MAC(prvTgfDevNum) == GT_TRUE)
    {
        ttiAction.continueToNextTtiLookup      = GT_TRUE;
    }
    else
    {
        ttiAction.continueToNextTtiLookup      = GT_FALSE;
    }
    ttiAction.arpPtr                           = prvTgfRouterArp2Index;
    ttiAction.modifyMacDa                      = GT_TRUE;
    ttiAction.bindToCentralCounter             = GT_TRUE;
    ttiAction.centralCounterIndex              = PRV_TGF_CNC1_COUNTER_NUM_CNS;

    /* build rule for lookup 1 */
    prvTgfTunnelTermParallelLookupTtiBasicRule(
        1,/*pclId*/
        &prvTgfPacketL2Part.daMac,
        NULL,
        &prvTgfPacketIpv4Part.dstAddr,
        PRV_TGF_SEND_VLANID_CNS,
        &ttiPattern, &ttiMask);

    /* AUTODOC:   add TTI rule with IPv4 key on port 0 VLAN 5 with action: */
    /* AUTODOC:   cmd FORWARD, continueToNextTtiLookup = GT_TRUE */
    /* AUTODOC:   IPv4 passenger packet type */
    /* AUTODOC:   DA=00:00:00:00:34:02 */
    /* AUTODOC:   dstIp=4.4.4.4 */
    rc = prvTgfTtiRule2Set(prvTgfTtiRuleLookup1Index, PRV_TGF_TTI_KEY_IPV4_E,
                          &ttiPattern, &ttiMask, &ttiAction);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTtiRule2Set");

    /* AUTODOC: write a ARP MAC 00:00:00:00:00:33 to the Router ARP Table */
    cpssOsMemCpy(arpMacAddr.arEther, prvTgfArp2Mac, sizeof(TGF_MAC_ADDR));

    rc = prvTgfIpRouterArpAddrWrite(prvTgfRouterArp2Index, &arpMacAddr);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpRouterArpAddrWrite: %d", prvTgfDevNum);

    PRV_UTF_LOG0_MAC("======= Setting CNC Configuration =======\n");

    /* AUTODOC: define CNC configuration */
    cpssOsMemSet(&cncCfg, 0, sizeof(cncCfg));
    cpssOsMemSet(&counterValue1, 0, sizeof(counterValue1));

    /* common parameters */
    cncCfg.countingUnit  = PRV_TGF_CNC_COUNTING_ENABLE_UNIT_TTI_E;
    cncCfg.clientType    = PRV_TGF_CNC_CLIENT_TTI_PARALLEL_1_E;
    cncCfg.blockNum      = PRV_TGF_CNC1_BLOCK_NUM_CNS;
    cncCfg.counterNum    = PRV_TGF_CNC1_COUNTER_NUM_CNS;
    cncCfg.counterFormat = PRV_TGF_CNC_COUNTER_FORMAT_MODE_0_E;
    /* prvTgfCncCountingEnableSet */
    cncCfg.configureCountingUnitEnable = GT_TRUE;
    cncCfg.countingUnitEnable          = GT_TRUE;
    /* prvTgfCncBlockClientEnableSet */
    cncCfg.configureClientEnable       = GT_TRUE;
    cncCfg.clientEnable                = GT_TRUE;
    /* prvTgfCncBlockClientRangesBmpArrSet */
    cncCfg.configureIndexRangeBitMap   = GT_TRUE;
    cncCfg.indexRangeBitMap[0]         = 1; /* [1,2,3] - zeros */
    /* prvTgfCncCounterSet */
    cncCfg.configureCounterValue       = GT_TRUE;
    cncCfg.counterValue                = counterValue1;
    /* prvTgfCncClientByteCountModeSet */
    cncCfg.configureByteCountMode      = GT_TRUE;
    cncCfg.byteCountMode               = PRV_TGF_CNC_BYTE_COUNT_MODE_L2_E;
    /* prvTgfCncCounterClearByReadEnableSet */
    cncCfg.configureClearByReadEnable  = GT_TRUE;
    cncCfg.clearByReadEnable           = GT_TRUE;

    rc = prvTgfCncGenConfigure(
        &cncCfg, GT_TRUE /*stopOnErrors*/);
    UTF_VERIFY_EQUAL0_STRING_MAC(
        GT_OK, rc, "prvTgfCncGenConfigure");

}

/**
* @internal prvTgfTunnelTermParallelLookupRedirectTtiRule2ConfigurationSet function
* @endinternal
*
* @brief   Set TTI Parallel lookup redirect Configuration
*
* @param[in] redirectCommand          - redirect command
* @param[in] changeVlan               - whether to change VlanId in TTI action
*                                       None
*/
GT_VOID prvTgfTunnelTermParallelLookupRedirectTtiRule2ConfigurationSet
(
    PRV_TGF_TTI_REDIRECT_COMMAND_ENT    redirectCommand,
    GT_BOOL                             changeVlan
)
{
    GT_STATUS                rc;
    PRV_TGF_TTI_ACTION_2_STC ttiAction;
    PRV_TGF_TTI_RULE_UNT     ttiPattern;
    PRV_TGF_TTI_RULE_UNT     ttiMask;

    PRV_UTF_LOG0_MAC("======= Setting TTI Configuration =======\n");

    prvTgfTunnelTermParallelLookupRuleIndexesCalculate();

    cpssOsMemSet(&ttiAction, 0, sizeof(ttiAction));
    cpssOsMemSet(&ttiPattern, 0, sizeof(ttiPattern));
    cpssOsMemSet(&ttiMask, 0, sizeof(ttiMask));

    /* AUTODOC: set the TTI Rule Action for matched packet: redirect to egress */
    ttiAction.tunnelTerminate                  = GT_TRUE;
    ttiAction.command                          = CPSS_PACKET_CMD_FORWARD_E;
    ttiAction.redirectCommand                  = redirectCommand;
    ttiAction.routerLttPtr                     = prvTgfPbrLeafIndex;
    ttiAction.egressInterface.type             = CPSS_INTERFACE_PORT_E;
    ttiAction.egressInterface.devPort.hwDevNum = prvTgfDevNum;
    ttiAction.egressInterface.devPort.portNum  = prvTgfPortsArray[PRV_TGF_NEXTHOPE2_PORT_IDX_CNS];
    ttiAction.tag0VlanCmd                      = PRV_TGF_TTI_VLAN_MODIFY_ALL_E;
    if ( changeVlan )
    {
        ttiAction.tag0VlanId                   = 0x05;
    }
    else
    {
        ttiAction.tag0VlanId                   = 0x06;
    }
    ttiAction.ttPassengerPacketType            = PRV_TGF_TTI_PASSENGER_IPV4_E;
    ttiAction.continueToNextTtiLookup          = GT_TRUE;
    ttiAction.bindToCentralCounter             = GT_FALSE;

    /* build rule for lookup 3 */
    prvTgfTunnelTermParallelLookupTtiBasicRule(
        1,/*pclId*/
        &prvTgfPacketL2Part.daMac,
        NULL,
        &prvTgfPacketIpv4Part.dstAddr,
        PRV_TGF_SEND_VLANID_CNS,
        &ttiPattern, &ttiMask);

    /* AUTODOC:   add TTI rule with IPv4 key on port 0 VLAN 5 with action: */
    /* AUTODOC:   cmd FORWARD, continueToNextTtiLookup = GT_FALSE */
    /* AUTODOC:   IPv4 passenger packet type */
    /* AUTODOC:   DA=00:00:00:00:34:02 */
    /* AUTODOC:   dstIp=4.4.4.4 */
    rc = prvTgfTtiRule2Set(prvTgfTtiRuleLookup2Index, PRV_TGF_TTI_KEY_IPV4_E,
                          &ttiPattern, &ttiMask, &ttiAction);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTtiRule2Set");

}

/**
* @internal prvTgfTunnelTermParallelLookupRedirectTtiRule3ConfigurationSet function
* @endinternal
*
* @brief   Set TTI Dual lookup redirect Configuration
*
* @param[in] command                  - packet command
*                                       None
*/
GT_VOID prvTgfTunnelTermParallelLookupRedirectTtiRule3ConfigurationSet
(
    CPSS_PACKET_CMD_ENT    command
)
{
    GT_STATUS                rc;
    PRV_TGF_TTI_ACTION_2_STC ttiAction;
    PRV_TGF_TTI_RULE_UNT     ttiPattern;
    PRV_TGF_TTI_RULE_UNT     ttiMask;
    GT_ETHERADDR             arpMacAddr;

    PRV_UTF_LOG0_MAC("======= Setting TTI Configuration =======\n");

    prvTgfTunnelTermParallelLookupRuleIndexesCalculate();

    cpssOsMemSet(&ttiAction, 0, sizeof(ttiAction));
    cpssOsMemSet(&ttiPattern, 0, sizeof(ttiPattern));
    cpssOsMemSet(&ttiMask, 0, sizeof(ttiMask));

    /* AUTODOC: set the TTI Rule Action for matched packet: redirect to egress */
    ttiAction.tunnelTerminate                  = GT_TRUE;
    ttiAction.command                          = command;
    ttiAction.redirectCommand                  = PRV_TGF_TTI_NO_REDIRECT_E;
    ttiAction.egressInterface.type             = CPSS_INTERFACE_PORT_E;
    ttiAction.egressInterface.devPort.hwDevNum = prvTgfDevNum;
    ttiAction.egressInterface.devPort.portNum  = prvTgfPortsArray[PRV_TGF_NEXTHOPE_PORT_IDX_CNS];
    ttiAction.ttPassengerPacketType            = PRV_TGF_TTI_PASSENGER_IPV4_E;
    ttiAction.arpPtr                           = prvTgfRouterArp1Index;
    ttiAction.modifyMacDa                      = GT_TRUE;
    ttiAction.continueToNextTtiLookup          = GT_FALSE;
    ttiAction.bindToCentralCounter             = GT_FALSE;

    /* build rule for lookup 3 */
    prvTgfTunnelTermParallelLookupTtiBasicRule(
        1,/*pclId*/
        &prvTgfPacketL2Part.daMac,
        &prvTgfPacketIpv4Part.srcAddr,
        NULL,
        PRV_TGF_SEND_VLANID_CNS,
        &ttiPattern, &ttiMask);

    /* AUTODOC:   add TTI rule with IPv4 key on port 0 VLAN 5 with action: */
    /* AUTODOC:   cmd FORWARD, continueToNextTtiLookup = GT_TRUE */
    /* AUTODOC:   IPv4 passenger packet type */
    /* AUTODOC:   DA=00:00:00:00:34:02 */
    /* AUTODOC:   srcIp=10.10.10.10 */
    rc = prvTgfTtiRule2Set(prvTgfTtiRuleLookup3Index, PRV_TGF_TTI_KEY_IPV4_E,
                          &ttiPattern, &ttiMask, &ttiAction);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTtiRule2Set");

    /* AUTODOC: write a ARP MAC 00:00:00:00:00:22 to the Router ARP Table */
    cpssOsMemCpy(arpMacAddr.arEther, prvTgfArp1Mac, sizeof(TGF_MAC_ADDR));

    rc = prvTgfIpRouterArpAddrWrite(prvTgfRouterArp1Index, &arpMacAddr);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpRouterArpAddrWrite: %d", prvTgfDevNum);

}

/**
* @internal prvTgfTunnelTermParallelLookupEnableLookup function
* @endinternal
*
* @brief   Enable/disable continuing to next TTI lookup
*
* @param[in] lookup                   -  number
* @param[in] nextLookupEnable         - enable/disable TTI lookup
*                                       None
*/
GT_VOID prvTgfTunnelTermParallelLookupEnableLookup
(
    GT_U32  lookup,
    GT_BOOL nextLookupEnable
)
{
    PRV_TGF_TTI_ACTION_2_STC ttiAction;
    GT_U32                   lookupIndex = prvTgfTtiRuleLookup0Index;
    CPSS_PACKET_CMD_ENT      command = CPSS_PACKET_CMD_FORWARD_E;

    switch (lookup)
    {
        case 0:
            lookupIndex = prvTgfTtiRuleLookup0Index;
            command = CPSS_PACKET_CMD_FORWARD_E;
            break;
        case 1:
            lookupIndex = prvTgfTtiRuleLookup1Index;
            command = CPSS_PACKET_CMD_MIRROR_TO_CPU_E;
            break;
        case 2:
            lookupIndex = prvTgfTtiRuleLookup2Index;
            command = CPSS_PACKET_CMD_TRAP_TO_CPU_E;
            break;
        case 3:
            lookupIndex = prvTgfTtiRuleLookup3Index;
            command = CPSS_PACKET_CMD_DROP_HARD_E;
            break;
        default:
            break;
    }
    prvTgfTunnelTermParallelLookupRuleIndexesCalculate();
    cpssOsMemSet(&ttiAction, 0, sizeof(PRV_TGF_TTI_ACTION_2_STC));
    prvTgfTunnelTermParallelLookupTtiBasicAction(nextLookupEnable, command, &ttiAction);
    prvTgfTtiRuleAction2Set(lookupIndex, &ttiAction);
}

/**
* @internal prvTgfTunnelTermParallelLookupEnable function
* @endinternal
*
* @brief   Set TCAM segment mode for IPv4 key
*
* @param[in] enable                   - parallel lookup is enabled/disabled
*                                       None
*/
GT_VOID prvTgfTunnelTermParallelLookupEnable
(
    GT_BOOL enable
)
{
    PRV_TGF_TTI_KEY_TCAM_SEGMENT_MODE_ENT segmentMode;
    GT_STATUS rc;

    if (enable == GT_TRUE)
    {
        if (PRV_CPSS_SIP_5_10_CHECK_MAC(prvTgfDevNum) == GT_TRUE)
        {
            segmentMode = PRV_TGF_TTI_KEY_TCAM_SEGMENT_MODE_FOUR_TCAM_E;
        }
        else
        {
            segmentMode = PRV_TGF_TTI_KEY_TCAM_SEGMENT_MODE_TWO_TCAM_E;
        }
    }
    else
    {
        segmentMode = PRV_TGF_TTI_KEY_TCAM_SEGMENT_MODE_ONE_TCAM_E;
    }
    /* AUTODOC: set TCAM segment mode for IPv4 key */
    rc = prvTgfTtiTcamSegmentModeSet(prvTgfDevNum, PRV_TGF_TTI_KEY_IPV4_E, segmentMode);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTtiTcamSegmentModeSet");
}

/**
* @internal prvTgfTunnelTermParallelLookupTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic
*
* @param[in] isPacketExpectedOnCpu    - is packet mirrored  to CPU
*                                       None
*/
GT_VOID prvTgfTunnelTermParallelLookupTrafficGenerate
(
    GT_BOOL    isPacketExpectedOnCpu
)
{
    prvTgfTunnelTermIpv4overIpv4TrafficGenerate(PRV_TGF_NEXTHOPE_PORT_IDX_CNS, GT_TRUE, PRV_TGF_DEFAULT_BOOLEAN_CNS);

    /* AUTODOC: check CPU Rx packets - packets expected */
    prvTgfTunnelTermParallelLookupCheckPacketOn(isPacketExpectedOnCpu);
}



/**
* @internal prvTgfTunnelTermParallelLookupTrafficGenerateExpectNoTraffic function
* @endinternal
*
* @brief   Generate traffic - expect no traffic
*
* @param[in] isPacketExpectedOnCpu    - is packet trapped to CPU
*                                       None
*/
GT_VOID prvTgfTunnelTermParallelLookupTrafficGenerateExpectNoTraffic
(
    GT_BOOL    isPacketExpectedOnCpu
)
{

    prvTgfTunnelTermIpv4overIpv4TrafficGenerateExpectNoTraffic(GT_TRUE, PRV_TGF_DEFAULT_BOOLEAN_CNS);

    /* AUTODOC: check CPU Rx packets - packets expected */
    prvTgfTunnelTermParallelLookupCheckPacketOn(isPacketExpectedOnCpu);
}

/**
* @internal prvTgfTunnelTermParallelLookupCncRestore function
* @endinternal
*
* @brief   Restore CNC Configuration per lookup.
*/
GT_VOID prvTgfTunnelTermParallelLookupCncRestore
(
    GT_VOID
)
{
    GT_U32 lookupNum;
    PRV_TGF_CNC_CONFIGURATION_STC     cncCfg;

    cpssOsMemSet(&cncCfg, 0, sizeof(cncCfg));
    /* common parameters */
    cncCfg.countingUnit  = PRV_TGF_CNC_COUNTING_ENABLE_UNIT_TTI_E;
    cncCfg.counterFormat = PRV_TGF_CNC_COUNTER_FORMAT_MODE_0_E;
    cncCfg.configureClientEnable     = GT_TRUE;
    cncCfg.clientEnable              = GT_FALSE;
    cncCfg.configureIndexRangeBitMap = GT_TRUE; /* indexRangeBitMap[0,1,2,3] - zeros */
    cncCfg.configureCounterValue = GT_TRUE;     /*counterValue == 0*/
    for (lookupNum = 0; lookupNum < 2; lookupNum++)
    {
        cncCfg.clientType  = (lookupNum == 0) ? PRV_TGF_CNC_CLIENT_TTI_PARALLEL_0_E :
                                                PRV_TGF_CNC_CLIENT_TTI_PARALLEL_1_E;
        cncCfg.counterNum  = (lookupNum == 0) ? PRV_TGF_CNC0_COUNTER_NUM_CNS :
                                                PRV_TGF_CNC1_COUNTER_NUM_CNS;
        cncCfg.blockNum    = (lookupNum == 0) ? PRV_TGF_CNC0_BLOCK_NUM_CNS :
                                                PRV_TGF_CNC1_BLOCK_NUM_CNS;

        prvTgfCncGenConfigure(
            &cncCfg, GT_FALSE /*stopOnErrors*/);
    }
}

/**
* @internal prvTgfTunnelTermParallelLookupConfigurationRestore function
* @endinternal
*
* @brief   Restore configuration
*/
GT_VOID prvTgfTunnelTermParallelLookupConfigurationRestore
(
    GT_VOID
)
{
    GT_STATUS  rc;

    prvTgfTunnelTermParallelLookupRuleIndexesCalculate();

    prvTgfTunnelTermIpv4overIpv4ConfigurationRestore(GT_FALSE, GT_TRUE);

    /* AUTODOC: restore TCAM segment mode for IPv4 key */
    rc = prvTgfTtiTcamSegmentModeSet(prvTgfDevNum, PRV_TGF_TTI_KEY_IPV4_E, prvTgfRestoreCfg.segmentMode);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTtiTcamSegmentModeSet");

    /* AUTODOC: invalidate lookup 0 TTI rule */
    rc = prvTgfTtiRuleValidStatusSet(prvTgfTtiRuleLookup0Index, GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiRuleValidStatusSet: %d", prvTgfDevNum);

    /* AUTODOC: invalidate lookup 1 TTI rule */
    rc = prvTgfTtiRuleValidStatusSet(prvTgfTtiRuleLookup1Index, GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiRuleValidStatusSet: %d", prvTgfDevNum);

    if (PRV_CPSS_SIP_5_10_CHECK_MAC(prvTgfDevNum) == GT_TRUE)
    {
        /* AUTODOC: invalidate lookup 2 TTI rule */
        rc = prvTgfTtiRuleValidStatusSet(prvTgfTtiRuleLookup2Index, GT_FALSE);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiRuleValidStatusSet: %d", prvTgfDevNum);

        /* AUTODOC: invalidate lookup 3 TTI rule */
        rc = prvTgfTtiRuleValidStatusSet(prvTgfTtiRuleLookup3Index, GT_FALSE);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiRuleValidStatusSet: %d", prvTgfDevNum);
    }

    /* AUTODOC: clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");
}


