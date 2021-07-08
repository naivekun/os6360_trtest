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
* @file prvTgfFdbIpv6UcRoutingAgingEnable.c
*
* @brief FDB IPv6 UC Routing + Aging configurations
*
* @version   1
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

#include <bridge/prvTgfFdbBasicIpv6UcRouting.h>
#include <bridge/prvTgfFdbIpv6UcRoutingAgingEnable.h>

/******************************************************************************\
 *                       Test configuration section                           *
\******************************************************************************/

static GT_HW_DEV_NUM hwDevNum = 0x10;

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
#define PRV_TGF_NEXTHOP_PORT_IDX_CNS      2
/*#define PRV_TGF_NEXTHOP_2_PORT_IDX_CNS    3*/

/* MAC2ME entry index */
#define PRV_TGF_MAC_TO_ME_INDEX_CNS     1

/* the counter set for a route entry is linked to */
#define PRV_TGF_COUNT_SET_CNS     CPSS_IP_CNT_SET0_E


/* the Virtual Router index */
static GT_U32        prvUtfVrfId  = 1;
static GT_U32        lpmDbId      = 0;

/* the Arp Address index of the Router ARP Table
   to write to the UC Route entry Arp nextHopARPPointer field */
static GT_U32        prvTgfRouterArpIndex      = 1;

static GT_ETHERADDR prvTgfArpMacAddrGet;

/* default next hop packet command used for restore */
static CPSS_PACKET_CMD_ENT  prvTgfNhPacketCmdGet;

/* default refresh value for restore */
static GT_BOOL prvTgfUcIpv6RouteAgingStatusFirstCall=GT_TRUE;/* is this the first call to sav ethe value */
static GT_BOOL prvTgfUcIpv6RouteAgingStatusGet;
static PRV_TGF_MAC_ACTION_MODE_ENT prvTgfUcIpv6RouteAgingModeGet;
static GT_BOOL prvTgfUcIpv6RouteAAandTAToCpuGet;
static GT_U32  origActDev;    /*Action active device */
static GT_U32  origActDevMask;/*Action active mask*/
static GT_U32  origActIsTrunk;/*Action active isTrunk */
static GT_U32  origActIsTrunkMask;/*Action active isTrunk mask */
static GT_U32  origActTrunkPort;/*Action active Trunk/Port  */
static GT_U32  origActTrunkPortMask;/*Action active Trunk/Port mask */

/* flag for first default values get  - the get should be done only once */
static GT_BOOL defaultValuesFirstGet = GT_TRUE;
/* keep address index and data index for future use*/
static GT_U32       prvTgfIpv6UcFdbEntryAddressIndex;
static GT_U32       prvTgfIpv6UcFdbEntryDataIndex;


/******************************* Test packet **********************************/

/* packet's IPv6 */
static TGF_PACKET_IPV6_STC prvTgfPacketIpv6Part = {
    6,                  /* version */
    0,                  /* trafficClass */
    0,                  /* flowLabel */
    0x02,               /* payloadLen */
    0x3b,               /* nextHeader */
    0x40,               /* hopLimit */
    {0x6545, 0, 0, 0, 0, 0, 0x0000, 0x3212}, /* TGF_IPV6_ADDR srcAddr */
    {0x1122, 0, 0, 0, 0, 0, 0xccdd, 0xeeff}  /* TGF_IPV6_ADDR dstAddr */
};

/* the ARP MAC address to write to the Router ARP Table */
static TGF_MAC_ADDR  prvTgfArpMac = {0x00, 0x00, 0x00, 0x00, 0x00, 0x22};



/**
* @internal prvTgfFdbIpv6UcRoutingAgingEnableConfigurationSet function
* @endinternal
*
* @brief   Set FDB Route Configuration
*
* @param[in] routingByIndex           - GT_TRUE:  set routing entry by index
*                                      GT_FALSE: set routing entry by message
*                                       None
*/
GT_VOID prvTgfFdbIpv6UcRoutingAgingEnableConfigurationSet
(
    GT_BOOL routingByIndex
)
{
    GT_ETHERADDR                            arpMacAddr;
    PRV_TGF_MAC_ENTRY_KEY_STC               entryKey;
    PRV_TGF_BRG_MAC_ENTRY_STC               macEntry;
    GT_U32                                  numOfBanks = 16;
    GT_U32                                  nextHopDataBankNumber;
    GT_U32                                  fdbEntryIndex;
    GT_STATUS rc = GT_OK;
    GT_U32    offset = 0;

    /* to avoid compilation warnings */
    routingByIndex = routingByIndex;

    if(defaultValuesFirstGet==GT_TRUE)
    {
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

        if(PRV_CPSS_SIP_5_10_CHECK_MAC(prvTgfDevNum))
        {
            /* AUTODOC: For Bobcat2 B0
               In case of IPv6 key or data, in order for an entry to be subject to aging
               and delete all the following should be set to 0:
               act_eport_trunk_mask, act_is_trunk_mask, act_dev_mask.
               get the default values before setting the new ones */

            /* get original act device info */
            rc = prvTgfBrgFdbActionActiveDevGet(prvTgfDevNum,&origActDev,&origActDevMask);
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbActionActiveDevGet");

            /* set new act device info */
            rc = prvTgfBrgFdbActionActiveDevSet(prvTgfDevNum,origActDev,0);
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbActionActiveDevSet");

            /* get original act device info */
            rc = prvTgfBrgFdbActionActiveInterfaceGet(prvTgfDevNum,&origActIsTrunk,&origActIsTrunkMask,&origActTrunkPort,&origActTrunkPortMask);
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbActionActiveInterfaceGet");

            /* set new act device info */
            rc = prvTgfBrgFdbActionActiveInterfaceSet(prvTgfDevNum,origActIsTrunk,0,origActTrunkPort,0);
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbActionActiveInterfaceSet");
        }

        defaultValuesFirstGet=GT_FALSE;
    }

    cpssOsMemSet(&entryKey, 0, sizeof(entryKey));
    /* fill a destination IP address for the prefix */

    for (offset = 0; offset < 8; offset++)
    {
        entryKey.key.ipv6Unicast.dip[offset << 1]       = (GT_U8)(prvTgfPacketIpv6Part.dstAddr[offset] >> 8);
        entryKey.key.ipv6Unicast.dip[(offset << 1) + 1] = (GT_U8) prvTgfPacketIpv6Part.dstAddr[offset];
    }

    /* set key virtual router */
    entryKey.key.ipv6Unicast.vrfId = prvUtfVrfId;

    /* AUTODOC: calculate index for ipv6 uc route data entry in FDB */

    /* set entry type */
    entryKey.entryType = PRV_TGF_FDB_ENTRY_TYPE_IPV6_UC_ADDR_ENTRY_E;
    rc =  prvTgfBrgFdbMacEntryIndexFind(&entryKey,&fdbEntryIndex); /* data */
    if (rc == GT_NOT_FOUND)
    {
        rc = GT_OK;
    }
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbMacEntryIndexFind: %d", fdbEntryIndex);

    if (PRV_CPSS_SIP_6_CHECK_MAC(prvTgfDevNum) == GT_FALSE)
    {
        prvTgfIpv6UcFdbEntryDataIndex = fdbEntryIndex;
        nextHopDataBankNumber = fdbEntryIndex % numOfBanks;
    }
    else
    {
        /* On SIP-6, IPV6 UC Address entry must be in even bank, and Data entry immediately follows */
        UTF_VERIFY_EQUAL1_STRING_MAC(0, fdbEntryIndex % 2, "prvTgfBrgFdbMacEntryIndexFind: %d", fdbEntryIndex);
        prvTgfIpv6UcFdbEntryDataIndex = fdbEntryIndex + 1;
        nextHopDataBankNumber = 0;
    }

    cpssOsMemSet(&macEntry, 0, sizeof(macEntry));
    cpssOsMemCpy(&macEntry.key.key.ipv6Unicast, &entryKey.key.ipv6Unicast,sizeof(macEntry.key.key.ipv6Unicast));
    macEntry.key.entryType = PRV_TGF_FDB_ENTRY_TYPE_IPV6_UC_DATA_ENTRY_E;
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
    macEntry.dstInterface.devPort.hwDevNum = hwDevNum;
    macEntry.dstInterface.devPort.portNum = prvTgfPortsArray[PRV_TGF_NEXTHOP_PORT_IDX_CNS];
   macEntry.dstInterface.vlanId = 10;

    macEntry.fdbRoutingInfo.nextHopDataBankNumber = 0;
    macEntry.fdbRoutingInfo.scopeCheckingEnable = GT_FALSE;
    macEntry.fdbRoutingInfo.siteId = CPSS_IP_SITE_ID_INTERNAL_E;

    macEntry.age = GT_TRUE;

    /* AUTODOC: set  ipv6 uc route data entry in FDB*/
    prvTgfFdbIpv6UcRoutingRouteByIndexConfigurationSet(&macEntry, prvTgfIpv6UcFdbEntryDataIndex);


    /* AUTODOC: calculate index for ipv6 uc route address entry in FDB */

    if (PRV_CPSS_SIP_6_CHECK_MAC(prvTgfDevNum) == GT_FALSE)
    {
        /* set entry type */
        entryKey.entryType = PRV_TGF_FDB_ENTRY_TYPE_IPV6_UC_ADDR_ENTRY_E;
        rc =  prvTgfBrgFdbMacEntryIndexFind(&entryKey,&fdbEntryIndex); /* address */
        if (rc == GT_NOT_FOUND)
        {
            rc = GT_OK;
        }
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbMacEntryIndexFind: %d", fdbEntryIndex);
    }

    prvTgfIpv6UcFdbEntryAddressIndex = fdbEntryIndex;

    macEntry.key.entryType = PRV_TGF_FDB_ENTRY_TYPE_IPV6_UC_ADDR_ENTRY_E;
    macEntry.fdbRoutingInfo.nextHopDataBankNumber = nextHopDataBankNumber;

    macEntry.age = GT_TRUE;

    /* AUTODOC: set  ipv6 uc route address entry in FDB*/
    prvTgfFdbIpv6UcRoutingRouteByIndexConfigurationSet(&macEntry, fdbEntryIndex);

    rc = prvTgfFdbIpv6UcUpdateAddrDataLinkedList(prvTgfDevNum,prvTgfIpv6UcFdbEntryAddressIndex,prvTgfIpv6UcFdbEntryDataIndex,GT_FALSE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfFdbIpv6UcUpdateAddrDataLinkedList: %d, %d", prvTgfIpv6UcFdbEntryAddressIndex,prvTgfIpv6UcFdbEntryDataIndex);
}


/**
* @internal prvTgfFdbIpv6UcRoutingAgingEnableSet function
* @endinternal
*
* @brief   Enable/Disable Aging of UC entries
*
* @param[in] agingWithRemoval         - GT_TRUE: age with removal
*                                      GT_FALSE: age without removal
* @param[in] enableFdbRoutingAging    - GT_TRUE:  Enable Aging of UC entries
*                                      GT_FALSE: Disable Aging of UC entries
* @param[in] enableFdbRoutingAAandTAToCpu - GT_TRUE  - AA and TA messages are not
*                                      forwarded to the CPU for UC route entries.
*                                      GT_FALSE - AA and TA messages are not
*                                      forwarded to the CPU for
*                                       None
*/
GT_VOID prvTgfFdbIpv6UcRoutingAgingEnableSet
(
    GT_BOOL     agingWithRemoval,
    GT_BOOL     enableFdbRoutingAging,
    GT_BOOL     enableFdbRoutingAAandTAToCpu
)
{
    GT_STATUS   rc;
    GT_U32      counterVal;

    PRV_UTF_LOG3_MAC("agingWithRemoval[%d], "
                     "enableFdbRoutingAging[%d], "
                     "enableFdbRoutingAAandTAToCpu[%d]\n",
                     agingWithRemoval,
                     enableFdbRoutingAging,
                     enableFdbRoutingAAandTAToCpu);

    if(prvTgfUcIpv6RouteAgingStatusFirstCall==GT_TRUE)
    {
        /* save FDB trigger mode */
        rc = prvTgfBrgFdbMacTriggerModeGet(prvTgfDevNum, &prvTgfUcIpv6RouteAgingModeGet);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc , "Error of prvTgfBrgFdbMacTriggerModeGet");


       /* save fdb routing uc Aging status*/
        rc =  prvTgfBrgFdbRoutingUcAgingEnableGet(prvTgfDevNum,
                                                 &prvTgfUcIpv6RouteAgingStatusGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbRoutingUcAgingEnableGet: %d", prvTgfDevNum);

        /* save fdb routing uc AAandTAToCpu status*/
        rc =  prvTgfBrgFdbRoutingUcAAandTAToCpuGet(prvTgfDevNum,
                                                 &prvTgfUcIpv6RouteAAandTAToCpuGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbRoutingUcAAandTAToCpuGet: %d", prvTgfDevNum);
    }

    /* AUTODOC: set trigger mode: action is done via trigger from CPU */
    rc = prvTgfBrgFdbMacTriggerModeSet(PRV_TGF_ACT_TRIG_E);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc , "Error of prvTgfBrgFdbMacTriggerModeSet");

    /* set Aging Enable */
    rc =  prvTgfBrgFdbRoutingUcAgingEnableSet(prvTgfDevNum,enableFdbRoutingAging);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbRoutingUcAgingEnableSet: %d", prvTgfDevNum);

    /* set fdb routing uc AAandTAToCpu status*/
    rc =  prvTgfBrgFdbRoutingUcAAandTAToCpuSet(prvTgfDevNum,enableFdbRoutingAAandTAToCpu);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbRoutingUcAAandTAToCpuSet: %d", prvTgfDevNum);

    PRV_UTF_LOG0_MAC("======= : trigger Aging by HW =======\n");
    /* trigger Aging by HW*/
    if(agingWithRemoval==GT_TRUE)
    {
        rc = prvTgfBrgFdbActionStart(PRV_TGF_FDB_ACTION_AGE_WITH_REMOVAL_E);
    }
    else
    {
        rc = prvTgfBrgFdbActionStart(PRV_TGF_FDB_ACTION_AGE_WITHOUT_REMOVAL_E);
    }
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);

    PRV_UTF_LOG0_MAC("======= : wait for trigger action to end =======\n");
    /* wait for trigger action to end */
    rc = prvTgfBrgFdbActionDoneWait(GT_FALSE);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);

    rc =  prvTgfBrgFdbBankCounterValueGet(prvTgfDevNum, prvTgfIpv6UcFdbEntryAddressIndex%16,&counterVal);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbBankCounterValueGet: %d", prvTgfIpv6UcFdbEntryAddressIndex%16);

    PRV_UTF_LOG2_MAC("after aging - prvTgfBrgFdbBankCounterValueGet prvTgfIpv6UcFdbEntryAddressIndex =[%d], counter = %d\n",prvTgfIpv6UcFdbEntryAddressIndex,counterVal);

    rc =  prvTgfBrgFdbBankCounterValueGet(prvTgfDevNum, prvTgfIpv6UcFdbEntryDataIndex%16,&counterVal);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbBankCounterValueGet: %d", prvTgfIpv6UcFdbEntryDataIndex%16);

    PRV_UTF_LOG2_MAC("after aging - prvTgfBrgFdbBankCounterValueGet prvTgfIpv6UcFdbEntryDataIndex =[%d], counter = %d\n",prvTgfIpv6UcFdbEntryDataIndex,counterVal);


}

/**
* @internal prvTgfFdbIpv6UcRoutingAgingCheckValidSkipAgeValues function
* @endinternal
*
* @brief   Check the Valid/Skip/Age values of the inserted FDB Entry is as expected
*
* @param[in] expectedValidAddress     -  GT_TRUE: valid is 1
*                                      GT_FALSE: valid is 0
* @param[in] expectedSkipAddress      -   GT_TRUE: skip is 1
*                                      GT_FALSE: skip is 0
* @param[in] expectedAgeAddress       -    GT_TRUE: age is 1
*                                      GT_FALSE: age is 0
* @param[in] expectedValidData        -     GT_TRUE: valid is 1
*                                      GT_FALSE: valid is 0
* @param[in] expectedSkipData         -      GT_TRUE: skip is 1
*                                      GT_FALSE: skip is 0
* @param[in] expectedAgeData          -       GT_TRUE: age is 1
*                                      GT_FALSE: age is 0
*                                       None
*/
GT_VOID prvTgfFdbIpv6UcRoutingAgingCheckValidSkipAgeValues
(
    GT_BOOL expectedValidAddress,
    GT_BOOL expectedSkipAddress,
    GT_BOOL expectedAgeAddress,
    GT_BOOL expectedValidData,
    GT_BOOL expectedSkipData,
    GT_BOOL expectedAgeData
)
{
    GT_STATUS                    rc = GT_OK;
    GT_BOOL                      tmp_valid_address;
    GT_BOOL                      tmp_skip_address;
    GT_BOOL                      tmp_aged_address;
    GT_HW_DEV_NUM                tmp_hwDevNum_address;
    PRV_TGF_BRG_MAC_ENTRY_STC    tmp_macEntry_address;
    GT_BOOL                      tmp_valid_data;
    GT_BOOL                      tmp_skip_data;
    GT_BOOL                      tmp_aged_data;
    GT_HW_DEV_NUM                tmp_hwDevNum_data;
    PRV_TGF_BRG_MAC_ENTRY_STC    tmp_macEntry_data;

    rc = prvTgfBrgFdbMacEntryRead(prvTgfIpv6UcFdbEntryAddressIndex,
                                  &tmp_valid_address,
                                  &tmp_skip_address,
                                  &tmp_aged_address,
                                  &tmp_hwDevNum_address,
                                  &tmp_macEntry_address);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbMacEntryRead: %d", prvTgfDevNum);

    rc = prvTgfBrgFdbMacEntryRead(prvTgfIpv6UcFdbEntryDataIndex,
                                  &tmp_valid_data,
                                  &tmp_skip_data,
                                  &tmp_aged_data,
                                  &tmp_hwDevNum_data,
                                  &tmp_macEntry_data);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbMacEntryRead: %d", prvTgfDevNum);


    PRV_UTF_LOG1_MAC("prvTgfIpv6UcFdbEntryAddressIndex [%d]\n", prvTgfIpv6UcFdbEntryAddressIndex);
    UTF_VERIFY_EQUAL0_PARAM_MAC(expectedValidAddress,tmp_valid_address);
    PRV_UTF_LOG2_MAC("expectedValidAddress[%d], receivedValidAddress[%d]\n", expectedValidAddress,tmp_valid_address);
    UTF_VERIFY_EQUAL0_PARAM_MAC(expectedSkipAddress, tmp_skip_address);
    PRV_UTF_LOG2_MAC("expectedSkipAddress[%d], receivedSkipAddress[%d]\n", expectedSkipAddress,tmp_skip_address);
    UTF_VERIFY_EQUAL0_PARAM_MAC(expectedAgeAddress,  tmp_aged_address);
    PRV_UTF_LOG2_MAC("expectedAgeAddress[%d], receivedAgeAddress[%d]\n", expectedAgeAddress,tmp_aged_address);
    PRV_UTF_LOG1_MAC("prvTgfIpv6UcFdbEntryDataIndex [%d]\n", prvTgfIpv6UcFdbEntryDataIndex);
    UTF_VERIFY_EQUAL0_PARAM_MAC(expectedValidData,tmp_valid_data);
    PRV_UTF_LOG2_MAC("expectedValidData[%d], receivedValidData[%d]\n", expectedValidData,tmp_valid_data);
    UTF_VERIFY_EQUAL0_PARAM_MAC(expectedSkipData, tmp_skip_data);
    PRV_UTF_LOG2_MAC("expectedSkipData[%d], receivedSkipData[%d]\n", expectedSkipData,tmp_skip_data);
    UTF_VERIFY_EQUAL0_PARAM_MAC(expectedAgeData,  tmp_aged_data);
    PRV_UTF_LOG2_MAC("expectedAgeData[%d], receivedAgeData[%d]\n", expectedAgeData,tmp_aged_data);
}


/**
* @internal prvTgfFdbIpv6UcRoutingAgingEnableConfigurationRestore function
* @endinternal
*
* @brief   Restore test configuration
*/
GT_VOID prvTgfFdbIpv6UcRoutingAgingEnableConfigurationRestore
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

    /* AUTODOC: restore FDB trigger mode */
    rc = prvTgfBrgFdbMacTriggerModeSet(prvTgfUcIpv6RouteAgingModeGet);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc , "Error of prvTgfBrgFdbMacTriggerModeSet");

    /* AUTODOC: restore Aging status */
    rc =  prvTgfBrgFdbRoutingUcAgingEnableSet(prvTgfDevNum,prvTgfUcIpv6RouteAgingStatusGet);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbRoutingUcAgingEnableSet: %d", prvTgfDevNum);

    /* AUTODOC: restore fdb routing uc AAandTAToCpu status*/
    rc =  prvTgfBrgFdbRoutingUcAAandTAToCpuSet(prvTgfDevNum,prvTgfUcIpv6RouteAAandTAToCpuGet);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbRoutingUcAAandTAToCpuSet: %d", prvTgfDevNum);

    if(PRV_CPSS_SIP_5_10_CHECK_MAC(prvTgfDevNum))
    {
        /* restore original act device info */
        rc = prvTgfBrgFdbActionActiveDevSet(prvTgfDevNum,origActDev,origActDevMask);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbActionActiveDevGet");

        /* restore original act interface info */
        rc = prvTgfBrgFdbActionActiveInterfaceSet(prvTgfDevNum,origActIsTrunk,origActIsTrunkMask,origActTrunkPort,origActTrunkPortMask);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbActionActiveInterfaceSet");
    }

    prvTgfUcIpv6RouteAgingStatusFirstCall=GT_TRUE;

    defaultValuesFirstGet=GT_TRUE;

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




