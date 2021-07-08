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
* @file prvTgfIpv4UcRoutingHiddenPrefixes.c
*
* @brief LPM test for "hidden" prefixes
*
* @version   1
********************************************************************************
*/

#include <cpssCommon/cpssPresteraDefs.h>
#include <cpss/extServices/private/prvCpssBindFunc.h>

#include <utf/utfMain.h>
#include <utf/private/prvUtfHelpers.h>

#include <trafficEngine/tgfTrafficGenerator.h>
#include <trafficEngine/tgfTrafficTable.h>

#include <common/tgfCommon.h>
#include <common/tgfBridgeGen.h>
#include <common/tgfIpGen.h>
#include <common/tgfTrunkGen.h>
#include <ip/prvTgfIpv4UcRoutingAddMany.h>
#include <ip/prvTgfBasicIpv4UcRouting.h>

/******************************************************************************\
 *                       Test configuration section                           *
\******************************************************************************/

/* default VLAN Id */
#define PRV_TGF_SEND_VLANID_CNS           5

/* nextHop VLAN Id */
#define PRV_TGF_NEXTHOPE_VLANID_CNS       6

/* VLANs array */
static GT_U8         prvTgfVlanArray[] = {PRV_TGF_SEND_VLANID_CNS, PRV_TGF_NEXTHOPE_VLANID_CNS};

/* number of ports */
#define PRV_TGF_PORT_COUNT_CNS            4

/* the Arp Address index of the Router ARP Table
   to write to the UC Route entry Arp nextHopARPPointer field */
static GT_U32        prvTgfRouterArpIndex   = 1;

/* the Route entry index for UC Route entry Table */
static GT_U32        prvTgfRouteEntryBaseIndexNH1 = 3;
static GT_U32        prvTgfRouteEntryBaseIndexNH2 = 4;
static GT_U32        prvTgfRouteEntryBaseIndexNH3 = 5;

/* the LPM DB id for LPM Table */
static GT_U32        prvTgfLpmDBId             = 0;

/* the ARP MAC address to write to the Router ARP Table */
static TGF_MAC_ADDR  prvTgfArpMac = {0x00, 0x00, 0x00, 0x00, 0x00, 0x22};

/* port number to send traffic to */
#define PRV_TGF_SEND_PORT_IDX_CNS            0

/* prefix lengths */
#define PREFIX_LEN_8_CNS     8
#define PREFIX_LEN_9_CNS     9
#define PREFIX_LEN_24_CNS    24
#define PREFIX_LEN_25_CNS    25

/******************************* Test packet **********************************/

/* L2 part of packet */
static TGF_PACKET_L2_STC prvTgfPacketL2Part = {
    {0x00, 0x00, 0x00, 0x00, 0x34, 0x02},               /* daMac */
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x01}                /* saMac */
};

/******************************************************************************\
 *                            Test implementation                             *
\******************************************************************************/

/**
* @internal prvTgfIpv4UcRoutingHiddenPrefixesBaseConfigurationSet function
* @endinternal
*
* @brief   Set Base Configuration
*/
GT_VOID prvTgfIpv4UcRoutingHiddenPrefixesBaseConfigurationSet(GT_VOID)
{
    GT_STATUS rc = GT_OK;
    GT_U8     tagArray[] = {0, 0, 1, 1, 1};
    PRV_TGF_BRG_MAC_ENTRY_STC   macEntry;

    /* AUTODOC: SETUP CONFIGURATION: */
    PRV_UTF_LOG0_MAC("======= Setting Base Configuration =======\n");

     /* AUTODOC: create VLAN 5 with untagged ports 0,1  */
    rc = prvTgfBrgDefVlanEntryWithPortsSet(PRV_TGF_SEND_VLANID_CNS,
            prvTgfPortsArray, NULL, tagArray, 2);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
            "prvTgfBrgDefVlanEntryWithPortsSet: %d", prvTgfDevNum);


    /* AUTODOC: create VLAN 6 with tagged ports 1,2,3 */
    rc = prvTgfBrgDefVlanEntryWithPortsSet(PRV_TGF_NEXTHOPE_VLANID_CNS,
            prvTgfPortsArray + 1, NULL, tagArray + 2, 3);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
            "prvTgfBrgDefVlanEntryWithPortsSet: %d", prvTgfDevNum);

     /* AUTODOC: enable VLAN based MAC learning */
    rc = prvTgfBrgVlanLearningStateSet(PRV_TGF_SEND_VLANID_CNS, GT_TRUE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanLearningStateSet: %d", prvTgfDevNum);

    /* -------------------------------------------------------------------------
     * Set the FDB entry With DA_ROUTE
     */

    /* create a macEntry with .daRoute = GT_TRUE */
    cpssOsMemSet(&macEntry, 0, sizeof(macEntry));
    cpssOsMemCpy(macEntry.key.key.macVlan.macAddr.arEther, prvTgfPacketL2Part.daMac, sizeof(TGF_MAC_ADDR));

    macEntry.key.entryType                  = PRV_TGF_FDB_ENTRY_TYPE_MAC_ADDR_E;
    macEntry.key.key.macVlan.vlanId         = PRV_TGF_SEND_VLANID_CNS;
    macEntry.dstInterface.type              = CPSS_INTERFACE_PORT_E;
    macEntry.dstInterface.devPort.hwDevNum    = prvTgfDevNum;
    macEntry.dstInterface.devPort.portNum   = prvTgfPortsArray[1];
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

    /* AUTODOC: add FDB entry with MAC 00:00:00:00:34:02, VLAN 5, port 1 */
    rc = prvTgfBrgFdbMacEntrySet(&macEntry);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbMacEntryWrite: %d", prvTgfDevNum);

}

/**
* @internal prvTgfIpv4UcRoutingHiddenPrefixesLttRouteConfigurationSet function
* @endinternal
*
* @brief   Set LTT Route Configuration
*/
GT_VOID prvTgfIpv4UcRoutingHiddenPrefixesLttRouteConfigurationSet(GT_VOID)
{
    GT_STATUS                               rc = GT_OK;
    GT_ETHERADDR                            arpMacAddr;
    PRV_TGF_IP_UC_ROUTE_ENTRY_STC           routeEntriesArray[1];
    PRV_TGF_IP_UC_ROUTE_ENTRY_STC           *regularEntryPtr;
    GT_U32                                  index;

    PRV_UTF_LOG0_MAC("==== Setting LTT Route Configuration ====\n");

    /* -------------------------------------------------------------------------
     * 1. Enable Routing
     */

    /* AUTODOC: enable Unicast IPv4 Routing on port 0 */
    rc = prvTgfIpPortRoutingEnable(PRV_TGF_SEND_PORT_IDX_CNS, CPSS_IP_UNICAST_E,
                                   CPSS_IP_PROTOCOL_IPV4_E, GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfIpPortRoutingEnable: %d %d",
                                 prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);

    /* AUTODOC: enable IPv4 Unicast Routing on Vlan 5 */
    rc = prvTgfIpVlanRoutingEnable(PRV_TGF_SEND_VLANID_CNS, CPSS_IP_UNICAST_E, CPSS_IP_PROTOCOL_IPV4_E, GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfIpVlanRoutingEnable: %d %d",
                                 prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);

    /* -------------------------------------------------------------------------
     * 2. Create the Route entry (Next hop) in Route table and Router ARP Table
     */

    for (index = 1; index <= 3; index++)
    {
        /* AUTODOC: write a ARP MAC 00:00:00:00:00:22 to the Router ARP Table */
        cpssOsMemCpy(arpMacAddr.arEther, prvTgfArpMac, sizeof(TGF_MAC_ADDR));

        rc = prvTgfIpRouterArpAddrWrite(prvTgfRouterArpIndex, &arpMacAddr);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpRouterArpAddrWrite: %d", prvTgfDevNum);

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
        regularEntryPtr->countSet                   = CPSS_IP_CNT_SET0_E;
        regularEntryPtr->trapMirrorArpBcEnable      = GT_FALSE;
        regularEntryPtr->sipAccessLevel             = 0;
        regularEntryPtr->dipAccessLevel             = 0;
        regularEntryPtr->ICMPRedirectEnable         = GT_FALSE;
        regularEntryPtr->scopeCheckingEnable        = GT_FALSE;
        regularEntryPtr->siteId                     = CPSS_IP_SITE_ID_INTERNAL_E;
        regularEntryPtr->mtuProfileIndex            = 0;
        regularEntryPtr->isTunnelStart              = GT_FALSE;
        regularEntryPtr->nextHopVlanId              = PRV_TGF_NEXTHOPE_VLANID_CNS;

        regularEntryPtr->nextHopInterface.type      = CPSS_INTERFACE_PORT_E;
        regularEntryPtr->nextHopInterface.devPort.hwDevNum = prvTgfDevNum;
        regularEntryPtr->nextHopInterface.devPort.portNum = prvTgfPortsArray[index];

        regularEntryPtr->nextHopARPPointer          = prvTgfRouterArpIndex;
        regularEntryPtr->nextHopTunnelPointer       = 0;

        /* AUTODOC: add 3 UC route entries with nexthop VLAN 6 and nexthop port [1..3] */
        rc = prvTgfIpUcRouteEntriesWrite(prvTgfRouteEntryBaseIndexNH1 + (index-1),
                                         routeEntriesArray, 1);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpUcRouteEntriesWrite: %d", prvTgfDevNum);
    }
}

/**
* @internal trafficGenerate function
* @endinternal
*
* @brief   Generate traffic
*
* @param[in] sendPortNum              - port sending traffic
* @param[in] nextHopPortNumArray[4]   - array of ports receiving traffic
*                                       None
*/
static GT_VOID trafficGenerate
(
    GT_U32               sendPortNum,
    GT_U32               nextHopPortNumArray[4]
)
{
    GT_IPADDR                               ipAddr;

    /* AUTODOC: Send Traffic */

    ipAddr.arIP[0]=4;
    ipAddr.arIP[1]=0;
    ipAddr.arIP[2]=0;
    ipAddr.arIP[3]=0;

    prvTgfIpv4UcRoutingAddManyTrafficGenerate(sendPortNum, nextHopPortNumArray[0], ipAddr, PRV_TGF_NEXTHOPE_VLANID_CNS);

    ipAddr.arIP[0]=4;
    ipAddr.arIP[1]=1;
    ipAddr.arIP[2]=0;
    ipAddr.arIP[3]=0;

    prvTgfIpv4UcRoutingAddManyTrafficGenerate(sendPortNum, nextHopPortNumArray[1], ipAddr, PRV_TGF_NEXTHOPE_VLANID_CNS);

    ipAddr.arIP[0]=4;
    ipAddr.arIP[1]=128;
    ipAddr.arIP[2]=0;
    ipAddr.arIP[3]=0;

    prvTgfIpv4UcRoutingAddManyTrafficGenerate(sendPortNum, nextHopPortNumArray[2], ipAddr, PRV_TGF_NEXTHOPE_VLANID_CNS);

    ipAddr.arIP[0]=4;
    ipAddr.arIP[1]=255;
    ipAddr.arIP[2]=0;
    ipAddr.arIP[3]=0;

    prvTgfIpv4UcRoutingAddManyTrafficGenerate(sendPortNum, nextHopPortNumArray[3], ipAddr, PRV_TGF_NEXTHOPE_VLANID_CNS);
}

/**
* @internal trafficGenerate1 function
* @endinternal
*
* @brief   Generate traffic1
*
* @param[in] sendPortNum              - port sending traffic
* @param[in] nextHopPortNumArray[4]   - array of ports receiving traffic
*                                       None
*/
static GT_VOID trafficGenerate1
(
    GT_U32               sendPortNum,
    GT_U32               nextHopPortNumArray[4]
)
{
    GT_IPADDR                               ipAddr;

    /* AUTODOC: Send Traffic */

    ipAddr.arIP[0]=4;
    ipAddr.arIP[1]=0;
    ipAddr.arIP[2]=0;
    ipAddr.arIP[3]=0;

    prvTgfIpv4UcRoutingAddManyTrafficGenerate(sendPortNum, nextHopPortNumArray[0], ipAddr, PRV_TGF_NEXTHOPE_VLANID_CNS);

    ipAddr.arIP[0]=4;
    ipAddr.arIP[1]=0;
    ipAddr.arIP[2]=0;
    ipAddr.arIP[3]=127;

    prvTgfIpv4UcRoutingAddManyTrafficGenerate(sendPortNum, nextHopPortNumArray[1], ipAddr, PRV_TGF_NEXTHOPE_VLANID_CNS);

    ipAddr.arIP[0]=4;
    ipAddr.arIP[1]=0;
    ipAddr.arIP[2]=0;
    ipAddr.arIP[3]=128;

    prvTgfIpv4UcRoutingAddManyTrafficGenerate(sendPortNum, nextHopPortNumArray[2], ipAddr, PRV_TGF_NEXTHOPE_VLANID_CNS);

    ipAddr.arIP[0]=4;
    ipAddr.arIP[1]=0;
    ipAddr.arIP[2]=0;
    ipAddr.arIP[3]=255;

    prvTgfIpv4UcRoutingAddManyTrafficGenerate(sendPortNum, nextHopPortNumArray[3], ipAddr, PRV_TGF_NEXTHOPE_VLANID_CNS);
}

/**
* @internal prvTgfIpv4UcRoutingHiddenPrefixesConfigurationAndTrafficGenerate function
* @endinternal
*
* @brief   Add Prefixes Configuration and generate traffic
*/
GT_VOID prvTgfIpv4UcRoutingHiddenPrefixesConfigurationAndTrafficGenerate(GT_VOID)
{
    GT_STATUS   rc = GT_OK;
    PRV_TGF_IP_TCAM_ROUTE_ENTRY_INFO_UNT    nextHopInfo;
    GT_IPADDR                               ipAddr;
    GT_U32       nextHopPortNumArray[4];


    PRV_UTF_LOG0_MAC("==== Adding Prefixes and Sending Traffic ====\n");

    /* AUTODOC: second senario */

    /* fill nexthope info for the prefix */
    cpssOsMemSet(&nextHopInfo, 0, sizeof(nextHopInfo));
    nextHopInfo.ipLttEntry.routeEntryBaseIndex = prvTgfRouteEntryBaseIndexNH1;
    nextHopInfo.ipLttEntry.routeType = PRV_TGF_IP_REGULAR_ROUTE_ENTRY_GROUP_MAC;

    /* fill destination IP address for the prefix */
    ipAddr.arIP[0]=4;
    ipAddr.arIP[1]=0;
    ipAddr.arIP[2]=0;
    ipAddr.arIP[3]=0;

    /* add prefix 4/8*/
    rc = prvTgfIpLpmIpv4UcPrefixAdd(prvTgfLpmDBId, 0, ipAddr, PREFIX_LEN_8_CNS, &nextHopInfo, GT_FALSE, GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpLpmIpv4UcPrefixAdd: %d", prvTgfDevNum);

    /* AUTODOC: send all packets to the prefixes added */
    /* AUTODOC: all should match NH1 */
    /* 000.000.000.000. - 003.000.000.000. / 08  -> NH 0
       004.000.000.000. - 004.000.000.000. / 08  -> NH 1
       005.000.000.000. - 223.000.000.000. / 08  -> NH 0
    */
    nextHopPortNumArray[0]=prvTgfPortsArray[1];
    nextHopPortNumArray[1]=prvTgfPortsArray[1];
    nextHopPortNumArray[2]=prvTgfPortsArray[1];
    nextHopPortNumArray[3]=prvTgfPortsArray[1];

    trafficGenerate(prvTgfPortsArray[0],nextHopPortNumArray);

    /* fill nexthope info for the prefix */
    cpssOsMemSet(&nextHopInfo, 0, sizeof(nextHopInfo));
    nextHopInfo.ipLttEntry.routeEntryBaseIndex = prvTgfRouteEntryBaseIndexNH2;
    nextHopInfo.ipLttEntry.routeType = PRV_TGF_IP_REGULAR_ROUTE_ENTRY_GROUP_MAC;

    /* fill destination IP address for the prefix */
    ipAddr.arIP[0]=4;
    ipAddr.arIP[1]=0;
    ipAddr.arIP[2]=0;
    ipAddr.arIP[3]=0;

    /* add prefix 4/9 */
    rc = prvTgfIpLpmIpv4UcPrefixAdd(prvTgfLpmDBId, 0, ipAddr, PREFIX_LEN_9_CNS, &nextHopInfo, GT_FALSE, GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpLpmIpv4UcPrefixAdd: %d", prvTgfDevNum);

    /* AUTODOC: send all packets to the prefixes added */
    /* AUTODOC: first 2 should match NH2 and last 2 should match NH1 */
    /* 000.000.000.000. - 003.000.000.000. / 08  -> NH 0
       004.000.000.000. - 004.127.000.000. / 16  -> NH 2
       004.128.000.000. - 004.255.000.000. / 16  -> NH 1
       005.000.000.000. - 223.000.000.000. / 08  -> NH 0
    */
    nextHopPortNumArray[0]=prvTgfPortsArray[2];
    nextHopPortNumArray[1]=prvTgfPortsArray[2];
    nextHopPortNumArray[2]=prvTgfPortsArray[1];
    nextHopPortNumArray[3]=prvTgfPortsArray[1];

    trafficGenerate(prvTgfPortsArray[0],nextHopPortNumArray);

    /* fill nexthope info for the prefix */
    cpssOsMemSet(&nextHopInfo, 0, sizeof(nextHopInfo));
    nextHopInfo.ipLttEntry.routeEntryBaseIndex = prvTgfRouteEntryBaseIndexNH3;
    nextHopInfo.ipLttEntry.routeType = PRV_TGF_IP_REGULAR_ROUTE_ENTRY_GROUP_MAC;

    /* fill destination IP address for the prefix */
    ipAddr.arIP[0]=4;
    ipAddr.arIP[1]=128;
    ipAddr.arIP[2]=0;
    ipAddr.arIP[3]=0;

    /* add prefix 4.128/9*/
    rc = prvTgfIpLpmIpv4UcPrefixAdd(prvTgfLpmDBId, 0, ipAddr, PREFIX_LEN_9_CNS, &nextHopInfo, GT_FALSE, GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpLpmIpv4UcPrefixAdd: %d", prvTgfDevNum);


    /* AUTODOC: send all packets to the prefixes added */
    /* AUTODOC: first 2 should match NH2 and last 2 should match NH3 */
    /* 000.000.000.000. - 003.000.000.000. / 08  -> NH 0
       004.000.000.000. - 004.127.000.000. / 16  -> NH 2
       004.128.000.000. - 004.255.000.000. / 16  -> NH 3
       005.000.000.000. - 223.000.000.000. / 08  -> NH 0
    */
    nextHopPortNumArray[0]=prvTgfPortsArray[2];
    nextHopPortNumArray[1]=prvTgfPortsArray[2];
    nextHopPortNumArray[2]=prvTgfPortsArray[3];
    nextHopPortNumArray[3]=prvTgfPortsArray[3];

    trafficGenerate(prvTgfPortsArray[0],nextHopPortNumArray);

    /* fill destination IP address for the prefix */
    ipAddr.arIP[0]=4;
    ipAddr.arIP[1]=0;
    ipAddr.arIP[2]=0;
    ipAddr.arIP[3]=0;

    /* AUTODOC: delete the Ipv4 prefix 4/8 */
    rc = prvTgfIpLpmIpv4UcPrefixDel(prvTgfLpmDBId, 0, ipAddr, PREFIX_LEN_8_CNS);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpLpmIpv4UcPrefixDel: %d", prvTgfDevNum);

    /* AUTODOC: Check traffic remain the same */
    /* AUTODOC: first 2 should match NH2 and last 2 should match NH3 */
    /* 000.000.000.000. - 003.000.000.000. / 08  -> NH 0
       004.000.000.000. - 004.127.000.000. / 16  -> NH 2
       004.128.000.000. - 004.255.000.000. / 16  -> NH 3
       005.000.000.000. - 223.000.000.000. / 08  -> NH 0
    */
    trafficGenerate(prvTgfPortsArray[0],nextHopPortNumArray);

    /* fill destination IP address for the prefix */
    ipAddr.arIP[0]=4;
    ipAddr.arIP[1]=0;
    ipAddr.arIP[2]=0;
    ipAddr.arIP[3]=0;

    /* fill nexthope info for the prefix */
    cpssOsMemSet(&nextHopInfo, 0, sizeof(nextHopInfo));
    nextHopInfo.ipLttEntry.routeEntryBaseIndex = prvTgfRouteEntryBaseIndexNH1;
    nextHopInfo.ipLttEntry.routeType = PRV_TGF_IP_REGULAR_ROUTE_ENTRY_GROUP_MAC;

    /* Re-add prefix 4/8 as NH1 */
    rc = prvTgfIpLpmIpv4UcPrefixAdd(prvTgfLpmDBId, 0, ipAddr, PREFIX_LEN_8_CNS, &nextHopInfo, GT_FALSE, GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpLpmIpv4UcPrefixAdd: %d", prvTgfDevNum);

    /* Last 2 steps shows that deleting and re-adding the “default 4/8” doesn’t affect the other prefixes */

    /* AUTODOC: Check traffic remain the same */
    /* AUTODOC: first 2 should match NH2 and last 2 should match NH3 */
    /* 000.000.000.000. - 003.000.000.000. / 08  -> NH 0
       004.000.000.000. - 004.127.000.000. / 16  -> NH 2
       004.128.000.000. - 004.255.000.000. / 16  -> NH 3
       005.000.000.000. - 223.000.000.000. / 08  -> NH 0
    */
    trafficGenerate(prvTgfPortsArray[0],nextHopPortNumArray);

    /* fill destination IP address for the prefix */
    ipAddr.arIP[0]=4;
    ipAddr.arIP[1]=0;
    ipAddr.arIP[2]=0;
    ipAddr.arIP[3]=0;

    /* AUTODOC: delete the Ipv4 prefix 4/9 */
    rc = prvTgfIpLpmIpv4UcPrefixDel(prvTgfLpmDBId, 0, ipAddr, PREFIX_LEN_9_CNS);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpLpmIpv4UcPrefixDel: %d", prvTgfDevNum);

    /* AUTODOC: Check traffic */
    /* AUTODOC: first 2 should match NH1 and last 2 should match NH3 */
    /* 000.000.000.000. - 003.000.000.000. / 08  -> NH 0
       004.000.000.000. - 004.127.000.000. / 16  -> NH 1
       004.128.000.000. - 004.255.000.000. / 16  -> NH 3
       005.000.000.000. - 223.000.000.000. / 08  -> NH 0
    */

    nextHopPortNumArray[0]=prvTgfPortsArray[1];
    nextHopPortNumArray[1]=prvTgfPortsArray[1];
    nextHopPortNumArray[2]=prvTgfPortsArray[3];
    nextHopPortNumArray[3]=prvTgfPortsArray[3];

    trafficGenerate(prvTgfPortsArray[0],nextHopPortNumArray);

    /* fill destination IP address for the prefix */
    ipAddr.arIP[0]=4;
    ipAddr.arIP[1]=128;
    ipAddr.arIP[2]=0;
    ipAddr.arIP[3]=0;

    /* AUTODOC: delete the Ipv4 prefix 4.128/9 */
    rc = prvTgfIpLpmIpv4UcPrefixDel(prvTgfLpmDBId, 0, ipAddr, PREFIX_LEN_9_CNS);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpLpmIpv4UcPrefixDel: %d", prvTgfDevNum);

    /* AUTODOC: send all packets to the prefixes added */
    /* AUTODOC: all should match NH1 */
    /* 000.000.000.000. - 003.000.000.000. / 08  -> NH 0
       004.000.000.000. - 004.000.000.000. / 08  -> NH 1
       005.000.000.000. - 223.000.000.000. / 08  -> NH 0
    */
    nextHopPortNumArray[0]=prvTgfPortsArray[1];
    nextHopPortNumArray[1]=prvTgfPortsArray[1];
    nextHopPortNumArray[2]=prvTgfPortsArray[1];
    nextHopPortNumArray[3]=prvTgfPortsArray[1];

    trafficGenerate(prvTgfPortsArray[0],nextHopPortNumArray);

    /* fill destination IP address for the prefix */
    ipAddr.arIP[0]=4;
    ipAddr.arIP[1]=0;
    ipAddr.arIP[2]=0;
    ipAddr.arIP[3]=0;

    /* AUTODOC: delete the Ipv4 prefix 4/8 */
    rc = prvTgfIpLpmIpv4UcPrefixDel(prvTgfLpmDBId, 0, ipAddr, PREFIX_LEN_8_CNS);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpLpmIpv4UcPrefixDel: %d", prvTgfDevNum);

    /* AUTODOC: Check traffic */
    /* AUTODOC: Expect no traffic */
    prvTgfBasicIpv4UcRoutingTrafficGenerate(prvTgfPortsArray[0], prvTgfPortsArray[3], PRV_TGF_DEFAULT_BOOLEAN_CNS, PRV_TGF_DEFAULT_BOOLEAN_CNS, GT_TRUE, GT_TRUE);

    /* AUTODOC: second senario */

    /* fill nexthope info for the prefix */
    cpssOsMemSet(&nextHopInfo, 0, sizeof(nextHopInfo));
    nextHopInfo.ipLttEntry.routeEntryBaseIndex = prvTgfRouteEntryBaseIndexNH1;
    nextHopInfo.ipLttEntry.routeType = PRV_TGF_IP_REGULAR_ROUTE_ENTRY_GROUP_MAC;

    /* fill destination IP address for the prefix */
    ipAddr.arIP[0]=4;
    ipAddr.arIP[1]=0;
    ipAddr.arIP[2]=0;
    ipAddr.arIP[3]=0;

    /* add prefix 4.0.0/24 */
    rc = prvTgfIpLpmIpv4UcPrefixAdd(prvTgfLpmDBId, 0, ipAddr, PREFIX_LEN_24_CNS, &nextHopInfo, GT_FALSE, GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpLpmIpv4UcPrefixAdd: %d", prvTgfDevNum);

    /* AUTODOC: send all packets to the prefixes added */
    /* AUTODOC: all should match NH1 */
    /* 000.000.000.000. - 003.000.000.000. / 08  -> NH 0
       004.000.000.000. - 004.000.000.000. / 24  -> NH 1
       004.000.001.000. - 004.000.255.000. / 24  -> NH 0
       004.001.000.000. - 004.255.000.000. / 16  -> NH 0
       005.000.000.000. - 223.000.000.000. / 08  -> NH 0
    */
    nextHopPortNumArray[0]=prvTgfPortsArray[1];
    nextHopPortNumArray[1]=prvTgfPortsArray[1];
    nextHopPortNumArray[2]=prvTgfPortsArray[1];
    nextHopPortNumArray[3]=prvTgfPortsArray[1];

    trafficGenerate1(prvTgfPortsArray[0],nextHopPortNumArray);

    /* fill nexthope info for the prefix */
    cpssOsMemSet(&nextHopInfo, 0, sizeof(nextHopInfo));
    nextHopInfo.ipLttEntry.routeEntryBaseIndex = prvTgfRouteEntryBaseIndexNH2;
    nextHopInfo.ipLttEntry.routeType = PRV_TGF_IP_REGULAR_ROUTE_ENTRY_GROUP_MAC;

    /* fill destination IP address for the prefix */
    ipAddr.arIP[0]=4;
    ipAddr.arIP[1]=0;
    ipAddr.arIP[2]=0;
    ipAddr.arIP[3]=0;

    /* add prefix 4.0.0/25 */
    rc = prvTgfIpLpmIpv4UcPrefixAdd(prvTgfLpmDBId, 0, ipAddr, PREFIX_LEN_25_CNS, &nextHopInfo, GT_FALSE, GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpLpmIpv4UcPrefixAdd: %d", prvTgfDevNum);

    /* AUTODOC: send all packets to the prefixes added */
    /* AUTODOC: first 2 should match NH2 and last 2 should match NH1 */
    /* 000.000.000.000. - 003.000.000.000. / 08  -> NH 0
       004.000.000.000. - 004.000.000.127. / 32  -> NH 2
       004.000.000.128. - 004.000.000.255. / 32  -> NH 1
       004.000.001.000. - 004.000.255.000. / 24  -> NH 0
       004.001.000.000. - 004.255.000.000. / 16  -> NH 0
       005.000.000.000. - 223.000.000.000. / 08  -> NH 0
    */
    nextHopPortNumArray[0]=prvTgfPortsArray[2];
    nextHopPortNumArray[1]=prvTgfPortsArray[2];
    nextHopPortNumArray[2]=prvTgfPortsArray[1];
    nextHopPortNumArray[3]=prvTgfPortsArray[1];

    trafficGenerate1(prvTgfPortsArray[0],nextHopPortNumArray);

    /* fill nexthope info for the prefix */
    cpssOsMemSet(&nextHopInfo, 0, sizeof(nextHopInfo));
    nextHopInfo.ipLttEntry.routeEntryBaseIndex = prvTgfRouteEntryBaseIndexNH3;
    nextHopInfo.ipLttEntry.routeType = PRV_TGF_IP_REGULAR_ROUTE_ENTRY_GROUP_MAC;

    /* fill destination IP address for the prefix */
    ipAddr.arIP[0]=4;
    ipAddr.arIP[1]=0;
    ipAddr.arIP[2]=0;
    ipAddr.arIP[3]=128;

    /* add prefix 4.0.0.128/25 */
    rc = prvTgfIpLpmIpv4UcPrefixAdd(prvTgfLpmDBId, 0, ipAddr, PREFIX_LEN_25_CNS, &nextHopInfo, GT_FALSE, GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpLpmIpv4UcPrefixAdd: %d", prvTgfDevNum);

    /* AUTODOC: send all packets to the prefixes added */
    /* AUTODOC: first 2 should match NH2 and last 2 should match NH3 */
    /* 000.000.000.000. - 003.000.000.000. / 08  -> NH 0
       004.000.000.000. - 004.000.000.127. / 32  -> NH 2
       004.000.000.128. - 004.000.000.255. / 32  -> NH 3
       004.000.001.000. - 004.000.255.000. / 24  -> NH 0
       004.001.000.000. - 004.255.000.000. / 16  -> NH 0
       005.000.000.000. - 223.000.000.000. / 08  -> NH 0
    */
    nextHopPortNumArray[0]=prvTgfPortsArray[2];
    nextHopPortNumArray[1]=prvTgfPortsArray[2];
    nextHopPortNumArray[2]=prvTgfPortsArray[3];
    nextHopPortNumArray[3]=prvTgfPortsArray[3];

    trafficGenerate1(prvTgfPortsArray[0],nextHopPortNumArray);

    /* fill destination IP address for the prefix */
    ipAddr.arIP[0]=4;
    ipAddr.arIP[1]=0;
    ipAddr.arIP[2]=0;
    ipAddr.arIP[3]=0;

    /* AUTODOC: delete the Ipv4 prefix 4.0.0/24 */
    rc = prvTgfIpLpmIpv4UcPrefixDel(prvTgfLpmDBId, 0, ipAddr, PREFIX_LEN_24_CNS);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpLpmIpv4UcPrefixDel: %d", prvTgfDevNum);

    /* AUTODOC: Check traffic remain the same */
    /* AUTODOC: first 2 should match NH2 and last 2 should match NH3 */
    /* 000.000.000.000. - 003.000.000.000. / 08  -> NH 0
       004.000.000.000. - 004.000.000.127. / 32  -> NH 2
       004.000.000.128. - 004.000.000.255. / 32  -> NH 3
       004.000.001.000. - 004.000.255.000. / 24  -> NH 0
       004.001.000.000. - 004.255.000.000. / 16  -> NH 0
       005.000.000.000. - 223.000.000.000. / 08  -> NH 0
    */
    nextHopPortNumArray[0]=prvTgfPortsArray[2];
    nextHopPortNumArray[1]=prvTgfPortsArray[2];
    nextHopPortNumArray[2]=prvTgfPortsArray[3];
    nextHopPortNumArray[3]=prvTgfPortsArray[3];

    trafficGenerate1(prvTgfPortsArray[0],nextHopPortNumArray);

    /* fill destination IP address for the prefix */
    ipAddr.arIP[0]=4;
    ipAddr.arIP[1]=0;
    ipAddr.arIP[2]=0;
    ipAddr.arIP[3]=0;

    /* fill nexthope info for the prefix */
    cpssOsMemSet(&nextHopInfo, 0, sizeof(nextHopInfo));
    nextHopInfo.ipLttEntry.routeEntryBaseIndex = prvTgfRouteEntryBaseIndexNH1;
    nextHopInfo.ipLttEntry.routeType = PRV_TGF_IP_REGULAR_ROUTE_ENTRY_GROUP_MAC;

    /* Re-add prefix 4.0.0/24 as NH1 */
    rc = prvTgfIpLpmIpv4UcPrefixAdd(prvTgfLpmDBId, 0, ipAddr, PREFIX_LEN_24_CNS, &nextHopInfo, GT_FALSE, GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpLpmIpv4UcPrefixAdd: %d", prvTgfDevNum);

    /* Last 2 steps shows that deleting and re-adding the “default 4.0.0/24” doesn’t affect the other prefixes */

    /* AUTODOC: Check traffic remain the same */
    /* AUTODOC: first 2 should match NH2 and last 2 should match NH3 */
    /* 000.000.000.000. - 003.000.000.000. / 08  -> NH 0
       004.000.000.000. - 004.000.000.127. / 32  -> NH 2
       004.000.000.128. - 004.000.000.255. / 32  -> NH 3
       004.000.001.000. - 004.000.255.000. / 24  -> NH 0
       004.001.000.000. - 004.255.000.000. / 16  -> NH 0
       005.000.000.000. - 223.000.000.000. / 08  -> NH 0
    */
    nextHopPortNumArray[0]=prvTgfPortsArray[2];
    nextHopPortNumArray[1]=prvTgfPortsArray[2];
    nextHopPortNumArray[2]=prvTgfPortsArray[3];
    nextHopPortNumArray[3]=prvTgfPortsArray[3];

    trafficGenerate1(prvTgfPortsArray[0],nextHopPortNumArray);

    /* fill destination IP address for the prefix */
    ipAddr.arIP[0]=4;
    ipAddr.arIP[1]=0;
    ipAddr.arIP[2]=0;
    ipAddr.arIP[3]=0;

    /* AUTODOC: delete the Ipv4 prefix 4.0.0/25 */
    rc = prvTgfIpLpmIpv4UcPrefixDel(prvTgfLpmDBId, 0, ipAddr, PREFIX_LEN_25_CNS);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpLpmIpv4UcPrefixDel: %d", prvTgfDevNum);

    /* AUTODOC: Check traffic */
    /* AUTODOC: first 2 should match NH1 and last 2 should match NH3 */
    /* 000.000.000.000. - 003.000.000.000. / 08  -> NH 0
       004.000.000.000. - 004.000.000.127. / 32  -> NH 1
       004.000.000.128. - 004.000.000.255. / 32  -> NH 3
       004.000.001.000. - 004.000.255.000. / 24  -> NH 0
       004.001.000.000. - 004.255.000.000. / 16  -> NH 0
       005.000.000.000. - 223.000.000.000. / 08  -> NH 0
    */

    nextHopPortNumArray[0]=prvTgfPortsArray[1];
    nextHopPortNumArray[1]=prvTgfPortsArray[1];
    nextHopPortNumArray[2]=prvTgfPortsArray[3];
    nextHopPortNumArray[3]=prvTgfPortsArray[3];

    trafficGenerate1(prvTgfPortsArray[0],nextHopPortNumArray);

    /* fill destination IP address for the prefix */
    ipAddr.arIP[0]=4;
    ipAddr.arIP[1]=0;
    ipAddr.arIP[2]=0;
    ipAddr.arIP[3]=128;

    /* AUTODOC: delete the Ipv4 prefix 4.0.0128/25 */
    rc = prvTgfIpLpmIpv4UcPrefixDel(prvTgfLpmDBId, 0, ipAddr, PREFIX_LEN_25_CNS);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpLpmIpv4UcPrefixDel: %d", prvTgfDevNum);

    /* AUTODOC: send all packets to the prefixes added */
    /* AUTODOC: all should match NH1 */
    /* 000.000.000.000. - 003.000.000.000. / 08  -> NH 0
       004.000.000.000. - 004.000.000.000. / 24  -> NH 1
       004.000.001.000. - 004.000.255.000. / 24  -> NH 0
       004.001.000.000. - 004.255.000.000. / 16  -> NH 0
       005.000.000.000. - 223.000.000.000. / 08  -> NH 0
    */
    nextHopPortNumArray[0]=prvTgfPortsArray[1];
    nextHopPortNumArray[1]=prvTgfPortsArray[1];
    nextHopPortNumArray[2]=prvTgfPortsArray[1];
    nextHopPortNumArray[3]=prvTgfPortsArray[1];

    trafficGenerate1(prvTgfPortsArray[0],nextHopPortNumArray);

    /* fill destination IP address for the prefix */
    ipAddr.arIP[0]=4;
    ipAddr.arIP[1]=0;
    ipAddr.arIP[2]=0;
    ipAddr.arIP[3]=0;

    /* AUTODOC: delete the Ipv4 prefix 4.0.0/24 */
    rc = prvTgfIpLpmIpv4UcPrefixDel(prvTgfLpmDBId, 0, ipAddr, PREFIX_LEN_24_CNS);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpLpmIpv4UcPrefixDel: %d", prvTgfDevNum);

    /* AUTODOC: Check traffic */
    /* AUTODOC: Expect no traffic */
    prvTgfBasicIpv4UcRoutingTrafficGenerate(prvTgfPortsArray[0], prvTgfPortsArray[3], PRV_TGF_DEFAULT_BOOLEAN_CNS, PRV_TGF_DEFAULT_BOOLEAN_CNS, GT_TRUE, GT_TRUE);
}

/**
* @internal prvTgfIpv4UcRoutingHiddenPrefixesConfigurationRestore function
* @endinternal
*
* @brief   Restore configuration
*
* @note 2. Restore Route Configuration
*       1. Restore Base Configuration
*
*/
GT_VOID prvTgfIpv4UcRoutingHiddenPrefixesConfigurationRestore(GT_VOID)
{
    GT_U32      vlanIter  = 0;
    GT_U32      portIter  = 0;
    GT_U32      vlanCount = sizeof(prvTgfVlanArray)/sizeof(prvTgfVlanArray[0]);
    GT_STATUS   rc        = GT_OK;

    /* AUTODOC: RESTORE CONFIGURATION: */

    PRV_UTF_LOG0_MAC("======= Restoring Configuration =======\n");

    /* delete the FDB entry */
    rc = prvTgfBrgFdbFlush(GT_TRUE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbFlush: %d", prvTgfDevNum);

    /* clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");

    /* AUTODOC: disable Unicast IPv4 Routing on port 0 */
    rc = prvTgfIpPortRoutingEnable(PRV_TGF_SEND_PORT_IDX_CNS, CPSS_IP_UNICAST_E,
                                   CPSS_IP_PROTOCOL_IPV4_E, GT_FALSE);
   UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfIpPortRoutingEnable: %d %d",
                                prvTgfDevNum, PRV_TGF_SEND_PORT_IDX_CNS);

    /* AUTODOC: disable IPv4 Unicast Routing on Vlan 5 */
    rc = prvTgfIpVlanRoutingEnable(PRV_TGF_SEND_VLANID_CNS, CPSS_IP_UNICAST_E, CPSS_IP_PROTOCOL_IPV4_E, GT_FALSE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfIpVlanRoutingEnable: %d %d",
                                 prvTgfDevNum, PRV_TGF_SEND_VLANID_CNS);

    /* enable Patricia trie validity */
    prvTgfIpValidityCheckEnable(GT_TRUE);


    /* delete port from both VLANs */
    for (vlanIter = 0; vlanIter < vlanCount; vlanIter++)
    {
        /* delete Ports from VLANs */
        for (portIter = 0; portIter < PRV_TGF_PORT_COUNT_CNS; portIter++)
        {
            rc = prvTgfBrgVlanMemberRemove(prvTgfDevNum, prvTgfVlanArray[vlanIter],
                                        prvTgfPortsArray[portIter]);
            UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanMemberRemove: %d, %d, %d",
                                         prvTgfDevNum, prvTgfVlanArray[vlanIter],
                                         prvTgfPortsArray[portIter]);
        }

        /* delete VLANs */

        /* AUTODOC: invalidate vlan entries 5,6 */
        rc = prvTgfBrgVlanEntryInvalidate(prvTgfVlanArray[vlanIter]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanEntryInvalidate: %d, %d",
                                     prvTgfDevNum, prvTgfVlanArray[vlanIter]);
    }

    /* AUTODOC: disable VLAN based MAC learning */
    rc = prvTgfBrgVlanLearningStateSet(PRV_TGF_SEND_VLANID_CNS, GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanLearningStateSet: %d", prvTgfDevNum);
}

