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
* @file prvTgfBasicIpv4UcRoutingTrapToCpu.c
*
* @brief Basic IPV4 UC Routing Trap To CPU
*
* @version   1
********************************************************************************
*/

#include <cpssCommon/cpssPresteraDefs.h>
#include <cpss/extServices/private/prvCpssBindFunc.h>

#include <utf/utfMain.h>
#include <utf/private/prvUtfHelpers.h>
#include <utf/private/prvUtfExtras.h>

#include <trafficEngine/tgfTrafficGenerator.h>
#include <trafficEngine/tgfTrafficTable.h>

#include <common/tgfCommon.h>
#include <common/tgfBridgeGen.h>
#include <common/tgfIpGen.h>
#include <common/tgfTrunkGen.h>
#include <ip/prvTgfBasicIpv4UcRouting.h>
#include <trunk/prvTgfTrunk.h>
#include <common/tgfPacketGen.h>
#include <common/tgfCscdGen.h>
#include <common/tgfConfigGen.h>
#include <gtOs/gtOsMem.h>
#include <pcl/prvTgfPclUdbOnlyKeys.h>

/******************************************************************************\
 *                       Test configuration section                           *
\******************************************************************************/
/* PCL rule & action index */
#define PCL_MATCH_INDEX_CNS      108

/* default VLAN Id */
#define PRV_TGF_SEND_VLANID_CNS           5

/* nextHop VLAN Id */
#define PRV_TGF_NEXTHOPE_VLANID_CNS       6

/* number of ports */
#define PRV_TGF_PORT_COUNT_CNS            4

/* the counter set for a route entry is linked to */
#define PRV_TGF_COUNT_SET_CNS     CPSS_IP_CNT_SET0_E

/* default number of packets to send */
#define PRV_TGF_BURST_COUNT_DEFAULT_CNS     1

/* default number of packets to send */
static GT_U32        prvTgfBurstCount   = PRV_TGF_BURST_COUNT_DEFAULT_CNS;

/* VLANs array */
static GT_U8         prvTgfVlanArray[] = {PRV_TGF_SEND_VLANID_CNS, PRV_TGF_NEXTHOPE_VLANID_CNS};

/* the Route entry index for UC Route entry Table */
static GT_U32        prvTgfRouteEntryBaseIndex = 3;

/* the ARP MAC address to write to the Router ARP Table */
static TGF_MAC_ADDR  prvTgfArpMac = {0x00, 0x00, 0x00, 0x00, 0x00, 0x22};

/******************************* Test packet **********************************/

/* L2 part of packet */
static TGF_PACKET_L2_STC prvTgfPacketL2Part = {
    {0x00, 0x00, 0x00, 0x00, 0x34, 0x02},               /* daMac */
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x01}                /* saMac */
};
/* packet's ethertype */
static TGF_PACKET_ETHERTYPE_STC prvTgfPacketEtherTypePart = {TGF_ETHERTYPE_0800_IPV4_TAG_CNS};
/* packet's IPv4 */
static TGF_PACKET_IPV4_STC prvTgfPacketIpv4Part = {
    4,                  /* version */
    5,                  /* headerLen */
    0,                  /* typeOfService */
    0x2A,               /* totalLen */
    0,                  /* id */
    0,                  /* flags */
    0,                  /* offset */
    0x40,               /* timeToLive */
    0x04,               /* protocol */
    TGF_PACKET_AUTO_CALC_CHECKSUM_CNS,             /* csum */
    { 1,  1,  1,  1},   /* srcAddr */
    { 1,  1,  1,  3}    /* dstAddr */
};
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

/* PARTS of untagged packet */
static TGF_PACKET_PART_STC prvTgfPacketPartArray[] = {
    {TGF_PACKET_PART_L2_E,        &prvTgfPacketL2Part},  /* type, partPtr */
    {TGF_PACKET_PART_SKIP_E ,      NULL},/*reserved for DSA tag (instead of the vlan tag)*/
    {TGF_PACKET_PART_ETHERTYPE_E, &prvTgfPacketEtherTypePart},
    {TGF_PACKET_PART_IPV4_E,      &prvTgfPacketIpv4Part},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacketPayloadPart}
};

/*capture type*/
static TGF_CAPTURE_MODE_ENT captureType = TGF_CAPTURE_MODE_MIRRORING_E;

/* parameters that is needed to be restored */
static struct
{
    GT_U16                                          vid;
    GT_BOOL                                         prvTgfSavedCpuEpclEnable;
    PRV_TGF_PCL_PORT_LOOKUP_CFG_TAB_ACC_MODE_ENT    prvTgfSavedCpuEpclAccessMode;
} prvTgfRestoreCfg;
/******************************************************************************\
 *                            Test implementation                             *
\******************************************************************************/

/**
* @internal prvTgfEPclTrapToCpuAndNextHopVidIsDrop function
* @endinternal
*
* @brief   build Pcl rule
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on HW error
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_BAD_PTR               - on NULL pointer
*/
static GT_VOID prvTgfEPclTrapToCpuAndNextHopVidIsDrop
(
    GT_VOID
)
{
    GT_STATUS rc;

    PRV_TGF_PCL_RULE_FORMAT_TYPE_ENT   ruleFormat     = PRV_TGF_PCL_RULE_FORMAT_EGRESS_UDB_10_E;
    CPSS_INTERFACE_INFO_STC            interfaceInfo;
    PRV_TGF_PCL_LOOKUP_CFG_STC         lookupCfg; /* PCL Configuration Table Entry */

    PRV_TGF_PCL_RULE_FORMAT_UNT mask;
    PRV_TGF_PCL_RULE_FORMAT_UNT pattern;
    PRV_TGF_PCL_ACTION_STC      action;

    cpssOsMemSet(&action, 0, sizeof(action));
    cpssOsMemSet(&mask, 0, sizeof(mask));
    cpssOsMemSet(&pattern, 0, sizeof(pattern));

    TGF_RESET_PARAM_MAC(interfaceInfo);
    TGF_RESET_PARAM_MAC(lookupCfg);

    PRV_UTF_LOG0_MAC("======= Setting Pcl Configuration =======\n");

     /* AUTODOC: Enables egress policy per device */
    rc = prvTgfPclEgressPolicyEnable(GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPclEgressPolicyEnable");

    /* AUTODOC: save CPU's EPCL settings */
    rc = prvTgfPclEgressPclPacketTypesGet(prvTgfDevNum, CPSS_CPU_PORT_NUM_CNS,
                    PRV_TGF_PCL_EGRESS_PKT_TO_CPU_E, &prvTgfRestoreCfg.prvTgfSavedCpuEpclEnable);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPclEgressPclPacketTypesGet");

    /* AUTODOC: Enables egress policy on port per packet type*/
    rc = prvTgfPclEgressPclPacketTypesSet(prvTgfDevNum, CPSS_CPU_PORT_NUM_CNS,
                                          CPSS_DXCH_PCL_EGRESS_PKT_TO_CPU_E, GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPclEgressPclPacketTypesSet");

    /* AUTODOC: Save access mode for EPCL lookup for restore */
    rc = prvTgfPclPortLookupCfgTabAccessModeGet(CPSS_CPU_PORT_NUM_CNS,
                           CPSS_PCL_DIRECTION_EGRESS_E, CPSS_PCL_LOOKUP_0_E, 0,
                           &prvTgfRestoreCfg.prvTgfSavedCpuEpclAccessMode);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,
                                 "prvTgfPclPortLookupCfgTabAccessModeGet");

    /* AUTODOC: Configure ePort access mode for EPCL lookup */
    rc = prvTgfPclPortLookupCfgTabAccessModeSet(CPSS_CPU_PORT_NUM_CNS,
                           CPSS_PCL_DIRECTION_EGRESS_E, CPSS_PCL_LOOKUP_0_E, 0,
                           PRV_TGF_PCL_PORT_LOOKUP_CFG_TAB_ACC_MODE_BY_PORT_E);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPclPortLookupCfgTabAccessModeSet");

    /* according to cider: UDBs Metadata */
    rc = prvTgfPclUdbOnlyKeysPclUdbRangeCfgSet(
        CPSS_PCL_DIRECTION_EGRESS_E,
        PRV_TGF_PCL_PACKET_TYPE_IPV4_OTHER_E,
        PRV_TGF_PCL_OFFSET_METADATA_E,
        30,
        1,
        26,/* bit_208   The eVLAN assigned to the packet.
                        Anchor offset: 26
                        Bit offset: 7:0

                        Anchor offset: 27
                        Bit offset: 4:0*/
        1);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPclUdbOnlyKeysPclUdbRangeCfgSet");

    rc = prvTgfPclUdbOnlyKeysPclUdbRangeCfgSet(
        CPSS_PCL_DIRECTION_EGRESS_E,
        PRV_TGF_PCL_PACKET_TYPE_IPV4_OTHER_E,
        PRV_TGF_PCL_OFFSET_METADATA_E,
        31,
        1,
        64,/* Egress Mtag Cmd bit_512   Contains the outgoing packet's command:
                                        0x0 = TO_CPU
                                        0x1 = FROM_CPU
                                        0x2 = TO_ANALYZER
                                        0x3 = FORWARD

                                        Anchor offset: 64
                                        Bit offset:1:0*/
        1);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPclUdbOnlyKeysPclUdbRangeCfgSet");

    rc = prvTgfPclUdbOnlyKeysPclUdbRangeCfgSet(
        CPSS_PCL_DIRECTION_EGRESS_E,
        PRV_TGF_PCL_PACKET_TYPE_IPV4_OTHER_E,
        PRV_TGF_PCL_OFFSET_METADATA_E,
        32,
        1,
        13,/*Trg Dev bit_104
            This field contains the target device assigned to the packet.

            Anchor offset: 13
            Bit offset: 7:0

            Anchor offset: 14
            Bit offset: 1:0
            */
        1);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPclUdbOnlyKeysPclUdbRangeCfgSet");

    rc = prvTgfPclUdbOnlyKeysUdbOnlyRangeSelectSet(
        0, PRV_TGF_PCL_PACKET_TYPE_IPV4_OTHER_E, PRV_TGF_PCL_RULE_FORMAT_EGRESS_UDB_10_E,
        0 /*udbReplaceBitmap*/,
        3,
        30,
        1);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPclUdbOnlyKeysUdbOnlyRangeSelectSet");

    /* write an entry into the EPCL configuration table */
    interfaceInfo.type = CPSS_INTERFACE_PORT_E;
    interfaceInfo.devPort.hwDevNum = prvTgfDevNum;
    interfaceInfo.devPort.portNum = CPSS_CPU_PORT_NUM_CNS;

    /* AUTODOC: setup EPCL configuration table entries. */
    lookupCfg.enableLookup           = GT_TRUE;
    lookupCfg.groupKeyTypes.nonIpKey = PRV_TGF_PCL_RULE_FORMAT_EGRESS_UDB_10_E;
    lookupCfg.groupKeyTypes.ipv4Key  = PRV_TGF_PCL_RULE_FORMAT_EGRESS_UDB_10_E;
    lookupCfg.groupKeyTypes.ipv6Key  = PRV_TGF_PCL_RULE_FORMAT_EGRESS_UDB_10_E;

    rc = prvTgfPclCfgTblSet(&interfaceInfo, CPSS_PCL_DIRECTION_EGRESS_E,
                            CPSS_PCL_LOOKUP_0_E, &lookupCfg);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPclCfgTblSet");


    /* AUTODOC: EPCL Pattern: if the packet is send to CPU and the vlan assigned
       to the packet is the NextHop_vlan and the target device is 0 the packets
       should be dropped --> this is an illegal situation */
    pattern.ruleEgrUdbOnly.udb[0] = PRV_TGF_NEXTHOPE_VLANID_CNS;/* eVlan
                                          when NextHop_cmd = CPSS_PACKET_CMD_TRAP_TO_CPU_E
                                          the evlan value assigned in the NextHop should not be use
                                          instead the packet should stay with the  original value
                                          arrived in the descriptor */
    pattern.ruleEgrUdbOnly.udb[1] = 0; /* TO_CPU value*/
    pattern.ruleEgrUdbOnly.udb[2] = 0; /* Trg_Dev
                                          Trg_Dev is assigned to be 0 by the CPSS API when
                                          NextHop_cmd = CPSS_PACKET_CMD_TRAP_TO_CPU_E,
                                          in this case all interface fields are not set and all the
                                          fields are set with default values=0
                                          in case of trap to CPU the packet should stay with the
                                          original values arrived in the descriptor */


    /* AUTODOC: EPCL Mask for vid */
    mask.ruleEgrUdbOnly.udb[0] = 0xFF;
    mask.ruleEgrUdbOnly.udb[1] = 3;
    mask.ruleEgrUdbOnly.udb[2] = 0xFF;

    /* AUTODOC: EPCL action - drop the packet */
    action.pktCmd  = CPSS_PACKET_CMD_DROP_HARD_E;
    action.egressPolicy = GT_TRUE;

    rc = prvTgfPclRuleSet(ruleFormat, PCL_MATCH_INDEX_CNS, &mask, &pattern, &action);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfPclRuleSet: %d, %d",
                                            ruleFormat, PCL_MATCH_INDEX_CNS);
}


/**
* @internal prvTgfBasicIpv4UcRoutingTrapToCpuConfigurationSet function
* @endinternal
*
* @brief   Trap To CPU Configuration Set
*
* @param[in] sendPortNum              - port sending traffic
*                                       None
*/
GT_VOID prvTgfBasicIpv4UcRoutingTrapToCpuConfigurationSet
(
    GT_U32               sendPortNum
)
{
    GT_STATUS rc = GT_OK;

    /* save default vlanId for restore */
    rc = prvTgfBrgVlanPortVidGet(prvTgfDevNum,
                                 sendPortNum,
                                 &prvTgfRestoreCfg.vid);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanPortVidGet: %d, %d",
                                 prvTgfDevNum, &prvTgfRestoreCfg.vid);

    /* AUTODOC: set PVID 5 for port 0 */
    rc = prvTgfBrgVlanPortVidSet(prvTgfDevNum,
                                 sendPortNum,
                                 PRV_TGF_SEND_VLANID_CNS);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanPortVidSet: %d, %d",
                                 prvTgfDevNum,
                                 sendPortNum);

    /* AUTODOC: Define EPCL rule to match packets with
       DSA tag command TO_CPU and vlanId = PRV_TGF_NEXTHOPE_VLANID_CNS.
       EPCL Action will be DROP
       if the packet is dropped then we have a bug since this case
       should not happen when router is configured to TRAP_TO_CPU.
       We expect the vlanId in the DSA tag to be the original vlan of the packet */

    prvTgfEPclTrapToCpuAndNextHopVidIsDrop();

}
/**
* @internal prvTgfTrapToCpuTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic
*
* @param[in] sendPortNum              - port sending traffic
* @param[in] nextHopPortNum           - port receiving traffic
* @param[in] nextHopVlanId            - nextHop VLAN
* @param[in] expectNoTraffic          - whether to expect traffic or not
*                                       None
*/
static GT_VOID prvTgfTrapToCpuTrafficGenerate
(
    GT_U32               sendPortNum,
    GT_U32               nextHopPortNum,
    GT_U16              nextHopVlanId,
    GT_BOOL             expectNoTraffic
)
{
    GT_STATUS                       rc          = GT_OK;
    GT_U32                          partsCount  = 0;
    GT_U32                          packetSize  = 0;
    TGF_PACKET_STC                  packetInfo;
    GT_U32                          portIter    = 0;
    GT_U32                          numTriggers = 0;
    TGF_VFD_INFO_STC                vfdArray[2];
    CPSS_INTERFACE_INFO_STC         portInterface;
    CPSS_PORT_MAC_COUNTER_SET_STC   portCntrs;
    CPSS_PORT_MAC_COUNTER_SET_STC   expectedCntrs;
    PRV_TGF_IP_COUNTER_SET_STC      ipCounters;
    PRV_TGF_IP_ROUTING_MODE_ENT     routingMode;
    GT_TRUNK_ID currTrunkId;/* trunk Id for the current port */
    GT_TRUNK_ID senderTrunkId;/* trunk Id for the sender port that is member of */
    GT_TRUNK_ID nextHopTrunkId;/* trunk Id for the next hop port that is member of */
    CPSS_TRUNK_MEMBER_STC   trunkMember;/* temporary trunk member */
    GT_BOOL     stormingDetected;/*indicates that the traffic to check
                   LBH that egress the trunk was originally INGRESSED from the trunk.
                   but since in the 'enhanced UT' the CPU send traffic to a port
                   due to loopback it returns to it, we need to ensure that the
                   member mentioned here should get the traffic since it is the
                   'original sender'*/
    GT_U32      vlanIter;/*vlan iterator*/
    GT_U32      vlanCount = sizeof(prvTgfVlanArray)/sizeof(prvTgfVlanArray[0]);/*number of vlans*/
    GT_U32      numVfd = 0;/* number of VFDs in vfdArray */
    GT_U32      egressPortIndex = 0xFF;

    GT_BOOL     getFirst = GT_TRUE;
    GT_U8       packetBuff[TGF_RX_BUFFER_MAX_SIZE_CNS] = {0};
    GT_U32      buffLen  = TGF_RX_BUFFER_MAX_SIZE_CNS;
    GT_U32      packetActualLength = 0;
    GT_U8       devNum;
    GT_U8       queue;
    TGF_NET_DSA_STC rxParam;

    /* AUTODOC: GENERATE TRAFFIC: */
    PRV_UTF_LOG0_MAC("======= Generating Traffic =======\n");

    /* clear VFD array */
    cpssOsMemSet((GT_VOID*) vfdArray, 0, sizeof(vfdArray));

    for (portIter = 0; portIter < PRV_TGF_PORT_COUNT_CNS; portIter++)
    {
        if(prvTgfPortsArray[portIter] == nextHopPortNum)
        {
            egressPortIndex = portIter;
            break;
        }
    }

    /* -------------------------------------------------------------------------
     * 1. Setup counters and enable capturing
     *
     */

    /* get routing mode */
    rc = prvTgfIpRoutingModeGet(&routingMode);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpRoutingModeGet: %d", prvTgfDevNum);

    for (portIter = 0; portIter < PRV_TGF_PORT_COUNT_CNS; portIter++)
    {
        /* reset ethernet counters */
        rc = prvTgfResetCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfResetCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);

        if (routingMode == PRV_TGF_IP_ROUTING_MODE_IP_LTT_ENTRY_E)
        {
            /* reset IP couters and set ROUTE_ENTRY mode */
            rc = prvTgfCountersIpSet(prvTgfPortsArray[portIter], portIter);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfCountersIpSet: %d, %d",
                                         prvTgfDevNum, prvTgfPortsArray[portIter]);
        }
    }

    /* Routing from trunk support:
       check if the port is trunk member.
       when trunk member , set all the members with same configuration.
    */
    trunkMember.port = sendPortNum;
    trunkMember.hwDevice = prvTgfDevNum;

    rc = prvTgfTrunkDbIsMemberOfTrunk(prvTgfDevNum,&trunkMember,&senderTrunkId);
    if(rc == GT_OK)
    {
    }
    else
    {
        senderTrunkId = 0;
    }

    /* Routing to trunk support:
       check if the NH port is trunk member.
       when trunk member --> use the trunk ID.
    */
    trunkMember.port = nextHopPortNum;
    trunkMember.hwDevice = prvTgfDevNum;

    rc = prvTgfTrunkDbIsMemberOfTrunk(prvTgfDevNum,&trunkMember,&nextHopTrunkId);
    if(rc == GT_OK)
    {
        /* the port is member of the trunk */

        /* setup nexthope portInterface for capturing */
        portInterface.type            = CPSS_INTERFACE_TRUNK_E;
        portInterface.trunkId         = nextHopTrunkId;

        /*since we have 2 members in the NH trunk send 8 packets*/
        prvTgfBurstCount = 8;

        vfdArray[0].modeExtraInfo = 0;
        vfdArray[0].offset = 6 + 5;/* last byte of the mac SA */
        cpssOsMemSet(vfdArray[0].patternPtr,0,sizeof(TGF_MAC_ADDR));
        vfdArray[0].patternPtr[0] = 0;
        vfdArray[0].cycleCount = 1;/*single byte*/
        vfdArray[0].mode = TGF_VFD_MODE_INCREMENT_E;

        numVfd = 1;
    }
    else
    {
        nextHopTrunkId = 0;

        /* setup nexthope portInterface for capturing */
        portInterface.type            = CPSS_INTERFACE_PORT_E;
        portInterface.devPort.hwDevNum  = prvTgfDevNum;
        portInterface.devPort.portNum = nextHopPortNum;
    }

    if((senderTrunkId != 0) && (nextHopTrunkId == senderTrunkId))
    {
        /*since we have 4 members in the NH trunk send 16 packets*/
        prvTgfBurstCount = 16;

        /* make all trunk ports members of the vlans */
        for (vlanIter = 0; vlanIter < vlanCount; vlanIter++)
        {
            for (portIter = 0; portIter < PRV_TGF_PORT_COUNT_CNS; portIter++)
            {
                rc = prvTgfBrgVlanMemberAdd(prvTgfDevNum, prvTgfVlanArray[vlanIter],
                                            prvTgfPortsArray[portIter], GT_FALSE);
                UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanMemberAdd: %d, %d, %d, %d",
                                             prvTgfDevNum, prvTgfVlanArray[vlanIter],
                                             prvTgfPortsArray[portIter], GT_FALSE);
            }
        }

        /* we route from a trunk to the same trunk */
        /* the 'Mirroring capture' is not good because it sets :
           force PVID = 0 for all traffic that ingress the 'Captured port' .
           but since the 'sender' is also member of the 'NH trunk' this port must
           also be 'captured' , but this will not allow the packets from the CPU
           to initiate routing at all !

           so we use 'Special PCL capture' that 'skip' the ingress vlan (before the routing)
           by this the traffic before route can ingress the device but the
           traffic after the rout will be captured to the CPU.
           */
        captureType = TGF_CAPTURE_MODE_PCL_E;

        /* state that no need to capture packets that are in the Vlan before the routing */
        rc = tgfTrafficGeneratorPortTxEthCapturePclExcludeVidSet(GT_TRUE,PRV_TGF_SEND_VLANID_CNS);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of tgfTrafficGeneratorPortTxEthCapturePclExcludeVidSet: %d, %d\n",
                                     prvTgfDevNum, nextHopPortNum);
    }
    else
    {
        captureType = TGF_CAPTURE_MODE_MIRRORING_E;
    }

     /* clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");

    if(expectNoTraffic==GT_FALSE)
    {
        /* enable capture on next hop port/trunk */
        rc = tgfTrafficGeneratorPortTxEthCaptureSet(&portInterface, captureType, GT_TRUE);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of tgfTrafficGeneratorPortTxEthCaptureSet: %d, %d\n",
                                     prvTgfDevNum, nextHopPortNum);
    }
    else
    {
        /* start Rx capture */
        rc = tgfTrafficTableRxStartCapture(GT_TRUE);
        UTF_VERIFY_EQUAL0_STRING_MAC(
            GT_OK, rc, "tgfTrafficTableRxStartCapture");
    }

    /* -------------------------------------------------------------------------
     * 2. Generating Traffic
     */

    /* number of parts in packet */
    partsCount = sizeof(prvTgfPacketPartArray) / sizeof(prvTgfPacketPartArray[0]);

    /* calculate packet size */
    rc = prvTgfPacketSizeGet(prvTgfPacketPartArray, partsCount, &packetSize);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPacketSizeGet:");

    /* build packet */
    packetInfo.totalLen   = packetSize;
    packetInfo.numOfParts = partsCount;
    packetInfo.partsArray = prvTgfPacketPartArray;

    /* setup packet */
    rc = prvTgfSetTxSetupEth(prvTgfDevNum, &packetInfo, prvTgfBurstCount, numVfd, &vfdArray[0]);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "ERROR of SetTxSetup: %d", prvTgfDevNum);

    /* set transmit timings */
    rc = prvTgfSetTxSetup2Eth(0, 50);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "ERROR of prvTgfSetTxSetup2Eth: %d", prvTgfDevNum);

    /* AUTODOC: send packet from port 0 with: */
    /* AUTODOC:   DA=00:00:00:00:34:02, SA=00:00:00:00:00:01 */
    /* AUTODOC:   VID=5, srcIP=1.1.1.1, dstIP=1.1.1.3 */

    /* send packet -- send from specific port -- even if member of a trunk */
    rc = prvTgfStartTransmitingEth(prvTgfDevNum, sendPortNum);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of StartTransmitting: %d, %d\n",
                                 prvTgfDevNum, sendPortNum);

    /* -------------------------------------------------------------------------
     * 3. Get Ethernet Counters
     */

    /* read and check ethernet counters */
    for (portIter = 0; portIter < PRV_TGF_PORT_COUNT_CNS; portIter++)
    {
        GT_BOOL isOk;

        /* Routing trunk support:
           check if the port is trunk member.
        */
        trunkMember.port = prvTgfPortsArray[portIter];
        trunkMember.hwDevice = prvTgfDevNum;

        rc = prvTgfTrunkDbIsMemberOfTrunk(prvTgfDevNum,&trunkMember,&currTrunkId);
        if(rc == GT_OK)
        {
            /* this port is member of trunk */

            if(currTrunkId == nextHopTrunkId)
            {
                /* this port is member of the next hop trunk */

                /* we need to SUM all those ports together before we check */
                /* we do the check in trunk dedicated function for 'target trunk'*/
                /* see after this loop */
                continue;
            }
        }

        /* read counters */
        rc = prvTgfReadPortCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter], GT_TRUE, &portCntrs);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfReadPortCountersEth: %d, %d\n",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);

        /* AUTODOC: verify routed packet on port 3 */
        if ((prvTgfPortsArray[portIter]==sendPortNum) ||
           ((expectNoTraffic==GT_FALSE)&&(portIter ==egressPortIndex)))
        {
                /* packetSize is not changed */
                if (prvTgfPortsArray[portIter]==sendPortNum)
                {
                    expectedCntrs.goodOctetsSent.l[0] = (packetSize + TGF_CRC_LEN_CNS) * prvTgfBurstCount;
                    expectedCntrs.goodOctetsRcv.l[0]  = (packetSize + TGF_CRC_LEN_CNS) * prvTgfBurstCount;
                }
                else /* packetSize is changed - vlan tag is added */
                {
                    expectedCntrs.goodOctetsSent.l[0] = (packetSize + TGF_CRC_LEN_CNS + TGF_VLAN_TAG_SIZE_CNS) * prvTgfBurstCount;
                    expectedCntrs.goodOctetsRcv.l[0]  = (packetSize + TGF_CRC_LEN_CNS + TGF_VLAN_TAG_SIZE_CNS) * prvTgfBurstCount;
                }

                expectedCntrs.goodPktsSent.l[0]   = prvTgfBurstCount;
                expectedCntrs.ucPktsSent.l[0]     = prvTgfBurstCount;
                expectedCntrs.brdcPktsSent.l[0]   = 0;
                expectedCntrs.mcPktsSent.l[0]     = 0;
                expectedCntrs.goodPktsRcv.l[0]    = prvTgfBurstCount;
                expectedCntrs.ucPktsRcv.l[0]      = prvTgfBurstCount;
                expectedCntrs.brdcPktsRcv.l[0]    = 0;
                expectedCntrs.mcPktsRcv.l[0]      = 0;
        }
        else
        {
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

        UTF_VERIFY_EQUAL0_STRING_MAC(isOk, GT_TRUE, "get another counters values.");

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

    if(nextHopTrunkId)
    {
        /* the traffic should egress the next hop trunk */
        /* we need to SUM all those ports together before we check */
        /* we do the check in trunk dedicated function for 'target trunk'*/
        if(nextHopTrunkId == senderTrunkId)
        {
            trunkMember.port = sendPortNum;
            trunkMember.hwDevice = prvTgfDevNum;

            prvTgfTrunkLoadBalanceCheck(nextHopTrunkId,
                PRV_TGF_TRUNK_EXPECTED_LOAD_BALANCE_MODE_EVEN_E,
                prvTgfBurstCount,
                0,/*tolerance*/
                &trunkMember,/* the port in the trunk that sent the traffic */
                &stormingDetected);/*was storming detected*/

            /* it will detect 'storming' but those are only the LBH of this port */
        }
        else
        {
            prvTgfTrunkLoadBalanceCheck(nextHopTrunkId,
                PRV_TGF_TRUNK_EXPECTED_LOAD_BALANCE_MODE_EVEN_E,
                prvTgfBurstCount,
                0,/*tolerance*/
                NULL,/*NA*/
                NULL);/*NA*/
        }
    }


    /* -------------------------------------------------------------------------
     * 4. Get Trigger Counters
     */

    if (expectNoTraffic==GT_FALSE)
    {
        PRV_UTF_LOG1_MAC("Port [%d] capturing:\n", nextHopPortNum);

        /* get trigger counters where packet has MAC DA as prvTgfArpMac */
        vfdArray[0].mode = TGF_VFD_MODE_STATIC_E;
        vfdArray[0].modeExtraInfo = 0;
        vfdArray[0].offset = 0;
        vfdArray[0].cycleCount = sizeof(TGF_MAC_ADDR);
        cpssOsMemCpy(vfdArray[0].patternPtr, prvTgfArpMac, sizeof(TGF_MAC_ADDR));

        /* get trigger counters where packet has VLAN as nextHopVlanId */
        vfdArray[1].mode = TGF_VFD_MODE_STATIC_E;
        vfdArray[1].modeExtraInfo = 0;
        vfdArray[1].offset = TGF_L2_HEADER_SIZE_CNS;
        vfdArray[1].cycleCount = 4;
        vfdArray[1].patternPtr[0] = 0x81;
        vfdArray[1].patternPtr[1] = 0x00;
        vfdArray[1].patternPtr[2] = 0x20;
        vfdArray[1].patternPtr[3] = (GT_U8)nextHopVlanId;

        /* disable capture on nexthope port , before check the packets to the CPU */
        rc = tgfTrafficGeneratorPortTxEthCaptureSet(&portInterface, captureType, GT_FALSE);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of tgfTrafficGeneratorPortTxEthCaptureSet: %d, %d\n",
                                     prvTgfDevNum, nextHopPortNum);

        /* check the packets to the CPU */
        rc = tgfTrafficGeneratorPortTxEthTriggerCountersGet(&portInterface, 2, vfdArray, &numTriggers);
        PRV_UTF_LOG2_MAC("    numTriggers = %d, rc = 0x%02X\n\n", numTriggers, rc);

        /* check TriggerCounters */
        rc = rc == GT_NO_MORE ? GT_OK : rc;
        if(portInterface.type  == CPSS_INTERFACE_PORT_E)
        {
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "tgfTrafficGeneratorPortTxEthTriggerCountersGet: %d, %d\n",
                                         portInterface.devPort.hwDevNum, portInterface.devPort.portNum);
        }
        else
        {
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "tgfTrafficGeneratorPortTxEthTriggerCountersGet: %d, trunkId [%d] \n",
                                         prvTgfDevNum, nextHopTrunkId);
        }

        /* AUTODOC: check if packet has the same MAC DA as ARP MAC */
        /* number of triggers should be according to number of transmit*/
        UTF_VERIFY_EQUAL6_STRING_MAC(((1<<prvTgfBurstCount)-1), numTriggers & BIT_0,
                 "\n   MAC DA of captured packet must be: %02X:%02X:%02X:%02X:%02X:%02X",
                prvTgfArpMac[0], prvTgfArpMac[1], prvTgfArpMac[2],
                prvTgfArpMac[3], prvTgfArpMac[4], prvTgfArpMac[5]);

        /* AUTODOC: check if captured packet has expected VLAN as nextHopVlanId */
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_TRUE, (numTriggers & BIT_1) >> 1,
                                     "\n   VLAN of captured packet must be: %02X",
                                     nextHopVlanId);
    }
    else
    {
        /* stop Rx capture */
        rc = tgfTrafficTableRxStartCapture(GT_FALSE);
        UTF_VERIFY_EQUAL0_STRING_MAC(
            GT_OK, rc, "tgfTrafficTableRxStartCapture");

        /* get first entry from rxNetworkIf table */
        rc = tgfTrafficGeneratorRxInCpuGet(TGF_PACKET_TYPE_REGULAR_E,
                                           getFirst, GT_TRUE, packetBuff,
                                           &buffLen, &packetActualLength,
                                           &devNum, &queue, &rxParam);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,
                                     "tgfTrafficGeneratorRxInCpuGet %d");

        rc = tgfTrafficGeneratorRxInCpuGet(TGF_PACKET_TYPE_REGULAR_E,
                                           GT_FALSE, GT_TRUE, packetBuff,
                                           &buffLen, &packetActualLength,
                                           &devNum, &queue, &rxParam);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_NO_MORE, rc,
                                                "tgfTrafficGeneratorRxInCpuGet %d");
    }
    /* -------------------------------------------------------------------------
     * 5. Get IP Counters
     */

    /* AUTODOC: get and print ip counters values */
    for (portIter = 0; portIter < PRV_TGF_PORT_COUNT_CNS; portIter++) {
        PRV_UTF_LOG1_MAC("IP counters for Port [%d]:\n", prvTgfPortsArray[portIter]);
        prvTgfCountersIpGet(prvTgfDevNum, portIter, GT_TRUE, &ipCounters);
    }
    PRV_UTF_LOG0_MAC("\n");

    /* restore transmit timings */
    rc = prvTgfSetTxSetup2Eth(0, 0);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "ERROR of prvTgfSetTxSetup2Eth: %d", prvTgfDevNum);

    /* disable the PCL exclude vid capturing */
    rc = tgfTrafficGeneratorPortTxEthCapturePclExcludeVidSet(GT_FALSE,0/*don't care*/);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of tgfTrafficGeneratorPortTxEthCapturePclExcludeVidSet: %d, %d\n",
                                 prvTgfDevNum, nextHopPortNum);

    /* just for 'cleanup' */
    captureType = TGF_CAPTURE_MODE_MIRRORING_E;
}

/**
* @internal prvTgfBasicIpv4UcRoutingTrapToCpuTrafficGenerate function
* @endinternal
*
* @brief   Add Prefixes Configuration and generate traffic
*
* @param[in] sendPortNum              - port sending traffic
* @param[in] nextHopPortNum           - port receiving traffic
*                                       None
*/
GT_VOID prvTgfBasicIpv4UcRoutingTrapToCpuTrafficGenerate
(
    GT_U32    sendPortNum,
    GT_U32    nextHopPortNum
)
{
    GT_STATUS rc = GT_OK;
    PRV_TGF_IP_UC_ROUTE_ENTRY_STC           routeEntriesArray[1];

    /* expect traffic to be routed, check nextHopVid is assigned */
    prvTgfTrapToCpuTrafficGenerate(sendPortNum,nextHopPortNum,PRV_TGF_NEXTHOPE_VLANID_CNS,GT_FALSE);

    /* AUTODOC: read and check the UC Route entry from the Route Table */
    cpssOsMemSet(routeEntriesArray, 0, sizeof(routeEntriesArray));

    rc = prvTgfIpUcRouteEntriesRead(prvTgfDevNum, prvTgfRouteEntryBaseIndex, routeEntriesArray, 1);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpUcRouteEntriesRead: %d", prvTgfDevNum);

    routeEntriesArray[0].cmd = CPSS_PACKET_CMD_TRAP_TO_CPU_E;

    /* AUTODOC: add UC route entry with nexthop VLAN 6 and nexthop port 3 */
    rc = prvTgfIpUcRouteEntriesWrite(prvTgfRouteEntryBaseIndex, routeEntriesArray, 1);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpUcRouteEntriesWrite: %d", prvTgfDevNum);

    /* AUTODOC: read and check the UC Route entry from the Route Table */
    cpssOsMemSet(routeEntriesArray, 0, sizeof(routeEntriesArray));

    rc = prvTgfIpUcRouteEntriesRead(prvTgfDevNum, prvTgfRouteEntryBaseIndex, routeEntriesArray, 1);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpUcRouteEntriesRead: %d", prvTgfDevNum);

    UTF_VERIFY_EQUAL1_STRING_MAC(CPSS_PACKET_CMD_TRAP_TO_CPU_E, routeEntriesArray[0].cmd, "prvTgfIpUcRouteEntriesRead: %d", prvTgfDevNum);

    /* expect traffic to be trapped to CPU, check nextHopVid is not assigned to the trapped packet */
    prvTgfTrapToCpuTrafficGenerate(sendPortNum,nextHopPortNum,PRV_TGF_NEXTHOPE_VLANID_CNS,GT_TRUE);
}

/**
* @internal prvTgfBasicIpv4UcRoutingTrapToCpuConfigurationRestore function
* @endinternal
*
* @brief   Trap To CPU Configuration Restore
*
* @param[in] sendPortNum              - port sending traffic
*                                       None
*/
GT_VOID prvTgfBasicIpv4UcRoutingTrapToCpuConfigurationRestore
(
    GT_U32               sendPortNum
)
{
    GT_STATUS rc = GT_OK;

    /* AUTODOC: restore default vlanId */
    rc = prvTgfBrgVlanPortVidSet(prvTgfDevNum,
                                 sendPortNum,
                                 prvTgfRestoreCfg.vid);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanPortVidSet: %d, %d",
                                 prvTgfDevNum,
                                 sendPortNum);

    /* AUTODOC: Disables PCL egress Policy */
    rc = prvTgfPclEgressPolicyEnable(GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPclEgressPolicyEnable: %d", GT_FALSE);

    /* AUTODOC: restore CPU's EPCL settings */
    rc = prvTgfPclEgressPclPacketTypesSet(prvTgfDevNum, CPSS_CPU_PORT_NUM_CNS,
                                          CPSS_DXCH_PCL_EGRESS_PKT_TO_CPU_E, prvTgfRestoreCfg.prvTgfSavedCpuEpclEnable);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPclEgressPclPacketTypesSet");

    rc = prvTgfPclPortLookupCfgTabAccessModeSet(CPSS_CPU_PORT_NUM_CNS,
                           CPSS_PCL_DIRECTION_EGRESS_E, CPSS_PCL_LOOKUP_0_E, 0,
                           prvTgfRestoreCfg.prvTgfSavedCpuEpclAccessMode);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,
                                 "prvTgfPclPortLookupCfgTabAccessModeSet");

    /* AUTODOC: invalidate EPCL rule */
    rc = prvTgfPclRuleValidStatusSet(CPSS_PCL_RULE_SIZE_STD_E, PCL_MATCH_INDEX_CNS, GT_FALSE);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfPclRuleValidStatusSet: %d, %d, %d",
                                 CPSS_PCL_RULE_SIZE_STD_E, PCL_MATCH_INDEX_CNS, GT_FALSE);

}

