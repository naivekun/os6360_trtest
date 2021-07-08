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
* @file prvTgfPclRedirectToLTT.c
*
* @brief PCL action redirect to LTT
*
* @version   13
********************************************************************************
*/

#include <cpssCommon/cpssPresteraDefs.h>
#include <cpss/extServices/private/prvCpssBindFunc.h>
#include <cpss/generic/pcl/cpssPcl.h>
#include <gtOs/gtOsTimer.h>

#include <utf/utfMain.h>
#include <utf/private/prvUtfHelpers.h>

#include <trafficEngine/tgfTrafficGenerator.h>
#include <trafficEngine/tgfTrafficTable.h>

#include <common/tgfCommon.h>
#include <common/tgfPclGen.h>
#include <common/tgfIpGen.h>
#include <common/tgfBridgeGen.h>
#include <pcl/prvTgfPclRedirectToLTT.h>

#define PRV_TGF_DUMP_PACKTS_CNS       0

/* default VLAN Id */
#define PRV_TGF_SEND_VLANID_CNS       1

/* port number to send traffic to */
#define PRV_TGF_SEND_PORT_IDX_CNS     1

/* nextHop port number to receive traffic from */
#define PRV_TGF_NEXTHOPE_PORT_IDX_CNS 3

/* amount of sent packets */
#define PRV_TGF_PACKET_NUM_CNS 1

/* the ARP MAC address to write to the Router ARP Table */
static TGF_MAC_ADDR  prvTgfArpMac = {0x00, 0x12, 0x23, 0x45, 0x67, 0x89};

/* PCL rule index */
static GT_U32   prvTgfPclRuleIndex = 1;

/* the Route entry index for UC Route entry Table */
static GT_U32   prvTgfRouteEntryBaseIndex = 8;

/* the leaf index in the RAM PBR section */
static GT_U32   prvTgfPbrLeafIndex = 964;

/* the Arp Address index of the Router ARP Table
   to write to the UC Route entry Arp nextHopARPPointer field */
static GT_U32   prvTgfRouterArpIndex      = 1;

/* stored default Vlan ID */
static GT_U16   prvTgfDefVlanId = 0;

/* original enabling status of bypassing the router triggering requirements for
   PBR packets */
static GT_BOOL pbrBypassTrigger;

/******************************* Test packet **********************************/

/* L2 part of packet */
static TGF_PACKET_L2_STC prvTgfPacketL2Part = {
    {0x00, 0x00, 0x00, 0x00, 0x34, 0x02},               /* daMac */
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x11}                /* saMac */
};

/* packet's ethertype */
static TGF_PACKET_ETHERTYPE_STC prvTgfPacketEtherTypePart = {TGF_ETHERTYPE_0800_IPV4_TAG_CNS};
/* packet's IPv4 */
static TGF_PACKET_IPV4_STC prvTgfPacketIpv4Part = {
    4,                  /* version */
    5,                  /* headerLen */
    0,                  /* typeOfService */
    0x2b,               /* totalLen */
    0,                  /* id */
    0,                  /* flags */
    0,                  /* offset */
    0x40,               /* timeToLive */
    0xFF,               /* protocol */
    0x73CD,             /* csum */
    { 2,  2,  2,  2},   /* srcAddr */
    { 1,  1,  1,  3}    /* dstAddr */
};
/* DATA of packet */
static GT_U8 prvTgfPayloadDataArr[] = {
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
    0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
    0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
    0x18, 0x19
};
/* PAYLOAD part */
static TGF_PACKET_PAYLOAD_STC prvTgfPacketPayloadPart = {
    sizeof(prvTgfPayloadDataArr),                       /* dataLength */
    prvTgfPayloadDataArr                                /* dataPtr */
};

/* PARTS of packet */
static TGF_PACKET_PART_STC prvTgfPacketPartArray[] = {
    {TGF_PACKET_PART_L2_E,        &prvTgfPacketL2Part},  /* type, partPtr */
    {TGF_PACKET_PART_ETHERTYPE_E, &prvTgfPacketEtherTypePart},
    {TGF_PACKET_PART_IPV4_E,      &prvTgfPacketIpv4Part},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacketPayloadPart}
};

/**
* @internal prvTgfTrafficGenerateWithCapture function
* @endinternal
*
* @brief   None
*/
static GT_STATUS prvTgfTrafficGenerateWithCapture
(
    GT_VOID
)
{
    GT_STATUS               rc = GT_OK;
    CPSS_INTERFACE_INFO_STC portInterface;
    GT_U32                  partsCount  = 0;
    GT_U32                  packetSize  = 0;
    TGF_PACKET_STC          packetInfo;

    /* clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    PRV_UTF_VERIFY_GT_OK(rc, "tgfTrafficTableRxPcktTblClear");

    /* setup receive portInterface for capturing */
    portInterface.type            = CPSS_INTERFACE_PORT_E;
    portInterface.devPort.hwDevNum  = prvTgfDevNum;
    portInterface.devPort.portNum =
        prvTgfPortsArray[PRV_TGF_NEXTHOPE_PORT_IDX_CNS];

    /* enable capture on receive port */
    rc = tgfTrafficGeneratorPortTxEthCaptureSet(
            &portInterface, TGF_CAPTURE_MODE_MIRRORING_E, GT_TRUE);
    PRV_UTF_VERIFY_GT_OK(rc, "tgftrafficgeneratorporttxethcaptureset");


    /* number of parts in packet */
    partsCount = sizeof(prvTgfPacketPartArray) / sizeof(prvTgfPacketPartArray[0]);

    /* calculate packet size */
    rc = prvTgfPacketSizeGet(prvTgfPacketPartArray, partsCount, &packetSize);
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfPacketSizeGet");

    /* build packet */
    packetInfo.totalLen   = packetSize;
    packetInfo.numOfParts = partsCount;
    packetInfo.partsArray = prvTgfPacketPartArray;

    /* setup transmit params */
    rc = prvTgfSetTxSetupEth(
        prvTgfDevNum, &packetInfo,
        PRV_TGF_PACKET_NUM_CNS /*burstCount*/, 0 /*numVfd*/, NULL);
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfSetTxSetupEth");

    /* AUTODOC: send Ethernet packet on port 1 with: */
    /* AUTODOC:   DA=00:00:00:00:34:02, SA=00:00:00:00:00:11 */
    /* AUTODOC:   srcIP=2.2.2.2, dstIP=1.1.1.3 */
    rc = prvTgfStartTransmitingEth(
        prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfStartTransmitingEth");

    /* enable capture on receive port */
    rc = tgfTrafficGeneratorPortTxEthCaptureSet(
            &portInterface, TGF_CAPTURE_MODE_MIRRORING_E, GT_FALSE);
    PRV_UTF_VERIFY_GT_OK(rc, "tgftrafficgeneratorporttxethcaptureset");

    return rc;
}

/**
* @internal prvTgfTrafficCheckCapturedByField function
* @endinternal
*
* @brief   None
*/
static GT_STATUS prvTgfTrafficCheckCapturedByField
(
    IN GT_U8                devNum,
    IN GT_U32                portNum,
    IN GT_U32               packetAmount,
    IN GT_U32               fieldOffset,
    IN GT_U32               fieldLength,
    IN GT_U8                fieldArr[]
)
{
    GT_STATUS       rc, rc1;
    GT_U8           dev      = 0;
    GT_U8           queue    = 0;
    GT_BOOL         getFirst = GT_TRUE;
    GT_U32          buffLen  = TGF_RX_BUFFER_MAX_SIZE_CNS;
    GT_U32          packetActualLength = 0;
    TGF_NET_DSA_STC rxParam;
    static GT_U8    packetBuff[TGF_RX_BUFFER_MAX_SIZE_CNS] = {0};
    CPSS_INTERFACE_INFO_STC portInterface;

    portInterface.type = CPSS_INTERFACE_PORT_E;
    portInterface.devPort.hwDevNum = devNum;
    portInterface.devPort.portNum = portNum;

    rc1  = GT_OK;

    for (; (packetAmount > 0); packetAmount--)
    {
        buffLen  = TGF_RX_BUFFER_MAX_SIZE_CNS;
        /* get entry from rxNetworkIf table */
        rc = tgfTrafficGeneratorRxInCpuFromExpectedInterfaceGet(&portInterface,
                                            TGF_PACKET_TYPE_CAPTURE_E,
                                           getFirst, GT_TRUE, packetBuff,
                                           &buffLen, &packetActualLength,
                                           &dev, &queue, &rxParam);
        PRV_UTF_VERIFY_GT_OK(rc, "tgfTrafficGeneratorRxInCpuGet");

        getFirst = GT_FALSE;

#if PRV_TGF_DUMP_PACKTS_CNS
        {
            int i;
            for (i = 0; (i < 24); i++)
            {
                if ((i % 6) == 0)
                {
                    PRV_UTF_LOG0_MAC("  ");
                }

                PRV_UTF_LOG1_MAC("%02X", packetBuff[i]);
            }
            PRV_UTF_LOG0_MAC("\n");
        }
#endif /*PRV_TGF_DUMP_PACKTS_CNS*/


        /* trace packet */

        if (packetActualLength < (fieldOffset + fieldLength))
        {
            rc1 = GT_FAIL;
        }

        if (0 != cpssOsMemCmp(fieldArr, &(packetBuff[fieldOffset]), fieldLength))
        {
            rc1 = GT_FAIL;
        }

    }
    return rc1;
}

/**
* @internal prvTgfConfigurationRestore function
* @endinternal
*
* @brief   None
*/
static GT_STATUS prvTgfConfigurationRestore
(
    GT_VOID
)
{
    GT_STATUS rc, rc1 = GT_OK;

    /* AUTODOC: disable ingress policy on port 1 */
    rc = prvTgfPclPortIngressPolicyEnable(
        prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], GT_FALSE);
    PRV_UTF_VERIFY_RC1(rc, "prvTgfPclPortIngressPolicyEnable");

    /* AUTODOC: invalidate PCL rules */
    rc = prvTgfPclRuleValidStatusSet(
            CPSS_PCL_RULE_SIZE_STD_E, prvTgfPclRuleIndex, GT_FALSE);
    PRV_UTF_VERIFY_RC1(rc, "prvTgfPclRuleValidStatusSet");

    /* enables ingress policy */
    rc = prvTgfPclPortIngressPolicyEnable(
        prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], GT_FALSE);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: prvTgfPclPortIngressPolicyEnable FAILED, rc = [%d]", rc);

        return rc;
    }

    /* clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    PRV_UTF_VERIFY_RC1(rc, "tgfTrafficTableRxPcktTblClear");


    /* Restore Route/Basic Configuration*/

    /* AUTODOC: restore default vlanId to all ports */
    rc = prvTgfBrgVlanPvidSet(prvTgfDefVlanId);
    PRV_UTF_VERIFY_RC1(rc, "prvTgfBrgVlanPvidSet");

    /* AUTODOC: disable UC IPv4 Routing on port 1 */
    rc = prvTgfIpPortRoutingEnable(PRV_TGF_SEND_PORT_IDX_CNS, CPSS_IP_UNICAST_E,
                                   CPSS_IP_PROTOCOL_IPV4_E, GT_FALSE);
    PRV_UTF_VERIFY_RC1(rc, "prvTgfIpPortRoutingEnable");

    /* AUTODOC: disable IPv4 Unicast Routing on Vlan 1 */
    rc = prvTgfIpVlanRoutingEnable(PRV_TGF_SEND_VLANID_CNS, CPSS_IP_UNICAST_E, CPSS_IP_PROTOCOL_IPV4_E, GT_FALSE);
    PRV_UTF_VERIFY_RC1(rc, "prvTgfIpVlanRoutingEnable");

    /* flush FDB include static entries */
    rc = prvTgfBrgFdbFlush(GT_TRUE);
    PRV_UTF_VERIFY_RC1(rc, "prvTgfBrgFdbFlush");

    if ((PRV_CPSS_SIP_5_CHECK_MAC(prvTgfDevNum)))
    {
        /* Restore enabling status of bypassing the router triggering requirements for PBR packets */
        rc = prvTgfIpPbrBypassRouterTriggerRequirementsEnableSet(prvTgfDevNum, pbrBypassTrigger);
        PRV_UTF_VERIFY_RC1(rc, "prvTgfIpPbrBypassRouterTriggerRequirementsEnableSet");
    }

    return rc1;
};

/**
* @internal prvTgfTrafficLttRouteConfigurationSet function
* @endinternal
*
* @brief   Set LTT Route Configuration
*/
static GT_STATUS prvTgfTrafficLttRouteConfigurationSet
(
    GT_VOID
)
{
    GT_STATUS                               rc, rc1 = GT_OK;
    GT_ETHERADDR                            arpMacAddr;
    PRV_TGF_IP_UC_ROUTE_ENTRY_STC           routeEntriesArray[1];
    PRV_TGF_IP_UC_ROUTE_ENTRY_STC           *regularEntryPtr;
    PRV_TGF_IP_LTT_ENTRY_STC                lttEntry;
    PRV_TGF_LPM_LEAF_ENTRY_STC              leafEntry;
    GT_U32                                  portIter;
    PRV_TGF_BRG_MAC_ENTRY_STC               macEntry;

    /* clear also counters at end of test */
    for(portIter = 0; portIter < prvTgfPortsNum ; portIter++)
    {
        rc = prvTgfResetCountersEth(prvTgfDevsArray[portIter],
                                    prvTgfPortsArray[portIter]);
        PRV_UTF_VERIFY_RC1(rc,"prvTgfResetCountersEth");
    }

    PRV_UTF_LOG0_MAC("======= Setting LTT Route Configuration =======\n");

    /* -------------------------------------------------------------------------
     * 0. Set base configuration
     */

    /* get default vlanId */
    rc = prvTgfBrgVlanPortVidGet(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], &prvTgfDefVlanId);
    PRV_UTF_VERIFY_RC1(rc, "prvTgfBrgVlanPortVidGet");

    /* AUTODOC: set PVID 2 for port 1 */
    rc = prvTgfBrgVlanPortVidSet(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], PRV_TGF_SEND_VLANID_CNS);
    PRV_UTF_VERIFY_RC1(rc, "prvTgfBrgVlanPortVidSet");

    /* -------------------------------------------------------------------------
     * 1. Enable Routing
     */

    /* AUTODOC: enable UC IPv4 Routing on port 1 */
    rc = prvTgfIpPortRoutingEnable(PRV_TGF_SEND_PORT_IDX_CNS, CPSS_IP_UNICAST_E,
                                   CPSS_IP_PROTOCOL_IPV4_E, GT_TRUE);
    PRV_UTF_VERIFY_RC1(rc, "prvTgfIpPortRoutingEnable");

    /* AUTODOC: enable IPv4 UC Routing on Vlan 1 */
    rc = prvTgfIpVlanRoutingEnable(PRV_TGF_SEND_VLANID_CNS, CPSS_IP_UNICAST_E, CPSS_IP_PROTOCOL_IPV4_E, GT_TRUE);
    PRV_UTF_VERIFY_RC1(rc, "prvTgfIpVlanRoutingEnable");

    /* -------------------------------------------------------------------------
     * 2. Create the Route entry (Next hop) in Route table and Router ARP Table
     */

    /* write a ARP MAC address to the Router ARP Table */
    cpssOsMemCpy(arpMacAddr.arEther, prvTgfArpMac, sizeof(TGF_MAC_ADDR));

    /* AUTODOC: write ARP MAC 00:12:23:45:67:89 to the Router ARP Table */
    rc = prvTgfIpRouterArpAddrWrite(prvTgfRouterArpIndex, &arpMacAddr);
    PRV_UTF_VERIFY_RC1(rc, "prvTgfIpRouterArpAddrWrite");

    /* write a UC Route entry to the Route Table */
    cpssOsMemSet(routeEntriesArray, 0, sizeof(routeEntriesArray));

    regularEntryPtr = &routeEntriesArray[0];
    regularEntryPtr->cmd                        = CPSS_PACKET_CMD_ROUTE_E;
    regularEntryPtr->cpuCodeIndex               = 0;
    regularEntryPtr->appSpecificCpuCodeEnable   = GT_FALSE;
    regularEntryPtr->unicastPacketSipFilterEnable = GT_FALSE;
    regularEntryPtr->ttlHopLimitDecEnable       = GT_FALSE;
    regularEntryPtr->ttlHopLimDecOptionsExtChkByPass = GT_FALSE;
    regularEntryPtr->ingressMirror              = GT_FALSE;
    regularEntryPtr->qosProfileMarkingEnable    = GT_FALSE;
    regularEntryPtr->qosProfileIndex            = 0;
    regularEntryPtr->qosPrecedence              = CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_SOFT_E;
    regularEntryPtr->modifyUp                   = CPSS_PACKET_ATTRIBUTE_MODIFY_KEEP_PREVIOUS_E;
    regularEntryPtr->modifyDscp                 = CPSS_PACKET_ATTRIBUTE_MODIFY_KEEP_PREVIOUS_E;
    regularEntryPtr->countSet                   = 0;
    regularEntryPtr->trapMirrorArpBcEnable      = GT_FALSE;
    regularEntryPtr->sipAccessLevel             = 0;
    regularEntryPtr->dipAccessLevel             = 0;
    regularEntryPtr->ICMPRedirectEnable         = GT_FALSE;
    regularEntryPtr->scopeCheckingEnable        = GT_FALSE;
    regularEntryPtr->siteId                     = CPSS_IP_SITE_ID_INTERNAL_E;
    regularEntryPtr->mtuProfileIndex            = 0;
    regularEntryPtr->isTunnelStart              = GT_FALSE;
    regularEntryPtr->nextHopVlanId              = PRV_TGF_SEND_VLANID_CNS;
    regularEntryPtr->nextHopInterface.type      = CPSS_INTERFACE_PORT_E;
    regularEntryPtr->nextHopInterface.devPort.hwDevNum = prvTgfDevNum;
    regularEntryPtr->nextHopInterface.devPort.portNum = prvTgfPortsArray[PRV_TGF_NEXTHOPE_PORT_IDX_CNS];
    regularEntryPtr->nextHopARPPointer          = prvTgfRouterArpIndex;
    regularEntryPtr->nextHopTunnelPointer       = 0;

    /* AUTODOC: add UC route entry 8 with nexthop VLAN 1 and nexthop port 3 */
    rc = prvTgfIpUcRouteEntriesWrite(prvTgfRouteEntryBaseIndex, routeEntriesArray, 1);
    PRV_UTF_VERIFY_RC1(rc, "prvTgfIpUcRouteEntriesWrite");

    /* AUTODOC: read and check the UC Route entry from the Route Table */
    cpssOsMemSet(routeEntriesArray, 0, sizeof(routeEntriesArray));

    rc = prvTgfIpUcRouteEntriesRead(prvTgfDevNum, prvTgfRouteEntryBaseIndex, routeEntriesArray, 1);
    PRV_UTF_VERIFY_RC1(rc, "prvTgfIpUcRouteEntriesRead");

    PRV_UTF_LOG2_MAC("nextHopVlanId = %d, portNum = %d\n",
                     routeEntriesArray[0].nextHopVlanId,
                     routeEntriesArray[0].nextHopInterface.devPort.portNum);

    /* -------------------------------------------------------------------------
     * 3. Create a LookUp Translation Table Entry
     */

    if ((PRV_CPSS_SIP_5_CHECK_MAC(prvTgfDevNum)))
    {
        cpssOsMemSet(&leafEntry, 0, sizeof(leafEntry));

        /* write a leaf entry to the PBR area in RAM */
        leafEntry.entryType                = PRV_TGF_LPM_ROUTE_ENTRY_NEXT_PTR_TYPE_E;
        leafEntry.index                    = prvTgfRouteEntryBaseIndex;
        leafEntry.ucRPFCheckEnable         = GT_FALSE;
        leafEntry.sipSaCheckMismatchEnable = GT_FALSE;
        leafEntry.ipv6MCGroupScopeLevel    = 0;
        /* AUTODOC: Write a PBR leaf */
        prvTgfLpmLeafEntryWrite(prvTgfDevNum, prvTgfPbrLeafIndex, &leafEntry);
        PRV_UTF_VERIFY_RC1(rc, "prvTgfLpmLeafEntryWrite");
    }
    else
    {
        /* write a LTT Entry to Route the test packet */
        lttEntry.routeType                = PRV_TGF_IP_REGULAR_ROUTE_ENTRY_GROUP_MAC;
        lttEntry.numOfPaths               = 0;
        lttEntry.routeEntryBaseIndex      = prvTgfRouteEntryBaseIndex;
        lttEntry.ucRPFCheckEnable         = GT_FALSE;
        lttEntry.sipSaCheckMismatchEnable = GT_FALSE;
        lttEntry.ipv6MCGroupScopeLevel    = 0;
        /* AUTODOC: add LTT Entry (row=2,col=0) for Route Entry 8 */
        rc = prvTgfIpLttWrite(prvTgfRouteEntryBaseIndex / 4, prvTgfRouteEntryBaseIndex % 4, &lttEntry);
        PRV_UTF_VERIFY_RC1(rc, "prvTgfIpLttWrite");
    }

    if ((PRV_CPSS_SIP_5_CHECK_MAC(prvTgfDevNum)))
    {
        /* Save the enabling status of bypassing the router triggering requirements for PBR packets */
        rc = prvTgfIpPbrBypassRouterTriggerRequirementsEnableGet(prvTgfDevNum, &pbrBypassTrigger);
        PRV_UTF_VERIFY_RC1(rc, "prvTgfIpPbrBypassRouterTriggerRequirementsEnableGet");

        /* Enable bypassing the router triggering requirements for PBR packets */
        rc = prvTgfIpPbrBypassRouterTriggerRequirementsEnableSet(prvTgfDevNum, GT_TRUE);
        PRV_UTF_VERIFY_RC1(rc, "prvTgfIpPbrBypassRouterTriggerRequirementsEnableSet");
   }
    else
    {
        /* -------------------------------------------------------------------------
         * 4. Create static FDB Entry with Routing
         */

        /* create a macEntry with .daRoute = GT_TRUE */
        cpssOsMemSet(&macEntry, 0, sizeof(macEntry));
        cpssOsMemCpy(macEntry.key.key.macVlan.macAddr.arEther, prvTgfPacketL2Part.daMac, sizeof(TGF_MAC_ADDR));

        macEntry.key.entryType                  = PRV_TGF_FDB_ENTRY_TYPE_MAC_ADDR_E;
        macEntry.key.key.macVlan.vlanId         = PRV_TGF_SEND_VLANID_CNS;
        macEntry.dstInterface.type              = CPSS_INTERFACE_PORT_E;
        macEntry.dstInterface.devPort.hwDevNum    = prvTgfDevNum;
        macEntry.dstInterface.devPort.portNum   = prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS];
        macEntry.isStatic                       = GT_TRUE;
        macEntry.daCommand                      = PRV_TGF_PACKET_CMD_FORWARD_E;
        macEntry.saCommand                      = PRV_TGF_PACKET_CMD_FORWARD_E;
        macEntry.daRoute                        = GT_TRUE;
        macEntry.sourceId                       = 0;
        macEntry.userDefined                    = 0;
        macEntry.daQosIndex                     = 0;
        macEntry.saQosIndex                     = 0;
        macEntry.daSecurityLevel                = 0;
        macEntry.saSecurityLevel                = 0;
        macEntry.appSpecificCpuCode             = GT_FALSE;
        macEntry.mirrorToRxAnalyzerPortEn       = GT_FALSE;

        /* AUTODOC: add FDB entry with MAC 00:00:00:00:34:02, VLAN 1, port 1 */
        rc = prvTgfBrgFdbMacEntrySet(&macEntry);
        PRV_UTF_VERIFY_RC1(rc, "prvTgfBrgFdbMacEntryWrite");
    }

    TGF_PARAM_NOT_USED(rc1); /* prevent warning: not used variable */
    return rc;
}

/**
* @internal prvTgfTrafficPclConfigurationSet function
* @endinternal
*
* @brief   None
*/
static GT_STATUS prvTgfTrafficPclConfigurationSet
(
    INOUT PRV_TGF_PCL_RULE_FORMAT_UNT *maskPtr,
    INOUT PRV_TGF_PCL_RULE_FORMAT_UNT *pattPtr,
    INOUT PRV_TGF_PCL_ACTION_STC      *actionPtr
)
{
    GT_STATUS      rc = GT_OK;

    /* mask for MAC address */
    cpssOsMemSet(maskPtr, 0, sizeof(*maskPtr));

    /* define mask, pattern and action */
    cpssOsMemSet(pattPtr, 0, sizeof(*pattPtr));

    /* action redirect */
    cpssOsMemSet(actionPtr, 0, sizeof(*actionPtr));
    actionPtr->pktCmd                                           = CPSS_PACKET_CMD_FORWARD_E;
    actionPtr->bypassIngressPipe                                = GT_FALSE;
    actionPtr->redirect.redirectCmd                             = PRV_TGF_PCL_ACTION_REDIRECT_CMD_ROUTER_E;
    if ((PRV_CPSS_SIP_5_CHECK_MAC(prvTgfDevNum)))
    {
        actionPtr->redirect.data.routerLttIndex                 = prvTgfPbrLeafIndex;
    }
    else
    {
        actionPtr->redirect.data.routerLttIndex                 = prvTgfRouteEntryBaseIndex;
    }

    rc = prvTgfTrafficLttRouteConfigurationSet();
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfTrafficLttRouteConfigurationSet");

    return rc;
};

/**
* @internal prvTgfPclRedirectToLTT function
* @endinternal
*
* @brief   Set test configuration
*/
GT_VOID prvTgfPclRedirectToLTT
(
    GT_VOID
)
{
    GT_STATUS                   rc = GT_OK;
    PRV_TGF_PCL_RULE_FORMAT_UNT mask;
    PRV_TGF_PCL_RULE_FORMAT_UNT pattern;
    PRV_TGF_PCL_ACTION_STC      action;

    /* AUTODOC: SETUP CONFIGURATION: */
    rc = prvTgfTrafficPclConfigurationSet(
        &mask, &pattern, &action);
    UTF_VERIFY_EQUAL0_STRING_MAC(
        GT_OK, rc, "prvTgfTrafficPclConfigurationSet");

    /* AUTODOC: init PCL Engine for send port 1: */
    /* AUTODOC:   ingress direction, lookup0 */
    /* AUTODOC:   nonIpKey INGRESS_STD_NOT_IP */
    /* AUTODOC:   ipv4Key INGRESS_STD_IP_L2_QOS */
    /* AUTODOC:   ipv6Key INGRESS_STD_IP_L2_QOS */
    rc = prvTgfPclDefPortInit(
        prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
        CPSS_PCL_DIRECTION_INGRESS_E,
        CPSS_PCL_LOOKUP_0_E,
        PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E /*nonIpKey*/,
        PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IP_L2_QOS_E /*ipv4Key*/,
        PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IP_L2_QOS_E /*ipv6Key*/);
    UTF_VERIFY_EQUAL1_STRING_MAC(
        GT_OK, rc, "prvTgfPclDefPortInit: %d", prvTgfDevNum);

    /* AUTODOC: set PCL rule 1 with: */
    /* AUTODOC:   format INGRESS_STD_IP_L2_QOS, cmd=FORWARD */
    /* AUTODOC:   redirect to routerLtt entry 8 */
    rc = prvTgfPclRuleSet(
        PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IP_L2_QOS_E,
        prvTgfPclRuleIndex, &mask, &pattern, &action);
    UTF_VERIFY_EQUAL2_STRING_MAC(
        GT_OK, rc, "prvTgfPclRuleSet: %d, %d",
        prvTgfDevNum, prvTgfPclRuleIndex);

    /* AUTODOC: GENERATE TRAFFIC: */
    rc = prvTgfTrafficGenerateWithCapture();
    UTF_VERIFY_EQUAL0_STRING_MAC(
        GT_OK, rc, "prvTgfTrafficGenerate");

    /* AUTODOC: verify traffic on port 3: */
    /* AUTODOC:   DA=00:12:23:45:67:89, SA=00:00:00:00:00:01 */
    rc = prvTgfTrafficCheckCapturedByField(
        prvTgfDevNum,
        prvTgfPortsArray[PRV_TGF_NEXTHOPE_PORT_IDX_CNS],
        PRV_TGF_PACKET_NUM_CNS,
        0 /*fieldOffset*/,
        6 /*fieldLength*/,
        prvTgfArpMac);

    UTF_VERIFY_EQUAL0_STRING_MAC(
        GT_OK, rc, "prvTgfTrafficCheckCapturedByField");

    /* AUTODOC: RESTORE CONFIGURATION: */
    prvTgfConfigurationRestore();
}


