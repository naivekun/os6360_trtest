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
* @file prvTgfBrgGenIcmpV6.c
*
* @brief IPv6 ICMP command test
*
* @version   1
********************************************************************************
*/

#include <cpssCommon/cpssPresteraDefs.h>
#include <cpss/extServices/private/prvCpssBindFunc.h>
#include <cpss/generic/config/private/prvCpssConfigTypes.h>

#include <utf/utfMain.h>
#include <utf/private/prvUtfHelpers.h>
#include <utf/private/prvUtfExtras.h>

#include <trafficEngine/tgfTrafficGenerator.h>
#include <trafficEngine/tgfTrafficTable.h>

#include <common/tgfCommon.h>
#include <common/tgfBridgeGen.h>
#include <common/tgfIpGen.h>

#include <bridge/prvTgfBrgGenIcmpV6.h>

/******************************************************************************\
 *                       Test configuration section                           *
\******************************************************************************/
/* VLAN Id */
#define PRV_TGF_VLANID_CNS               100

/* port to send traffic to */
#define PRV_TGF_SEND_PORT_IDX_CNS         0

/* number of entries in ICMP msgType table*/
#define PRV_TGF_MAX_MSG_TYPE              8


/* default number of packets to send */
static GT_U32        prvTgfBurstCount =   1;

                                      
/******************************* Test packet **********************************/

/* L2 part of packet */
static TGF_PACKET_L2_STC prvTgfPacketL2Part = {
    {0x01, 0x00, 0x00, 0x00, 0x22, 0x22},               /* daMac = MC */
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x01}                /* saMac */
};
/* VLAN_TAG part */
static TGF_PACKET_VLAN_TAG_STC prvTgfPacketVlanTagPart = {
    TGF_ETHERTYPE_8100_VLAN_TAG_CNS,                    /* etherType */
    0, 0, PRV_TGF_VLANID_CNS                           /* pri, cfi, VlanId */
};
/* packet's ethertype */
static TGF_PACKET_ETHERTYPE_STC prvTgfPacketEtherTypePart = {
    TGF_ETHERTYPE_86DD_IPV6_TAG_CNS
};
/* packet's IPv6 header - IcmpV6*/
static TGF_PACKET_IPV6_STC prvTgfPacketIpv6Part = {
    6,                  /* version */
    0,                  /* trafficClass */
    0,                  /* flowLabel */
    0x00,               /* payloadLen */
    0x3a,               /* nextHeader - IcmpV6  58=0x3a */
    0x40,               /* hopLimit */
    {0x2222, 0, 0, 0, 0, 0, 0x0000, 0x2222}, /* TGF_IPV6_ADDR srcAddr */
    {0xff00, 0, 0, 0, 0, 0, 0x0000, 0x2222}  /* TGF_IPV6_ADDR dstAddr */
};

/* DATA of packet */
static GT_U8 prvTgfPayloadDataArr[] =
{
    0x81, 0x00, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
    0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x10,
    0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19,
    0x1A, 0x1B
};

/* PAYLOAD part */
static TGF_PACKET_PAYLOAD_STC prvTgfPacketPayloadPart =
{
    sizeof(prvTgfPayloadDataArr),                       /* dataLength */
    prvTgfPayloadDataArr                                /* dataPtr */
};

/* PARTS of MC IPv6 packet */
static TGF_PACKET_PART_STC prvTgfPacketPartArray[] =
{
    {TGF_PACKET_PART_L2_E,        &prvTgfPacketL2Part},
    {TGF_PACKET_PART_VLAN_TAG_E,  &prvTgfPacketVlanTagPart},
    {TGF_PACKET_PART_ETHERTYPE_E, &prvTgfPacketEtherTypePart},
    {TGF_PACKET_PART_IPV6_E,      &prvTgfPacketIpv6Part},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacketPayloadPart}
};

/* LENGTH of IPv6 packet */
#define PRV_TGF_IPV6_PACKET_LEN_CNS \
    TGF_L2_HEADER_SIZE_CNS + TGF_VLAN_TAG_SIZE_CNS + TGF_ETHERTYPE_SIZE_CNS + \
    TGF_IPV6_HEADER_SIZE_CNS + sizeof(prvTgfPayloadDataArr)

/* IPv6 PACKET to send */
static TGF_PACKET_STC prvTgfPacketInfo =
{
    PRV_TGF_IPV6_PACKET_LEN_CNS,                                       /* totalLen */
    sizeof(prvTgfPacketPartArray) / sizeof(TGF_PACKET_PART_STC), /* numOfParts */
    prvTgfPacketPartArray                                        /* partsArray */
};


static GT_U8    msgTypeToSendArr[3]={0x82,0x83,0x84}; /*ICMPv6 MLDv1 message types*/

/* LENGTH of buffer*/
#define TGF_RX_BUFFER_SIZE_CNS   0x100


/******************************************************************************/

/* stored default VID */
static GT_U16   prvTgfDefVlanId = 0;
/*stored* default msgTypeCmd and msgType table*/
static CPSS_PACKET_CMD_ENT saveMsgTypeCmd[PRV_TGF_MAX_MSG_TYPE]={
                         CPSS_PACKET_CMD_FORWARD_E,CPSS_PACKET_CMD_FORWARD_E,
                         CPSS_PACKET_CMD_FORWARD_E,CPSS_PACKET_CMD_FORWARD_E,
                         CPSS_PACKET_CMD_FORWARD_E,CPSS_PACKET_CMD_FORWARD_E,
                         CPSS_PACKET_CMD_FORWARD_E,CPSS_PACKET_CMD_FORWARD_E
};
static GT_U8    saveMsgType[PRV_TGF_MAX_MSG_TYPE]={0,0,0,0,0,0,0,0};


/******************************************************************************\
 *                            Test implementation                             *
\******************************************************************************/


/**
* @internal prvTgfBrgGenIcmpV6TrafficGenerator function
* @endinternal
*
* @brief   Generate traffic and compare counters
*
* @param[in] packetToCpu              - send packet to cpu?
* @param[in] floodOrDrop              - send packet to ports?
* @param[in] msgTypeIndex             - index of msgType to sent (0 to 2)
*                                       None
*/
GT_VOID prvTgfBrgGenIcmpV6TrafficGenerator
(
    IN GT_BOOL  packetToCpu,
    IN GT_BOOL  floodOrDrop,
    IN GT_U32   msgTypeIndex
    
)
{
    GT_STATUS                       rc;
    GT_U32                          portIter = 0;
   
    
   
    GT_BOOL     getFirst = GT_TRUE;
    GT_U8       packetBuff[TGF_RX_BUFFER_SIZE_CNS] = {0};
    GT_U32      buffLen  = TGF_RX_BUFFER_SIZE_CNS;
    GT_U32      packetActualLength = 0;   
    GT_U8       devNum;
    GT_U8       queue;
    TGF_NET_DSA_STC rxParam;
    
    GT_STATUS   expectedRcRxToCpu = GT_OK;



    /* AUTODOC: GENERATE TRAFFIC: */
    
    /* AUTODOC:   send IPv6 ICMP packet with: */
    /* AUTODOC:   DA=01:00:00:00:22:22, SA=00:00:00:00:00:01 */
 
    prvTgfPayloadDataArr[0]=msgTypeToSendArr[msgTypeIndex];

    /* reset ETH counters */
    rc = prvTgfEthCountersReset(prvTgfDevNum);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfEthCountersReset");

    /* setup packet */
    rc = prvTgfSetTxSetupEth(prvTgfDevNum, &prvTgfPacketInfo, 
                             prvTgfBurstCount, 0, NULL);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "ERROR of SetTxSetup: %d", 
                                 prvTgfDevNum);

    /* clear table */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");

   /* start Rx capture */
    rc = tgfTrafficTableRxStartCapture(GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(
        GT_OK, rc, "tgfTrafficTableRxStartCapture");

    rc = prvTgfStartTransmitingEth(prvTgfDevNum, 
                                   prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);

    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, 
                                 "ERROR of StartTransmitting: %d, %d\n",
                                 prvTgfDevNum, 
                                 prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);                                 
    /* read and check ethernet counters */
    /* AUTODOC: verify traffic is dropped */

    rc = prvTgfEthCountersCheck(
        prvTgfDevNum,
        prvTgfPortsArray[0],
        /*expected Rx*/1,
        /*expected Tx*/1,
        prvTgfPacketInfo.totalLen,
        prvTgfBurstCount);
    UTF_VERIFY_EQUAL2_STRING_MAC(
        GT_OK, rc, "prvTgfEthCountersCheck: %d, %d\n",
        prvTgfDevNum, prvTgfPortsArray[0]);

    for (portIter = 1; portIter < prvTgfPortsNum; portIter++)
    {
        
        rc = prvTgfEthCountersCheck(
            prvTgfDevNum,
            prvTgfPortsArray[portIter],
            /*expected Rx*/ 0,
            /*expected Tx*/ ((floodOrDrop == GT_TRUE) ? 1 : 0),
            prvTgfPacketInfo.totalLen-4, /* packet without VLAN tag */
            prvTgfBurstCount);
        UTF_VERIFY_EQUAL2_STRING_MAC(
            GT_OK, rc, "prvTgfEthCountersCheck: %d, %d\n",
            prvTgfDevNum, prvTgfPortsArray[portIter]);

    }
    
    if ( GT_FALSE == packetToCpu )
    {
        expectedRcRxToCpu = GT_NO_MORE;
    }

    /* stop Rx capture */
    rc = tgfTrafficTableRxStartCapture(GT_FALSE);
    UTF_VERIFY_EQUAL0_STRING_MAC(
        GT_OK, rc, "tgfTrafficTableRxStartCapture");

    /* get first entry from rxNetworkIf table */
    rc = tgfTrafficGeneratorRxInCpuGet(TGF_PACKET_TYPE_REGULAR_E,
                                       getFirst, GT_TRUE, packetBuff,
                                       &buffLen, &packetActualLength,
                                       &devNum, &queue, &rxParam);
    UTF_VERIFY_EQUAL0_STRING_MAC(expectedRcRxToCpu, rc, 
                                 "tgfTrafficGeneratorRxInCpuGet %d");
    rc = tgfTrafficGeneratorRxInCpuGet(TGF_PACKET_TYPE_REGULAR_E,
                                       GT_FALSE, GT_TRUE, packetBuff,
                                       &buffLen, &packetActualLength,
                                       &devNum, &queue, &rxParam);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_NO_MORE, rc, 
                                            "tgfTrafficGeneratorRxInCpuGet %d");
}

/**
* @internal prvTgfBrgGenIcmpV6Set function
* @endinternal
*
* @brief   Set test bridge configuration
*/
GT_VOID prvTgfBrgGenIcmpV6Set
(
    IN GT_VOID
)
{
    GT_STATUS            rc;
    GT_U32               index;
    GT_U8                type;
    CPSS_PACKET_CMD_ENT  cmd;
    /* AUTODOC: SETUP CONFIGURATION: */

    /* get default vlanId */
    rc = prvTgfBrgVlanPortVidGet(prvTgfDevNum, 
                                 prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], 
                                 &prvTgfDefVlanId);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanPortVidGet: %d, %d",
                                 prvTgfDevNum, 
                                 prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);
    rc = prvTgfBrgVlanPortVidSet(prvTgfDevNum, 
                                 prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], 
                                 PRV_TGF_VLANID_CNS);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, 
                                 "ERROR of prvTgfBrgVlanPortVidSet: %d",
                                 prvTgfDevNum);
    /* get default msgType table */
    for(index=0;index<PRV_TGF_MAX_MSG_TYPE;index++)
    {
        rc=prvTgfBrgGenIcmpv6MsgTypeGet(index,&type,&cmd);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, 
                                     "ERROR of prvTgfBrgGenIcmpv6MsgTypeGet: %d",
                                     prvTgfDevNum);
        saveMsgTypeCmd[index]=cmd;
        saveMsgType[index]=type;
    }
    /* AUTODOC: create VLAN 100 with all untagged ports */
    rc = prvTgfBrgDefVlanEntryWithPortsSet(PRV_TGF_VLANID_CNS, prvTgfPortsArray,
                                           NULL, NULL, prvTgfPortsNum);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, 
                               "ERROR of prvTgfBrgDefVlanEntryWithPortsSet: %d",
                                 prvTgfDevNum);

    /* AUTODOC: fill the message type table*/  
    
    /* AUTODOC:forward command for IcmpV6 MLD v1 Query message packets */
    rc = prvTgfBrgGenIcmpv6MsgTypeSet(0,msgTypeToSendArr[0],CPSS_PACKET_CMD_FORWARD_E);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, 
                             "prvTgfBrgGenIcmpv6MsgTypeSet: %d",
                                 prvTgfDevNum);

    /* AUTODOC: mirroring command for IcmpV6 MLD v1 Report message packets */
    rc = prvTgfBrgGenIcmpv6MsgTypeSet(1,msgTypeToSendArr[1],CPSS_PACKET_CMD_MIRROR_TO_CPU_E);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, 
                             "prvTgfBrgGenIcmpv6MsgTypeSet: %d",
                                 prvTgfDevNum);

    /* AUTODOC: trapping command for IcmpV6 MLD v1 Done message packets */
    rc = prvTgfBrgGenIcmpv6MsgTypeSet(2,msgTypeToSendArr[2],CPSS_PACKET_CMD_TRAP_TO_CPU_E);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, 
                                 "prvTgfBrgGenIcmpv6MsgTypeSet: %d",
                                 prvTgfDevNum);
   
}

/**
* @internal prvTgfBrgGenIcmpV6Restore function
* @endinternal
*
* @brief   Restore configuration
*/
GT_VOID prvTgfBrgGenIcmpV6Restore
(
    GT_VOID
)
{
    GT_STATUS rc;
    GT_U32    index;
    /* AUTODOC: RESTORE CONFIGURATION: */


    /* flush FDB include static entries */
    rc = prvTgfBrgFdbFlush(GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbFlush: %d, %d", 
                                 prvTgfDevNum, GT_TRUE);
    /* reset ETH counters */
    rc = prvTgfEthCountersReset(prvTgfDevNum);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfEthCountersReset");

    /* AUTODOC: disable trapping command for IPv6 ICMP packets */
    rc = prvTgfBrgVlanIpV6IcmpToCpuEnable(PRV_TGF_VLANID_CNS,GT_FALSE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, 
                                 "prvTgfBrgVlanIpV6IcmpToCpuEnable: %d");
    /* AUTODOC: restore the ICMP msgType table */
    for(index=0;index<PRV_TGF_MAX_MSG_TYPE;index++)
    {
        rc=prvTgfBrgGenIcmpv6MsgTypeSet(index,saveMsgType[index],saveMsgTypeCmd[index]);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, 
                                     "ERROR of prvTgfBrgGenIcmpv6MsgTypeSet: %d",
                                     prvTgfDevNum);
    }
    /* AUTODOC: restore default vlanId to all ports */
    rc = prvTgfBrgVlanPvidSet(prvTgfDefVlanId);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanPvidSet: %d, %d",
                                 prvTgfDevNum, prvTgfDefVlanId);

    /* invalidate vlan entry (and reset vlan entry) */
    rc = prvTgfBrgDefVlanEntryInvalidate(PRV_TGF_VLANID_CNS);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, 
                                 "prvTgfBrgDefVlanEntryInvalidate: %d, %d",
                                 prvTgfDevNum, PRV_TGF_VLANID_CNS);
    
    /* clear table */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");
}

/**
* @internal prvTgfBrgGenIcmpV6Test function
* @endinternal
*
* @brief   IPv6 ICMP configuration test
*/
GT_VOID prvTgfBrgGenIcmpV6Test
(
    GT_VOID
)
{
    GT_STATUS   rc;

    /* AUTODOC: enable ICMP TO CPU for VLAN 100. 
    generate ICMPv6 packet of each msgType,
    and check the packet is handled correctly by the PP.
    */

    /* Enable ICMP to CPU*/
    rc = prvTgfBrgVlanIpV6IcmpToCpuEnable(PRV_TGF_VLANID_CNS,GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, 
                             "prvTgfBrgVlanIpV6IcmpToCpuEnable: %d");



    /* #1 msg type 130 - forwarding - Generate traffic */
    prvTgfBrgGenIcmpV6TrafficGenerator(GT_FALSE, GT_TRUE, 0);

    /* #2 msg type 131 - mirrorring - Generate traffic */
    prvTgfBrgGenIcmpV6TrafficGenerator(GT_TRUE, GT_TRUE, 1);

    /* #3 msg type 132 - trapping - Generate traffic */
    prvTgfBrgGenIcmpV6TrafficGenerator(GT_TRUE, GT_FALSE, 2);


    /* AUTODOC: Disable ICMP to CPU - EXPECT "FORWARDING" FOR ALL ICMP PACKETS.
    generate ICMPv6 packets of msgTypes associated with MIRROR and TRAP cmds,
    and check the packets are FORWARDED by the PP. */
    rc = prvTgfBrgVlanIpV6IcmpToCpuEnable(PRV_TGF_VLANID_CNS,GT_FALSE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, 
                             "prvTgfBrgVlanIpV6IcmpToCpuEnable: %d");

    /* #4 msg type 131 - mirroring - Generate traffic */
    prvTgfBrgGenIcmpV6TrafficGenerator(GT_FALSE, GT_TRUE, 1);


    /* #5 msg type 132 - trapping - Generate traffic */
    prvTgfBrgGenIcmpV6TrafficGenerator(GT_FALSE, GT_TRUE, 2);
   
}


