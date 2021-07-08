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
* @file prvTgfFdbBasicIpv4UcRoutingFillFdb.c
*
* @brief FDB IPv4 UC Routing
*
* @version   9
********************************************************************************
*/

#include <cpssCommon/cpssPresteraDefs.h>
#include <cpss/extServices/private/prvCpssBindFunc.h>

#include <utf/utfMain.h>
#include <utf/private/prvUtfExtras.h>
#include <utf/private/prvUtfHelpers.h>

#include <trafficEngine/tgfTrafficGenerator.h>
#include <trafficEngine/tgfTrafficTable.h>

#include <common/tgfCommon.h>
#include <common/tgfBridgeGen.h>
#include <common/tgfTrunkGen.h>
#include <common/tgfIpGen.h>
#include <trunk/prvTgfTrunk.h>

/*#include <bridge/prvTgfFdbBasicIpv4UcRoutingFillFdb.h>*/
#include <ip/prvTgfBasicIpv4UcRouting.h>

/******************************************************************************\
 *                       Test configuration section                           *
\******************************************************************************/
/* default VLAN Id */
#define PRV_TGF_SEND_VLANID_CNS           5

/* nextHop VLAN Id */
#define PRV_TGF_NEXTHOPE_VLANID_CNS       6

/* number of ports */
#define PRV_TGF_PORT_COUNT_CNS            4

/* port number to send traffic to */
#define PRV_TGF_SEND_PORT_IDX_CNS         0

/* port number to send traffic to */
#define PRV_TGF_FDB_PORT_IDX_CNS          1

/* nextHop port number to receive traffic from */
#define PRV_TGF_NEXTHOPE_2_PORT_IDX_CNS   3

/* MAC2ME entry index */
#define PRV_TGF_MAC_TO_ME_INDEX_CNS       1

/* the counter set for a route entry is linked to */
#define PRV_TGF_COUNT_SET_CNS     CPSS_IP_CNT_SET0_E

/* the Virtual Router index */
static GT_U32        prvUtfVrfId  = 1;
static GT_U32        lpmDbId      = 0;

static PRV_TGF_MAC_HASH_FUNC_MODE_ENT   prvTgfHashModeGet;
static GT_BOOL                          prvTgfIpv4UcPortRoutingEnableGet;
static GT_BOOL                          prvTgfFdbRoutingPortEnableGet;

/* default next hop packet command used for restore */
static CPSS_PACKET_CMD_ENT              prvTgfNhPacketCmdGet;

/* the Arp Address index of the Router ARP Table
   to write to the UC Route entry Arp nextHopARPPointer field */
static GT_U32        prvTgfRouterArpIndex      = 1;

static GT_ETHERADDR prvTgfArpMacAddrGet;
static GT_U32       prvTgfIpv4UcFdbEntryIndex;


/* isRoutingByIndex - GT_TRUE:  set routing entry by index
                      GT_FALSE: set routing entry by message */
static GT_BOOL      isRoutingByIndex = GT_FALSE;

/* default number of packets to send */
#ifdef GM_USED /*GM*/
    static GT_U32        prvTgfBurstCount   = 1025;
#else /* WM/BM */
    static GT_U32        prvTgfBurstCount   = 0xFFFF-1;  /* two bytes used */
#endif



/******************************* Test packet **********************************/

/* L2 part of packet */
static TGF_PACKET_L2_STC prvTgfPacketL2Part = {
    {0x00, 0x00, 0x00, 0x00, 0x34, 0x02},               /* daMac */
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x01}                /* saMac */
};
/* packet's IPv4 */
static TGF_PACKET_IPV4_STC prvTgfPacketIpv4Part = {
    4,                  /* version */
    5,                  /* headerLen */
    0,                  /* typeOfService */
    0x2E,               /* totalLen */
    0,                  /* id */
    0,                  /* flags */
    0,                  /* offset */
    0x40,               /* timeToLive */
    0xFF,               /* protocol */
    TGF_PACKET_AUTO_CALC_CHECKSUM_CNS,             /* csum */
    { 2,  2,  2,  4},   /* srcAddr */
    { 1,  1,  0,  0}    /* dstAddr */
};

/* VLAN_TAG part */
static TGF_PACKET_VLAN_TAG_STC prvTgfPacketVlanTagPart = {
    TGF_ETHERTYPE_8100_VLAN_TAG_CNS,                    /* etherType */
    0, 0, PRV_TGF_SEND_VLANID_CNS                       /* pri, cfi, VlanId */
};
/* packet's ethertype */
static TGF_PACKET_ETHERTYPE_STC prvTgfPacketEtherTypePart = {TGF_ETHERTYPE_0800_IPV4_TAG_CNS};

/* DATA of packet */
static GT_U8 prvTgfPayloadDataArr[] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00
};

/* PAYLOAD part */
static TGF_PACKET_PAYLOAD_STC prvTgfPacketPayloadPart = {
    sizeof(prvTgfPayloadDataArr),                       /* dataLength */
    prvTgfPayloadDataArr                                /* dataPtr */
};

/* LENGTH of packet */
#define PRV_TGF_PACKET_LEN_CNS \
    TGF_L2_HEADER_SIZE_CNS + TGF_VLAN_TAG_SIZE_CNS + TGF_ETHERTYPE_SIZE_CNS \
    + TGF_IPV4_HEADER_SIZE_CNS + sizeof(prvTgfPayloadDataArr)

/* the ARP MAC address to write to the Router ARP Table */
static TGF_MAC_ADDR  prvTgfArpMac = {0x00, 0x00, 0x00, 0x00, 0x00, 0x22};

/* PARTS of packet */
static TGF_PACKET_PART_STC prvTgfPacketPartArray[] = {
    {TGF_PACKET_PART_L2_E,        &prvTgfPacketL2Part},  /* type, partPtr */
    {TGF_PACKET_PART_VLAN_TAG_E,  &prvTgfPacketVlanTagPart},
    {TGF_PACKET_PART_SKIP_E ,      NULL},/*reserved for DSA tag (instead of the vlan tag)*/
    {TGF_PACKET_PART_ETHERTYPE_E, &prvTgfPacketEtherTypePart},
    {TGF_PACKET_PART_IPV4_E,      &prvTgfPacketIpv4Part},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacketPayloadPart}
};

/* PACKET Ethernet to send */
static TGF_PACKET_STC prvTgfPacketInfo = {
    TGF_PACKET_AUTO_CALC_LENGTH_CNS,
    sizeof(prvTgfPacketPartArray) / sizeof(TGF_PACKET_PART_STC), /* numOfParts */
    prvTgfPacketPartArray                                        /* partsArray */
};



/**
* @internal prvTgfFdbBasicIpv4UcRoutingFillFdbBaseConfigurationSet function
* @endinternal
*
* @brief   Set Base Configuration
*/
static GT_VOID prvTgfFdbBasicIpv4UcRoutingFillFdbBaseConfigurationSet
(
    GT_VOID
)
{
    GT_STATUS rc = GT_OK;
    GT_U8     tagArray[] = {1, 1, 1, 1};
    PRV_TGF_TTI_MAC_VLAN_STC                    macToMePattern;
    PRV_TGF_TTI_MAC_VLAN_STC                    macToMeMask;


    /* AUTODOC: SETUP CONFIGURATION: */

    /* AUTODOC: create virtual router */
    if (0 != prvUtfVrfId)
    {
        PRV_TGF_IP_TCAM_ROUTE_ENTRY_INFO_UNT    defIpv4UcRouteEntryInfo;
        PRV_TGF_IP_TCAM_ROUTE_ENTRY_INFO_UNT    defIpv6UcRouteEntryInfo;

        /* clear entry */
        cpssOsMemSet(&defIpv4UcRouteEntryInfo, 0, sizeof(PRV_TGF_IP_TCAM_ROUTE_ENTRY_INFO_UNT));
        cpssOsMemSet(&defIpv6UcRouteEntryInfo, 0, sizeof(PRV_TGF_IP_TCAM_ROUTE_ENTRY_INFO_UNT));

        /* create Virtual Router [prvUtfVrfId] with created default LTT entries */
        rc = prvTgfIpLpmVirtualRouterAdd(lpmDbId, prvUtfVrfId,
                                         &defIpv4UcRouteEntryInfo,
                                         &defIpv6UcRouteEntryInfo,
                                         NULL, NULL, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpLpmVirtualRouterAdd: %d", prvTgfDevNum);
    }

    /* AUTODOC: create VLAN 5 with tagged ports 0,1 */
    rc = prvTgfBrgDefVlanEntryWithPortsSet(PRV_TGF_SEND_VLANID_CNS,
            prvTgfPortsArray, NULL, tagArray, 2);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
            "prvTgfBrgDefVlanEntryWithPortsSet: %d", prvTgfDevNum);

    /* AUTODOC: create VLAN 6 with tagged ports 2,3 */
    rc = prvTgfBrgDefVlanEntryWithPortsSet(PRV_TGF_NEXTHOPE_VLANID_CNS,
            prvTgfPortsArray + 2, NULL, tagArray + 2, 2);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
            "prvTgfBrgDefVlanEntryWithPortsSet: %d", prvTgfDevNum);


    /* AUTODOC: set MAC to ME table */
    cpssOsMemCpy((GT_VOID*)macToMePattern.mac.arEther,
                 (GT_VOID*)prvTgfPacketL2Part.daMac,
                 sizeof(macToMePattern.mac.arEther));
    macToMePattern.vlanId = PRV_TGF_SEND_VLANID_CNS;


    cpssOsMemSet((GT_VOID*)macToMeMask.mac.arEther, 0xFF,
                 sizeof(macToMeMask.mac.arEther));
    macToMeMask.vlanId = 0xFFF;

    /* AUTODOC: config MAC to ME table entry index 1: */
    /* AUTODOC:   DA=00:00:00:00:34:02, VID=5 */
    rc = prvTgfTtiMacToMeSet(PRV_TGF_MAC_TO_ME_INDEX_CNS, &macToMePattern, &macToMeMask);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTtiMacToMeSet");

}

/**
* @internal prvTgfFdbIpv4UcRoutingRouteFillFdbConfigurationSet function
* @endinternal
*
* @brief   Set FDB Route Configuration
*
* @param[in] macEntryPtr              - (pointer to) mac entry
*                                       None
*/
static GT_VOID prvTgfFdbIpv4UcRoutingRouteFillFdbConfigurationSet
(
    PRV_TGF_BRG_MAC_ENTRY_STC               *macEntryPtr
)
{
    GT_STATUS rc = GT_OK;
    GT_BOOL                      valid;
    GT_BOOL                      skip;
    GT_U32                       counterVal;

    if (isRoutingByIndex == GT_TRUE)
    {
        rc =  prvTgfBrgFdbMacEntryIndexFind(&(macEntryPtr->key),&prvTgfIpv4UcFdbEntryIndex);
        if (rc == GT_NOT_FOUND)
        {
            rc = GT_OK;
        }
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbMacEntryIndexFind_MultiHash: %d", prvTgfDevNum);

        rc = prvTgfBrgFdbMacEntryStatusGet(prvTgfIpv4UcFdbEntryIndex,&valid,&skip);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbMacEntryStatusGet: %d", prvTgfDevNum);

        rc = prvTgfBrgFdbMacEntryWrite(prvTgfIpv4UcFdbEntryIndex, GT_FALSE, macEntryPtr);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbMacEntryWrite: %d", prvTgfDevNum);

        if(valid==GT_FALSE)/* new entry is added need to update the counter */
        {
            rc =  prvTgfBrgFdbBankCounterUpdate(prvTgfDevNum, prvTgfIpv4UcFdbEntryIndex%16,GT_TRUE);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbBankCounterUpdate: %d", prvTgfIpv4UcFdbEntryIndex%16);
        }

        rc =  prvTgfBrgFdbBankCounterValueGet(prvTgfDevNum, prvTgfIpv4UcFdbEntryIndex%16,&counterVal);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbBankCounterValueGet: %d", prvTgfIpv4UcFdbEntryIndex%16);
        /* limit log size by dump only one/1K lines */
        if ((counterVal % 1024) == 0)
        {
            PRV_UTF_LOG1_MAC("after adding an entry prvTgfBrgFdbBankCounterValueGet counter = %d\n",counterVal);
        }
    }
    else
    {
        rc = prvTgfBrgFdbMacEntrySet(macEntryPtr);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbMacEntrySet: %d", prvTgfDevNum);

    }
}

/**
* @internal prvTgfFdbBasicIpv4UcGenericRoutingFillFdbConfigurationSet function
* @endinternal
*
* @brief   Set FDB Generic Route Configuration
*/
static GT_VOID prvTgfFdbBasicIpv4UcGenericRoutingFillFdbConfigurationSet
(
    GT_VOID
)
{
    GT_STATUS rc = GT_OK;

    /* AUTODOC: get current  CRC Multi HASH mode */
    rc =  prvTgfBrgFdbHashModeGet(prvTgfDevNum, &prvTgfHashModeGet);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbHashModeGet");

    /* AUTODOC: set CRC Multi HASH mode */
    rc =  prvTgfBrgFdbHashModeSet(prvTgfDevNum,PRV_TGF_MAC_HASH_FUNC_CRC_MULTI_HASH_E);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbHashModeSet");


    /* AUTODOC: get enable Unicast IPv4 Routing status on port 0 */
    rc = prvTgfIpPortRoutingEnableGet( PRV_TGF_SEND_PORT_IDX_CNS, CPSS_IP_UNICAST_E,
                                   CPSS_IP_PROTOCOL_IPV4_E,
                                   &prvTgfIpv4UcPortRoutingEnableGet);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpPortRoutingEnableGet");


    /* AUTODOC: enable Unicast IPv4 Routing on port 0 */
    rc = prvTgfIpPortRoutingEnable(PRV_TGF_SEND_PORT_IDX_CNS, CPSS_IP_UNICAST_E,
                               CPSS_IP_PROTOCOL_IPV4_E, GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfIpPortRoutingEnable: %d %d",
                             prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);

    /* AUTODOC: Get enable/disable status of  using of FDB for Destination IP lookup for IP UC routing */
    rc = prvTgfBrgFdbRoutingPortIpUcEnableGet( prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
                                           CPSS_IP_PROTOCOL_IPV4_E, &prvTgfFdbRoutingPortEnableGet );
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbRoutingPortIpUcEnableGet");

    /* AUTODOC: Enable using of FDB for Destination IP lookup for IP UC routing */
    rc = prvTgfBrgFdbRoutingPortIpUcEnableSet(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
                                          CPSS_IP_PROTOCOL_IPV4_E, GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbRoutingPortIpUcEnableSet: %d %d",
                             prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);

    /* AUTODOC: enable IPv4 Unicast Routing on Vlan 5 */
    rc = prvTgfIpVlanRoutingEnable(PRV_TGF_SEND_VLANID_CNS, CPSS_IP_UNICAST_E, CPSS_IP_PROTOCOL_IPV4_E, GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfIpVlanRoutingEnable: %d %d",
                             prvTgfDevNum, PRV_TGF_SEND_VLANID_CNS);
}

/**
* @internal prvTgfFdbBasicIpv4UcRoutingFillFdbRouteConfigurationSet function
* @endinternal
*
* @brief   Set FDB Route Configuration
*/
static GT_VOID prvTgfFdbBasicIpv4UcRoutingFillFdbRouteConfigurationSet
(
    GT_VOID
)
{
    GT_STATUS                   rc;
    GT_ETHERADDR                arpMacAddr;
    PRV_TGF_BRG_MAC_ENTRY_STC   macEntry;
    GT_U32                      i;
    GT_U32                      tblSize;

    /* AUTODOC: write a ARP MAC 00:00:00:00:00:22 to the Router ARP Table */
    cpssOsMemCpy(arpMacAddr.arEther, prvTgfArpMac, sizeof(TGF_MAC_ADDR));

    /* Create the Route entry (Next hop) in FDB table and Router ARP Table */
    rc = prvTgfIpRouterArpAddrRead(prvTgfDevNum,prvTgfRouterArpIndex,&prvTgfArpMacAddrGet);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpRouterArpAddrRead: %d", prvTgfDevNum);

    rc = prvTgfIpRouterArpAddrWrite(prvTgfRouterArpIndex, &arpMacAddr);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpRouterArpAddrWrite: %d", prvTgfDevNum);

    /* AUTODOC: get Packet Commands for FDB routed packets */
    rc = prvTgfBrgFdbRoutingNextHopPacketCmdGet( prvTgfDevNum,&prvTgfNhPacketCmdGet);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbRoutingNextHopPacketCmdGet");

    /* AUTODOC: set Packet Commands for FDB routed packets */
    rc = prvTgfBrgFdbRoutingNextHopPacketCmdSet( prvTgfDevNum,CPSS_PACKET_CMD_ROUTE_E);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbRoutingNextHopPacketCmdSet");

    /* AUTODOC: update VRF Id in vlan[PRV_TGF_SEND_VLANID_CNS] */
    rc = prvTgfBrgVlanVrfIdSet(PRV_TGF_SEND_VLANID_CNS, prvUtfVrfId);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanVrfIdSet: %d %d",
                             prvTgfDevNum, PRV_TGF_SEND_VLANID_CNS);


    /* AUTODOC: calculate index for ipv4 uc route entry in FDB */
    /* AUTODOC: set  ipv4 uc route entry in FDB*/
    cpssOsMemSet(&macEntry, 0, sizeof(macEntry));
    macEntry.key.entryType = PRV_TGF_FDB_ENTRY_TYPE_IPV4_UC_E;

    /* set key virtual router */
    macEntry.key.key.ipv4Unicast.vrfId = prvUtfVrfId;
    /* fill a destination IP address for the prefix */
    cpssOsMemCpy(&macEntry.key.key.ipv4Unicast.dip, prvTgfPacketIpv4Part.dstAddr, sizeof(macEntry.key.key.ipv4Unicast.dip));

    macEntry.key.entryType = PRV_TGF_FDB_ENTRY_TYPE_IPV4_UC_E;
    macEntry.fdbRoutingInfo.ttlHopLimDecOptionsExtChkByPass = GT_FALSE;
    macEntry.fdbRoutingInfo.ttlHopLimitDecEnable = GT_FALSE;
    macEntry.fdbRoutingInfo.ingressMirror = GT_FALSE;
    macEntry.fdbRoutingInfo.ingressMirrorToAnalyzerIndex = 0;
    macEntry.fdbRoutingInfo.qosProfileMarkingEnable = GT_FALSE;
    macEntry.fdbRoutingInfo.qosProfileIndex = 0;
    macEntry.fdbRoutingInfo.qosPrecedence = CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_SOFT_E;
    macEntry.fdbRoutingInfo.modifyUp = CPSS_PACKET_ATTRIBUTE_MODIFY_KEEP_PREVIOUS_E;
    macEntry.fdbRoutingInfo.modifyDscp = CPSS_PACKET_ATTRIBUTE_MODIFY_KEEP_PREVIOUS_E;
    macEntry.fdbRoutingInfo.countSet = PRV_TGF_COUNT_SET_CNS;
    macEntry.fdbRoutingInfo.trapMirrorArpBcEnable = GT_FALSE;
    macEntry.fdbRoutingInfo.dipAccessLevel = 0;
    macEntry.fdbRoutingInfo.ICMPRedirectEnable = GT_FALSE;
    macEntry.fdbRoutingInfo.mtuProfileIndex = 0;
    macEntry.fdbRoutingInfo.isTunnelStart = GT_FALSE;
    macEntry.fdbRoutingInfo.nextHopVlanId = PRV_TGF_NEXTHOPE_VLANID_CNS;
    macEntry.fdbRoutingInfo.nextHopTunnelPointer = 0;
    macEntry.fdbRoutingInfo.nextHopARPPointer = prvTgfRouterArpIndex;
    macEntry.dstInterface.type = CPSS_INTERFACE_PORT_E;
    macEntry.dstInterface.devPort.hwDevNum = prvTgfDevNum;
    macEntry.dstInterface.devPort.portNum = prvTgfPortsArray[PRV_TGF_NEXTHOPE_2_PORT_IDX_CNS];

    rc = prvUtfCfgTableNumEntriesGet(prvTgfDevNum, PRV_TGF_CFG_TABLE_FDB_E, &tblSize);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvUtfCfgTableNumEntriesGet: FDB size dev %d", prvTgfDevNum);

    if ((prvTgfBurstCount > tblSize) || (tblSize == 65536))
    {
        prvTgfBurstCount = tblSize / 2;
    }

    for (i = 0; i < prvTgfBurstCount; i++)
    {
        /* ovveride dip bytes in key */
        macEntry.key.key.ipv4Unicast.dip[2] = (GT_U8) (i >> 8);
        macEntry.key.key.ipv4Unicast.dip[3] = (GT_U8) (i & 0xFF);

        /* set the entry */
        prvTgfFdbIpv4UcRoutingRouteFillFdbConfigurationSet(&macEntry);
    }
}


/**
* @internal prvTgfFdbBasicIpv4UcGenericRoutingFillFdbConfigurationRestore function
* @endinternal
*
* @brief   Restore generic routing test configuration
*/
static GT_VOID prvTgfFdbBasicIpv4UcGenericRoutingFillFdbConfigurationRestore
(
    GT_VOID
)
{
    GT_STATUS       rc          = GT_OK;
    PRV_TGF_TTI_MAC_VLAN_STC  macToMePattern;
    PRV_TGF_TTI_MAC_VLAN_STC  macToMeMask;
    GT_BOOL                   ucRouteDelStatus;
    GT_U32                    counterVal;

    /* AUTODOC: RESTORE GENERIC ROUTING CONFIGURATION: */

    PRV_UTF_LOG0_MAC("\n========== Restore generic routing config: ==================\n\n");

    /* save fdb routing uc delete status*/
    rc =  prvTgfBrgFdbRoutingUcDeleteEnableGet(prvTgfDevNum,&ucRouteDelStatus);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbRoutingUcDeleteEnableGet: %d", prvTgfDevNum);

    /* enable fdb uc route entries to be deleted */
    rc =  prvTgfBrgFdbRoutingUcDeleteEnableSet(prvTgfDevNum,GT_TRUE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbRoutingUcDeleteEnableSet: %d", prvTgfDevNum);

    /* flush FDB include static entries */
    rc = prvTgfBrgFdbFlush(GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of prvTgfBrgFdbFlush");

    rc =  prvTgfBrgFdbBankCounterValueGet(prvTgfDevNum, prvTgfIpv4UcFdbEntryIndex%16,&counterVal);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbBankCounterValueGet: index=%d, conterVal=%d\n", prvTgfIpv4UcFdbEntryIndex%16,counterVal);
    PRV_UTF_LOG1_MAC("after flush prvTgfBrgFdbBankCounterValueGet counter = %d\n",counterVal);


    /* restore FDB uc route entry delete status */
    rc =  prvTgfBrgFdbRoutingUcDeleteEnableSet(prvTgfDevNum,ucRouteDelStatus);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbRoutingUcDeleteEnableSet: %d", prvTgfDevNum);

    /* AUTODOC: restore CRC  HASH mode */
    rc =  prvTgfBrgFdbHashModeSet(prvTgfDevNum,prvTgfHashModeGet);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbHashModeSet");

    /* AUTODOC: restore Unicast IPv4 Routing  enable status on port 0 */
    rc = prvTgfIpPortRoutingEnable(PRV_TGF_SEND_PORT_IDX_CNS, CPSS_IP_UNICAST_E,
                                   CPSS_IP_PROTOCOL_IPV4_E, prvTgfIpv4UcPortRoutingEnableGet);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfIpPortRoutingEnable: %d %d",
                                 prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);

    /* AUTODOC: disable IPv4 Unicast Routing on Vlan 5 */
    rc = prvTgfIpVlanRoutingEnable(PRV_TGF_SEND_VLANID_CNS, CPSS_IP_UNICAST_E, CPSS_IP_PROTOCOL_IPV4_E, GT_FALSE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfIpVlanRoutingEnable: %d %d",
                             prvTgfDevNum,PRV_TGF_SEND_VLANID_CNS);

    /* AUTODOC: restore status of using of FDB for Destination IP lookup for IP UC routing */
    rc = prvTgfBrgFdbRoutingPortIpUcEnableSet(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
                                              CPSS_IP_PROTOCOL_IPV4_E, prvTgfFdbRoutingPortEnableGet);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbRoutingPortIpUcEnableSet: %d %d",
                                 prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);

    /* invalidate and reset vlan entry */
    rc = prvTgfBrgDefVlanEntryInvalidate(PRV_TGF_SEND_VLANID_CNS);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,
                                 "ERROR of prvTgfBrgDefVlanEntryInvalidate");
    rc = prvTgfBrgDefVlanEntryInvalidate(PRV_TGF_NEXTHOPE_VLANID_CNS);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,
                                 "ERROR of prvTgfBrgDefVlanEntryInvalidate");


    /* AUTODOC: Invalidate MAC to ME table entry index 1 */

    /* full mask for VLAN and MAC */
    macToMeMask.mac.arEther[0] =
    macToMeMask.mac.arEther[1] =
    macToMeMask.mac.arEther[2] =
    macToMeMask.mac.arEther[3] =
    macToMeMask.mac.arEther[4] =
    macToMeMask.mac.arEther[5] = 0xFF;
    macToMeMask.vlanId = 0xFFF;

    /* use 0 MAC DA and VID = 4095 */
    macToMePattern.mac.arEther[0] =
    macToMePattern.mac.arEther[1] =
    macToMePattern.mac.arEther[2] =
    macToMePattern.mac.arEther[3] =
    macToMePattern.mac.arEther[4] =
    macToMePattern.mac.arEther[5] = 0;
    macToMePattern.vlanId = 0xFFF;

    rc = prvTgfTtiMacToMeSet(PRV_TGF_MAC_TO_ME_INDEX_CNS, &macToMePattern, &macToMeMask);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTtiMacToMeSet");
}

/**
* @internal prvTgfFdbBasicIpv4UcRoutingFillFdbConfigurationRestore function
* @endinternal
*
* @brief   Restore test configuration
*/
static GT_VOID prvTgfFdbBasicIpv4UcRoutingFillFdbConfigurationRestore
(
    GT_VOID
)
{
    GT_STATUS       rc          = GT_OK;

    /* AUTODOC: RESTORE CONFIGURATION: */

    PRV_UTF_LOG0_MAC("\n========== Restore config: ==================\n\n");

    /* AUTODOC: restore a ARP MAC  to the Router ARP Table */
    rc = prvTgfIpRouterArpAddrWrite(prvTgfRouterArpIndex, &prvTgfArpMacAddrGet);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpRouterArpAddrWrite: %d", prvTgfDevNum);

    /* AUTODOC: restore Packet Commands for FDB routed packets */
    rc = prvTgfBrgFdbRoutingNextHopPacketCmdSet( prvTgfDevNum,prvTgfNhPacketCmdGet);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbRoutingNextHopPacketCmdSet");

    /* clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");

    if (0 != prvUtfVrfId)
    {
        /* Remove Virtual Router [prvUtfVrfId] with created default LTT entries */
        rc = prvTgfIpLpmVirtualRouterDel(lpmDbId, prvUtfVrfId);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpLpmVirtualRouterDel: %d", prvTgfDevNum);
    }
}


/**
* @internal prvTgfFdbBasicIpv4UcRoutingFillFdbSendTraffic function
* @endinternal
*
* @brief   Send Traffic
*/
static GT_VOID prvTgfFdbBasicIpv4UcRoutingFillFdbSendTraffic
(
    GT_VOID
)
{
    GT_STATUS                       rc          = GT_FAIL;
    GT_U32                          portIter    = 0;
    CPSS_PORT_MAC_COUNTER_SET_STC   portCntrs;
    CPSS_PORT_MAC_COUNTER_SET_STC   expectedCntrs;
    GT_BOOL                         isOk;

    TGF_VFD_INFO_STC                vfdArray[1];

    GT_U32                   packetSize=0;
    TGF_PACKET_PART_STC      *packetPartsPtr;
    GT_U32                   partsCount=0;


    cpssOsMemSet((GT_VOID*) vfdArray, 0, sizeof(vfdArray[0]));
    cpssOsMemSet((GT_VOID*) &expectedCntrs, 0, sizeof(expectedCntrs));

    vfdArray[0].mode          = TGF_VFD_MODE_INCREMENT_E;
    vfdArray[0].offset        = 0x24; /* offset in the packet to override */

    vfdArray[0].patternPtr[0] = 0;
    vfdArray[0].patternPtr[1] = 0;

    vfdArray[0].cycleCount = 2; /* two bytes used in vfdArray[0].patternPtr */


    /* AUTODOC: GENERATE TRAFFIC: */
    PRV_UTF_LOG0_MAC("======= Generating Traffic =======\n");

    /* -------------------------------------------------------------------------
     * 1. Setup counters
     */

    /* Reset all Ethernet port's counters and clear capturing RxPcktTable */
    rc = prvTgfEthCountersReset(prvTgfDevNum);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfEthCountersReset: %d", prvTgfDevNum);

    /* Setup transmit params */
    rc = prvTgfSetTxSetupEth(prvTgfDevNum, &prvTgfPacketInfo, prvTgfBurstCount, 1/*numVfd*/, vfdArray);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfSetTxSetupEth\n");

    /* AUTODOC: calculate packet size */
    partsCount = sizeof(prvTgfPacketPartArray) / sizeof(TGF_PACKET_PART_STC);
    packetPartsPtr = prvTgfPacketPartArray;
    rc = prvTgfPacketSizeGet(packetPartsPtr, partsCount, &packetSize);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPacketSizeGet:");

    /* start transmitting */
    rc = prvTgfStartTransmitingEth(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfStartTransmitingEth\n");

    /* -------------------------------------------------------------------------
     * 3. Get Ethernet Counters
     */

    /* AUTODOC: read counters */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        rc = prvTgfReadPortCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter], GT_TRUE, &portCntrs);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfReadPortCountersEth\n");

        switch (portIter)
        {
            case PRV_TGF_SEND_PORT_IDX_CNS:
                /* packetSize is not changed */
                expectedCntrs.goodOctetsSent.l[0] = (packetSize + TGF_CRC_LEN_CNS) * prvTgfBurstCount;
                expectedCntrs.goodPktsSent.l[0]   = prvTgfBurstCount;
                expectedCntrs.ucPktsSent.l[0]     = prvTgfBurstCount;
                expectedCntrs.brdcPktsSent.l[0]   = 0;
                expectedCntrs.mcPktsSent.l[0]     = 0;
                expectedCntrs.goodOctetsRcv.l[0]  = (packetSize + TGF_CRC_LEN_CNS) * prvTgfBurstCount;
                expectedCntrs.goodPktsRcv.l[0]    = prvTgfBurstCount;
                expectedCntrs.ucPktsRcv.l[0]      = prvTgfBurstCount;
                expectedCntrs.brdcPktsRcv.l[0]    = 0;
                expectedCntrs.mcPktsRcv.l[0]      = 0;
                break;
            case PRV_TGF_NEXTHOPE_2_PORT_IDX_CNS:
                /* packetSize is not changed */
                expectedCntrs.goodOctetsSent.l[0] = (packetSize + TGF_CRC_LEN_CNS) * prvTgfBurstCount;
                expectedCntrs.goodPktsSent.l[0]   = prvTgfBurstCount;
                expectedCntrs.ucPktsSent.l[0]     = prvTgfBurstCount;
                break;
            default:
                 /* for other ports */
                expectedCntrs.goodOctetsSent.l[0] = 0;
                expectedCntrs.goodPktsSent.l[0]   = 0;
                expectedCntrs.ucPktsSent.l[0]     = 0;
                expectedCntrs.brdcPktsSent.l[0]   = 0;
                expectedCntrs.mcPktsSent.l[0]     = 0;
                expectedCntrs.goodOctetsRcv.l[0]  = 0;
                expectedCntrs.goodPktsRcv.l[0]    = 0;
                expectedCntrs.ucPktsRcv.l[0]      = 0;
                expectedCntrs.brdcPktsRcv.l[0]    = 0;
                expectedCntrs.mcPktsRcv.l[0]      = 0;
        }

        isOk =
            portCntrs.goodOctetsSent.l[0] == expectedCntrs.goodOctetsSent.l[0] &&
            portCntrs.goodPktsSent.l[0]   == expectedCntrs.goodPktsSent.l[0] &&
            portCntrs.ucPktsSent.l[0]     == expectedCntrs.ucPktsSent.l[0] &&
            portCntrs.brdcPktsSent.l[0]   == expectedCntrs.brdcPktsSent.l[0] &&
            portCntrs.mcPktsSent.l[0]     == expectedCntrs.mcPktsSent.l[0] &&
            portCntrs.goodOctetsRcv.l[0]  == expectedCntrs.goodOctetsRcv.l[0] &&
            portCntrs.goodPktsRcv.l[0]    == expectedCntrs.goodPktsRcv.l[0] &&
            portCntrs.ucPktsRcv.l[0]      == expectedCntrs.ucPktsRcv.l[0] &&
            portCntrs.brdcPktsRcv.l[0]    == expectedCntrs.brdcPktsRcv.l[0] &&
            portCntrs.mcPktsRcv.l[0]      == expectedCntrs.mcPktsRcv.l[0];


        UTF_VERIFY_EQUAL1_STRING_MAC(isOk, GT_TRUE, "Got another counters values on port [%d]",
                                                                prvTgfPortsArray[portIter]);

        /* print expected values if bug */
        if (isOk != GT_TRUE) {

            PRV_UTF_LOG0_MAC("Expected values:\n");
            PRV_UTF_LOG1_MAC("  goodOctetsSent = %d\n", expectedCntrs.goodOctetsSent.l[0]);
            PRV_UTF_LOG1_MAC("    goodPktsSent = %d\n", expectedCntrs.goodPktsSent.l[0]);
            PRV_UTF_LOG1_MAC("      ucPktsSent = %d\n", expectedCntrs.ucPktsSent.l[0]);
            PRV_UTF_LOG1_MAC("    brdcPktsSent = %d\n", expectedCntrs.brdcPktsSent.l[0]);
            PRV_UTF_LOG1_MAC("      mcPktsSent = %d\n", expectedCntrs.mcPktsSent.l[0]);
            PRV_UTF_LOG1_MAC("   goodOctetsRcv = %d\n", expectedCntrs.goodOctetsRcv.l[0]);
            PRV_UTF_LOG1_MAC("     goodPktsRcv = %d\n", expectedCntrs.goodPktsRcv.l[0]);
            PRV_UTF_LOG1_MAC("       ucPktsRcv = %d\n", expectedCntrs.ucPktsRcv.l[0]);
            PRV_UTF_LOG1_MAC("     brdcPktsRcv = %d\n", expectedCntrs.brdcPktsRcv.l[0]);
            PRV_UTF_LOG1_MAC("       mcPktsRcv = %d\n", expectedCntrs.mcPktsRcv.l[0]);
            PRV_UTF_LOG0_MAC("\n");
        }
    }
}




/**
* @internal prvTgfFdbBasicIpv4UcRoutingFillFdb function
* @endinternal
*
* @brief   Test prvTgfFdbBasicIpv4UcRoutingFillFdb main function
*
* @param[in] routingByIndex           - GT_TRUE:  set routing entry by index
*                                      GT_FALSE: set routing entry by message
*                                       None
*/
GT_VOID prvTgfFdbBasicIpv4UcRoutingFillFdb
(
    GT_BOOL routingByIndex
)
{
    isRoutingByIndex = routingByIndex;


    /* Set Base configuration */
    prvTgfFdbBasicIpv4UcRoutingFillFdbBaseConfigurationSet();

    /* Set Generic Route configuration */
    prvTgfFdbBasicIpv4UcGenericRoutingFillFdbConfigurationSet();

    /* Set Route configuration */
    prvTgfFdbBasicIpv4UcRoutingFillFdbRouteConfigurationSet();

    /* Generate traffic */
    prvTgfFdbBasicIpv4UcRoutingFillFdbSendTraffic();

    /* Restore generic route configuration */
    prvTgfFdbBasicIpv4UcGenericRoutingFillFdbConfigurationRestore();

    /* Restore configuration */
    prvTgfFdbBasicIpv4UcRoutingFillFdbConfigurationRestore();
}


